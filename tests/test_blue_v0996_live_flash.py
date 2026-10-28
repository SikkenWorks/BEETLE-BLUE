#!/usr/bin/env python3
"""V2 flash journal test: global Live Setlist saved atomically, separate from projects.
Mocks NOR only; no hardware is connected or flashed.
"""
import ctypes,subprocess,tempfile,sys,zlib
from pathlib import Path
r=Path(__file__).resolve().parents[1];src=r/'firmware/fm1-native'
sys.path.insert(0,str(r/'tools/storage_v2'))
from storage_v2_sim import Nor,Storage,ProjectCodec,PowerFailure
U8=ctypes.c_uint8
RD=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_uint32,ctypes.POINTER(U8),ctypes.c_uint32)
PG=RD
ER=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_uint32)
class Device(ctypes.Structure):
 _fields_=[('read',ctypes.c_void_p),('context',ctypes.c_void_p),('index_address',ctypes.c_uint32),('generation',ctypes.c_uint32)]
class Writer(ctypes.Structure):
 _fields_=[('device',Device),('program',ctypes.c_void_p),('erase',ctypes.c_void_p),('context',ctypes.c_void_p),('domain',U8*72),('occupied',ctypes.c_uint16*72),('active_meta',U8)]
with tempfile.TemporaryDirectory() as d:
 so=Path(d)/'v2.so'
 subprocess.run(['cc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror',str(src/'beetle_storage_v2_writer.c'),str(src/'beetle_storage_v2_reader.c'),str(src/'beetle_project_pack_v2.c'),'-o',str(so)],check=True)
 lib=ctypes.CDLL(str(so));codec=ProjectCodec(lib)
 lib.beetle_v2_writer_open.argtypes=[ctypes.POINTER(Writer),RD,PG,ER,ctypes.c_void_p,ctypes.POINTER(U8),ctypes.POINTER(U8)]
 lib.beetle_v2_write_blue_setlist.argtypes=[ctypes.POINTER(Writer),ctypes.POINTER(U8),ctypes.POINTER(U8),ctypes.POINTER(U8)]
 p1=(r/'tests/fixtures/NUGGIE-SLOT-07.trk').read_bytes()
 p2=(r/'tests/fixtures/MOUNTAINKING-SLOT-01.trk').read_bytes()
 nor=Nor(authorized=True);st=Storage(nor,codec);st.save_project(0,p1);st.save_project(4,p2)
 image=bytes(nor.bytes)
 def setup(image,fault=None):
  flash=Nor(image,authorized=True);flash.fault_after=fault
  idx=(U8*4096)();other=(U8*4096)();writer=Writer()
  def rd(ctx,pos,out,n):
   pos-=0x97000
   if pos<0 or pos+n>len(flash.bytes):return -1
   ctypes.memmove(out,bytes(flash.bytes[pos:pos+n]),n);return 0
  def pg(ctx,pos,buf,n):
   try:flash.program_page(pos-0x97000,bytes(buf[:n]));return 0
   except (PowerFailure,ValueError):return -1
  def er(ctx,pos):
   try:flash.erase_sector((pos-0x97000)//4096);return 0
   except (PowerFailure,ValueError):return -1
  cb=(RD(rd),PG(pg),ER(er))
  assert lib.beetle_v2_writer_open(ctypes.byref(writer),*cb,None,idx,other)==0
  return flash,writer,idx,other,cb
 def record(slots):
  b=bytearray(b'\xff'*64);b[0:4]=b'BLST';b[4]=1;b[5]=len(slots);b[6:8]=b'\0\0';b[8:8+len(slots)]=bytes(slots)
  b[32:36]=(zlib.crc32(b[:32])).to_bytes(4,'little')
  return (U8*64).from_buffer_copy(b)
 def write(flash,writer,idx,other,slots):
  wire=record(slots)
  return lib.beetle_v2_write_blue_setlist(ctypes.byref(writer),wire,idx,other)
 def stored(flash):
  index=Storage(Nor(bytes(flash.bytes),authorized=True),codec).current
  # current index address is 0x0000 or 0x1000 from the highest B2SB generation
  meta=bytes(flash.bytes[0:8192]);gen0=int.from_bytes(meta[8:12],'little');gen1=int.from_bytes(meta[4104:4108],'little');at=4096 if gen1>gen0 else 0
  b=meta[at+2610:at+2610+64]
  if b[:4]!=b'BLST':return None
  assert zlib.crc32(b[:32])==int.from_bytes(b[32:36],'little')
  assert index.projects[0] is not None
  return list(b[8:8+b[5]])
 flash,w,idx,other,cb=setup(image)
 assert write(flash,w,idx,other,[0,4,0])==0
 assert stored(flash)==[0,4,0]
 st=Storage(Nor(bytes(flash.bytes),authorized=True),codec)
 assert st.load_project(0)==p1 and st.load_project(4)==p2
 # Recommit after another setlist edit while preserving projects.
 assert write(flash,w,idx,other,[4,0,4,0])==0
 assert stored(flash)==[4,0,4,0]
 operations=flash.mutations
 for cut in range(1,260):
  f,wr,a,b,cbs=setup(image,cut)
  result=write(f,wr,a,b,[9,1])
  if result==0:break
  assert stored(f) in (None,[9,1]),(cut,stored(f))
  state=Storage(Nor(bytes(f.bytes),authorized=True),codec)
  assert state.load_project(0)==p1 and state.load_project(4)==p2
 print('PASS V2 A/B setlist save, slot isolation and torn-write recovery',operations,'flash mutations')

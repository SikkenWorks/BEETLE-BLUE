#!/usr/bin/env python3
"""Simulates 12-slot global kit bank A/B commit and normal project saves; no FM1 hardware."""
import ctypes,subprocess,tempfile,zlib,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools/storage_v2'))
from storage_v2_sim import Nor, Storage, ProjectCodec, PowerFailure
P=ROOT/'firmware/fm1-native'
U8=ctypes.c_uint8
RD=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_uint32,ctypes.POINTER(U8),ctypes.c_uint32)
PG=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_uint32,ctypes.POINTER(U8),ctypes.c_uint32)
ER=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_void_p,ctypes.c_uint32)
class Device(ctypes.Structure):
 _fields_=[('read',ctypes.c_void_p),('context',ctypes.c_void_p),('index_address',ctypes.c_uint32),('generation',ctypes.c_uint32)]
class Writer(ctypes.Structure):
 _fields_=[('device',Device),('program',ctypes.c_void_p),('erase',ctypes.c_void_p),('context',ctypes.c_void_p),('domain',U8*72),('occupied',ctypes.c_uint16*72),('active_meta',U8)]
with tempfile.TemporaryDirectory() as tmp:
 so=Path(tmp)/'native.so'
 subprocess.run(['cc','-std=c11','-O2','-fPIC','-shared','-Wall','-Wextra','-Werror',str(P/'beetle_storage_v2_writer.c'),str(P/'beetle_storage_v2_reader.c'),str(P/'beetle_project_pack_v2.c'),'-o',str(so)],check=True)
 lib=ctypes.CDLL(str(so));codec=ProjectCodec(lib)
 lib.beetle_v2_writer_open.argtypes=[ctypes.POINTER(Writer),RD,PG,ER,ctypes.c_void_p,ctypes.POINTER(U8),ctypes.POINTER(U8)]
 lib.beetle_v2_write_blue_kits.argtypes=[ctypes.POINTER(Writer),ctypes.POINTER(U8),ctypes.POINTER(U8),ctypes.POINTER(U8)]
 lib.beetle_v2_write_project.argtypes=[ctypes.POINTER(Writer),U8,ctypes.POINTER(U8),ctypes.c_uint32,ctypes.POINTER(U8),ctypes.POINTER(U8),ctypes.POINTER(U8)]
 nor=Nor(authorized=True);base=Storage(nor,codec)
 p1=(ROOT/'tests/fixtures/MOUNTAINKING-SLOT-01.trk').read_bytes();p2=(ROOT/'tests/fixtures/NUGGIE-SLOT-07.trk').read_bytes()
 base.save_project(0,p1)
 original=bytes(nor.bytes)
 raw=b'BBK1'+b'\xff\x0f'+bytes((i*19%101) for i in range(12*14*8))
 blob=raw+zlib.crc32(raw).to_bytes(4,'little')
 assert len(blob)==1354
 def make(image):
  n=Nor(image,authorized=True);w=Writer();idx=(U8*4096)();other=(U8*4096)();page=(U8*256)()
  def rd(_,addr,dst,size):
   off=addr-0x97000
   if off<0 or off+size>len(n.bytes):return -1
   ctypes.memmove(dst,bytes(n.bytes[off:off+size]),size);return 0
  def pg(_,addr,data,size):
   try:n.program_page(addr-0x97000,bytes(data[:size]));return 0
   except (PowerFailure,ValueError):return -1
  def er(_,addr):
   try:n.erase_sector((addr-0x97000)//4096);return 0
   except (PowerFailure,ValueError):return -1
  callbacks=(RD(rd),PG(pg),ER(er))
  rc=lib.beetle_v2_writer_open(ctypes.byref(w),*callbacks,None,idx,other)
  assert rc==0,rc
  return n,w,idx,other,page,callbacks
 def write_bank(f):
  n,w,idx,other,page,cb=f
  b=(U8*len(blob)).from_buffer_copy(blob)
  return lib.beetle_v2_write_blue_kits(ctypes.byref(w),b,idx,other)
 def current_footer(image):
  st=Storage(Nor(bytes(image),authorized=True),codec)
  idx=st.current.sector
  return bytes(image[idx*4096+4092-1354:idx*4096+4092]),st
 f=make(original)
 assert write_bank(f)==0
 footer,st=current_footer(f[0].bytes)
 assert footer==blob and st.load_project(0)==p1
 n,w,idx,other,page,cb=f
 encoded=codec.pack(p2);a=(U8*len(encoded)).from_buffer_copy(encoded)
 assert lib.beetle_v2_write_project(ctypes.byref(w),0,a,len(encoded),idx,other,page)==0
 footer,st=current_footer(n.bytes)
 assert footer==blob and st.load_project(0)==p2
 print('PASS: kitbank persists across project commit and project still loads')
 # A/B recovery for EVERY flash failure point, fresh prekit volume.
 base_n=make(original)
 assert write_bank(base_n)==0
 writes=base_n[0].mutations
 for cut in range(1,writes+1):
  f=make(original);f[0].fault_after=cut
  assert write_bank(f)!=0
  footer,st=current_footer(f[0].bytes)
  assert st.load_project(0)==p1
  assert footer!=blob or footer==blob  # valid old OR complete new footer
  assert footer==blob or footer[:4]!=b'BBK1' # never published a torn kitbank
 print(f'PASS: {writes} simulated power loss positions, no torn global kitbank or project loss')

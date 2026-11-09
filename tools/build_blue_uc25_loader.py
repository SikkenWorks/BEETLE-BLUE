#!/usr/bin/env python3
"""Reproduce an OTA loader from Felucca GPL source, with two-JEDEC-ID write safety.

The original loader is Felucca's RAM update loader, matching the V1.3 source
except for address/layout relocation and an explicit JEDEC guard. Do not
claim physical OTA validation from a successful compilation alone.
"""
from pathlib import Path
import os,sys,subprocess,struct
root=Path(__file__).resolve().parents[1]
ldr=root/'firmware/fm1-loader';out=root/'build/uc25-loader';out.mkdir(parents=True,exist_ok=True)
tc=Path(os.environ['JIELI_TOOLCHAIN']);cc=tc/'pi32v2/bin/clang';ld=tc/'pi32v2/bin/ld';objcopy=tc/'common/bin/objcopy';objdump=tc/'common/bin/objdump'
for bin in [cc,ld,objcopy]:
 if not bin.exists():raise SystemExit('JieLi toolchain missing '+str(bin))
flags=['-target','pi32v2','-Os','-ffunction-sections','-fno-builtin','-Wall','-Wno-unused-function','-I'+str(ldr/'hal')]
def call(cmd):subprocess.run(list(map(str,cmd)),check=True,cwd=root)
call([cc,'-target','pi32v2','-c',ldr/'crt0_ldr.S','-o',out/'crt0_ldr.o'])
call([cc,*flags,'-c',ldr/'loader.c','-o',out/'loader.o'])
call([ld,'--gc-sections','-e','_start','-T',ldr/'loader.ld',out/'crt0_ldr.o',out/'loader.o','-o',out/'loader.elf'])
call([objcopy,'-O','binary','-j','.text',out/'loader.elf',out/'loader.bin'])
image=(out/'loader.bin').read_bytes()
assert 0<len(image)<0x14000
sys.path.insert(0,str(root/'tools'));import lz4blk, fm1_fwsc as fw
from struct import pack
def jl_entry(dcrc,a,b,attr,name):
 nm=name.encode()+b'\0';nm=nm.ljust(16,b'\0')
 body=pack('<HIIBBH16s',dcrc,a,b,attr,0,0,nm)
 return pack('<H',fw.crc16(body))+body
blocks=bytearray()
for off in range(0,len(image),4096):
 chunk=image[off:off+4096];packed=lz4blk.compress(chunk)
 assert 0<len(packed)<=4096 and lz4blk.decompress(packed)==chunk
 blocks.extend(pack('<II',len(packed),len(chunk))+packed)
inner=jl_entry(fw.crc16(image),len(image),0x01c0a800,0,'usb_hid_ota.bin')
body=inner+blocks
ota=jl_entry(fw.crc16(body),0x20,len(body),0x41,'usb_hid_ota.bin')+body
assert fw.validate_ota(ota)['inner_data_crc_ok']
(out/'ota.bin').write_bytes(ota)
print('Built supported-chip update loader',len(image),'B; ota',len(ota),'B')

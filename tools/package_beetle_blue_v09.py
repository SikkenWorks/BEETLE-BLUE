#!/usr/bin/env python3
"""Recreate the experimental V0.9 .fwsc from newly compiled native app and bundled V1.7.14 reference.
Preserves loader/header and verifies payload round-trip. It does NOT flash or validate physical hardware.
"""
from pathlib import Path
import sys,hashlib
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'tools'))
import fm1_fwsc as fw
ref=r/'reference/BEETLE-V1.7.14-REFERENCE.fwsc'
app=r/'build/track-fm1-native-beetle-storage-lab-updater-uac/app.bin'
out=r/'BEETLE-BLUE-V0.9-FM1-EXPERIMENTAL.fwsc'
orig,err=fw.verify_fwsc(ref)
assert not err and orig['product']=='FM-1_2123',err
assert fw.validate_app_audit(app,Path(str(app)+'.audit.json'))[1]==[]
assets=fw.extract_stock_assets(ref)
old=next(e['data'] for e in fw.parse_ufw(ref.read_bytes())['entries'] if e['name']=='flash.bin')
flash=bytearray(fw.build_flash_image_from_assets(app.read_bytes(),assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload']))
flash[:0x4000]=old[:0x4000]
blob=fw.build_fwsc(bytes(flash),assets['ota'],'FM-1_2123')
out.write_bytes(blob)
result,err=fw.verify_fwsc(out)
assert not err and result['product']=='FM-1_2123',err
parts=fw.parse_ufw(blob)['entries']
newflash=next(e['data'] for e in parts if e['name']=='flash.bin')
loader=next(e['data'] for e in parts if e['name']=='ota.bin')
assert old[:0x4000]==newflash[:0x4000] and loader==assets['ota']
assert fw.compare_packages(ref,out)['same_container_shape']
plain=bytearray(newflash[fw.APP_AREA_FLASH:])
fw.sfc_xor(plain,0,len(plain),0,assets['key'])
assert plain[0x120:0x120+app.stat().st_size]==app.read_bytes()
print('PASS source build, protected header, OTA loader, UFW package integrity, app round-trip',hashlib.sha256(blob).hexdigest())
print('CAUTION: no device flash/recovery/audio validation performed')

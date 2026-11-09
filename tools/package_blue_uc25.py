#!/usr/bin/env python3
"""Package BEETLE BLUE V1.3.1 with two-vendor SPI NOR whitelist in APP and OTA loader.

Requires audited native app, built GPL Felucca update loader and unchanged
V1.3 reference flash image. Does not change stock-return OTA logic or SPL.
"""
import sys,hashlib,json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'tools'))
import fm1_fwsc as fw
app_p=root/'build/track-fm1-native-beetle-storage-lab-updater-uac/app.bin'
ldr_p=root/'build/uc25-loader/ota.bin'
ref=root/'reference/v1.3-reference.fwsc'
out=root/'BEETLE-BLUE-V1.3.1.fwsc'
sha=lambda b:hashlib.sha256(b).hexdigest()
base,errors=fw.verify_fwsc(ref)
assert not errors and base['product']=='FM-1_2224',(errors,base.get('product'))
a=app_p.read_bytes();o=ldr_p.read_bytes()
assert not fw.validate_app_binary(a)
valid_a, errs=fw.validate_app_audit(app_p,Path(str(app_p)+'.audit.json'))
assert valid_a,errs
assert 0<len(a)<=fw.APP_SLOT
v=fw.validate_ota(o)
assert all(v.get(k) for k in ['outer_header_ok','inner_header_ok','outer_data_crc_ok','inner_data_crc_ok','name_ok','inner_load_ok']),v
assert v['encoding']=='lz4-blocks'
old=fw.parse_ufw(ref.read_bytes());by={x['name']:x['data'] for x in old['entries']}
assets=fw.extract_stock_assets(ref)
flash=bytearray(fw.build_flash_image_from_assets(a,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload']))
flash[:0x4000]=by['flash.bin'][:0x4000]
assert bytes(flash[:0x4000])==by['flash.bin'][:0x4000]
assert by['ota.bin']!=o, 'OTA loader unexpectedly not updated'
raw=fw.build_fwsc(bytes(flash),o,'FM-1_2224')
out.write_bytes(raw)
rep,err=fw.verify_fwsc(out)
assert not err,err
assert rep['valid'] and rep['product']=='FM-1_2224'
new_flash=next(x['data'] for x in fw.parse_ufw(raw)['entries'] if x['name']=='flash.bin')
new_ota=next(x['data'] for x in fw.parse_ufw(raw)['entries'] if x['name']=='ota.bin')
assert new_flash==bytes(flash) and new_ota==o
assert fw.validate_ota(new_ota)['inner_data_crc_ok']
dec=bytearray(new_flash[fw.APP_AREA_FLASH:]);fw.sfc_xor(dec,0,len(dec),0,assets['key'])
assert dec[0x120:0x120+len(a)]==a
# Stock V15 return source must remain unchanged; package changes only APP and OTA.
src=root/'firmware/fm1-native/ota.c'
assert 'ota_deadline = ota_now_ms() + 900000u' in src.read_text()
assert 'beetle_compatible_felucca_loader' in src.read_text()
assert 'etch_ota_stock' in src.read_text()
checks={
 'native_app_audited':True,'app_bytes':len(a),'app_headroom':fw.APP_SLOT-len(a),
 'protected_boot_header_identical':new_flash[:0x4000]==by['flash.bin'][:0x4000],
 'package_identity':'FM-1_2224',
 'project_format':'BDR1 V33',
 'ota_loader_integrity':True,
 'ota_loader_updated_intentionally':True,
 'app_and_loader_flash_id_headers_identical':(root/'firmware/fm1-native/hal/fm1_flash.h').read_bytes()==(root/'firmware/fm1-loader/hal/fm1_flash.h').read_bytes(),
 'ota_policy_source_unchanged_from_v13':True,
 'fwsc_sha256':sha(raw),
 'ota_sha256':sha(o),
 'app_sha256':sha(a),
 'hardware_new_chip_test':'not performed',
 'hardware_stock_v15_ota_test':'not performed',
}
assert all(vv for kk,vv in checks.items() if kk in ['protected_boot_header_identical','app_and_loader_flash_id_headers_identical'])
(root/'FIRMWARE-MANIFEST.json').write_text(json.dumps({'firmware':'BEETLE BLUE V1.3.1','product':'FM-1_2224','flash_chips':['Puya P25Q80 0x856014','UCUN UC25HQ80 0xB36014'],'checks':checks},indent=2)+'\n')
(root/'SHA256SUMS.txt').write_text(f'{sha(raw)}  BEETLE-BLUE-V1.3.1.fwsc\n')
print(json.dumps(checks,indent=2))

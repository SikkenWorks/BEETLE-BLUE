#!/usr/bin/env python3
"""Repackage BEETLE BLUE RC11 from the user's RC10, preserving protected header and OTA loader.
No device I/O. Lab only; physical flash/recovery not proven."""
import json,hashlib,sys
from pathlib import Path
import fm1_fwsc as fw
if len(sys.argv)!=4:raise SystemExit('usage: build_blue_rc8_fwsc.py RC10.fwsc APP.bin OUT_RC11.fwsc')
reference,app_path,out=map(Path,sys.argv[1:])
ref,errors=fw.verify_fwsc(reference)
if errors or ref['product']!='FM-1_2124':raise SystemExit('Bad BLUE RC10 reference '+str(errors))
app=app_path.read_bytes();audit,errors=fw.validate_app_audit(app_path,Path(str(app_path)+'.audit.json'))
if errors or not audit:raise SystemExit('Bad APP audit '+str(errors))
assets=fw.extract_stock_assets(reference)
old=fw.parse_ufw(reference.read_bytes());oldflash=next(e['data'] for e in old['entries'] if e['name']=='flash.bin')
newflash=bytearray(fw.build_flash_image_from_assets(app,assets['uboot'],assets['cfg_tool'],assets['eq_cfg'],assets['key'],isd_config=assets['isd_config'],flash_header_payload=assets['flash_header_payload']))
newflash[:0x4000]=oldflash[:0x4000]
data=fw.build_fwsc(bytes(newflash),assets['ota'],'FM-1_2124')
out.write_bytes(data)
result,errors=fw.verify_fwsc(out)
if errors or result['product']!='FM-1_2124':out.unlink();raise SystemExit('Failed package verification '+str(errors))
comparison=fw.compare_packages(reference,out)
new=fw.parse_ufw(data);flash=next(e['data'] for e in new['entries'] if e['name']=='flash.bin');ota=next(e['data'] for e in new['entries'] if e['name']=='ota.bin')
dec=bytearray(flash[fw.APP_AREA_FLASH:]);fw.sfc_xor(dec,0,len(dec),0,assets['key'])
checks={'protected_header_identical':flash[:0x4000]==oldflash[:0x4000],'ota_identical':ota==assets['ota'],'decrypted_app_matches':dec[0x120:0x120+len(app)]==app,'container_shape_identical':comparison['same_container_shape'],'native_app_audit':bool(audit)}
if not all(checks.values()):out.unlink();raise SystemExit('Failed verification '+str(checks))
manifest={'firmware':'BEETLE BLUE V1.0.0 RC11 PER-PATTERN INDEPENDENT LANE LENGTHS','product':'FM-1_2124','package_sha256':hashlib.sha256(data).hexdigest(),'app_sha256':hashlib.sha256(app).hexdigest(),'source_reference':reference.name,'reference_sha256':hashlib.sha256(reference.read_bytes()).hexdigest(),'checks':checks,'hardware_tested':False,'ota_recovery_proven':False}
(out.parent/'RC11-FIRMWARE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(manifest,indent=2))

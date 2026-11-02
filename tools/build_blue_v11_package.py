#!/usr/bin/env python3
"""Build byte-verified BLUE V1.1 FWSC from the audited native app and the packaged reference.

This is not `fm1_fwsc.release-pack` and does not assert or falsify R0 recovery proof.
Use only for developer testing when a separate device-recovery method is available.
Only explicit --acknowledge-hardware-risk authorizes this not independently recovery-certified output.
"""
from __future__ import annotations
import argparse,hashlib,json,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import fm1_fwsc as fw

def sha(data:bytes)->str:return hashlib.sha256(data).hexdigest()

def build(ref_path:Path,app_path:Path,out:Path,ack:bool):
 if not ack:raise SystemExit('Refusing not independently recovery-certified development FWSC without explicit --acknowledge-hardware-risk')
 ref,errs=fw.verify_fwsc(ref_path)
 if errs or ref['product']!='FM-1_2124':raise SystemExit('Invalid packaging base: '+repr(errs))
 app=app_path.read_bytes()
 if fw.validate_app_binary(app):raise SystemExit('App binary invalid')
 auditok,auditerrors=fw.validate_app_audit(app_path,Path(str(app_path)+'.audit.json'))
 if not auditok:raise SystemExit('App audit missing/invalid: '+repr(auditerrors))
 stock=fw.extract_stock_assets(ref_path)
 raw=fw.parse_ufw(ref_path.read_bytes())
 prevflash=next(e['data'] for e in raw['entries'] if e['name']=='flash.bin')
 prevota=next(e['data'] for e in raw['entries'] if e['name']=='ota.bin')
 newflash=bytearray(fw.build_flash_image_from_assets(app,stock['uboot'],stock['cfg_tool'],stock['eq_cfg'],stock['key'],isd_config=stock['isd_config'],flash_header_payload=stock['flash_header_payload']))
 newflash[:0x4000]=prevflash[:0x4000]
 candidate=fw.build_fwsc(bytes(newflash),prevota,'FM-1_2124')
 if len(app)>fw.APP_SLOT:raise SystemExit('app too large')
 out.parent.mkdir(parents=True,exist_ok=True)
 out.write_bytes(candidate)
 report,errs=fw.verify_fwsc(out)
 if errs:
  out.unlink(missing_ok=True);raise SystemExit('FWSC invalid: '+repr(errs))
 comparison=fw.compare_packages(ref_path,out)
 decrypted=bytearray(newflash[fw.APP_AREA_FLASH:]);fw.sfc_xor(decrypted,0,len(decrypted),0,stock['key'])
 checks={
 'package_crc_and_layout_valid':report['valid'],
 'target_identity_exact':report['product']=='FM-1_2124',
 'native_app_audit_match':auditok,
 'app_size_within_slot':len(app)<=fw.APP_SLOT,
 'protected_boot_header_identical':bytes(newflash[:0x4000])==prevflash[:0x4000],
 'ota_byte_identical':prevota==stock['ota'],
 'decrypted_app_matches':decrypted[0x120:0x120+len(app)]==app,
 'container_shape_matches_reference':comparison['same_container_shape'],
 }
 if not all(checks.values()):
  out.unlink(missing_ok=True);raise SystemExit('Failed development build checks: '+repr(checks))
 manifest={'firmware':'BEETLE BLUE V1.1',
           'status':'V1.1 / DEVELOPER-TESTED ON TWO FM1 UNITS',
           'target':'M-VAVE FM1', 'product':'FM-1_2124',
           'fwsc_sha256':sha(candidate),'firmware_bytes':len(candidate),
           'app_sha256':sha(app),'app_bytes':len(app),'app_headroom_bytes':fw.APP_SLOT-len(app),
           'reference_sha256':sha(ref_path.read_bytes()),'reference_filename':ref_path.name,
           'checks':checks,'r0_recovery_proof_present':False,'developer_tested_on_two_fm1':True,
           'formal_release_pack_gate_bypassed':False,
           'warning':'Complete FWSC. The developer reports two-FM1 hardware testing; specific installation and recovery evidence for this exact build is not included.'}
 out.with_name('V1.1-FIRMWARE-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
 print(json.dumps(manifest,indent=2))

if __name__=='__main__':
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('reference_fwsc',type=Path);ap.add_argument('app_bin',type=Path);ap.add_argument('output_fwsc',type=Path)
 ap.add_argument('--acknowledge-hardware-risk',action='store_true')
 args=ap.parse_args()
 build(args.reference_fwsc,args.app_bin,args.output_fwsc,args.acknowledge_hardware_risk)

#!/usr/bin/env python3
"""Offline checks for BEETLE BLUE's stock-V15 return policy.

Optional: python tests/test_stock_v15_exit.py /private/path/to/official-FM-1-V15.fwsc

The optional official package is never copied into the release. This is NOT an
FM1 OTA hardware proof and cannot guarantee a successful power-loss recovery.
"""
from __future__ import annotations
import hashlib,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import fm1_fwsc as fw

V15_SHA='db1642b2b6fa5c2cccb11ffd13878068bb28601678d3644049f99dc40e7edb8a'
V15_FLASH_SHA='c1068a403c04a6378a626afb66cc217f72d13f10aa200e983b7a9817cc1dac03'
V15_SIZE=699956
src=(ROOT/'firmware/fm1-native/ota.c').read_text()
main=(ROOT/'firmware/fm1-native/bringup.c').read_text()
transfer=(ROOT/'firmware/fm1-native/beetle_project_transfer.inc').read_text()
assert 'ota_deadline = ota_now_ms() + 900000u' in src
assert 'ota_read(off+i,b,512)' in src
assert 'ota_len != len + 0x20u' in src
assert 'len==OTA_LDR_LEN && ota_rd16(b)==OTA_LDR_HCRC' in src
assert 'ota_rd16(b+2)==OTA_LDR_DCRC && etch_ota_stock(fl_off,b)' in src
assert '0xE0000000u' in src and '"success"' in src
assert 'if(beetle_transfer_active() && !usb.ota_req)' in main
assert 'beetle_transfer_preempt_for_ota();' in main
assert main.index('native_pause();native_updating=1;') < main.index('beetle_transfer_preempt_for_ota();')
assert 'beetle_xfer_reset();' in transfer.split('static void beetle_transfer_preempt_for_ota(void){',1)[1].split('}',1)[0]
assert 'beetle_v2_sample_upload_abort(beetle_sx_up())' in transfer
assert 'beetle_bd4_live_bind(beetle_v2_workspace(),sizeof(beetle_staged))' in transfer
want_bytes=bytes.fromhex(V15_FLASH_SHA)
assert ','.join(f'0x{x:02x}' for x in want_bytes) in src
assert V15_SHA in src
assert '#define OTA_LDR_HCRC 0xEBAAu' in src
assert '#define OTA_LDR_DCRC 0x5881u' in src
assert '#define OTA_LDR_LEN 0x4DE1u' in src
print('PASS: V15 pinned package policy, fail-closed loader check, OTA preemption, bounded transfer budget')

fwfile=ROOT/'BEETLE-BLUE-V1.3.1.fwsc'
if fwfile.exists():
 pkg, errs=fw.verify_fwsc(fwfile)
 assert not errs, errs
 assert pkg['product']=='FM-1_2224'
 assert fw.validate_ota(next(e['data'] for e in fw.parse_ufw(fwfile.read_bytes())['entries'] if e['name']=='ota.bin'))['inner_data_crc_ok']
 print('PASS: V1.3.1 complete firmware container and new FM-1_2224 identity')

if len(sys.argv)>1:
 f=Path(sys.argv[1]);data=f.read_bytes()
 assert len(data)==V15_SIZE,(len(data),V15_SIZE)
 assert hashlib.sha256(data).hexdigest()==V15_SHA,'Not the exact official Hub V15 package'
 parsed=fw.parse_ufw(data)
 assert parsed['product']=='FM-1_015',parsed['product']
 entries={e['name']:e['data'] for e in parsed['entries']}
 flash=entries['flash.bin'];ota=entries['ota.bin']
 assert len(flash)==0x93000,len(flash)
 assert hashlib.sha256(flash).hexdigest()==V15_FLASH_SHA,'Official flash digest differs from in-device gate'
 assert len(ota)==0x4de1+32,len(ota)
 assert int.from_bytes(ota[0:2],'little')==0xEBAA
 assert int.from_bytes(ota[2:4],'little')==0x5881
 assert int.from_bytes(ota[8:12],'little')==0x4DE1
 print('PASS: exact official Hub V15 image matches all in-device acceptance gates')
else:
 print('NOTE: exact official stock-V15 package was not provided; stock package byte test SKIPPED')

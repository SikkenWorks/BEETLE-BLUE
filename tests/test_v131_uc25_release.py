#!/usr/bin/env python3
"""Offline BEETLE BLUE V1.3.1 dual-NOR-ID, OTA and web-tool regression.

Does not claim physical FM1 support or update/recovery success on the UC25HQ80.
"""
from pathlib import Path
import base64,hashlib,json,re,subprocess,sys,tempfile
r=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(r/'tools'))
import fm1_fwsc as fw
fwfile=r/'BEETLE-BLUE-V1.3.1.fwsc'
fwsc=fwfile.read_bytes();pkg,errors=fw.verify_fwsc(fwfile)
assert not errors,errors
assert pkg['product']=='FM-1_2224' and pkg['chip']=='AC791N'
newraw=fw.parse_ufw(fwsc)
parts={e['name']:e['data'] for e in newraw['entries']}
assert list(parts)==['flash.bin','ota.bin']
v=fw.validate_ota(parts['ota.bin'])
for field in ('outer_header_ok','inner_header_ok','outer_data_crc_ok','inner_data_crc_ok','name_ok','inner_load_ok'):
 assert v[field],field
assert v['encoding']=='lz4-blocks'
assert parts['ota.bin']==(r/'release-artifacts/v1.3.1-ota.bin').read_bytes()
ref=fw.parse_ufw((r/'reference/v1.3-reference.fwsc').read_bytes())
old={e['name']:e['data'] for e in ref['entries']}
assert old['flash.bin'][:0x4000]==parts['flash.bin'][:0x4000],'protected ROM/SPL changed'
assert old['ota.bin']!=parts['ota.bin'],'loader still old'
assert len(parts['flash.bin'])==len(old['flash.bin'])==0x93000
# One ID header, byte-identical in app and loader.
a=(r/'firmware/fm1-native/hal/fm1_flash.h').read_text()
b=(r/'firmware/fm1-loader/hal/fm1_flash.h').read_text()
assert a==b
assert (r/'firmware/fm1-native/hal/fm1_flash_id.h').read_bytes()==(r/'firmware/fm1-loader/hal/fm1_flash_id.h').read_bytes()
for source in (a,b):
 assert '#include "fm1_flash_id.h"' in source
 assert source.count('if (!FL_JEDEC_OK(fl_read_jedec_spi())) { fl_exit(&s); return -5; }')==2
 # Check the guards in each function run before WREN.
 for name,next_name in [('fl_erase4k_ram','fl_prog_ram'),('fl_prog_ram','fl_read_ram')]:
  p=source.index('static RAMFN int '+name+'(')
  end=source.index('static RAMFN int '+next_name+'(',p) if next_name=='fl_prog_ram' else source.index('#else\n/* Explicitly',p)
  body=source[p:end]
  assert body.index('!FL_JEDEC_OK')<body.index('fl_wren();')
assert '#define FL_JEDEC_OK' in (r/'firmware/fm1-native/hal/fm1_flash_id.h').read_text()
# Actually execute the C condition on the build host with real values.
ccode='''#include "fm1_flash_id.h"
int main(void) {
 if (!FL_JEDEC_OK(0x856014u) || !FL_JEDEC_OK(0xB36014u)) return 1;
 if (FL_JEDEC_OK(0u) || FL_JEDEC_OK(0xFFFFFFu) || FL_JEDEC_OK(0x856013u) || FL_JEDEC_OK(0xB36015u)) return 2;
 return 0;
}'''
with tempfile.TemporaryDirectory() as d:
 from pathlib import Path
 src=Path(d)/'test.c';exe=Path(d)/'test';src.write_text(ccode)
 subprocess.run(['cc','-std=c11','-Werror','-I'+str(r/'firmware/fm1-native/hal'),str(src),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
# Native app changed on purpose, but source return-to-stock policy MUST be identical.
app=r/'release-artifacts/v1.3.1-app.bin';audit=Path(str(app)+'.audit.json')
ok,ae=fw.validate_app_audit(app,audit)
assert ok,ae
assert not fw.validate_app_binary(app.read_bytes())
ref_app=(r/'release-artifacts/v1.3.1-app.bin').read_bytes()
assert b'BEETLE BLUE V1.3.1' in ref_app
assert 'ota_deadline = ota_now_ms() + 900000u' in (r/'firmware/fm1-native/ota.c').read_text()
assert 'beetle_transfer_preempt_for_ota();' in (r/'firmware/fm1-native/bringup.c').read_text()
# Web installer and manager must embed the exact new .fwsc.
inst=(r/'WebInstaller.html').read_text();proj=(r/'BEETLE-BLUE-Project-Manager.html').read_text();mgr=(r/'BEETLE-BLUE-Manager.html').read_text()
sha=hashlib.sha256(fwsc).hexdigest()
assert "HASH_ET: '"+sha+"'" in inst
assert base64.b64decode(re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inst,re.S).group(1))==fwsc
for key,s in [('source-installer',inst),('source-projectmanager',proj)]:
 assert base64.b64decode(re.search(r'<script id="'+key+'"[^>]*>(.*?)</script>',mgr,re.S).group(1)).decode()==s
for p in ('WebInstaller.html','BEETLE-BLUE-Project-Manager.html','BEETLE-BLUE-Manager.html'):
 assert (r/'web'/p).read_bytes()==(r/p).read_bytes()
manifest=json.loads((r/'FIRMWARE-MANIFEST.json').read_text())
assert manifest['checks']['fwsc_sha256']==sha
assert manifest['product']=='FM-1_2224'
assert 'V1.3.1' in (r/'README.md').read_text()
print('PASS: both JEDEC IDs accepted in app/loader, unknown rejected before writes, original boothead intact, valid new loader+FWSC, OTA source guards, embedded installer/manager parity')

#!/usr/bin/env python3
"""Fail closed: V1.3 may change the version text, not the original V1.2 ID2224 OTA implementation."""
from pathlib import Path
import sys,hashlib,base64,re
r=Path(__file__).resolve().parents[1];sys.path.insert(0,str(r/'tools'))
import fm1_fwsc as fw
old=r/'archive/identity-baseline/BEETLE-BLUE-V1.2-ID2224-REFERENCE.fwsc'
new=r/'BEETLE-BLUE-V1.3.fwsc'
a,ae=fw.verify_fwsc(old);b,be=fw.verify_fwsc(new)
assert not ae and not be, (ae,be)
assert a['product']==b['product']=='FM-1_2224'
assert a['chip']==b['chip']=='AC791N'
def entries(path):return {e['name']:e['data'] for e in fw.parse_ufw(path.read_bytes())['entries']}
x,y=entries(old),entries(new)
assert x['ota.bin']==y['ota.bin'],'OTA loader changed'
assert x['flash.bin'][:0x4000]==y['flash.bin'][:0x4000],'Boot sector changed'
assert len(x['flash.bin'])==len(y['flash.bin'])==0x93000
key=a['flash']['chip_key']
def dec_app(flash):
 z=bytearray(flash[fw.APP_AREA_FLASH:]);fw.sfc_xor(z,0,len(z),0,key)
 h=fw.parse_jlf_entry(bytes(z[0x20:0x40]));assert h['name']=='app.bin' and h['offset']==0x120
 return bytes(z[h['offset']:h['offset']+h['size']])
p,q=dec_app(x['flash.bin']),dec_app(y['flash.bin'])
assert len(p)==len(q),'Unexpected compiled app size change'
assert p.count(b'BEETLE BLUE V1.2')==1 and q.count(b'BEETLE BLUE V1.3')==1
assert p.replace(b'BEETLE BLUE V1.2',b'BEETLE BLUE V1.3')==q,'Unexpected compiled application modifications'
assert sum(u!=v for u,v in zip(p,q))==1,'More than the 1 version byte changed'
assert q.startswith((r/'release-artifacts/v1.3-app.bin').read_bytes()), 'packaged app does not match audited image'
assert 'FM-1_2224' in (r/'firmware/fm1-native/bringup.c').read_text()
inst=(r/'WebInstaller.html').read_text();mgr=(r/'BEETLE-BLUE-Manager.html').read_text()
assert "ID_ET: 'FM-1_2224'" in inst
assert hashlib.sha256(new.read_bytes()).hexdigest() in inst
assert base64.b64decode(re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inst,re.S).group(1))==new.read_bytes()
assert base64.b64decode(re.search(r'<script id="source-installer"[^>]*>(.*?)</script>',mgr,re.S).group(1)).decode()==inst
assert 'ota_deadline = ota_now_ms() + 900000u' in (r/'firmware/fm1-native/ota.c').read_text()
assert 'beetle_transfer_preempt_for_ota();' in (r/'firmware/fm1-native/bringup.c').read_text()
print('PASS: V1.3 changes exactly one app-version byte; identical V1.2 bootloader, OTA and stock-return code; correct product ID, installer and Manager')

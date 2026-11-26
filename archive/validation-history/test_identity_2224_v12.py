#!/usr/bin/env python3
"""Fail-closed identity-only upgrade regression: V1.2 ID2124 -> ID2224.

This compares the actual pre-change V1.2 firmware with the release. It checks
identity, CRCs, inner app bytes, OTA loader, boot sectors and installer embedding.
No FM1 device or official stock V15 download is required for these offline checks.
"""
from pathlib import Path
import base64, hashlib, re, sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import fm1_fwsc as fw
old=ROOT/'archive/identity-baseline/BEETLE-BLUE-V1.2-ID2124-REFERENCE.fwsc'
new=ROOT/'BEETLE-BLUE-V1.2.fwsc'
o,oe=fw.verify_fwsc(old)
n,ne=fw.verify_fwsc(new)
assert not oe and not ne,(oe,ne)
assert o['product']=='FM-1_2124',o['product']
assert n['product']=='FM-1_2224',n['product']
assert o['chip']==n['chip']=='AC791N'
old_entries={e['name']:e['data'] for e in fw.parse_ufw(old.read_bytes())['entries']}
new_entries={e['name']:e['data'] for e in fw.parse_ufw(new.read_bytes())['entries']}
assert old_entries['ota.bin']==new_entries['ota.bin'],'OTA loader changed'
assert old_entries['flash.bin'][:0x4000]==new_entries['flash.bin'][:0x4000],'boot sector changed'
assert len(old_entries['flash.bin'])==len(new_entries['flash.bin'])==0x93000
# Compare the two original compiler app images after decrypting app area.
key=o['flash']['chip_key']
def app_plain(flash):
 b=bytearray(flash[fw.APP_AREA_FLASH:]);fw.sfc_xor(b,0,len(b),0,key)
 header=fw.parse_jlf_entry(bytes(b[0x20:0x40]));assert header['name']=='app.bin'
 assert header['offset']==0x120
 return bytes(b[header['offset']:header['offset']+header['size']])
a=app_plain(old_entries['flash.bin']);b=app_plain(new_entries['flash.bin'])
assert a.count(b'FM-1_2124')==1
assert b.count(b'FM-1_2224')==1
assert a.replace(b'FM-1_2124',b'FM-1_2224')==b,'Unexpected compiled application changes'
assert sum(x!=y for x,y in zip(a,b))==1,'Application should change only one product-ID byte'
assert (ROOT/'firmware/fm1-native/bringup.c').read_text().count('"FM-1_2224"')==1
inst=(ROOT/'WebInstaller.html').read_text()
assert "ID_ET: 'FM-1_2224'" in inst
m=re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inst,re.S);assert m
assert base64.b64decode(m.group(1))==new.read_bytes(),'Installer embeds wrong firmware'
assert hashlib.sha256(new.read_bytes()).hexdigest() in inst
manager=(ROOT/'BEETLE-BLUE-Manager.html').read_text()
m=re.search(r'<script id="source-installer"[^>]*>(.*?)</script>',manager,re.S);assert m
assert base64.b64decode(m.group(1)).decode()==inst,'Manager embeds wrong installer'
print('PASS: ID2224 flash, USB application identity, exact one byte native delta, unchanged OTA+boot, complete installer parity')

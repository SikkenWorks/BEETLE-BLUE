#!/usr/bin/env python3
"""V1.3 current-source release checks. Run: python tests/test_v13_release.py"""
from pathlib import Path
import hashlib,re,base64,json,subprocess,sys
r=Path(__file__).resolve().parents[1]
fw=(r/'BEETLE-BLUE-V1.3.fwsc').read_bytes();h=hashlib.sha256(fw).hexdigest()
inst=(r/'WebInstaller.html').read_text();proj=(r/'BEETLE-BLUE-Project-Manager.html').read_text();mgr=(r/'BEETLE-BLUE-Manager.html').read_text()
assert base64.b64decode(re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inst,re.S).group(1))==fw
assert "HASH_ET: '"+h+"'" in inst
for k,data in [('source-installer',inst),('source-projectmanager',proj)]:
 assert base64.b64decode(re.search(r'<script id="'+k+r'"[^>]*>(.*?)</script>',mgr,re.S).group(1)).decode()==data
manifest=json.loads((r/'FIRMWARE-MANIFEST.json').read_text())
assert manifest['fwsc_sha256']==h
assert manifest['app_sha256']==hashlib.sha256((r/'release-artifacts/v1.3-app.bin').read_bytes()).hexdigest()
assert manifest['product']=='FM-1_2224'
assert b'BEETLE BLUE V1.3' in (r/'release-artifacts/v1.3-app.bin').read_bytes()
assert 'BEETLE BLUE V1.3' in (r/'firmware/fm1-native/beetle_help.h').read_text()
assert 'BEETLE BLUE V1.3' in (r/'firmware/fm1-native/track_runtime.h').read_text()
assert 'BDR1 V33' in (r/'README.md').read_text()
for webname in ('WebInstaller.html','BEETLE-BLUE-Project-Manager.html','BEETLE-BLUE-Manager.html'):
 assert (r/'web'/webname).read_bytes()==(r/webname).read_bytes()
assert not re.search(r'V1\.0 RC12|V1\.0\.0 RC12', (r/'WebInstaller.html').read_text())
for f in ('README.md','STATUS.md','WebInstaller.html','BEETLE-BLUE-Project-Manager.html'):
 assert not re.search('experimental', (r/f).read_text(),re.I),f
assert 'two physical' in (r/'README.md').read_text().lower()
print('PASS current V1.3 firmware, two nested webtools, pinned hashes, manifest and release text')

#!/usr/bin/env python3
"""Sync V1.3.1 FWSC, loader digest and packaged web tools reproducibly.

Run after package_blue_uc25.py. Never broadens the WebInstaller beyond its
pinned old and new approved loader families.
"""
from pathlib import Path
import re,base64,hashlib,shutil
r=Path(__file__).resolve().parents[1]
fw=(r/'BEETLE-BLUE-V1.3.1.fwsc').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
loader=(r/'release-artifacts/v1.3.1-ota.bin').read_bytes()
loader_sha=hashlib.sha256(loader[32:]).hexdigest()
assert len(loader)>64
instp=r/'WebInstaller.html';inst=instp.read_text()
inst,n=re.subn(r"(HASH_ET: ')[0-9a-f]{64}(')",lambda m:m.group(1)+sha+m.group(2),inst,count=1)
assert n==1,'HASH_ET pin missing'
# New format intentionally pinned in both length and SHA.
assert f"len:{len(loader)},body:{len(loader)-32},sha:'{loader_sha}'" in inst, 'Built loader is not the audited V1.3.1 version'
pat=r'(<script id="firmware-embedded"[^>]*>)(.*?)(</script>)'
m=re.search(pat,inst,re.S);assert m
inst=inst[:m.start(2)]+base64.b64encode(fw).decode('ascii')+inst[m.end(2):]
instp.write_text(inst)
proj=(r/'BEETLE-BLUE-Project-Manager.html').read_text()
mp=r/'BEETLE-BLUE-Manager.html';mgr=mp.read_text()
for key,data in [('source-installer',inst),('source-projectmanager',proj)]:
 m=re.search(r'(<script id="'+key+r'"[^>]*>)(.*?)(</script>)',mgr,re.S);assert m,key
 mgr=mgr[:m.start(2)]+base64.b64encode(data.encode()).decode('ascii')+mgr[m.end(2):]
mp.write_text(mgr)
for name in ('WebInstaller.html','BEETLE-BLUE-Project-Manager.html','BEETLE-BLUE-Manager.html'):
 shutil.copyfile(r/name,r/'web'/name)
print('Synced WebInstaller and Manager to FWSC SHA256',sha)

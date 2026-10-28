#!/usr/bin/env python3
"""Regenerate BEETLE BLUE V0.99.4 offline WebInstaller and TWO-TOOL Manager.
Never builds/embeds a simulator. Does not flash hardware.
"""
from pathlib import Path
import hashlib,base64,re
R=Path(__file__).resolve().parents[1]
F=(R/'BEETLE-BLUE-V0.99.4-FM1-EXPERIMENTAL.fwsc').read_bytes()
H=hashlib.sha256(F).hexdigest()
def put(h,k,b):
 tag='<script id="'+k+'" type="application/octet-stream">'
 a=h.index(tag)+len(tag);z=h.index('</script>',a)
 return h[:a]+base64.b64encode(b).decode()+h[z:]
web=(R/'WebInstaller.html').read_text()
web=put(web,'firmware-embedded',F)
web,n=re.subn(r"(HASH_ET: ')[a-f0-9]{64}(')",lambda m:m[1]+H+m[2],web,count=1)
assert n==1
(R/'WebInstaller.html').write_text(web)
(R/'BEETLE-BLUE-V0.99.4-WebInstaller.html').write_text(web)
manager=(R/'BEETLE-BLUE-Manager.html').read_text()
assert 'source-simulator' not in manager and 'source-emulator' not in manager
manager=put(manager,'source-projectmanager',(R/'BEETLE-BLUE-Project-Manager.html').read_bytes())
manager=put(manager,'source-installer',web.encode())
(R/'BEETLE-BLUE-Manager.html').write_text(manager)
for f in ('WebInstaller.html','BEETLE-BLUE-Manager.html','BEETLE-BLUE-Project-Manager.html'):
 (R/'web'/f).write_bytes((R/f).read_bytes())
(R/'web/BEETLE-BLUE-MANAGER.html').write_bytes((R/'BEETLE-BLUE-Manager.html').read_bytes())
print('PASS two webtools only, FWSC SHA-256',H)

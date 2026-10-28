#!/usr/bin/env python3
"""Embed byte-identical versioned .fwsc into WebInstaller and offline Manager.
Only the existing two tools are included, no misleading simulated FM1 emulator.
"""
import base64,re,hashlib
from pathlib import Path
r=Path(__file__).resolve().parents[1]
old=r/'BEETLE-BLUE-V0.99.6-FM1-EXPERIMENTAL.fwsc'
fw=r/'BEETLE-BLUE-V0.99.7-FM1-EXPERIMENTAL.fwsc'
oldhash=hashlib.sha256(old.read_bytes()).hexdigest()
newhash=hashlib.sha256(fw.read_bytes()).hexdigest()
web=r/'WebInstaller.html'
s=web.read_text()
s=s.replace('V0.99.5','V0.99.7').replace('V0.99.6','V0.99.7')
s,n_hash=re.subn(r"(HASH_ET:\s*')[a-f0-9]{64}(')",lambda m:m.group(1)+newhash+m.group(2),s)
assert n_hash==1
s,n=re.subn(r'(<script id="firmware-embedded" type="application/octet-stream">)[A-Za-z0-9+/=]+(</script>)',lambda m:m.group(1)+base64.b64encode(fw.read_bytes()).decode()+m.group(2),s)
assert n==1 and newhash in s and oldhash not in s
web.write_text(s)
(r/'web/WebInstaller.html').write_text(s)
pm=r/'BEETLE-BLUE-Project-Manager.html'
p=pm.read_text().replace('V0.99.5','V0.99.7').replace('V0.99.6','V0.99.7');pm.write_text(p)
(r/'web/BEETLE-BLUE-Project-Manager.html').write_text(p)
man=r/'BEETLE-BLUE-Manager.html'
s=man.read_text().replace('V0.99.5','V0.99.7').replace('V0.99.6','V0.99.7')
for id,source in [('source-projectmanager',pm.read_bytes()),('source-installer',web.read_bytes())]:
 pat=r'(<script id="'+id+r'" type="application/octet-stream">)[A-Za-z0-9+/=]+(</script>)'
 s,n=re.subn(pat,lambda m:m.group(1)+base64.b64encode(source).decode()+m.group(2),s)
 assert n==1
man.write_text(s)
(r/'web/BEETLE-BLUE-Manager.html').write_text(s)
(r/'web/BEETLE-BLUE-MANAGER.html').write_text(s)
print('PASS pinned firmware SHA',newhash,'and byte-matched WebInstaller+Manager')

#!/usr/bin/env python3
"""Finalize RC11 web wrappers with firmware SHA and byte-identical nested tools."""
from pathlib import Path
import re,base64,hashlib,tempfile,subprocess
root=Path(__file__).resolve().parents[1]
fw=Path('/mnt/data/BEETLE-BLUE-V1.0.0-RC11-LANE-LENGTH-EXPERIMENTAL.fwsc').read_bytes()
inst=(root/'BEETLE-BLUE-V1.0.0-RC10-WebInstaller.html').read_text()
inst=inst.replace('RC10','RC11').replace('BDR1 V32','BDR1 V33')
pattern=r'(<script id="firmware-embedded"[^>]*>).*?(</script>)'
inst,n=re.subn(pattern,lambda m:m.group(1)+base64.b64encode(fw).decode('ascii')+m.group(2),inst,flags=re.S);assert n==1
inst,n=re.subn(r"HASH_ET: '[a-f0-9]{64}'","HASH_ET: '"+hashlib.sha256(fw).hexdigest()+"'",inst);assert n==1
(root/'WebInstaller.html').write_text(inst)
(root/'BEETLE-BLUE-V1.0.0-RC11-WebInstaller.html').write_text(inst)
proj=(root/'BEETLE-BLUE-Project-Manager.html').read_text().replace('RC10','RC11').replace('BDR1 V32','BDR1 V33')
# The one-shot editor appends a trailer; project schema >=33 has an added three-byte pattern header.
# The trailer stays unchanged, so a V33 project may still be opened/edited without rewriting pattern internals.
proj=proj.replace('if(v<22||v>32)', 'if(v<22||v>33)').replace('supports V22–V32 projects','supports V22–V33 projects')
proj=proj.replace("if(!sampleBase)return;const choice=await modal('LEGACY PROJECT EXPORT'", "if(!sampleBase)return;if(u16(sampleBase,4)>=33)throw Error('V33 lane lengths cannot safely be downgraded to V22. Export the native V33 project instead.');const choice=await modal('LEGACY PROJECT EXPORT'")
(root/'BEETLE-BLUE-Project-Manager.html').write_text(proj)
(root/'BEETLE-BLUE-V1.0.0-RC11-Project-Manager.html').write_text(proj)
manager=(root/'BEETLE-BLUE-Manager.html').read_text().replace('RC10','RC11')
for key,body in [('source-installer',inst),('source-projectmanager',proj)]:
 pat=r'(<script id="'+key+r'"[^>]*>)(.*?)(</script>)'
 manager,n=re.subn(pat,lambda m:m.group(1)+base64.b64encode(body.encode()).decode()+m.group(3),manager,flags=re.S)
 assert n==1,key
(root/'BEETLE-BLUE-Manager.html').write_text(manager)
(root/'BEETLE-BLUE-V1.0.0-RC11-Manager.html').write_text(manager)
for key,body in [('source-installer',inst),('source-projectmanager',proj)]:
 pat=r'<script id="'+key+r'"[^>]*>(.*?)</script>'
 inner=base64.b64decode(re.search(pat,manager,re.S).group(1)).decode()
 assert inner==body
embedded=base64.b64decode(re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inst,re.S).group(1))
assert embedded==fw
assert "HASH_ET: '"+hashlib.sha256(fw).hexdigest()+"'" in inst
with tempfile.TemporaryDirectory() as td:
 for name,content in [('manager',manager),('installer',inst),('project-manager',proj)]:
  js='\n'.join(m.group(2) for m in re.finditer(r'<script([^>]*)>(.*?)</script>',content,re.S) if not ('application/octet-stream' in m.group(1) or 'src=' in m.group(1) or 'type="application/' in m.group(1)))
  dst=Path(td)/(name+'.js');dst.write_text(js)
  r=subprocess.run(['node','--check',str(dst)],capture_output=True,text=True)
  if r.returncode:raise ValueError(name+': '+r.stderr)
print('PASS RC11: firmware checksum, native embedded installer, both Manager wrappers, Project Manager and JS syntax')

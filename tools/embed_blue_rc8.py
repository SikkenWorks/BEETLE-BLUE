#!/usr/bin/env python3
"""Refresh the existing all-in-one Manager, retaining its transport/UI logic."""
from pathlib import Path
import re,base64,hashlib,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
p=root/'BEETLE-BLUE-Manager.html';shell=p.read_text()
pattern=r'(<script id="source-installer"[^>]*>)(.*?)(</script>)'
match=re.search(pattern,shell,re.S);assert match
html=base64.b64decode(match.group(2)).decode()
fw=(root/'BEETLE-BLUE-V1.0.0-RC8-PROJECT-SAVE-FIX.fwsc').read_bytes()
html=html.replace('RC7','RC8').replace('BDR1 V31','BDR1 V32')
html,n=re.subn(r'(<script id="firmware-embedded"[^>]*>).*?(</script>)',lambda m:m[1]+base64.b64encode(fw).decode()+m[2],html,flags=re.S);assert n==1
html,n=re.subn(r"HASH_ET: '[a-f0-9]{64}'","HASH_ET: '"+hashlib.sha256(fw).hexdigest()+"'",html);assert n==1
(root/'BEETLE-BLUE-V1.0.0-RC8-WebInstaller.html').write_text(html)
shell=re.sub(pattern,lambda m:m[1]+base64.b64encode(html.encode()).decode()+m[3],shell,flags=re.S).replace('RC7','RC8')
p.write_text(shell)
# Verify both embedding layers, pinned hash, and executable script syntax.
inner=base64.b64decode(re.search(pattern,shell,re.S)[2]).decode();assert inner==html
embedded=base64.b64decode(re.search(r'<script id="firmware-embedded"[^>]*>(.*?)</script>',inner,re.S)[1]);assert embedded==fw
with tempfile.TemporaryDirectory() as td:
 for name,content in [('manager',shell),('installer',html)]:
  scripts=[m[2] for m in re.finditer(r'<script([^>]*)>(.*?)</script>',content,re.S) if 'application/octet-stream' not in m[1] and 'src=' not in m[1]]
  js=Path(td)/(name+'.js');js.write_text('\n'.join(scripts));subprocess.run(['node','--check',str(js)],check=True)
print('PASS Manager + standalone installer: embedded firmware identical, SHA256 pinned, JS syntax valid')

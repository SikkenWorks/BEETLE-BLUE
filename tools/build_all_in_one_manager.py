#!/usr/bin/env python3
"""Build an offline BEETLE BLUE Manager from ONLY Project Manager and WebInstaller.

Usage: python tools/build_all_in_one_manager.py MANAGER_TEMPLATE.html PROJECT_MANAGER.html WEBINSTALLER.html OUTPUT.html
The template must contain the two source-<tool> HTML script slots and the two-tab navigation.
Historical three-tool templates are deliberately rejected.
"""
from pathlib import Path
import base64, hashlib, re, sys
if len(sys.argv)!=5:
 raise SystemExit('usage: build_all_in_one_manager.py TEMPLATE.html PROJECT_MANAGER.html WEBINSTALLER.html OUTPUT.html')
template,project,installer,out=map(Path,sys.argv[1:])
html=template.read_text(encoding='utf-8')
if re.search(r'emulator|simulator',html,re.I) or 'frame.srcdoc=base64ToUtf8' not in html:
 raise SystemExit('Requires a two-tool srcdoc template without an emulator')
for key,path in [('projectmanager',project),('installer',installer)]:
 pattern=rf'(<script id="source-{key}" type="application/octet-stream">)[\s\S]*?(</script>)'
 data=path.read_bytes()
 html,n=re.subn(pattern,lambda m:m.group(1)+base64.b64encode(data).decode('ascii')+m.group(2),html,count=1)
 if n!=1:raise SystemExit('Missing source slot '+key)
 print(key,len(data),'SHA-256',hashlib.sha256(data).hexdigest())
version=re.search(r'V\d+(?:\.\d+){1,2}',installer.name)
if version:
 html=re.sub(r'V\d+(?:\.\d+){1,2}',version.group(0),html)
out.write_text(html,encoding='utf-8')
print('Built',out)

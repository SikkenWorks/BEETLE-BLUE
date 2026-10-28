#!/usr/bin/env python3
"""Read-only, offline verification of V0.99.8 installer and two-tab Manager."""
from pathlib import Path
import base64,hashlib,re,subprocess,html.parser
R=Path(__file__).resolve().parents[1]
fw=(R/'BEETLE-BLUE-V0.99.8-FM1-EXPERIMENTAL.fwsc').read_bytes()
web=(R/'WebInstaller.html').read_text(encoding='utf-8')
pm=(R/'BEETLE-BLUE-Project-Manager.html').read_bytes()
manager=(R/'BEETLE-BLUE-Manager.html').read_text(encoding='utf-8')
get=lambda s,key: re.search(r'<script id="'+re.escape(key)+r'" type="application/octet-stream">([A-Za-z0-9+/=]+)</script>',s).group(1)
assert base64.b64decode(get(web,'firmware-embedded'))==fw
sha=hashlib.sha256(fw).hexdigest()
assert sha in web and web.count(sha)==1
assert b'V0.99.8' in pm and 'V0.99.8' in web and 'V0.99.8' in manager
assert base64.b64decode(get(manager,'source-projectmanager'))==pm
assert base64.b64decode(get(manager,'source-installer'))==web.encode()
assert not re.search(r'id="(?:tab|frame|view|source)-(?:emulator|simulator)',manager,re.I)
class Scripts(html.parser.HTMLParser):
 def __init__(self):super().__init__();self.t=None;self.text='';self.scripts=[]
 def handle_starttag(self,t,attrs):
  if t=='script':self.t=dict(attrs).get('type','');self.text=''
 def handle_data(self,data):
  if self.t is not None:self.text+=data
 def handle_endtag(self,t):
  if t=='script' and self.t is not None:
   if self.t not in ('application/octet-stream','application/json'):
    self.scripts.append(self.text)
   self.t=None
for name in ('WebInstaller.html','BEETLE-BLUE-Project-Manager.html','BEETLE-BLUE-Manager.html'):
 p=R/name;parser=Scripts();parser.feed(p.read_text())
 for i,js in enumerate(parser.scripts):
  done=subprocess.run(['node','--check'],input=js,text=True,capture_output=True)
  assert done.returncode==0, (name,i,done.stderr[:500])
 print('PASS',name,len(parser.scripts),'JavaScript code blocks')
print('PASS embedded firmware SHA256',sha,'and byte-identical two-tool Manager')

from pathlib import Path
import re,subprocess,hashlib,base64,tempfile
r=Path(__file__).resolve().parents[2]/'web'
for name in ['simulator.html','BEETLE-BLUE-Project-Manager.html','WebInstaller.html','BEETLE-BLUE-Manager.html']:
 s=(r/name).read_text()
 scripts=re.findall(r'<script(?:\s[^>]*)?>(.*?)</script>',s,re.S|re.I)
 count=0
 for script in scripts:
  if len(script.strip())<10 or 'firmware-embedded' in script[:100]:continue
  if re.search(r'^\s*(?:[\w+\-/=]{500,})\s*$',script):continue
  with tempfile.NamedTemporaryFile(suffix='.js') as f:
   f.write(script.encode());f.flush()
   p=subprocess.run(['node','--check',f.name],text=True,capture_output=True)
   if p.returncode: print(name,'ERROR',p.stderr[:700]);raise SystemExit(1)
  count+=1
 print('PASS JS parser',name,'script blocks:',count)
manager=(r/'BEETLE-BLUE-Manager.html').read_text()
for source,expected in [('source-emulator','simulator.html'),('source-projectmanager','BEETLE-BLUE-Project-Manager.html'),('source-installer','WebInstaller.html')]:
 text=re.search(r'<script id="'+source+r'" type="application/octet-stream">(.*?)</script>',manager,re.S).group(1)
 assert base64.b64decode(text)==(r/expected).read_bytes(),source
 print('PASS embedded:',source)
firmware=(r.parent/'BEETLE-BLUE-V0.91-FM1-EXPERIMENTAL.fwsc').read_bytes()
inst=(r/'WebInstaller.html').read_text()
payload=re.search(r'<script id="firmware-embedded" type="application/octet-stream">(.*?)</script>',inst,re.S).group(1)
assert base64.b64decode(payload)==firmware
sha=hashlib.sha256(firmware).hexdigest()
assert "HASH_ET: '"+sha+"'" in inst
print('PASS installer embedded firmware exact hash',sha)

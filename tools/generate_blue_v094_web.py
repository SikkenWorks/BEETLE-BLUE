from pathlib import Path
import re,base64,json,hashlib
r=Path(__file__).resolve().parents[1]
def extract(doc,id):
 tag='<script id="'+id+'"'; start=doc.index('>',doc.index(tag))+1;end=doc.index('</script>',start)
 return doc[start:end]
def replace(doc,id,content):
 tag='<script id="'+id+'"';start=doc.index('>',doc.index(tag))+1;end=doc.index('</script>',start)
 return doc[:start]+content+doc[end:]
fw=(r/'BEETLE-BLUE-V0.94-FM1-EXPERIMENTAL.fwsc').read_bytes();sha=hashlib.sha256(fw).hexdigest()
names=['kick','snare_1','hihat_1','open_hihat','tom_1','tom_2','tom_3','crash','ride','clap','perc_1','perc_2','perc_3','rim','snare_2','hihat_2']
previews=r/'assets/BEETLE-BLUE-V094-ROM-PREVIEWS'
rom={name+'.wav':base64.b64encode((previews/(name+'.wav')).read_bytes()).decode() for name in names}
sim=(r/'BEETLE-BLUE-V0.93-simulator.html').read_text()
sim=replace(sim,'monolith-original-wavs',json.dumps(rom,separators=(',',':')))
sim=sim.replace('V0.93','V0.94').replace('MONOLITH SAMPLE KIT','MONOLITH / ROM').replace('DEFAULT / BEETLE ROM','BEETLE')
sim=sim.replace('MONOLITH uses supplied WAVs','MONOLITH uses C-generated ROM preview audio')
sim=sim.replace('all MONOLITH preview voices use the 16 built-in compressed samples','all MONOLITH preview voices use 16 C-rendered ROM synthesis previews')
sim=sim.replace('16 built-in compressed samples','16 MONOLITH ROM drum models').replace('MONOLITH BUILT-IN kit ready:','MONOLITH ROM previews loaded:').replace('MONOLITH sample decoding failed','MONOLITH ROM preview decoding failed')
(r/'simulator.html').write_text(sim);(r/'BEETLE-BLUE-V0.94-simulator.html').write_text(sim)
print('sim',len(sim),flush=True)
pm=(r/'BEETLE-BLUE-Project-Manager.html').read_text().replace('V0.93','V0.94')
pm=pm.replace('16 MONOLITH drum sounds are embedded','16 MONOLITH synthesized ROM drum models are embedded').replace('MONOLITH sample kit','MONOLITH ROM kit')
(r/'BEETLE-BLUE-Project-Manager.html').write_text(pm)
print('pm',len(pm),flush=True)
ins=(r/'BEETLE-BLUE-V0.93-WebInstaller.html').read_text().replace('V0.93','V0.94')
ins=replace(ins,'firmware-embedded',base64.b64encode(fw).decode())
ins,n=re.subn(r"(HASH_ET: ')[a-f0-9]{64}(')",lambda m:m[1]+sha+m[2],ins,count=1);assert n==1
(r/'WebInstaller.html').write_text(ins);(r/'BEETLE-BLUE-V0.94-WebInstaller.html').write_text(ins)
print('installer',len(ins),flush=True)
manager=(r/'BEETLE-BLUE-Manager.html').read_text().replace('V0.93','V0.94')
for key,f in [('emulator','simulator.html'),('projectmanager','BEETLE-BLUE-Project-Manager.html'),('installer','WebInstaller.html')]:
 manager=replace(manager,'source-'+key,base64.b64encode((r/f).read_bytes()).decode())
print('manager',len(manager),flush=True)
(r/'BEETLE-BLUE-Manager.html').write_text(manager)
assert base64.b64decode(extract(ins,'firmware-embedded'))==fw
for key,f in [('emulator','simulator.html'),('projectmanager','BEETLE-BLUE-Project-Manager.html'),('installer','WebInstaller.html')]:assert base64.b64decode(extract(manager,'source-'+key))==(r/f).read_bytes()
assert len(rom)==16
print('PASS webtools firmware sha',sha,flush=True)

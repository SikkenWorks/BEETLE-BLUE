from pathlib import Path
import re,json,struct,wave,io,base64,hashlib,zipfile, shutil
r=Path(__file__).resolve().parents[1]
fw=(r/'BEETLE-BLUE-V0.93-FM1-EXPERIMENTAL.fwsc').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
# Decode exact embedded ADPCM into browser-playable PCM16 WAV for a firmware-faithful source sample.
code=(r/'firmware/fm1-native/beetle_blue_monolith.c').read_text()
offs=list(map(int,re.findall(r'\d+',re.search(r'blue_monolith_offset\[17\]=\{(.+?)\};',code).group(1))))
bits=bytes(map(int,re.findall(r'\d+',re.search(r'blue_monolith_adpcm\[BLUE_MONOLITH_BYTES\]=\{(.+?)\};',code,re.S).group(1))))
assert len(offs)==17 and offs[-1]==len(bits)
names=['kick','snare_1','hihat_1','open_hihat','tom_1','tom_2','tom_3','crash','ride','clap','perc_1','perc_2','perc_3','rim','snare_2','hihat_2']
steps=[7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767]
idxs=[-1,-1,-1,-1,2,4,6,8]
obj={}
for i,name in enumerate(names):
 pred=0;idx=0;pcm=[]
 for b in bits[offs[i]:offs[i+1]]:
  for code in (b&15,b>>4):
   st=steps[idx];dif=st>>3
   if code&4:dif+=st
   if code&2:dif+=st>>1
   if code&1:dif+=st>>2
   pred=max(-32768,min(32767,pred+(-dif if code&8 else dif)))
   idx=max(0,min(88,idx+idxs[code&7]))
   pcm.append(pred)
 out=io.BytesIO()
 with wave.open(out,'wb') as w:
  w.setnchannels(1);w.setframerate(8000);w.setsampwidth(2)
  w.writeframes(struct.pack('<%dh'%len(pcm),*pcm))
 obj[name+'.wav']=base64.b64encode(out.getvalue()).decode()
# Simulator V0.92 already includes real WAV playback. Replace with audio-equivalent embedded samples.
sim=(r/'BEETLE-BLUE-V0.92-simulator.html').read_text()
sim=sim.replace('V0.92','V0.93').replace('V0.91','V0.93')
sim=re.sub(r'(<script id="monolith-original-wavs" type="application/json">)(.*?)(</script>)',lambda m:m[1]+json.dumps(obj,separators=(',',':'))+m[3],sim,flags=re.S)
sim=re.sub(r'const monolithPadFiles=\[.*?\];','const monolithPadFiles='+json.dumps([k+'.wav' for k in names],separators=(',',':'))+';',sim,count=1)
sim=sim.replace('MONOLITH WAV drumkit ready: ', 'MONOLITH BUILT-IN kit ready: ')
sim=sim.replace('16 WAV drumkit', '16 embedded drum samples')
sim=sim.replace('16 WAV samples', '16 built-in compressed samples')
sim=sim.replace('Original WAVs', 'Embedded ADPCM drum samples')
(r/'BEETLE-BLUE-V0.93-simulator.html').write_text(sim)
(r/'simulator.html').write_text(sim)
# Project manager V0.92 project transfer logic preserved; its MONOLITH upload instructions removed.
pm=(r/'BEETLE-BLUE-Project-Manager.html').read_text()
pm=pm.replace('V0.92','V0.93').replace('V0.91','V0.93')
pm=pm.replace('MONOLITH WAVs must be uploaded separately to U01–U13.','MONOLITH is now included in the V0.93 firmware. No sample-bank upload is needed.')
pm=re.sub(r'<div class="note"><strong>MONOLITH KIT:</strong>.*?</div>', '<div class="note"><strong>MONOLITH KIT:</strong> All 16 MONOLITH drum sounds are embedded in BEETLE BLUE V0.93. They do not occupy your U01–U13 user-sample slots. The global library below remains for your OWN samples.</div>',pm,count=1,flags=re.S)
pm=re.sub(r'<a class="button quiet" href="BEETLE-BLUE-MONOLITH-DRUM-KIT\.beetlebank"[^>]*>.*?</a>','',pm,count=1,flags=re.S)
(r/'BEETLE-BLUE-Project-Manager.html').write_text(pm)
# Installer real payload and digest; untouched loader and 16k protected head confirmed via packaging tool.
ins=(r/'BEETLE-BLUE-V0.92-WebInstaller.html').read_text()
ins=ins.replace('V0.92','V0.93').replace('V0.91','V0.93')
ins=re.sub(r'(<script id="firmware-embedded" type="application/octet-stream">)(.*?)(</script>)',lambda m:m[1]+base64.b64encode(fw).decode()+m[3],ins,count=1,flags=re.S)
ins=re.sub(r"(HASH_ET: ')[0-9a-f]{64}(')",lambda m:m[1]+sha+m[2],ins,count=1)
ins=ins.replace('MONOLITH WAV upload','MONOLITH built-in sample kit')
(r/'BEETLE-BLUE-V0.93-WebInstaller.html').write_text(ins)
(r/'WebInstaller.html').write_text(ins)
# Rebuild all-in-one manager with identical bundled standalones.
manager=(r/'BEETLE-BLUE-Manager.html').read_text().replace('V0.92','V0.93').replace('V0.91','V0.93')
for kind,filename in [('emulator','simulator.html'),('projectmanager','BEETLE-BLUE-Project-Manager.html'),('installer','WebInstaller.html')]:
 doc=(r/filename).read_bytes()
 marker=r'(<script id="source-'+kind+r'" type="application/octet-stream">)(.*?)(</script>)'
 manager,n=re.subn(marker,lambda m:m[1]+base64.b64encode(doc).decode()+m[3],manager,count=1,flags=re.S)
 assert n==1,(kind,n)
(r/'BEETLE-BLUE-Manager.html').write_text(manager)
# Validate all embedded contents and all scripts are coherent with release bytes.
embedded=re.search(r'<script id="firmware-embedded" type="application/octet-stream">([^<]+)</script>',ins)
assert base64.b64decode(embedded.group(1))==fw
assert sha in ins
for kind,fn in [('emulator','simulator.html'),('projectmanager','BEETLE-BLUE-Project-Manager.html'),('installer','WebInstaller.html')]:
 b=base64.b64decode(re.search(r'<script id="source-'+kind+'" type="application/octet-stream">([^<]+)</script>',manager).group(1))
 assert b==(r/fn).read_bytes(),kind
assert len(obj)==16
print('PASS webtools, 16 built-in browser decoded ADPCM PCM WAV, SHA pinned .fwsc, all-in-one byte-identical assets',sha)

from pathlib import Path
import shutil, zipfile, io, wave, numpy as np, hashlib, json
from scipy.signal import resample_poly
dest=Path(__file__).resolve().parents[1]
root=dest/'firmware/fm1-native'
sound_names=['kick','snare_1','hihat_1','open_hihat','tom_1','tom_2','tom_3','crash','ride','clap','perc_1','perc_2','perc_3','rim','snare_2','hihat_2']
lengths=[.95,.75,.40,.70,.95,.95,.95,1.50,.95,.55,.85,.65,.85,.65,.75,.30]
index_tab=[-1,-1,-1,-1,2,4,6,8]
steps=[7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552]
# full IMA 89-step table
steps += [1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767]
assert len(steps)==89
data=bytearray(); offsets=[0]; meta=[]
for name,dur in zip(sound_names,lengths):
 fn=name+'.wav'; b=(dest/'assets/MONOLITH-Kit'/fn).read_bytes()
 with wave.open(io.BytesIO(b)) as w:
  rate,channels,sw=w.getframerate(),w.getnchannels(),w.getsampwidth()
  pcm=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').reshape(-1,channels).astype(np.float64).mean(axis=1)
 assert rate==44100 and sw==2
 pcm=pcm[:int(dur*rate)]
 # Polyphase anti-aliased downsampling
 pcm=resample_poly(pcm,80,441).astype(np.float64) # exactly 8000 Hz
 fade=min(75,len(pcm)//8)
 pcm[-fade:]*=np.linspace(1,0,fade)
 pcm=np.clip(np.round(pcm),-32768,32767).astype(np.int16)
 pred=0;idx=0;nibbles=[]
 for sample in pcm:
  delta=int(sample)-pred;sign=8 if delta<0 else 0;mag=abs(delta);step=steps[idx];diff=0
  if mag>=step:diff|=4;mag-=step
  if mag>=step//2:diff|=2;mag-=step//2
  if mag>=step//4:diff|=1
  delta=step>>3
  if diff&4:delta+=step
  if diff&2:delta+=step>>1
  if diff&1:delta+=step>>2
  pred+= -delta if sign else delta;pred=max(-32768,min(32767,pred))
  idx=max(0,min(88,idx+index_tab[diff]));nibbles.append(sign|diff)
 blob=bytes(nibbles[j] | ((nibbles[j+1] if j+1<len(nibbles) else 0)<<4) for j in range(0,len(nibbles),2))
 data.extend(blob);offsets.append(len(data));meta.append({'source':fn,'original_sha256':hashlib.sha256(b).hexdigest(),'original_rate':rate,'original_duration_s':round(len(b)/(rate*channels*2),2),'embedded_rate':8000,'embedded_duration_s':round(len(pcm)/8000,3),'embedded_frames':len(pcm),'encoded_bytes':len(blob)})
print('Encoded total',len(data),'clip lengths', [m['embedded_duration_s'] for m in meta])
# C code data
content='/* Embedded MONOLITH WAV one-shots; IMA ADPCM4, 8000 Hz. GPL-3.0-only firmware component.\n * Generated from user-supplied MONOLITH Kit(1).zip; see manifest for provenance/trimming.\n */\n#include "beetle_blue_monolith.h"\nconst uint32_t blue_monolith_offset[17]={' + ','.join(str(x)+'u' for x in offsets) + '};\n'
content+='const uint8_t blue_monolith_adpcm[BLUE_MONOLITH_BYTES]={\n'
for j in range(0,len(data),40):content+=','.join(str(x) for x in data[j:j+40])+',\n'
content+='};\n'
(root/'beetle_blue_monolith.c').write_text(content)
(root/'beetle_blue_monolith.h').write_text('''/* Embedded, read-only MONOLITH sample kit. */
#ifndef BEETLE_BLUE_MONOLITH_H
#define BEETLE_BLUE_MONOLITH_H
#include <stdint.h>
#define BLUE_MONOLITH_COUNT 16u
#define BLUE_MONOLITH_RATE 8000u
#define BLUE_MONOLITH_BYTES %du
extern const uint8_t blue_monolith_adpcm[BLUE_MONOLITH_BYTES];
extern const uint32_t blue_monolith_offset[BLUE_MONOLITH_COUNT+1u];
#endif
''' % len(data))
(dest/'assets/MONOLITH-Kit/BEETLE-BLUE-V0.93-EMBED-MANIFEST.json').write_text(json.dumps({'source_archive':'MONOLITH Kit(1).zip','container_note':'All sixteen drum sounds included as lossy IMA-ADPCM4 at 8000Hz with lengths shortened to fit existing protected firmware application region. Original source WAVs retained in assets.', 'notes':'Not the unmodified original WAV quality; no sample transfer needed; MONOLITH sources do not use Storage V2.', 'samples':meta},indent=2)+'\n')
print('prepared',dest)

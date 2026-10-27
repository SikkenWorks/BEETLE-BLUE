from pathlib import Path
import re, subprocess, shutil,sys
from PIL import Image
r=Path(__file__).resolve().parents[1]
src=(r/'tools/blue_v092_capture_renderer.c').read_text()
fm=(r/'firmware/fm1-native/track_runtime.h').read_text()
needle='static void blue_drum_component_mixer_screen(void){'
func= fm[fm.index(needle):fm.index('static void beetle_snapshot_screen(void){')]
a=src.index(needle);b=src.index('static void native_credits(void){',a)
src=src[:a]+func+'\n'+src[b:]
(r/'tools/blue_v093_capture_renderer.c').write_text(src)
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=0','-I','firmware/fm1-native','tools/blue_v093_capture_renderer.c','-o','build/blue_v093_capture'], cwd=r,check=True)
raw=r/'build/v093-captures';raw.mkdir(parents=True,exist_ok=True)
subprocess.run([str(r/'build/blue_v093_capture'),str(raw)],check=True)
# Independently compile the exact tracker source-extracted from FM1 code.
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-I','firmware/fm1-native','tools/blue_v092_tracker_exact_capture.c','-o','build/blue_v093_tracker_capture'],cwd=r,check=True)
# previous capture needs args inspect
p=subprocess.run([str(r/'build/blue_v093_tracker_capture'),str(raw/'TRACKER-NATIVE-EXACT.ppm')],cwd=r,capture_output=True,text=True)
print('tracker status',p.returncode,p.stdout[:300],p.stderr[:300]);assert p.returncode==0
out=r/'UI-CAPTURES-NATIVE-1920';out.mkdir(parents=True,exist_ok=True)
for file in sorted(raw.glob('*.ppm')):
 im=Image.open(file).convert('RGB')
 assert im.size==(240,240)
 hi=im.resize((1920,1920),Image.Resampling.NEAREST)
 assert list(hi.getdata())[0]==im.getpixel((0,0))
 hi.save(out/(file.stem+'.png'))
print('Rendered',len(list(out.glob('*.png'))),'screens, 1920x1920 from extracted C')

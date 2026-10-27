#!/usr/bin/env python3
"""Native C-rendered reference PNGs; captures 240x240 LCD framebuffer -> 8x NEAREST.
No artistic redraw; test states (not physical device captures) are seeded in harness.
"""
from pathlib import Path
import subprocess
from PIL import Image
r=Path(__file__).resolve().parents[1]
src=(r/'tools/blue_v092_capture_renderer.c').read_text()
src=src.replace('DEFAULT / ROM','BEETLE').replace('MONOLITH / SAMPLES','MONOLITH / ROM')
src=src.replace('DEFAULT / BEETLE ROM','BEETLE').replace('MONOLITH SAMPLE KIT','MONOLITH ROM KIT')
# Replace the copied mixer draw routine with verbatim V0.94 firmware source.
rt=(r/'firmware/fm1-native/track_runtime.h').read_text()
needle='static void blue_drum_component_mixer_screen(void){'
func=rt[rt.index(needle):rt.index('static void beetle_snapshot_screen(void){')]
a=src.index(needle);b=src.index('static void native_credits(void){',a)
src=src[:a]+func+'\n'+src[b:]
(r/'tools/blue_v094_capture_renderer.c').write_text(src)
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=0','-I','firmware/fm1-native','tools/blue_v094_capture_renderer.c','-o','build/blue_v094_capture'],cwd=r,check=True)
raw=r/'build/v094-captures';raw.mkdir(parents=True,exist_ok=True)
subprocess.run([str(r/'build/blue_v094_capture'),str(raw)],cwd=r,check=True)
# Original firmware track_fm1_grid() source extracted earlier; unchanged in V0.94.
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-I','firmware/fm1-native','tools/blue_v092_tracker_exact_capture.c','-o','build/blue_v094_tracker_capture'],cwd=r,check=True)
subprocess.run([str(r/'build/blue_v094_tracker_capture'),str(raw/'TRACKER-NATIVE-EXACT.ppm')],cwd=r,check=True)
out=r/'UI-CAPTURES-NATIVE-1920';out.mkdir(parents=True,exist_ok=True)
for f in sorted(raw.glob('*.ppm')):
 im=Image.open(f).convert('RGB');assert im.size==(240,240)
 up=im.resize((1920,1920),Image.Resampling.NEAREST)
 assert up.getpixel((13,13))==im.getpixel((1,1))
 up.save(out/(f.stem+'.png'))
print('PASS:',len(list(out.glob('*.png'))),'source-derived LCD captures at 1920x1920 nearest-neighbour; host seeded, not hardware')

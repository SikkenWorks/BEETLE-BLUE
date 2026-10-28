#!/usr/bin/env python3
"""Generate genuine native-source 240px LCD framebuffer captures, 8x nearest-neighbor.
Host test states only; no physical FM1 display capture is claimed.
"""
from pathlib import Path
import subprocess
from PIL import Image
r=Path(__file__).resolve().parents[1]
build=r/'build';build.mkdir(exist_ok=True)
raw=build/'v0991-native-captures';raw.mkdir(parents=True,exist_ok=True)
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=0','-I','firmware/fm1-native','tools/blue_v097_capture_renderer.c','-o',str(build/'blue_v0991_lcd')],cwd=r,check=True)
subprocess.run([str(build/'blue_v0991_lcd'),str(raw)],check=True)
subprocess.run(['gcc','-O2','-std=c11','-DBEETLE_DRUM_EDITION=1','-I','firmware/fm1-native','tools/blue_v092_tracker_exact_capture.c','-o',str(build/'blue_v0991_tracker')],cwd=r,check=True)
subprocess.run([str(build/'blue_v0991_tracker'),str(raw/'TRACKER-NATIVE-EXACT.ppm')],cwd=r,check=True)
out=r/'UI-CAPTURES-NATIVE-1920';out.mkdir(exist_ok=True)
for p in sorted(raw.glob('*.ppm')):
 im=Image.open(p).convert('RGB');assert im.size==(240,240)
 up=im.resize((1920,1920),resample=Image.Resampling.NEAREST)
 up.save(out/(p.stem+'.png'))
 # verify the entire 8x8 tile mapping, not just a sample pixel
 for xx,yy in ((0,0),(17,11),(111,72),(239,239)):
  assert up.getpixel((xx*8+7,yy*8+7))==im.getpixel((xx,yy))
print('PASS native C LCD captures:',len(list(out.glob('*.png'))),'1920x1920 8x nearest neighbor')

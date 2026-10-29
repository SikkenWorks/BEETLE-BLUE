#!/usr/bin/env python3
"""Compile the actual RC12 on-device HELP draw branch and export native LCD captures.
No approximated/mock text positioning. Reference captures are NOT hardware photographs.
"""
from pathlib import Path
from PIL import Image
import subprocess
r=Path(__file__).resolve().parents[1]
src=(r/'firmware/fm1-native/track_runtime.h').read_text()
start=src.index(' if(track_page==19u){\n  /* RC12 HELP is paginated cards')
end=src.index(' if(track_page==47u){',start)
page=src[start:end]
base=(r/'tools/blue_v097_capture_renderer.c').read_text()
base=base[:base.index('static const char *const track_web_stepfx')]
extra='''#include "beetle_help.h"
static uint8_t track_help_page=0u, track_help_chapter=0u;
static void draw_help(void){
'''+page+'''\n}
int main(int argc,char **argv){
 if(argc!=4)return 2;
 track_page=19u;
 track_state.transport.bpm=120u;
 track_help_chapter=(uint8_t)atoi(argv[1]);
 track_help_page=(uint8_t)atoi(argv[2]);
 if(track_help_chapter>=BEETLE_HELP_TOPIC_COUNT||track_help_page>=beetle_help_page_count(track_help_chapter))return 3;
 draw_help();
 FILE *f=fopen(argv[3],"wb");if(!f)return 4;
 fprintf(f,"P6\\n240 240\\n255\\n");
 for(unsigned i=0;i<240u*240u;i++){
  uint16_t v=fb[i];unsigned char rgb[3]={(unsigned char)(((v>>11)&31)*255/31),(unsigned char)(((v>>5)&63)*255/63),(unsigned char)((v&31)*255/31)};
  fwrite(rgb,1,3,f);
 }
 fclose(f);return 0;
}
'''
b=r/'build';b.mkdir(exist_ok=True)
source=b/'blue_rc12_help_capture.c';source.write_text(base+extra)
exe=b/'blue_rc12_help_capture'
subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=0','-I','firmware/fm1-native',str(source),'-o',str(exe)],cwd=r,check=True,stdout=subprocess.DEVNULL)
out=r/'UI-CAPTURES-NATIVE-1920';out.mkdir(exist_ok=True)
for topic,page,name in [(0,0,'HELP-START'),(1,0,'HELP-LANE-LENGTHS'),(7,0,'HELP-LIVE-MODE'),(7,1,'HELP-LIVE-MODE-2'),(9,1,'HELP-SAFETY')]:
 ppm=b/(name+'.ppm');subprocess.run([str(exe),str(topic),str(page),str(ppm)],check=True)
 im=Image.open(ppm).convert('RGB');im.resize((1920,1920),Image.Resampling.NEAREST).save(out/(name+'.png'))
 print(name, (out/(name+'.png')).stat().st_size)
print('PASS: five RC12 HELP captures extracted from actual native LCD rendering branch')

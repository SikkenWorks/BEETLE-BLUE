#!/usr/bin/env python3
"""Compile/draw the exact native LCD page-10 branch, test overlap at pixel level.

The test extracts the block directly from firmware/fm1-native/track_runtime.h.
A C native-source framebuffer is emitted at 240x240 and a PNG at 1920x1920.
Requires only host gcc and Pillow; DOES NOT test a physical FM1 display.
"""
from pathlib import Path
import subprocess, re, tempfile, struct, sys
from PIL import Image
R=Path(__file__).resolve().parents[1]
source=(R/'firmware/fm1-native/track_runtime.h').read_text()
anchor='if(track_page==10u){\n  /* 5x7 font:'
assert anchor in source, 'Punch-In mixer code not found or refactored'
start=source.index(anchor)
opening=source.index('{',start)
level=0
end=None
for i in range(opening,len(source)):
 if source[i]=='{': level+=1
 if source[i]=='}':
  level-=1
  if level==0:
   end=i+1;break
assert end
branch=source[start:end]
# Regressions: real labels and actual loop positions in the extracted C code.
assert 'track_page_header("LANE 0 / REC FX",37u)' in branch
assert 'track_pix_text("STEP",12u,43u' in branch
assert 'track_pix_text("FX",69u,43u' in branch
assert 'track_pix_text("ON/OFF",154u,43u' in branch
assert 'uint16_t y=(uint16_t)(54u+i*9u)' in branch
assert 37 < 43 and 43+6 < 54 and 54+15*9+6 < 201
assert 'track_pix_text("OCT+ ADD OCT- DELETE",9u,213u' in branch
assert 'track_pix_text("SEL CLEAR STEP  RIGHT=CH1",9u,225u' in branch
# Native PUNCH page uses fixed 5x7 uppercase font; verify no accidental overlap
# with title/header and 16 rows. Check the second similar issue too.
assert '"RC AUDIO LOAD",36u);' in source
assert 'track_pix_text("READ ONLY / HARDWARE PROBE",10u,40u' in source
preamble=r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define BEETLE_DRUM_EDITION 1
#define TRACK_PINK 0x047Fu
#define TRACK_BRIGHT 0x7DFFu
#define TRACK_WHITE 0xffffu
#define TRACK_DARK 0x0000u
#define TRACK_ROW_BG 0x1008u
#include "track_core.h"
#include "track_song.h"
static uint16_t framebuffer[240*240];
static void lcd_fill(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t c){
 for(uint32_t j=y;j<(uint32_t)y+h&&j<240;j++)for(uint32_t i=x;i<(uint32_t)x+w&&i<240;i++)framebuffer[j*240u+i]=c;
}
#include "track_pixel_font.h"
static struct track_engine track_state;
static struct track_song_bank track_song;
static uint8_t track_page=10,track_battery_low20=0,track_punch_edit_row=0,track_punch_edit_index=0;
static void track_fmt_int(char *d,uint32_t v){char t[12];uint8_t n=0;do{t[n++]=(char)('0'+v%10);v/=10;}while(v&&n<11);uint8_t i=0;while(n)d[i++]=t[--n];d[i]=0;}
static void track_u8(char *d,uint32_t v){d[0]=(char)('0'+(v/10)%10);d[1]=(char)('0'+v%10);d[2]=0;}
static void track_page_header(const char *title,uint16_t line_y){
 track_pix_top(title,track_state.transport.edit_channel,track_state.transport.bpm);
 if(track_page==10u){lcd_fill(7u,3u,27u,9u,TRACK_DARK);track_pix_text("0/6",8u,4u,TRACK_PINK);}
 if(line_y)lcd_fill(22u,line_y,196u,1u,TRACK_PINK);
}
static void draw_native_punch(void){
'''
# Replace only the C 'return;' statement closing the extracted branch, which
# in native runtime leaves track_render page and in this wrapper is equivalent.
render_code=preamble+branch+'\n}\n'+r'''
int main(int argc,char **argv){
 track_state.transport.bpm=120;
 track_state.transport.edit_channel=0;
 track_state.pattern_slot=0;
 track_punch_edit_row=0;
 struct track_song_clip *p=&track_song.clips[0];
 p->count=16;
 for(uint8_t i=0;i<16;i++)p->data[i]=(struct track_song_event){.step=i,.fx=(uint8_t)(i%15),.down=1,.param=50};
 draw_native_punch();
 FILE *f=fopen(argv[1],"wb");if(!f)return 2;
 fwrite(framebuffer,sizeof(uint16_t),240u*240u,f);fclose(f);return 0;
}
'''
build=R/'build/v0992-lcd';build.mkdir(parents=True,exist_ok=True)
generated=build/'punch_lcd_from_native.c';generated.write_text(render_code)
exe=build/'punch_lcd_test'
subprocess.run(['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-parameter','-Wno-misleading-indentation','-Wno-unused-variable','-I','firmware/fm1-native',str(generated),'-o',str(exe)],cwd=R,check=True)
raw=build/'punch_lcd.rgb565'
subprocess.run([str(exe),str(raw)],check=True)
pixels=raw.read_bytes();assert len(pixels)==240*240*2
at=lambda x,y: struct.unpack_from('<H',pixels,(y*240+x)*2)[0]
PINK=0x047F
# Dividing rule is 1px at y=37; cannot cut columns (y 43..49).
assert all(at(x,37)==PINK for x in range(22,218))
assert all(at(x,42)!=PINK for x in range(22,218)), 'header rule still intersects label vicinity'
for x0,w in ((12,23),(69,11),(154,35)):
 assert any(at(x,y)==PINK for y in range(43,50) for x in range(x0,x0+w)), 'missing column heading'
# Check all 16 events actually visible, from row 54 to row 189; footer at 201.
for i in range(16):
 y=54+i*9
 assert any(at(x,yy)!=0 for x in range(68,102) for yy in range(y,y+7)), f'missing event {i}'
assert any(at(x,y) in (PINK,0xffff) for x in range(10,120) for y in range(201,208)), 'footer missing'
# Exact 8x nearest-neighbor mapping.
img=Image.new('RGB',(240,240));img.putdata([((p>>11)*255//31,((p>>5)&63)*255//63,(p&31)*255//31) for p in struct.unpack('<'+str(240*240)+'H',pixels)])
thumb=build/'PUNCH-IN-FX-REC-MIXER-240x240.png';img.save(thumb)
large=build/'PUNCH-IN-FX-REC-MIXER-1920x1920.png'
up=img.resize((1920,1920),Image.Resampling.NEAREST);up.save(large)
for xx,yy in ((15,43),(96,37),(23,54),(154,188),(120,203)):
 assert up.getpixel((xx*8+7,yy*8+7))==img.getpixel((xx,yy))
print('PASS: source-extracted Punch-In REC FX 240x240, labels/divider clear, all 16 event rows and footer present; 1920x1920 8x nearest-neighbor')
print('IMAGE',large)

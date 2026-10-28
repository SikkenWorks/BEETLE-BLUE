#!/usr/bin/env python3
"""Compile EXACT native Live Mode screen C block into 240x240 LCD pixels.
Host-rendered, not device proof; export 1920px using NEAREST scaling.
"""
from pathlib import Path
import subprocess
from PIL import Image
r=Path(__file__).resolve().parents[1]
src=(r/'firmware/fm1-native/track_runtime.h').read_text()
start=src.index(' if(track_page==43u){\n  track_page_header("LIVE MODE",43u);')
end=src.index(' if(track_page==29u){\n  track_page_header("PROJECTS",43u);',start)
pages=src[src.index(' if(track_page==47u){'):start]+src[start:end]
bootstart=end
bootend=src.index(' if(track_page==28u){',bootstart)
pages+=src[bootstart:bootend]
homestart=src.index(' if(track_page==1u){\n  track_page_header("PROJECTS",43u);')
homeend=src.index(' if(track_page==7u){',homestart)
pages+=src[homestart:homeend]
base=(r/'tools/blue_v097_capture_renderer.c').read_text()
base=base[:base.index('static const char *const track_web_stepfx')]
battery=src[src.index('static void blue_battery_hud(uint16_t x,uint16_t y){'):src.index('static void track_page_header(',src.index('static void blue_battery_hud('))]
extra=r'''#include "beetle_blue_live_setlist.h"
#include "beetle_blue_live_policy.h"
#include "beetle_battery_policy.h"
static uint8_t track_battery_valid=1u,track_battery_usb_link=1u;
static uint16_t track_battery_adc_filtered=659u;
static uint8_t native_safe=0u,native_tripped=0u;
'''+battery+r''' 
static struct blue_live_setlist blue_live;
static uint8_t track_load_status[TRACK_PROJECTS];
static char track_load_titles[TRACK_PROJECTS][13];
static char track_title[13];
static uint8_t blue_live_save_status,blue_live_dirty,track_boot_menu_choice,track_load_scan;
static uint8_t blue_live_progress_percent=67u;
static uint8_t track_menu_index,track_new_slot_pending;
static void blue_live_screen(void){
'''+pages+'''\n}\nint main(int argc,char **argv){
 if(argc<3)return 2;
 blue_live_init(&blue_live);
 blue_live.slots[0]=0;blue_live.slots[1]=4;blue_live.slots[2]=11;
 blue_live.count=3;blue_live.cursor=1;blue_live.position=0;
 blue_live.active=1;blue_live.ready=1;
 for(unsigned i=0;i<TRACK_PROJECTS;i++)track_load_status[i]=2;
 track_load_status[0]=track_load_status[4]=track_load_status[11]=1;
 strcpy(track_load_titles[0],"OPENING ACT");strcpy(track_load_titles[4],"INTERLUDE");strcpy(track_load_titles[11],"FINALE");
 strcpy(track_title,"OPENING ACT");track_state.transport.bpm=138;
 const int page=atoi(argv[1]);track_page=(uint8_t)page;
 if(page==46){blue_live.paused=1;blue_live.stop_choice=0;}
 if(argc>=4 && !strcmp(argv[3],"empty")){blue_live.count=0;blue_live.cursor=0;track_load_scan=TRACK_PROJECTS;}
 if(page==29)track_boot_menu_choice=2;
 if(page==1)track_menu_index=7;
 if(page==45)track_state.transport.playing=0;
 blue_live_screen();
 FILE *f=fopen(argv[2],"wb");if(!f)return 3;
 fprintf(f,"P6\\n240 240\\n255\\n");
 for(unsigned i=0;i<240u*240u;i++){
  uint16_t v=fb[i];unsigned char rgb[3]={(unsigned char)(((v>>11)&31)*255/31),
    (unsigned char)(((v>>5)&63)*255/63),(unsigned char)((v&31)*255/31)};
  fwrite(rgb,1,3,f);
 }
 fclose(f);return 0;
}
'''
out=r/'build/blue_live_capture_v0998.c';out.write_text(base+extra)
exe=r/'build/blue_live_capture_v0998'
subprocess.run(['gcc','-std=c11','-O2','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=0','-I','firmware/fm1-native',str(out),'-o',str(exe)],cwd=r,check=True)
folder=r/'UI-CAPTURES-NATIVE-1920';folder.mkdir(exist_ok=True)
for page,name in [(1,'HOME-LIVE-MODE-ENTRY'),(29,'BOOT-LIVE-MODE-ENTRY'),(43,'LIVE-MODE-MENU'),(44,'LIVE-SETLIST'),(45,'LIVE-PERFORM-READY'),(46,'LIVE-STOP-OPTIONS'),(47,'LIVE-PROJECT-LOADING')]:
 ppm=r/'build'/f'live-{page}.ppm'
 subprocess.run([str(exe),str(page),str(ppm)],check=True)
 im=Image.open(ppm).convert('RGB');im.resize((1920,1920),Image.Resampling.NEAREST).save(folder/f'{name}.png')
ppm=r/'build/live-empty-44.ppm'
subprocess.run([str(exe),'44',str(ppm),'empty'],check=True)
Image.open(ppm).convert('RGB').resize((1920,1920),Image.Resampling.NEAREST).save(folder/'LIVE-SETLIST-NO-PROJECTS.png')
print('PASS seven native-source Live Mode C screen captures including no saved projects, 1920px nearest neighbor')

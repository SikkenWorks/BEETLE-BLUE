/* Native renderer capture harness: source-extracted C functions, not a redrawn mockup.
 * Provides FM1 LCD pixel RAM and artificial UI *state*, not real FM1 hardware.
 * GPL-3.0-only; build: gcc -O2 -std=c11 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=0 -I firmware/fm1-native tools/blue_v09_capture_renderer.c -o capture
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "track_core.h"
#include "track_song.h"
#include "track_drum.h"
#include "beetle_blue_kits.h"
#include "beetle_drum6_kits.h"
#include "beetle_boot_frames.h"
#include "beetle_play_icon.h"
#define TRACK_PINK 0x047Fu
#define TRACK_BRIGHT 0x7DFFu
#define TRACK_WHITE 0xffffu
#define TRACK_DARK 0x0000u
#define TRACK_ROW_BG 0x1008u
static uint16_t fb[240*240];
static void lcd_fill(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
 for(uint32_t j=y;j<(uint32_t)y+h && j<240u;j++)for(uint32_t i=x;i<(uint32_t)x+w&&i<240u;i++)fb[j*240u+i]=color;
}
#include "track_pixel_font.h"
static struct track_engine track_state;
static struct track_song_bank track_song;
static uint8_t track_fx_slot=0u,track_fx_drum_lane=0u;

static struct track_drum_engine track_drum;
static uint8_t track_page=16u,track_battery_low20=0u,track_drum_component=0u,track_drum_field=1u,track_drum_kit_index=0u;
static uint8_t blue_trim_start[BEETLE_ONESHOT_NOTES],blue_trim_end[BEETLE_ONESHOT_NOTES],blue_adsr[BEETLE_ONESHOT_NOTES][4];
static struct blue_kit_bank blue_global_kits;
static uint8_t blue_drum_mix_kit=0u,blue_drum_mix_save_status=0u;
static uint8_t blue_drum_mix_component=0u,blue_drum_mix_field=1u,blue_kit_edit_slot=2u,blue_kit_edit_notice=0u,blue_kit_palette[8]={0,1,2,3,4,5,6,7};


const char *const bd6_kit_names[BD6_KIT_COUNT]={"BLUEPRINT","808 INSPIRED","909 INSPIRED","LINN INSPIRED","DIGITAL","INDUSTRIAL","LO-FI","ACID ELECTRO","DUB","BREAKBEAT","DUST","MINIMAL"};
static const char *const names[TRACK_DRUM_COUNT]={"KIK","SNR","HHC","HHO","TM1","TM2","TM3","CRS","RDE","CLP","PC1","PC2","PC3","RIM"};

const char *track_drum_name(uint8_t id){
 static const char *const user_names[BEETLE_ONESHOT_NOTES]={"U01","U02","U03","U04","U05","U06","U07","U08","U09","U10","U11","U12","U13"};
 if(id==TRACK_NOTE_STOP)return "STP";
 if(id<TRACK_DRUM_COUNT)return names[id];
 if(id>=TRACK_DRUM_NOTE_COUNT)return "---";
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 const char *label=beetle_bd4_live_label((uint8_t)(id-BEETLE_ONESHOT_FIRST));
 if(label)return label;
#endif
 return user_names[id-BEETLE_ONESHOT_FIRST];
}
static void track_fmt_int(char *dst,uint32_t value){
 char tmp[12];uint8_t n=0u;
 do{tmp[n++]=(char)('0'+value%10u);value/=10u;}while(value&&n<11u);
 uint8_t i=0u;while(n)dst[i++]=tmp[--n];dst[i]=0;
}
static void track_page_header(const char *title,uint16_t line_y){
 track_pix_top(title,track_state.transport.edit_channel,track_state.transport.bpm);
 if(track_battery_low20){lcd_fill(101u,3u,39u,9u,TRACK_DARK);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 track_pix_center("BLUE",120u,4u,0x04FFu);
#else
 track_pix_center("BEETLE",120u,4u,0xF800u);
#endif
 }
 if(track_page==10u){lcd_fill(7u,3u,27u,9u,TRACK_DARK);track_pix_text("0/6",8u,4u,TRACK_PINK);}
 if(line_y)lcd_fill(22u,line_y,196u,1u,TRACK_PINK);
}
static void track_web_list(const char *const *items,uint8_t count,uint8_t selected,uint16_t first,uint8_t pitch){
 for(uint8_t i=0;i<count;i++){
  uint16_t y=(uint16_t)(first+(uint16_t)i*pitch);
  if(i==selected)lcd_fill(24u,y-2u,194u,pitch,0x1814u);
  track_pix_text(i==selected?"> ":"  ",29u,y,i==selected?TRACK_BRIGHT:TRACK_WHITE);
  track_pix_text(items[i],41u,y,i==selected?TRACK_BRIGHT:TRACK_WHITE);
 }
}
static const char *const track_web_stepfx[TRACK_FX_COUNT]={
 "---","FILTER","DRIVE","DELAY","REVERB","CHORUS","VIB","ARP","CUT","TREMOLO","VOLUME","PITCH","REV"};
static void track_u8(char *dst,uint32_t v){dst[0]=(char)('0'+(v/10u)%10u);dst[1]=(char)('0'+v%10u);dst[2]=0;}
static void track_note_label(uint8_t note,char buf[5]){
 if(note==TRACK_NOTE_EMPTY){buf[0]='-';buf[1]='-';buf[2]='-';buf[3]=0;return;}
 if(note==TRACK_NOTE_STOP){buf[0]='S';buf[1]='T';buf[2]='P';buf[3]=0;return;}
 buf[0]='C';buf[1]='-';buf[2]=(char)('0'+note/12u);buf[3]=0;
}
static void native_fx_editor_screen(void){
  track_page_header("PER STEP FX",42u);
  const struct track_step *st=&track_state.active.ch[track_state.transport.edit_channel][track_state.transport.edit_step];
  char line[32]="STEP ";char n[4];track_u8(n,track_state.transport.edit_step);
  line[5]=n[0];line[6]=n[1];line[7]=' ';line[8]=' ';line[9]='N';line[10]='O';line[11]='T';line[12]='E';
  if(track_state.transport.edit_channel==TRACK_DRUM_CHANNEL){line[13]=(char)('1'+track_fx_drum_lane);line[14]=0;}
  else line[13]=0;
  track_pix_text(line,16u,52u,TRACK_PINK);
  if(track_state.transport.edit_channel==TRACK_DRUM_CHANNEL){
   uint8_t hit=track_fx_drum_lane?track_note2_id(st):st->note;
   track_pix_text(track_drum_name(hit),174u,52u,TRACK_BRIGHT);
  }else{char v[5];track_note_label(st->note,v);track_pix_text(v,174u,52u,TRACK_BRIGHT);}
  const uint8_t use2=(uint8_t)(track_state.transport.edit_channel==TRACK_DRUM_CHANNEL&&track_fx_drum_lane);
  const uint8_t key=track_extra_key(track_state.transport.edit_channel,track_state.transport.edit_step,use2);
  char nv[12],item[3][38];const char *items[3];
  const uint8_t primary=use2?track_state.active.note2_fx[track_state.transport.edit_step].effect:st->effect;
  const uint8_t pv=use2?track_state.active.note2_fx[track_state.transport.edit_step].value:st->value;
  for(uint8_t j=0u;j<3u;j++){
   const struct track_extra_fx *ext=j?track_extra_find(&track_song,track_state.pattern_slot,key,(uint8_t)(j-1u)):0;
   const uint8_t effect=j?track_extra_effect(ext):primary;
   const uint8_t value=j?(ext?ext->value:50u):pv;
   const char *nm=track_web_stepfx[effect<TRACK_FX_COUNT?effect:0u];
   uint8_t at=0u;item[j][at++]='F';item[j][at++]='X';item[j][at++]=(char)('1'+j);item[j][at++]=' ';
   for(uint8_t k=0u;nm[k]&&at<24u;k++)item[j][at++]=nm[k];
   item[j][at++]=' ';
   if(effect==TRACK_FX_REVERSE){nv[0]='O';nv[1]='N';nv[2]=0;}
   else track_fmt_int(nv,value);
   for(uint8_t k=0u;nv[k]&&at<35u;k++)item[j][at++]=nv[k];item[j][at]=0;
   items[j]=item[j];
  }
  track_web_list(items,3u,track_fx_slot,80u,26u);
  track_pix_text("K1 ROW   K2 FX TYPE",15u,177u,TRACK_PINK);
  track_pix_text("K3 VALUE OCT- REMOVE",15u,189u,TRACK_PINK);
  track_pix_text("OCT+ NEXT FX SLOT",15u,201u,TRACK_WHITE);
  char cap[30]="FX STORAGE ";track_fmt_int(nv,track_song.extras.count);
  uint8_t at=11u;for(uint8_t k=0u;nv[k]&&at<19u;k++)cap[at++]=nv[k];cap[at++]='/';cap[at++]='4';cap[at++]='8';cap[at]=0;
  track_pix_text(cap,15u,214u,TRACK_PINK);
}

static void track_fm1_drum_editor_screen(void){
 track_page_header("DRUM ENGINE / EDIT",43u);
 track_pix_text("COMPONENT",12u,54u,TRACK_PINK);
 track_pix_text(track_drum_name(track_drum_component),126u,54u,TRACK_WHITE);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 /* Caption, kit name and fields occupy separate, non-overlapping bands.
  * 5x7 font, 6 px advance, 12 px row pitch, 240x240 native LCD. */
 track_pix_text(bd6_kit_names[track_drum_kit_index],12u,65u,TRACK_BRIGHT);
 if(track_drum_component>=TRACK_DRUM_COUNT){
  const struct track_drum_timbre *u=&track_drum.user[track_drum_component-TRACK_DRUM_COUNT];
  static const char *const fields[]={"GAIN","DRIVE","CHOKE","START","END","ATTACK","DECAY","SUSTAIN","RELEASE"};
  uint8_t component=(uint8_t)(track_drum_component-TRACK_DRUM_COUNT);
  const uint8_t vals[]={u->gain,u->drive,u->choke_group,blue_trim_start[component],blue_trim_end[component],
   blue_adsr[component][0],blue_adsr[component][1],blue_adsr[component][2],blue_adsr[component][3]};
  for(uint8_t i=0u;i<9u;i++){
   uint16_t y=(uint16_t)(83u+i*13u);
   if(track_drum_field==i)lcd_fill(8u,y-2u,222u,12u,0x0814u);
   track_pix_text(fields[i],15u,y,track_drum_field==i?TRACK_PINK:TRACK_BRIGHT);
   char n[8];track_fmt_int(n,vals[i]);track_pix_text(n,184u,y,TRACK_WHITE);
  }
  track_pix_text("KEY: PREVIEW SAMPLE",7u,209u,TRACK_PINK);
  track_pix_text("SEL PAD K1 ROW K3 SET",7u,224u,TRACK_BRIGHT);
  return;
 }
#endif
 static const char *const f[]={"ATTACK","DECAY","SUSTAIN","RELEASE","PITCH","EQ LOW","EQ MID","EQ HIGH","GAIN","DRIVE","CHOKE"};
 const struct track_drum_timbre *p=&track_drum.components[track_drum_component];
 int values[11]={p->attack,p->decay,p->sustain,p->release,p->pitch,p->eq_low,p->eq_mid,p->eq_high,p->gain,p->drive,p->choke_group};
 for(uint8_t i=0u;i<11u;i++){
  uint16_t y=(uint16_t)(82u+(uint16_t)i*11u);
  if(track_drum_field==i)lcd_fill(8u,y-2u,222u,11u,
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 0x0814u
#else
 0x1814u
#endif
 );
  track_pix_text(f[i],15u,y,track_drum_field==i?TRACK_PINK:TRACK_BRIGHT);
  char num[12];int v=values[i];uint8_t prefix=0u;if(v<0){num[0]='-';v=-v;prefix=1u;}track_fmt_int(num+prefix,(uint32_t)v);
  track_pix_text(num,182u,y,TRACK_WHITE);
 }
 track_pix_text("KEY: PREVIEW SELECTED DRUM",7u,209u,TRACK_PINK);
 #if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 track_pix_text("SEL PAD K4 KIT K1 ROW K3 SET",4u,224u,TRACK_BRIGHT);
#else
 track_pix_text("SELECT DRUM K1 FIELD K3 SET",4u,224u,TRACK_BRIGHT);
#endif
}
static void blue_drum_component_mixer_screen(void){
 track_page_header("DRUM MIXER",43u);
 /* SELECT chooses a kit source, K1 the component inside that kit. Global
  * USER KIT values come directly from BBK1, never from a copy of the lane mix. */
 track_pix_text("KIT",6u,50u,TRACK_PINK);
 if(blue_drum_mix_kit==0u)track_pix_text("BEETLE",45u,50u,TRACK_BRIGHT);
 else if(blue_drum_mix_kit==1u)track_pix_text("MONOLITH / ROM",45u,50u,TRACK_BRIGHT);
 else{
  char kit[16]="USER KIT 01";
  uint8_t n=(uint8_t)(blue_drum_mix_kit-1u);
  kit[9]=(char)('0'+n/10u);kit[10]=(char)('0'+n%10u);
  track_pix_text(kit,45u,50u,TRACK_BRIGHT);
 }
 track_pix_text("PAD",5u,66u,TRACK_PINK);
 track_pix_text("GAIN",106u,66u,TRACK_PINK);track_pix_text("PIT",145u,66u,TRACK_PINK);
 track_pix_text("DRV",180u,66u,TRACK_PINK);track_pix_text("CH",214u,66u,TRACK_PINK);
 uint8_t limit=blue_drum_mix_kit==0u?TRACK_DRUM_NOTE_COUNT:blue_drum_mix_kit==1u?16u:TRACK_DRUM_COUNT;
 uint8_t first=blue_drum_mix_component>2u?(uint8_t)(blue_drum_mix_component-2u):0u;
 if(first>limit-7u)first=(uint8_t)(limit-7u);
 for(uint8_t j=0u;j<7u;j++){
  uint8_t id=(uint8_t)(first+j);uint16_t y=(uint16_t)(83u+j*17u);
  uint8_t base=(uint8_t)(blue_drum_mix_kit==1u&&id>=14u?(id==14u?1u:2u):id);
  const struct track_drum_timbre *v=base<TRACK_DRUM_COUNT?&track_drum.components[base]:&track_drum.user[base-TRACK_DRUM_COUNT];
  int32_t pitch=v->pitch;uint8_t gain=v->gain,drive=v->drive,choke=v->choke_group;
  if(blue_drum_mix_kit>=2u&&id<TRACK_DRUM_COUNT){
   const struct blue_kit_tone *tone=&blue_global_kits.user[blue_drum_mix_kit-2u].drums[id];
   if(blue_global_kits.valid_mask&(1u<<(blue_drum_mix_kit-2u))){
    pitch=tone->pitch;gain=tone->gain;drive=tone->drive;choke=tone->choke;
   }
  }
  if(id==blue_drum_mix_component)lcd_fill(3u,y-3u,234u,15u,0x0814u);
  char name[21];const char *nm=(blue_drum_mix_kit==1u&&id==14u)?"SNR2":(blue_drum_mix_kit==1u&&id==15u)?"HHT2":track_drum_name(id);uint8_t k=0u;
  while(nm[k]&&k<14u){name[k]=nm[k];k++;}name[k]=0;
  track_pix_text(name,4u,y,TRACK_WHITE);
  char value[12];track_fmt_int(value,gain);
  track_pix_text(value,107u,y,blue_drum_mix_field==0u&&id==blue_drum_mix_component?TRACK_PINK:TRACK_WHITE);
  if(id<TRACK_DRUM_COUNT||blue_drum_mix_kit==1u){
   value[0]=pitch<0?'-':'+';track_fmt_int(value+1u,(uint32_t)(pitch<0?-pitch:pitch));
   track_pix_text(value,146u,y,blue_drum_mix_field==1u&&id==blue_drum_mix_component?TRACK_PINK:TRACK_WHITE);
  }else track_pix_text("--",146u,y,TRACK_WHITE);
  track_fmt_int(value,drive);track_pix_text(value,183u,y,blue_drum_mix_field==2u&&id==blue_drum_mix_component?TRACK_PINK:TRACK_WHITE);
  track_fmt_int(value,choke);track_pix_text(value,220u,y,blue_drum_mix_field==3u&&id==blue_drum_mix_component?TRACK_PINK:TRACK_WHITE);
 }
 track_pix_text("SELECT KIT / K1 PAD / K2 COL",4u,206u,TRACK_PINK);
 if(blue_drum_mix_kit==1u){
  track_pix_text("16 SAMPLES IN FIRMWARE",4u,221u,TRACK_BRIGHT);
 }else if(blue_drum_mix_kit>=2u){
  track_pix_text(blue_drum_mix_save_status==1u?"GLOBAL KIT SAVED":
     blue_drum_mix_save_status==2u?"SAVE FAILED / CHECK STORAGE":
     "K3 SET / SEL SAVE GLOBAL",4u,221u,
     blue_drum_mix_save_status==2u?TRACK_WHITE:TRACK_BRIGHT);
 }else track_pix_text("K3 SET / GLO: LANE MIX",4u,221u,TRACK_BRIGHT);
}
/* Mixer-adjacent performance screen; full 240x240 bounded layout. */

static void native_credits(void){
if(track_page==28u){
  track_page_header("CREDITS",43u);
  static const char *const lines[]={
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   "BEETLE BLUE / SIKKENWORKS",
   "JOPPE SIKKEN",
   "BEETLE + ETCH DERIVATIVE",
   "",
   "FELUCCA / LEO KUROSHITA",
   "HUGELTON INSTRUMENTS",
   "FM1 HAL / AUDIO / OTA",
   "BATTERY BANDS: FELUCCA",
   "",
   "JIELI / AC79 PLATFORM",
   "GPL-3.0-ONLY",
   "FULL CREDITS IN SOURCE"
#else
   "BEETLE / SIKKENWORKS",
   "JOPPE SIKKEN",
   "DESIGN / FIRMWARE / UI",
   "",
   "FELUCCA / LEO KUROSHITA",
   "HUGELTON INSTRUMENTS",
   "FM1 HAL / AUDIO / OTA",
   "",
   "RESID / DAG LEM (SID REF)",
   "NES_SND_EMU / SHAY GREEN",
   "JAMES ATHEY (MAINTAINER)",
   "GPL V3 / SEE SOURCE"
#endif
  };
  /* Keep all credit lines visible above the footer on the 240px display. */
  for(uint8_t i=0u;i<12u;i++){
   uint16_t y=(uint16_t)(53u+i*12u);
   uint16_t color=(i==0u||i==4u||i==8u||i==9u)?TRACK_PINK:TRACK_WHITE;
   track_pix_text(lines[i],8u,y,color);
  }
  track_pix_text("HOME MENU  SEQ TRACKER",9u,217u,TRACK_BRIGHT);
  return;
 }
}
static void native_kit_designer(void){
if(track_page==42u){
  track_page_header("KIT DESIGNER",43u);
  track_pix_text("8 ACTIVE KIT REFERENCES",8u,48u,TRACK_PINK);
  for(uint8_t i=0;i<8u;i++){
   uint16_t y=(uint16_t)(70u+i*17u);
   if(blue_kit_edit_slot==i)lcd_fill(5u,y-2u,230u,16u,0x0814u);
   char num[12]="INST ";track_fmt_int(num+5u,i+1u);
   track_pix_text(num,12u,y,blue_kit_edit_slot==i?TRACK_PINK:TRACK_BRIGHT);
   uint8_t src=blue_kit_palette[i];
   if(src==0u)track_pix_text("BEETLE",105u,y,TRACK_WHITE);
   else if(src==1u)track_pix_text("MONOLITH",105u,y,TRACK_WHITE);
   else {char n[12]="USER KIT ";track_fmt_int(n+9u,src-1u);track_pix_text(n,105u,y,TRACK_WHITE);}
  }
  track_pix_text("K1 INST  K3 KIT  SEL STORE",6u,213u,TRACK_PINK);
  track_pix_text(blue_kit_edit_notice==1u?"GLOBAL KIT SAVED":
   blue_kit_edit_notice==3u?"GLOBAL SAVE FAILED":
   "EDIT DRUM EDIT / SEQ TRACK",6u,227u,blue_kit_edit_notice==3u?TRACK_WHITE:TRACK_BRIGHT);
  return;
 }
}

static void native_boot_frame(uint8_t frame){
 lcd_fill(0u,0u,240u,240u,0x0000u);
 for(uint32_t i=0;i<BEETLE_BOOT_TREE_RUNS;i++){
  const struct beetle_boot_base_run *r=&beetle_boot_tree[i];
  lcd_fill(r->x,r->y,r->w,1u,beetle_boot_colors[r->color]);
 }
 const struct beetle_boot_pink_run *bug=(frame&1u)?beetle_boot_bug1:beetle_boot_bug0;
 uint32_t count=(frame&1u)?BEETLE_BOOT_BUG1_RUNS:BEETLE_BOOT_BUG0_RUNS;
 for(uint32_t i=0u;i<count;i++){
  const struct beetle_boot_pink_run *r=&bug[i];
  lcd_fill(r->x,r->y,r->w,1u,0x04FFu);
 }
}
#include "native_ota_matrix.h"
static void native_ota_frame(unsigned frame){
 uint16_t stripe[240u*40u];
 for(uint16_t y=0u;y<240u;y+=40u){
  etch_matrix_band(stripe,y,40u,frame,7,-0);
  for(uint32_t i=0u;i<240u*40u;i++){
   uint16_t v=stripe[i];fb[y*240u+i]=(uint16_t)((v<<8)|(v>>8));
  }
 }
}
static void write_ppm(const char *p){
 FILE *f=fopen(p,"wb");if(!f){perror(p);exit(1);}
 fprintf(f,"P6\n240 240\n255\n");
 for(int i=0;i<240*240;i++){
  uint16_t v=fb[i];
  uint8_t rgb[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};
  fwrite(rgb,1,3,f);
 }
 fclose(f);
}
static uint32_t etch_ms=80u;
static uint8_t blue_lane_overlay=1u,blue_lane_overlay_lane=0u,blue_lane_overlay_source=0u;
static uint32_t blue_lane_overlay_until=2000u;
static void blue_lane_kit_overlay_draw(void){
 if(track_page!=0u || !blue_lane_overlay || (int32_t)(blue_lane_overlay_until-etch_ms)<=0)return;
 lcd_fill(16u,79u,208u,84u,TRACK_DARK);
 lcd_fill(16u,79u,208u,2u,TRACK_PINK);
 lcd_fill(16u,161u,208u,2u,TRACK_PINK);
 char title[16]="LANE 1 KIT";title[5]=(char)('1'+blue_lane_overlay_lane);
 track_pix_center(title,120u,90u,TRACK_PINK);
 uint8_t src=blue_lane_overlay_source;
 if(src==0u)track_pix_center("BEETLE",120u,115u,TRACK_BRIGHT);
 else if(src==1u)track_pix_center("MONOLITH ROM KIT",120u,115u,TRACK_BRIGHT);
 else {char msg[18]="USER KIT 01";msg[9]=(char)('0'+(src-1u)/10u);msg[10]=(char)('0'+(src-1u)%10u);track_pix_center(msg,120u,115u,TRACK_BRIGHT);}
 track_pix_center("SEQ + SELECT",120u,144u,TRACK_PINK);
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 char path[1024];track_state.transport.bpm=120u;
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++){
  track_drum.components[i].gain=96u-i;
  track_drum.components[i].pitch=i<3?0:i-4;
  track_drum.components[i].drive=i*5;
  track_drum.components[i].choke_group=i%4;
 }
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++){
  track_drum.user[i].gain=85u;track_drum.user[i].drive=20u;
  blue_trim_start[i]=11u;blue_trim_end[i]=91u;
  blue_adsr[i][0]=5u;blue_adsr[i][1]=40u;blue_adsr[i][2]=70u;blue_adsr[i][3]=22u;
 }
#define CAP(name, expr) do{memset(fb,0,sizeof fb);expr;snprintf(path,sizeof path,"%s/%s.ppm",argv[1],name);write_ppm(path);}while(0)
 track_page=16u;CAP("DRUM-EDITOR-ROM",track_fm1_drum_editor_screen());
 track_drum_component=14u;track_drum_field=3u;CAP("DRUM-EDITOR-USER",track_fm1_drum_editor_screen());
 track_page=41u;blue_drum_mix_kit=0u;blue_drum_mix_component=2u;
 CAP("DRUM-MIXER-DEFAULT",blue_drum_component_mixer_screen());
 blue_drum_mix_kit=1u;CAP("DRUM-MIXER-MONOLITH",blue_drum_component_mixer_screen());
 memset(&blue_global_kits,0,sizeof blue_global_kits);
 blue_global_kits.valid_mask=1u;
 for(uint8_t n=0u;n<TRACK_DRUM_COUNT;n++){
  blue_global_kits.user[0].drums[n].gain=track_drum.components[n].gain;
  blue_global_kits.user[0].drums[n].pitch=track_drum.components[n].pitch;
  blue_global_kits.user[0].drums[n].drive=track_drum.components[n].drive;
  blue_global_kits.user[0].drums[n].choke=track_drum.components[n].choke_group;
 }
 blue_global_kits.user[0].drums[2].gain=65u;
 blue_drum_mix_kit=2u;CAP("DRUM-MIXER-USER-KIT-01",blue_drum_component_mixer_screen());
 blue_drum_mix_kit=13u;blue_drum_mix_component=12u;CAP("DRUM-MIXER-USER-KIT-12",blue_drum_component_mixer_screen());
 track_page=42u;CAP("KIT-DESIGNER",native_kit_designer());
 track_page=3u;track_fx_slot=1u;
 track_state.transport.edit_channel=0u;track_state.transport.edit_step=3u;
 track_state.active.length=16u;track_state.active.ch[0][3].note=0u;
 track_state.active.ch[0][3].effect=2u;track_state.active.ch[0][3].value=70u;
 track_song.extras.count=0u;
 CAP("STEP-FX-EDITOR",native_fx_editor_screen());
 track_page=28u;CAP("CREDITS",native_credits());
 CAP("BOOT-FRAME-0",native_boot_frame(0u));
 CAP("BOOT-FRAME-1",native_boot_frame(1u));
 CAP("OTA-MATRIX-0",native_ota_frame(0u));
 CAP("OTA-MATRIX-1",native_ota_frame(1u));
 track_page=0u;blue_lane_overlay_lane=0u;blue_lane_overlay_source=0u;
 CAP("LANE-1-DEFAULT-KIT-OVERLAY",blue_lane_kit_overlay_draw());
 blue_lane_overlay_lane=3u;blue_lane_overlay_source=1u;
 CAP("LANE-4-MONOLITH-KIT-OVERLAY",blue_lane_kit_overlay_draw());
 blue_lane_overlay_lane=5u;blue_lane_overlay_source=13u;
 CAP("LANE-6-USER-KIT-12-OVERLAY",blue_lane_kit_overlay_draw());

 return 0;
}

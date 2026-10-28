#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define BEETLE_DRUM_EDITION 1
#define BEETLE_NATIVE_V2 0
#define TRACK_PINK 0x047Fu
#define TRACK_BRIGHT 0x7DFFu
#define TRACK_WHITE 0xFFFFu
#define TRACK_DARK 0x0000u
#define TRACK_ROW_BG 0x1008u
#include "track_core.h"
#include "track_song.h"
#include "track_drum.h"
#include "track_navigation.h"
#include "beetle_play_icon.h"
static uint16_t fb[240*240];
static void lcd_fill(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
 for(uint32_t yy=y;yy<(uint32_t)y+h&&yy<240;yy++)for(uint32_t xx=x;xx<(uint32_t)x+w&&xx<240;xx++)fb[yy*240+xx]=color;
}
#include "track_pixel_font.h"
static struct track_engine track_state;
static struct track_song_bank track_song;
static uint8_t track_page=0,track_battery_low20=0,track_drum_note3_enabled=0;
static uint8_t track_ui_step_override=0,track_ui_step_snapshot=0,track_ui_song_pos_snapshot=0;
static uint8_t beetle_range_on=0,beetle_range_anchor=0,beetle_range_end=0;
static uint8_t etch_scope_active=0,etch_panel_composite=0;
static uint16_t etch_scope_origin=0,etch_scope_height=0;
static uint32_t etch_ms=0,track_saved_toast_until=0;
static const char *const track_web_stepfx[TRACK_FX_COUNT]={"---","FILTER","DRIVE","DELAY","REVERB","CHORUS","VIB","ARP","CUT","TREMOLO","VOLUME","PITCH"};
const char *track_drum_name(uint8_t id){
 static const char *nm[]={"KIK","SNR","HHC","HHO","TM1","TM2","TM3","CRS","RDE","CLP","PC1","PC2","PC3","RIM","U01","U02","U03","U04","U05","U06","U07","U08","U09","U10","U11","U12","U13"};
 if(id==TRACK_NOTE_STOP)return "STP";
 return id<sizeof(nm)/sizeof(nm[0])?nm[id]:"---";
}
uint8_t track_lane_has_data(const struct track_pattern *p,uint8_t lane){
 for(uint8_t k=0;k<TRACK_STEPS;k++)if(p->ch[lane][k].note!=TRACK_NOTE_EMPTY)return 1;
 return 0;
}


static void track_note_label(uint8_t note,char buf[5]){
 static const char *const n[]={"C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B "};
 if(note==TRACK_NOTE_STOP){buf[0]='S';buf[1]='T';buf[2]='P';buf[3]=0;return;}
 if(note==TRACK_NOTE_EMPTY){buf[0]='-';buf[1]='-';buf[2]='-';buf[3]=0;return;}
 const char *k=n[note%12u];buf[0]=k[0];buf[1]=k[1]==' '?'-':'#';
 int octave=(int)(note/12u)-1;buf[2]=octave<0?'-':(char)('0'+(octave>9?9:octave));buf[3]=0;
}

static void track_u8(char *dst,uint32_t v){dst[0]=(char)('0'+(v/10u)%10u);dst[1]=(char)('0'+v%10u);dst[2]=0;}

static uint8_t track_insect_frame(uint8_t playing,uint32_t now_ms){
 (void)now_ms;
 return playing?(uint8_t)((track_ui_step_override?track_ui_step_snapshot:track_state.transport.play_step)&1u):1u;
}

static void track_insect_draw(uint16_t x,uint16_t y,uint8_t frame){
 const uint32_t *pixels=beetle_play_icon[frame&1u];
 for(uint16_t row=0u;row<23u;row++)for(uint16_t col=0u;col<22u;col++)
  if(pixels[row]&(1u<<(21u-col)))lcd_fill(x+col,y+row,1u,1u,TRACK_PINK);
}

static void track_pix_cell(const char *v,uint16_t center,uint16_t y){
 if(!v)v="---";
 uint16_t n=0u;while(v[n]&&n<12u)n++;
 /* JS Math.round(x-(len+2)*3+.5) == x-(len+2)*3+1 */
 uint16_t left=(uint16_t)(center-(uint16_t)((n+2u)*3u)+1u);
 track_pix_text("[",left,y,TRACK_WHITE);
 track_pix_text(v,(uint16_t)(left+6u),y,TRACK_PINK);
 track_pix_text("]",(uint16_t)(left+6u*(n+1u)),y,TRACK_WHITE);
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

static uint8_t track_grid_value_chars(const struct track_step *step,uint8_t drum,uint8_t column){
 if(column==0u){
  if(drum){const char *text=track_drum_name(step->note);uint8_t n=0u;while(text[n]&&n<12u)n++;return n;}
  char name[5];track_note_label(step->note,name);
  uint8_t n=0u;while(name[n]&&n<4u)n++;return n;
 }
 if(column==1u){
  if(drum){const uint8_t second=track_note2_id(step);
   const char *text=second<TRACK_DRUM_NOTE_COUNT?track_drum_name(second):(step->chord==15u?"STP":"---");
   uint8_t n=0u;while(text[n]&&n<12u)n++;return n;}
  return step->note==TRACK_NOTE_EMPTY?2u:2u; /* INST always 2 digits when note exists. */
 }
 if(column==2u)return drum?2u:3u; /* Drum INST vs synth CHD. */
 return 3u; /* FX: exactly three characters. */
}

static void track_grid_lane_status(void){
 /* Two columns, three rows, matching user's 240x240 reference at x=36..46.
  * The indicator is for the visible pattern, not the whole project. */
 for(uint8_t lane=0u;lane<TRACK_CHANNELS;lane++){
  uint16_t x=(uint16_t)(36u+(uint16_t)(lane&1u)*6u);
  uint16_t y=(uint16_t)(210u+(uint16_t)(lane>>1u)*6u);
  lcd_fill(x,y,5u,5u,track_lane_has_data(&track_state.active,lane)?TRACK_PINK:TRACK_WHITE);
 }
}

static void track_fm1_grid(void){
 track_page_header(0,0u);
 static const char *const headings[]={"STEP","NOTE","INST","CHD","FX"};
 static const char *const drums[]={"STEP","NOTE","NOTE2","KIT","FX"};
 static const char *const drums3[]={"STEP","NOTE","NOTE2","NOTE3","KIT"};
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 static const char *const drum_lanes[]={"STEP","PAD","LVL","RAT","FX"};
#endif
 static const uint16_t xs[]={36u,77u,121u,163u,202u};
 static const char *const chords[TRACK_CHORD_COUNT]={"---","MAJ","MIN","DIM","AUG","SU2","SU4","MJ7","MN7","DO7","AD9","MN9"};
 static const char *const chord_full[TRACK_CHORD_COUNT]={"EMPTY","MAJOR","MINOR","DIMINISHED","AUGMENTED","SUSPENDED 2","SUSPENDED 4","MAJOR 7","MINOR 7","DOMINANT 7","ADD 9","MINOR 9"};
 uint8_t drum=(uint8_t)(track_state.transport.edit_channel==TRACK_DRUM_CHANNEL);
 for(uint8_t i=0u;i<5u;i++)track_pix_center(
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 drum? (track_drum_note3_enabled?drums3[i]:drums[i]):drum_lanes[i],
#else
 drum? (track_drum_note3_enabled?drums3[i]:drums[i]):headings[i],
#endif
 xs[i],28u,TRACK_WHITE);
 const uint8_t drawn_step=track_ui_step_override?track_ui_step_snapshot:track_state.transport.play_step;
 uint8_t first=(uint8_t)((track_state.transport.playing?drawn_step:track_state.transport.edit_step)&0xF0u);
 for(uint8_t i=0u;i<16u;i++){
  uint8_t row=(uint8_t)(first+i); if(row>=track_state.active.length || row>=TRACK_STEPS)continue;
  uint16_t y=(uint16_t)(43u+(uint16_t)i*10u);
  if(etch_scope_active && etch_panel_composite &&
     (y+11u<=etch_scope_origin || y-2u>=etch_scope_origin+etch_scope_height))continue;
  uint8_t focused=(uint8_t)(!track_state.transport.playing&&track_state.transport.edit_step==row);
  /* The audio ISR can advance while SPI DMA is busy. Both repainted rows
   * must use the same step snapshot, with no long IRQ-off critical region. */
  uint8_t live=(uint8_t)(track_state.transport.playing&&drawn_step==row);
  if(beetle_range_on && !track_state.transport.playing){
   uint8_t a=beetle_range_anchor,b=beetle_range_end;
   if(a>b){uint8_t t=a;a=b;b=t;}
   if(row>=a && row<=b)lcd_fill(22u,y-1u,198u,9u,
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 0x1811u
#else
 0x6810u
#endif
 );
  }
  if(focused){
   lcd_fill(21u,y-1u,201u,11u,0x100Au);
   uint8_t col=track_state.transport.edit_column;
   if(col>=TRACK_NAV_COLUMNS)col=0u;
   /* Editable columns are NOTE, INST, CHD, FX: headings [1..4]. */
   uint8_t index=(uint8_t)(col+1u);
   const struct track_step *cell=&track_state.active.ch[track_state.transport.edit_channel][row];
   uint8_t chars=track_grid_value_chars(cell,drum,col);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   if(!drum){
    if(col==0u){const char *nm=track_drum_name(cell->note);chars=0u;while(nm[chars]&&chars<10u)chars++;}
    else if(col==1u||col==2u)chars=2u;
   }
#endif
   if(drum&&track_drum_note3_enabled){
    if(col==2u){const char *t=track_state.active.note3[row]==TRACK_NOTE_STOP?"STP":
      track_state.active.note3[row]==TRACK_NOTE_EMPTY?"---":track_drum_name(track_state.active.note3[row]);
      chars=0u;while(t[chars]&&chars<10u)chars++;
    }else if(col==3u)chars=3u;
   }
   /* Same center and glyph pitch as track_pix_cell(); frame stays within
    * its own row (y-1 .. y+7), and exactly surrounds white brackets. */
   uint16_t cell_width=(uint16_t)(6u*(chars+2u));
   uint16_t left=(uint16_t)(xs[index]-(uint16_t)((chars+2u)*3u)+1u);
   uint16_t x=(uint16_t)(left-2u);
   lcd_fill(x,y-1u,cell_width+4u,9u,TRACK_PINK);
   lcd_fill(x+1u,y,cell_width+2u,7u,
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 0x100Au
#else
 0x680Bu
#endif
 );
  }
  if(live){
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 lcd_fill(21u,y-2u,201u,13u,0x047Fu);lcd_fill(23u,y,197u,9u,0x2012u);
#else
 lcd_fill(21u,y-2u,201u,13u,0xF018u);lcd_fill(23u,y,197u,9u,0x880Du);
#endif
 }
  const struct track_step *st=&track_state.active.ch[track_state.transport.edit_channel][row];
  char number[12];track_u8(number,row);track_pix_cell(number,xs[0],y);
  if(drum){
   track_pix_cell(track_drum_name(st->note),xs[1],y);
   uint8_t second=track_note2_id(st);
   track_pix_cell(second<TRACK_DRUM_NOTE_COUNT?track_drum_name(second):(st->chord==15u?"STP":"---"),xs[2],y);
   if(track_drum_note3_enabled){
    uint8_t third=track_state.active.note3[row];
    track_pix_cell(third==TRACK_NOTE_STOP?"STP":third==TRACK_NOTE_EMPTY?"---":track_drum_name(third),xs[3],y);
   }else{track_pix_cell("KIT",xs[3],y);}
  }else{
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   char number2[12];track_pix_cell(track_drum_name(st->note),xs[1],y);
   track_u8(number2,(uint8_t)(st->instrument+1u));track_pix_cell(number2,xs[2],y);
   track_u8(number2,TRACK_GROOVE_RATCHET(st));track_pix_cell(number2,xs[3],y);
#else
   char pitch[5];track_note_label(st->note,pitch);track_pix_cell(pitch,xs[1],y);
   if(st->note==255u)track_pix_cell("--",xs[2],y);
   else{track_u8(number,st->instrument+1u);track_pix_cell(number,xs[2],y);}
   track_pix_cell(chords[st->chord<TRACK_CHORD_COUNT?st->chord:0u],xs[3],y);
#endif
  }
  const char *fx=track_web_stepfx[st->effect<TRACK_FX_COUNT?st->effect:0u];
  char fx3[4]={fx[0],fx[1],fx[2],0};
  uint8_t source=track_state.pattern_slot,fxrow=row;
  const uint8_t drawn_song_pos=track_ui_step_override?track_ui_song_pos_snapshot:track_song.position;
 if(track_song.enabled && track_song.composite_active && drawn_song_pos<track_song.count){
   uint8_t src=track_song.chain[drawn_song_pos].lane[track_state.transport.edit_channel];
   if(src<TRACK_PATTERNS){source=src;uint8_t len=track_song.patterns[src].length;
    fxrow=(uint8_t)(row%(len?len:TRACK_STEPS));}
  }
  const uint8_t fxkey=track_extra_key(track_state.transport.edit_channel,fxrow,0u);
  const uint8_t synth_extra=track_extra_count(&track_song,source,fxkey);
  uint8_t mul=(uint8_t)((st->effect!=0u)+synth_extra>1u);
  if(drum){
   uint8_t note2fx=track_state.active.note2_fx[row].effect;
   uint8_t drum_n2=(uint8_t)((note2fx!=0u)+track_extra_count(&track_song,source,track_extra_key(TRACK_DRUM_CHANNEL,fxrow,1u)));
   mul|=(uint8_t)(drum_n2>1u);
   if(!st->effect && note2fx){const char *f2=track_web_stepfx[note2fx<TRACK_FX_COUNT?note2fx:0u];
    fx3[0]=f2[0];fx3[1]=f2[1];fx3[2]=f2[2];}
  }
  if(!st->effect && !mul && synth_extra){
   const struct track_extra_fx *single=track_extra_find(&track_song,source,fxkey,0u);
   if(!single)single=track_extra_find(&track_song,source,fxkey,1u);
   if(single){const char *label=track_web_stepfx[track_extra_effect(single)];
    fx3[0]=label[0];fx3[1]=label[1];fx3[2]=label[2];}
  }
  if(mul){fx3[0]='M';fx3[1]='U';fx3[2]='L';}
  track_pix_cell(drum&&track_drum_note3_enabled?"KIT":fx3,xs[4],y);
 }
 if(track_state.transport.edit_column==2u&&!drum
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 && 0
#endif
 ){
  const struct track_step *st=&track_state.active.ch[track_state.transport.edit_channel][track_state.transport.edit_step];
  char hint[32]="CHD: ";const char *ch=chord_full[st->chord<TRACK_CHORD_COUNT?st->chord:0u];
  uint8_t i=5u;while(*ch&&i<31u)hint[i++]=*ch++;hint[i]=0;
  track_pix_text(hint,48u,216u,TRACK_BRIGHT);
 }
 char pat[32]="PAT ";char num[12];track_u8(num,track_state.pattern_slot+1u);
 pat[4]=num[0];pat[5]=num[1];pat[6]=' ';pat[7]='/';pat[8]=' ';pat[9]='4';pat[10]='8';pat[11]=0;
 track_pix_text(pat,168u,224u,TRACK_PINK);
 if(drum)track_pix_text(track_drum_note3_enabled?"N3 ON":"N3 OFF",48u,224u,TRACK_BRIGHT);

 track_grid_lane_status();
 track_insect_draw(10u,205u,track_insect_frame(track_state.transport.playing,etch_ms));
 if((int32_t)(track_saved_toast_until-etch_ms)>0){
  lcd_fill(104u,202u,126u,13u,TRACK_DARK);
  track_pix_text("PROJECT SAVED",112u,204u,TRACK_PINK);
 }
}

static void write_ppm(const char *p){FILE*f=fopen(p,"wb");if(!f){perror(p);exit(1);}fprintf(f,"P6\n240 240\n255\n");for(int i=0;i<240*240;i++){uint16_t v=fb[i];unsigned char rgb[3]={((v>>11)&31)*255/31,((v>>5)&63)*255/63,(v&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);}
int main(int argc,char**argv){
 if(argc!=2)return 2;
 memset(&track_state,0,sizeof track_state);
 track_state.transport.bpm=120;
 track_state.active.length=16;
 track_state.transport.edit_step=3;
 track_state.transport.edit_channel=0;
 track_state.transport.edit_column=1;
 track_state.pattern_slot=0;
 for(uint8_t c=0;c<TRACK_CHANNELS;c++)for(uint8_t i=0;i<TRACK_STEPS;i++){
  struct track_step*st=&track_state.active.ch[c][i];
  st->note=TRACK_NOTE_EMPTY;
  st->instrument=c;
 }
 const uint8_t notes[16]={0,2,1,3,0,9,1,2,0,4,1,8,0,2,9,13};
 const uint8_t fx[16]={2,0,0,1,0,3,0,0,4,0,8,0,0,6,0,0};
 for(uint8_t i=0;i<16;i++){
  struct track_step*st=&track_state.active.ch[0][i];
  st->note=notes[i];st->instrument=0;st->effect=fx[i];
  st->value=(uint8_t)(50+i);
  track_groove_set_ratchet(st,(uint8_t)(i%4==3?2:1));
  track_step_set_vel(st,(uint8_t)(9+(i%5)));
 }
 track_fm1_grid();write_ppm(argv[1]);return 0;
}

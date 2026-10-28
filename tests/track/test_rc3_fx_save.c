#include <stdio.h>
#include <string.h>
#include "track_project_store.h"
#include "beetle_fx_save_safety.h"
#include <assert.h>
#include "track_master_fx.h"
#include "beetle_instrument_library.h"
static struct track_store_project p;
static uint8_t buffer[12256];
int main(){
 struct track_engine e;
 struct track_chip_engine chip;
 struct track_drum_engine drum;
 track_init(&e); track_song_init(&p.song,&e);track_chip_init(&chip);track_drum_init(&drum);
 track_mixer_init(&p.mixer);track_master_default(&p.master);beetle_library_init(&p.library);
 p.slot=1;p.bpm=120; memcpy(p.title,"PROJECT 02  ",12);p.drum_kit=0;
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++)p.drum[i]=drum.components[i];
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++){
  p.drum_user[i]=drum.user[i];p.sample_start[i]=0;p.sample_end[i]=100;
  p.sample_attack[i]=0;p.sample_decay[i]=50;p.sample_sustain[i]=100;p.sample_release[i]=50;
 }
 for(unsigned i=0;i<8;i++)p.kit_palette[i]=i;
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  struct track_store_instrument *v=&p.instruments[i];
  v->type=chip.osc_type[i];v->attack=(i==5?18:1);v->decay=(i==4?25:i==5?32:14);
  v->sustain=(i==4?70:i==5?80:100);v->release=(i==4?18:i==5?48:16);
  v->duty=50;v->volume=65;v->cutoff=(i==4?3200:i==5?8400:7000);
  v->va=chip.patches[i];p.type_valid[i]=1<<v->type;p.type_presets[i][v->type]=*v;
 }
 for(unsigned i=0;i<13;i++)p.punch_param[i]=chip.punch_param[i<5?i:i+3];
 uint32_t bytes=0;int r=track_store_encode(&p,buffer,sizeof(buffer),&bytes);
 assert(r==0);
 /* RC2 reproduction: VOL=200 is valid, changing FX TYPE without remapping
  * the value leaves an invalid FILTER=200 and makes SAVE return -1. */
 struct track_step *st=&p.song.patterns[5].ch[0][3];
 st->note=60;st->effect=TRACK_FX_VOLUME;st->value=200;
 assert(track_store_encode(&p,buffer,sizeof(buffer),&bytes)==0);
 st->effect=2; /* a non-volume effect */
 assert(track_store_encode(&p,buffer,sizeof(buffer),&bytes)==-1);
 assert(beetle_fx_change_value(TRACK_FX_VOLUME,2,200)==100);
 assert(beetle_fx_change_value(TRACK_FX_VOLUME,0,200)==50);
 assert(beetle_fx_change_value(2,TRACK_FX_VOLUME,50)==100);
 assert(beetle_fx_change_value(2,TRACK_FX_DRUM_PITCH,50)==12);
 assert(beetle_fx_repair_staged_values(&p.song)==1u);
 assert(st->note==60 && st->effect==2 && st->value==100);
 assert(track_store_encode(&p,buffer,sizeof(buffer),&bytes)==0);
 /* CH6 NOTE2 and the independent FX2/FX3 pool can be affected by the same
  * bug; never clear any of these notes or effect types when repairing. */
 p.song.patterns[6].ch[TRACK_DRUM_CHANNEL][4].note=1;
 p.song.patterns[6].note2_fx[4].effect=2;
 p.song.patterns[6].note2_fx[4].value=200;
 p.song.extras.count=1;
 p.song.extras.data[0]=(struct track_extra_fx){6,4,2,200};
 assert(track_store_encode(&p,buffer,sizeof(buffer),&bytes)==-1);
 assert(beetle_fx_repair_staged_values(&p.song)==2u);
 assert(p.song.patterns[6].ch[TRACK_DRUM_CHANNEL][4].note==1);
 assert(p.song.patterns[6].note2_fx[4].effect==2 && p.song.patterns[6].note2_fx[4].value==100);
 assert(p.song.extras.data[0].effect==2 && p.song.extras.data[0].value==100);
 assert(track_store_encode(&p,buffer,sizeof(buffer),&bytes)==0);
 puts("PASS RC3: FX type transitions, FX1/2/3 invalid legacy values, no note loss, encode succeeds");
 printf("default return=%d bytes=%u\n",r,bytes);
 for(unsigned j=0;j<TRACK_PATTERNS;j++){
  p.song.patterns[j].ch[0][0].note=60;
  int x=track_store_encode(&p,buffer,sizeof(buffer),&bytes);
  if(x){printf("failed after %u patterns rc=%d\n",j+1,x);break;}
 }
 return r?1:0;
}

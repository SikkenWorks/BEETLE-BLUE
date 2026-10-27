#include "track_chip.h"
#include "track_drum.h"
#include "track_mixer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct track_drum_engine drums;
static struct track_mixer mix;
static int16_t out[2*512],stems[6*512];
int main(void){
 track_drum_init(&drums);track_mixer_init(&mix);
 mix.ch[0].pan=-100;mix.ch[1].pan=100;
 assert(track_drum_trigger(&drums,0));drums.voices[0].source_lane=0;
 assert(track_drum_trigger(&drums,1));drums.voices[1].source_lane=1;
 beetle_stem_capture=stems;beetle_stem_offset=0;
 track_drum_render(&drums,&mix,out,512);
 unsigned counts[6]={0}, l=0,r=0;
 for(unsigned i=0;i<512;i++){
  for(unsigned k=0;k<6;k++)counts[k]+=(stems[i*6+k]!=0);
  l+=(out[i*2]!=0);r+=(out[i*2+1]!=0);
 }
 assert(counts[0]>10&&counts[1]>10 && l>10&&r>10);
 for(unsigned k=2;k<6;k++)assert(counts[k]==0);
 assert(drums.voices[0].source_lane==0&&drums.voices[1].source_lane==1);
 /* A live MUTE immediately silences a playing tail of only that lane. */
 memset(out,0,sizeof(out));memset(stems,0,sizeof(stems));
 mix.ch[0].mute=1;track_drum_render(&drums,&mix,out,512);
 unsigned alive_b=0;
 for(unsigned i=0;i<512;i++){
  assert(stems[i*6]==0);
  alive_b+=(stems[i*6+1]!=0);
 }
 assert(alive_b>10);
 beetle_stem_capture=0;
 puts("PASS DRUM V0.4: six native stem routing, pan, independent active tail mute");
}

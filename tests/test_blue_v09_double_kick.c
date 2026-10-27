/* BEETLE BLUE V0.9: rapid double-kick and sustained multi-lane drum host stress.
 * This is NOT a physical DMA-interrupt test; that requires an FM1. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "track_drum.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
int main(void){
 static struct track_drum_engine d;
 static struct track_mixer mix;
 static int16_t pcm[2*128];
 track_drum_init(&d);track_mixer_init(&mix);
 uint32_t clipped=0, max_amplitude=0, voice_triggers=0;
 for(unsigned n=0;n<1000;n++){
  assert(track_drum_trigger(&d,0)>=0);voice_triggers++;
  assert(track_drum_trigger(&d,0)>=0);voice_triggers++;
  for(unsigned b=0;b<7;b++){
   if((n%3)==0){assert(track_drum_trigger(&d,(uint8_t)(b%14))>=0);voice_triggers++;}
   memset(pcm,0,sizeof pcm);
   track_drum_render(&d,&mix,pcm,128);
   for(unsigned j=0;j<256;j++){
    int32_t x=pcm[j]<0?-(int32_t)pcm[j]:pcm[j];
    if(x>max_amplitude)max_amplitude=x;
    if(x>=32767)clipped++;
   }
  }
 }
 assert(max_amplitude>0);
 assert(clipped==0);
 printf("PASS: %u fast kick/drum triggers, 7000 native blocks; peak %u; no host clipping\n",voice_triggers,max_amplitude);
 return 0;
}

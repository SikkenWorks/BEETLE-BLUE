/* SPDX-License-Identifier: GPL-3.0-only
 * V0.99 - exact pitch/Q16 simplification and deterministic stereo mix limiter.
 * This host test does not emulate FM1 DMA timing or analogue output.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_core.h"
#include "track_mixer.h"
#include "track_drum.h"
/* Intentionally compile the actual native implementation into this test, so
 * the internal saturator and pitch lookup can be verified precisely. */
#include "../firmware/fm1-native/track_drum.c"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
static struct track_drum_engine d;
static struct track_mixer mix;
static int16_t block[128*2];
static uint64_t checksum;
static unsigned peak;
static uint16_t abs16(int16_t x){return (uint16_t)(x<0?-(int32_t)x:x);}
int main(void){
 for(int semitone=-36; semitone<=36; semitone++){
  const uint32_t q=tune_q16[semitone+36];
  uint32_t original=(uint32_t)(((uint64_t)32768u*q+32768u)>>16u);
  assert(rate_step((int8_t)semitone)==original);
 }
 assert(blue_stereo_soft_limit(0)==0);
 assert(blue_stereo_soft_limit(23000)==23000);
 assert(blue_stereo_soft_limit(-23000)==-23000);
 assert(blue_stereo_soft_limit(24008)==24001);
 assert(blue_stereo_soft_limit(-24008)==-24001);
 assert(blue_stereo_soft_limit(1000000)==31500);
 assert(blue_stereo_soft_limit(-1000000)==-31500);
 track_drum_init(&d);track_mixer_init(&mix);
 /* Trigger all 6 source lanes repeatedly with direct MONOLITH/BEETLE
  * kit changes, stereo panning and FX. Sustain dense 128-frame blocks. */
 for(unsigned block_number=0u;block_number<1300u;block_number++){
  if(!(block_number%2u)){
   for(uint8_t lane=0u;lane<6u;lane++){
    int ok=track_drum_trigger(&d,(uint8_t)((block_number&4u)?64u+lane:lane));
    assert(ok); struct track_drum_voice *v=0;
    uint32_t serial=d.trigger_serial;
    for(unsigned j=0;j<TRACK_DRUM_POLY;j++)if(d.voices[j].active&&d.voices[j].sequence==serial){v=&d.voices[j];break;}
    assert(v);v->source_lane=lane;
    if((lane&1u)==0u)track_drum_step_fx(&d,v,0u,2u,60u,120u);
   }
  }
  for(uint8_t lane=0;lane<6;lane++)mix.ch[lane].pan=(int8_t)(((block_number+lane)%3u==0u)?-50:((block_number+lane)%3u==1u)?50:0);
  memset(block,0,sizeof block);
  track_drum_render(&d,&mix,block,128u);
  for(unsigned i=0;i<256u;i++){
   unsigned mag=abs16(block[i]); if(mag>peak)peak=mag;
   assert(mag<=31500u);
   checksum+=(uint16_t)block[i];
  }
 }
 assert(checksum!=0u);
 printf("PASS V0.99: all 73 tuning values exact, symmetric soft knee, 3900 kit triggers, 166400 audio frames, peak=%u checksum=%llu\n",peak,(unsigned long long)checksum);
 return 0;
}

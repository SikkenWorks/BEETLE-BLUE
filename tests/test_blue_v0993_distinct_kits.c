/* SPDX-License-Identifier: GPL-3.0-only. Actual host drum DSP comparison. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_drum.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture;
uint32_t beetle_stem_offset;
static struct track_drum_engine drums;
static struct track_mixer mixer;
static int16_t pcm[256u*2u];
static uint64_t render_hash(uint8_t note,uint32_t *peak){
 track_drum_init(&drums);
 assert(track_drum_trigger(&drums,note));
 uint64_t hash=1469598103934665603ULL;
 *peak=0;
 for(unsigned b=0;b<20;b++){
  memset(pcm,0,sizeof pcm);
  track_drum_render(&drums,&mixer,pcm,256u);
  for(unsigned j=0;j<512u;j++){
   uint32_t p=(uint32_t)(pcm[j]<0?-(int32_t)pcm[j]:pcm[j]);
   if(p>*peak)*peak=p;
   hash^=(uint16_t)pcm[j];hash*=1099511628211ULL;
  }
 }
 return hash;
}
int main(void){
 track_mixer_init(&mixer);
 for(uint8_t component=0;component<14u;component++){
  uint32_t p_a,p_b;
  uint64_t a=render_hash(component,&p_a);
  uint64_t b=render_hash((uint8_t)(64u+component),&p_b);
  assert(p_a>0 && p_b>0 && a!=b);
  printf("component %02u distinct BEETLE=%016llx MONOLITH=%016llx\n",component,(unsigned long long)a,(unsigned long long)b);
 }
 puts("PASS all 14 logical drum components emit different native PCM (host only)");return 0;
}

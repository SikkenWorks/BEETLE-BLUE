/* GPL-3.0-only. Functional host-only ROM drum audio tests; no FM1 timing claim. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_blue_monolith_rom.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
static int16_t out[256*2];
static struct track_drum_engine d;
static struct track_mixer m;
static uint32_t hashes[30];
static uint32_t render(uint8_t note){
 track_drum_init(&d);track_mixer_init(&m);
 assert(track_drum_trigger(&d,note)==1);
 uint64_t energy=0;uint32_t hash=2166136261u;
 for(unsigned i=0;i<150;i++){
  memset(out,0,sizeof out);
  track_drum_render(&d,&m,out,256u);
  for(unsigned j=0;j<512;j++){
   const int32_t v=out[j];energy+=(uint32_t)(v<0?-v:v);
   hash=(hash^(uint16_t)v)*16777619u;
  }
 }
 assert(energy>1000000u);
 return hash;
}
int main(void){
 assert(BLUE_MONOLITH_COUNT==16u);
 for(uint8_t i=0u;i<16u;i++){
  assert(blue_rom_patches[i].length_22050>=2000u);
  hashes[i]=render((uint8_t)(64u+i));
  assert(hashes[i]!=render(0u)); /* distinct from BEETLE kick */
 }
 for(uint8_t i=0u;i<16u;i++)for(uint8_t j=(uint8_t)(i+1u);j<16u;j++)assert(hashes[i]!=hashes[j]);
 for(uint8_t i=0u;i<14u;i++)(void)render(i); /* retained BEETLE ROM */
 track_drum_init(&d);track_mixer_init(&m);
 for(uint8_t i=0u;i<6u;i++)assert(track_drum_trigger(&d,(uint8_t)(64u+i))==1);
 for(unsigned n=0u;n<1500u;n++){
  assert(track_drum_trigger(&d,64u));assert(track_drum_trigger(&d,64u));
  memset(out,0,sizeof out);track_drum_render(&d,&m,out,128u);
 }
 track_drum_stop(&d);memset(out,0,sizeof out);track_drum_render(&d,&m,out,128u);
 for(unsigned i=0u;i<256u;i++)assert(out[i]==0);
 puts("PASS: 16 distinct procedural MONOLITH ROM patches, 14 BEETLE PCM drums, 3000 kick retriggers, zero-voice idle; hardware DMA not tested");
 return 0;
}

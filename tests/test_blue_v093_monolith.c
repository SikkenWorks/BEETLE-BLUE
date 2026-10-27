/* Read-only embedded MONOLITH audio regression, target-independent host test.
 * The FM1 deadline/analog outputs are NOT exercised here. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_blue_monolith.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
static int16_t stereo[256*2];
int main(void){
 static struct track_drum_engine d;
 static struct track_mixer m;
 assert(BLUE_MONOLITH_COUNT==16);
 assert(blue_monolith_offset[16]==BLUE_MONOLITH_BYTES);
 track_mixer_init(&m);
 unsigned success=0;
 uint32_t hashes[16]={0};
 for(uint8_t n=0;n<16;n++){
  uint32_t total_frames=(blue_monolith_offset[n+1]-blue_monolith_offset[n])*2;
  assert(total_frames>=1000 && total_frames<=15000);
  track_drum_init(&d);
  assert(track_drum_trigger(&d,(uint8_t)(64+n))==1);
  assert(d.voices[0].id==(uint8_t)(64+n));
  assert(d.voices[0].phase_step>0);
  uint32_t energy=0,h=2166136261u;
  for(unsigned chunk=0;chunk<170;chunk++){
   memset(stereo,0,sizeof stereo);
   track_drum_render(&d,&m,stereo,256);
   for(unsigned i=0;i<512;i++){
    int32_t sm=stereo[i];
    energy+=(uint32_t)((sm<0?-sm:sm)>>8u);
    h=(h ^ (uint16_t)sm)*16777619u;
    assert(sm>=-32768 && sm<=32767);
   }
  }
  assert(energy>300);
  hashes[n]=h;
  success++;
 }
 for(unsigned i=0;i<16;i++)for(unsigned j=i+1;j<16;j++)assert(hashes[i]!=hashes[j]);
 /* Play six simultaneous embedded voices, rapidly retrigger kick 2x. */
 track_drum_init(&d);
 for(unsigned i=0;i<6;i++)assert(track_drum_trigger(&d,(uint8_t)(64+i))==1);
 for(unsigned i=0;i<1000;i++){
  assert(track_drum_trigger(&d,64u));assert(track_drum_trigger(&d,64u));
  memset(stereo,0,sizeof stereo);
  track_drum_render(&d,&m,stereo,128);
 }
 assert(track_drum_trigger_pair(&d,64u,65u)==2);
 puts("PASS: 16 distinct embedded one-shots, stereo rendering, 2000 kick retriggers, CH6 pair; not hardware DMA-tested");
 return 0;
}

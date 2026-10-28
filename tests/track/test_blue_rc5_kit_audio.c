/* Verify both factory kits produce distinctly routed native PCM. */
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_blue_lane_source.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
int16_t *beetle_stem_capture; uint16_t beetle_stem_offset;
static struct track_drum_engine engine;
static struct track_mixer mixer;
static int16_t buffer[1024*2];
static uint32_t digest(uint8_t id){
 track_drum_init(&engine);track_mixer_init(&mixer);
 assert(track_drum_trigger(&engine,id));
 uint32_t h=2166136261u;
 for(unsigned k=0;k<8u;k++){
  memset(buffer,0,sizeof buffer);
  track_drum_render(&engine,&mixer,buffer,1024u);
  for(unsigned i=0;i<2048u;i++)h=(h^(uint16_t)buffer[i])*16777619u;
 }
 return h;
}
int main(void){
 uint8_t lane_kits[6]={0u,1u,0u,1u,0u,1u};
 for(uint8_t lane=0;lane<6;lane++)assert(beetle_blue_lane_source(lane_kits,lane)==(lane&1u));
 for(unsigned note=0;note<14;note++){
  uint32_t beetle=digest((uint8_t)note),monolith=digest((uint8_t)(64u+note));
  printf("PAD %02u BEETLE %08x MONOLITH %08x\n",note,beetle,monolith);
  assert(beetle!=monolith);
 }
 puts("PASS: 14 distinct MONOLITH/BEETLE native drum renders + per-lane selector");
 return 0;
}

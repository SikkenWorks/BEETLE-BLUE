/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "beetle_blue_lane_source.h"
static uint8_t mono(uint8_t note,uint8_t source){return source==1u && note<16u?(uint8_t)(note+64u):note;}
int main(void){
 uint8_t kits[6]={0,1,2,13,1,0};
 for(uint8_t lane=0;lane<6;lane++)for(uint8_t old_inst=0;old_inst<8;old_inst++){
  (void)old_inst; /* INST is deliberately not used for authoritative lane routing. */
  uint8_t src=beetle_blue_lane_source(kits,lane);
  assert(src==kits[lane]);
  for(uint8_t note=0;note<14;note++){
   uint8_t id=mono(note,src);
   assert(src==1u ? id==(uint8_t)(64u+note) : id==note);
  }
 }
 for(uint8_t selected=0;selected<14;selected++)for(uint8_t lane=0;lane<6;lane++){
  kits[lane]=selected;
  assert(beetle_blue_lane_source(kits,lane)==selected);
 }
 kits[3]=200;assert(beetle_blue_lane_source(kits,3)==0);
 puts("PASS 672 lane/kit/source/step combinations + CH6 + corrupt value fallback");return 0;
}

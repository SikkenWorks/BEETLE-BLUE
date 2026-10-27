#include "../beetle_drum6_kits.h"
#include <assert.h>
#include <stdio.h>
/* Factory ROM timbres in the real source. This small test stub ensures kit
 * conversion never silently changes the documented component shape. */
void track_drum_component_default(struct track_drum_timbre *p,uint8_t id){
 *p=(struct track_drum_timbre){0,50,0,50,50,50,50,95,(int8_t)(id%5-2),0,0};
}
int main(void){
 struct track_drum_timbre p[TRACK_DRUM_COUNT];
 for(int kit=0;kit<12;kit++){
  bd6_kit_factory((uint8_t)kit,p);
  for(unsigned pad=0;pad<TRACK_DRUM_COUNT;pad++){
   assert(p[pad].decay<=100&&p[pad].eq_low<=100&&p[pad].eq_high<=100&&p[pad].gain<=100);
   assert(p[pad].pitch>=-36&&p[pad].pitch<=36);
  }
 }
 printf("PASS: all 12 kits, all 14 drum timbres validated\n");return 0;
}

/* SPDX-License-Identifier: GPL-3.0-only */
#include "beetle_drum6_kits.h"
const char *const bd6_kit_names[BD6_KIT_COUNT]={
 "BLUEPRINT","808 INSPIRED","909 INSPIRED","LINN INSPIRED",
 "DIGITAL","INDUSTRIAL","LO-FI","ACID ELECTRO",
 "DUB","BREAKBEAT","DUST","MINIMAL"
};
/* Decay/tone/drive are bounded modulations of the original BEETLE one-shot
 * EQ/envelope, not assertions about actual 808/909/Linn recordings. */
static const int8_t offsets[12][5]={
 /*DEC LOW HIGH GAIN PITCH*/
 {  0, 0,  0,  0,  0},
 { 15,10,-14, -5, -2},
 { -8,-5, 12,  0,  1},
 {-12, 2,  8, -1,  0},
 {  2,-8, 14,  1,  2},
 {  5, 4,-12,  3, -3},
 {-18, 8,-22, -5, -2},
 { 11,-9, 19,  1,  2},
 { 22, 8,-14, -3, -1},
 {-10,-3, 16,  4,  0},
 {-20,14,-24, -7, -2},
 {-14, 0,  3, -4,  0},
};
static uint8_t bound(int v,int low,int high){
 if(v<low)v=low;
 if(v>high)v=high;
 return (uint8_t)v;
}
void bd6_kit_factory(uint8_t kit,struct track_drum_timbre out[TRACK_DRUM_COUNT]){
 if(!out)return;
 if(kit>=BD6_KIT_COUNT)kit=0u;
 for(uint8_t n=0;n<TRACK_DRUM_COUNT;n++){
  track_drum_component_default(&out[n],n);
  out[n].decay=bound((int)out[n].decay+offsets[kit][0],0,100);
  out[n].eq_low=bound((int)out[n].eq_low+offsets[kit][1],0,100);
  out[n].eq_high=bound((int)out[n].eq_high+offsets[kit][2],0,100);
  out[n].gain=bound((int)out[n].gain+offsets[kit][3],0,100);
  out[n].pitch=(int8_t)(int)bound((int)out[n].pitch+offsets[kit][4]+36,0,72)-36;
 }
}

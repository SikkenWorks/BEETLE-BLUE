/* SPDX-License-Identifier: GPL-3.0-only
 * Host regression: cross-kit hi-hat chokes and NOTE1/NOTE2 assignment.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_core.h"
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_blue_pair_route.h"
static struct track_drum_engine d;
static struct track_mixer mixer;
static int16_t pcm[256*2];
static struct track_drum_voice *voice_for(uint32_t serial){
 for(uint8_t i=0;i<TRACK_DRUM_POLY;i++)if(d.voices[i].active&&d.voices[i].sequence==serial)return &d.voices[i];
 return 0;
}
static void exercise_hat(uint8_t first,uint8_t second){
 track_drum_init(&d);
 assert(track_drum_trigger(&d,first));
 struct track_drum_voice *a=voice_for(d.trigger_serial);assert(a);
 assert(a->release_frames==0);
 assert(track_drum_trigger(&d,second));
 assert(a->release_frames==48); /* choke MONOLITH and BEETLE alike */
}
int main(void){
 track_mixer_init(&mixer);
 exercise_hat(66u,67u);exercise_hat(67u,66u);exercise_hat(79u,66u);
 exercise_hat(66u,2u);exercise_hat(2u,67u);exercise_hat(2u,79u);
 track_drum_init(&d);
 uint32_t s=d.trigger_serial;
 assert(track_drum_trigger_pair(&d,66u,67u)==1);
 assert(d.trigger_serial==s+1u);
 assert(beetle_blue_pair_is_note2(66u,67u,66u,2u,s+1u,s)==0u);
 track_drum_init(&d);s=d.trigger_serial;
 assert(track_drum_trigger_pair(&d,67u,66u)==1);
 assert(d.trigger_serial==s+1u);
 assert(beetle_blue_pair_is_note2(67u,66u,66u,2u,s+1u,s)==1u);
 /* Source-correct NOTE1/NOTE2 mapping, even when MONOLITH uses IDs 64+. */
 assert(!beetle_blue_pair_is_note2(64u,65u,64u,0u,11u,10u));
 assert(beetle_blue_pair_is_note2(64u,65u,65u,1u,12u,10u));
 assert(beetle_blue_pair_is_note2(TRACK_NOTE_EMPTY,65u,65u,1u,11u,10u));
 assert(!beetle_blue_pair_is_note2(64u,TRACK_NOTE_EMPTY,64u,0u,11u,10u));
 assert(!beetle_blue_pair_is_note2(64u,64u,64u,0u,11u,10u));
 assert(beetle_blue_pair_is_note2(64u,64u,64u,0u,12u,10u));
 /* BEETLE physical alt snare ID may equal the OTHER note number. */
 assert(!beetle_blue_pair_is_note2(1u,2u,2u,1u,11u,10u));
 assert(beetle_blue_pair_is_note2(1u,2u,6u,2u,12u,10u));
 track_drum_init(&d);
 for(unsigned k=0;k<4096;k++){
  const uint8_t id=(k&1)?66u:(k&2)?67u:(k&4)?79u:2u;
  assert(track_drum_trigger(&d,id));
  memset(pcm,0,sizeof pcm);track_drum_render(&d,&mixer,pcm,128u);
  for(unsigned j=0;j<256u;j++)assert(pcm[j]>=-32768&&pcm[j]<=32767);
 }
 puts("PASS V0.98: cross-kit hat choke, MONOLITH/BEETLE CH6 note2 classification, 4096 retriggers (host only)");
 return 0;
}

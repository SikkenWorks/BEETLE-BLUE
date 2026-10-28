/* GPL-3.0-only. BLUE 0.99.5: per-slot self-choke and cross-kit hi-hat.
 * This exercises real track_drum_trigger with the native mixer, no mock DSP. */
#include "track_drum.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int16_t *beetle_stem_capture;
uint32_t beetle_stem_offset;
static struct track_drum_engine d;
static struct beetle_oneshot_bank bank;
static uint8_t pcm[BEETLE_ONESHOT_MAX_BYTES];
static struct track_drum_voice *by_serial(uint32_t serial){
 for(unsigned v=0;v<TRACK_DRUM_POLY;v++)
  if(d.voices[v].active&&d.voices[v].sequence==serial)return &d.voices[v];
 return 0;
}
static void note_pair(uint8_t a,uint8_t b,uint8_t should_choke){
 track_drum_init(&d);track_drum_bind_oneshots(&d,&bank);
 assert(track_drum_trigger(&d,a));
 struct track_drum_voice *old=by_serial(d.trigger_serial);
 assert(old&&old->release_frames==0);
 assert(track_drum_trigger(&d,b));
 assert(old->release_frames==(should_choke?48u:0u));
}
int main(void){
 memset(&bank,0,sizeof(bank));bank.first=pcm;
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++)bank.length[i]=64u;
 /* Independent U slots do not choke; repeated same user source always does. */
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++){
  note_pair((uint8_t)(TRACK_DRUM_COUNT+i),(uint8_t)(TRACK_DRUM_COUNT+i),1u);
  note_pair((uint8_t)(TRACK_DRUM_COUNT+i),(uint8_t)(TRACK_DRUM_COUNT+(i+1u)%BEETLE_ONESHOT_NOTES),0u);
 }
 note_pair(2u,3u,1u);note_pair(3u,2u,1u);
 note_pair(66u,67u,1u);note_pair(79u,2u,1u);
 note_pair(2u,79u,1u);note_pair(67u,3u,1u);
 /* Explicit choke groups must keep working for different user samples. */
 track_drum_init(&d);track_drum_bind_oneshots(&d,&bank);
 d.user[0].choke_group=4u;d.user[1].choke_group=4u;
 assert(track_drum_trigger(&d,14u));
 struct track_drum_voice *old=by_serial(d.trigger_serial);assert(old);
 assert(track_drum_trigger(&d,15u));assert(old->release_frames==48u);
 puts("PASS U01-U13 self choke, independent slots, explicit groups, and cross-kit hi-hats");
 return 0;
}

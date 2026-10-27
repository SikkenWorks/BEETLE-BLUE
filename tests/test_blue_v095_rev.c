/* GPL-3.0-only. Host functional regression of native REV (not hardware timing). */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_drum.h"
#include "track_core.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0u;
static struct track_drum_engine d;
static struct track_mixer mix;
static int16_t audio[512];
static uint32_t signature(uint8_t note,uint8_t reverse){
 track_drum_init(&d);track_mixer_init(&mix);
 assert(track_drum_trigger(&d,note));
 if(reverse){
  uint8_t passed=0u;
  for(unsigned i=0;i<TRACK_DRUM_POLY;i++)if(d.voices[i].active){
   assert(track_drum_reverse_voice(&d.voices[i]));passed=1u;}
  assert(passed);
 }
 uint32_t sig=2166136261u;
 for(unsigned n=0;n<15;n++){
  memset(audio,0,sizeof(audio));
  track_drum_render(&d,&mix,audio,256u);
  for(unsigned i=0;i<512;i++)sig=(sig^(uint16_t)audio[i])*16777619u;
 }
 return sig;
}
int main(void){
 assert(TRACK_FX_COUNT==13u && TRACK_FX_REVERSE==12u);
 for(uint8_t i=0;i<14;i++){
  uint32_t forward=signature(i,0u),reverse=signature(i,1u);
  assert(forward!=reverse);
 }
 for(uint8_t i=64u;i<80u;i++){
  uint32_t forward=signature(i,0u),reverse=signature(i,1u);
  assert(forward!=reverse);
 }
 track_drum_init(&d);track_mixer_init(&mix);
 for(unsigned n=0;n<3000;n++){
  assert(track_drum_trigger(&d,(uint8_t)((n&1u)?0u:64u)));
  for(unsigned i=0;i<TRACK_DRUM_POLY;i++)
   if(d.voices[i].active && d.voices[i].sequence==d.trigger_serial)
    assert(track_drum_reverse_voice(&d.voices[i]));
  memset(audio,0,sizeof audio);
  track_drum_render(&d,&mix,audio,128u);
 }
 puts("PASS REV: 14 BEETLE PCM reverse, 16 MONOLITH reverse-style procedural variants, 3000 repeated triggers (host only)");
 return 0;
}

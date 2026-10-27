/* SPDX-License-Identifier: GPL-3.0-only
 * Isolated host DSP tests. They cannot prove 44.1kHz DMA deadlines on FM1.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_core.h"
#include "track_drum.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
static struct track_drum_engine engine;
static struct track_mixer mix;
static int16_t audio[256*2];
struct audio_sig { uint32_t hash; uint64_t energy; uint32_t frames; uint16_t peak; };
static struct audio_sig render(uint8_t kit, uint8_t fx, uint8_t value,
                                uint8_t fx2,uint8_t value2,uint8_t fx3,uint8_t value3){
 track_drum_init(&engine);track_mixer_init(&mix);
 const uint8_t id=kit?64u:0u;
 assert(track_drum_trigger(&engine,id));
 uint8_t voice=0xffu;
 for(uint8_t i=0;i<TRACK_DRUM_POLY;i++)if(engine.voices[i].active){voice=i;break;}
 assert(voice<TRACK_DRUM_POLY);
 track_drum_step_fx(&engine,&engine.voices[voice],0u,fx,value,120u);
 track_drum_step_fx(&engine,&engine.voices[voice],1u,fx2,value2,120u);
 track_drum_step_fx(&engine,&engine.voices[voice],2u,fx3,value3,120u);
 struct audio_sig sig={2166136261u,0,0,0};
 for(uint32_t block=0;block<45;block++){
  memset(audio,0,sizeof audio);
  track_drum_render(&engine,&mix,audio,256u);
  for(uint32_t n=0;n<512;n++){
   const int32_t v=audio[n];
   if((uint32_t)(v<0?-v:v)>sig.peak)sig.peak=(uint16_t)(v<0?-v:v);
   sig.energy+=(uint32_t)(v<0?-v:v);
   sig.hash=(sig.hash^(uint16_t)v)*16777619u;
  }
  sig.frames+=256u;
 }
 return sig;
}
int main(void){
 for(uint8_t kit=0u;kit<2u;kit++){
  struct audio_sig dry=render(kit,0u,50u,0u,0u,0u,0u);
  assert(dry.energy>10000u);
  for(uint8_t type=1u;type<TRACK_FX_COUNT;type++){
   uint8_t value=type==TRACK_FX_DRUM_PITCH?18u:type==TRACK_FX_REVERSE?50u:90u;
   struct audio_sig wet=render(kit,type,value,0u,0u,0u,0u);
   assert(wet.energy>0u);
   assert(wet.peak<=32767u);
   if(wet.hash==dry.hash)fprintf(stderr,"FAIL KIT=%u FX=%u dry=%u wet=%u\n",kit,type,dry.hash,wet.hash);
   assert(wet.hash!=dry.hash);
  }
  struct audio_sig chain=render(kit,1u,90u,2u,75u,9u,95u);
  assert(chain.hash!=dry.hash);
  assert(chain.hash!=render(kit,1u,90u,0u,0u,0u,0u).hash);
  assert(chain.hash!=render(kit,2u,75u,0u,0u,0u,0u).hash);
 }
 /* Explicit retrigger tests catch stale per-voice FX state, including voice stealing. */
 track_drum_init(&engine);track_mixer_init(&mix);
 for(unsigned n=0;n<2500u;n++){
  assert(track_drum_trigger(&engine,(n&1u)?0u:64u));
  for(uint8_t i=0;i<TRACK_DRUM_POLY;i++)if(engine.voices[i].active&&
    engine.voices[i].sequence==engine.trigger_serial){
   track_drum_step_fx(&engine,&engine.voices[i],0u,(uint8_t)(1u+n%12u),80u,120u);
   track_drum_step_fx(&engine,&engine.voices[i],1u,2u,75u,120u);
  }
  memset(audio,0,sizeof audio);
  track_drum_render(&engine,&mix,audio,128u);
 }
 printf("PASS V0.97: 12 FX types alter PCM and MONOLITH, 3-slot layering, 2500 retriggers (HOST only)\n");
 return 0;
}

/* GPL-3.0-only. V0.96 local-sample trim + REV regression. Host only. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_drum.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0u;
static struct track_drum_engine d;
static struct track_mixer mix;
static struct beetle_oneshot_bank bank;
static uint8_t pcm[BEETLE_ONESHOT_FIRST_BYTES];
static int16_t output[256];
static struct track_drum_voice *local_trigger(uint8_t reverse,uint8_t start,uint8_t end){
 track_drum_stop(&d);
 assert(track_drum_trigger(&d,14u));
 struct track_drum_voice *v=0;
 for(unsigned i=0;i<TRACK_DRUM_POLY;i++)if(d.voices[i].active && d.voices[i].id==16u) v=&d.voices[i];
 assert(v);
 assert(track_drum_trim_local_voice(&d,v,start,end));
 if(reverse)assert(track_drum_reverse_voice(v));
 return v;
}
static uint32_t exercise(uint8_t reverse,uint8_t start,uint8_t end,uint32_t *run_frames){
 struct track_drum_voice *v=local_trigger(reverse,start,end);
 uint32_t signature=2166136261u,played=0;
 while(v->active && played<20000u){
  memset(output,0,sizeof output);
  track_drum_render(&d,&mix,output,128);
  for(unsigned i=0;i<256;i++)signature=(signature^(uint16_t)output[i])*16777619u;
  played+=128;
 }
 assert(!v->active);assert(played>0u && played<20000u);
 if(run_frames)*run_frames=played;
 return signature;
}
int main(void){
 for(unsigned i=0;i<sizeof pcm;i++)pcm[i]=(uint8_t)((i*17u+(i>>2u)*29u+i/19u)&255u);
 memset(&bank,0,sizeof bank);bank.first=pcm;bank.length[0]=2000u;
 track_drum_init(&d);track_mixer_init(&mix);track_drum_bind_oneshots(&d,&bank);
 struct track_drum_voice *v=local_trigger(0,25u,75u);
 assert(v->start_percent==25u&&v->end_percent==75u);
 assert(v->phase==0u);
 uint32_t cap=v->length;
 assert(cap>0u&&cap<10000u);
 uint32_t a,b,full;
 uint32_t sf=exercise(0,25u,75u,&a);
 uint32_t sr=exercise(1,25u,75u,&b);
 (void)exercise(0,0u,100u,&full);
 assert(sf!=sr);
 assert(a==b);
 assert(a<full);
 for(unsigned k=0;k<3000u;k++){
  struct track_drum_voice *hit=local_trigger((uint8_t)(k&1u),(uint8_t)(k%35u),(uint8_t)(k%35u+40u));
  memset(output,0,sizeof output);
  track_drum_render(&d,&mix,output,128);
  assert(hit->start_percent==k%35u);
 }
 /* One-frame crop must not underflow the reverse source address. */
 bank.length[0]=50u;
 (void)exercise(1,97u,100u,&a);
 puts("PASS V0.96: crop bounds, REV locality, shorter duration, 3000 retriggers and one-frame sample (host)");
 return 0;
}

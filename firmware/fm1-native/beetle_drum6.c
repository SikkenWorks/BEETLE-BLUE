/* SPDX-License-Identifier: GPL-3.0-only */
#include "beetle_drum6.h"
#include <string.h>
static uint32_t next_rand(uint32_t *seed){
 uint32_t x=*seed?*seed:0xA1B2C3D4u;
 x^=x<<13;x^=x>>17;x^=x<<5;
 *seed=x;return x;
}
void bd6_project_init(struct bd6_project *p){
 if(!p)return;
 memset(p,0,sizeof(*p));p->bpm=120u;p->song_count=1u;
 p->song[0].pattern=0u;p->song[0].repeats=1u;
 const uint8_t pad[6]={0u,1u,2u,3u,4u,5u};
 for(uint8_t i=0u;i<6u;i++){
  p->lanes[i].pad=pad[i];p->lanes[i].level=90u;
  for(uint8_t pattern=0u;pattern<BD6_PATTERNS;pattern++)
   for(uint8_t s=0u;s<BD6_STEPS;s++){
    struct bd6_step *step=&p->patterns[pattern].lane[i][s];
    step->velocity=110u;step->probability=100u;step->ratchet=1u;
   }
 }
}
int bd6_project_validate(const struct bd6_project *p){
 if(!p||p->bpm<40u||p->bpm>260u||p->swing>75u||!p->song_count||p->song_count>64u)return 0;
 for(uint8_t i=0u;i<6u;i++){
  const struct bd6_lane *lane=&p->lanes[i];
  if(lane->pad>=BD6_TOTAL_PADS||lane->level>120u||lane->pan< -100||lane->pan>100||lane->mute>1u||lane->choke_group>7u)return 0;
  for(uint8_t pat=0u;pat<BD6_PATTERNS;pat++)
   for(uint8_t s=0u;s<BD6_STEPS;s++){
    const struct bd6_step *st=&p->patterns[pat].lane[i][s];
    if(st->active>1u||st->velocity>127u||st->probability>100u||st->ratchet<1u||st->ratchet>BD6_MAX_RATCHET||st->micro>7u||st->pitch_lock< -24||st->pitch_lock>24)return 0;
   }
 }
 for(uint8_t i=0u;i<p->song_count;i++)if(p->song[i].pattern>=BD6_PATTERNS||!p->song[i].repeats||p->song[i].repeats>16u)return 0;
 return 1;
}
uint8_t bd6_schedule_step(const struct bd6_project *p,uint8_t pattern,uint8_t row,
                         uint32_t *rng_state,struct bd6_batch *out){
 if(!out)return 0u;
 out->count=0u;out->step_frames=0u;
 if(!p||!rng_state||pattern>=BD6_PATTERNS||row>=BD6_STEPS||p->bpm<40u||p->bpm>260u||p->swing>75u)return 0u;
 const uint32_t base=661500u/p->bpm; /* 44,100 Hz * 60 / 4 = 661,500 */
 /* Pairwise complementary timings, with a common fixed audio clock. */
 const uint32_t swing=(uint32_t)((uint64_t)base*p->swing/100u);
 const uint32_t frames=(row&1u)?base-swing:base+swing;
 out->step_frames=frames;
 for(uint8_t c=0u;c<BD6_CHANNELS;c++){
  const struct bd6_lane *l=&p->lanes[c];
  const struct bd6_step *s=&p->patterns[pattern].lane[c][row];
  if(!s->active||l->mute||!l->level||s->probability>100u||!s->probability||s->ratchet<1u||s->ratchet>4u||s->micro>7u)continue;
  if(next_rand(rng_state)%100u>=s->probability)continue;
  /* Clamp ratchet positions inside this exact audio step. */
  const uint32_t late=(frames*(uint32_t)s->micro)/64u;
  uint8_t vel=(uint8_t)(((uint16_t)s->velocity*l->level+60u)/120u);
  if(vel>127u)vel=127u;
  if(!vel)continue;
  for(uint8_t r=0u;r<s->ratchet;r++){
   struct bd6_event *ev=&out->events[out->count++];
   ev->frame_offset=late+(uint32_t)(((uint64_t)(frames-late)*r)/s->ratchet);
   if(ev->frame_offset>=frames)ev->frame_offset=frames-1u;
   ev->channel=c;ev->pad=l->pad;ev->velocity=vel;
   ev->choke_group=l->choke_group;ev->pitch_lock=s->pitch_lock;
  }
 }
 /* Stable, bounded insertion sort: 24 maximum, not used in audio sample loop. */
 for(uint8_t i=1u;i<out->count;i++){
  struct bd6_event key=out->events[i];uint8_t j=i;
  while(j>0u&&out->events[j-1u].frame_offset>key.frame_offset){out->events[j]=out->events[j-1u];j--;}
  out->events[j]=key;
 }
 return out->count;
}

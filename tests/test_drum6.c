#include "../beetle_drum6.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void){
 struct bd6_project *p=malloc(sizeof(*p));assert(p);bd6_project_init(p);assert(bd6_project_validate(p));
 uint32_t seed=13579;struct bd6_batch a,b;
 p->patterns[0].lane[0][0]=(struct bd6_step){1,120,100,4,7,-8};
 p->patterns[0].lane[1][0]=(struct bd6_step){1,100,100,2,0,6};
 assert(bd6_schedule_step(p,0,0,&seed,&a)==6);
 assert(a.step_frames==5512); /* 661500 /120 */
 for(int i=0;i<a.count;i++){
  assert(a.events[i].frame_offset<a.step_frames);
  if(i)assert(a.events[i].frame_offset>=a.events[i-1].frame_offset);
 }
 uint32_t seed2=13579;
 assert(bd6_schedule_step(p,0,0,&seed2,&b)==6);
 assert(b.count==a.count);
 for(int i=0;i<a.count;i++)assert(a.events[i].frame_offset==b.events[i].frame_offset);
 p->swing=50;seed=3;
 bd6_schedule_step(p,0,0,&seed,&a);bd6_schedule_step(p,0,1,&seed,&b);
 assert(a.step_frames+b.step_frames==2u*(661500u/120u));
 p->lanes[0].mute=1;seed=3;
 assert(bd6_schedule_step(p,0,0,&seed,&a)==2);
 p->lanes[0].mute=0;p->patterns[0].lane[0][0].probability=0;seed=3;
 assert(bd6_schedule_step(p,0,0,&seed,&a)==2);
 p->patterns[0].lane[0][0].ratchet=5;
 assert(!bd6_project_validate(p));
 p->patterns[0].lane[0][0].ratchet=4;
 assert(bd6_project_validate(p));
 /* 48 patterns x six lanes, no event buffer overflow, thousands of triggers. */
 for(int pattern=0;pattern<48;pattern++)for(int ch=0;ch<6;ch++)for(int s=0;s<16;s++)
  p->patterns[pattern].lane[ch][s]=(struct bd6_step){1,110,100,4,7,0};
 assert(bd6_project_validate(p));
 uint64_t frames=0,events=0;
 for(int pat=0;pat<48;pat++)for(int st=0;st<16;st++){
  seed=(uint32_t)(pat+1);assert(bd6_schedule_step(p,pat,st,&seed,&a)<=BD6_MAX_EVENTS);
  for(int i=0;i<a.count;i++)assert(a.events[i].frame_offset<a.step_frames);
  frames+=a.step_frames;events+=a.count;
 }
 assert(events==48ull*16ull*24ull);
 printf("PASS drum6: %llu events, %llu frames, sizeof(project)=%zu bytes\n",
        (unsigned long long)events,(unsigned long long)frames,sizeof(*p));
 free(p);return 0;
}

/* GPL-3.0-only. Compare the old ready+render path against single-pass PCM.
 * Simulate moving 128-frame windows and deliberately delayed refill. */
#include "beetle_bd4_prefetch.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static struct beetle_bd4_stream a,b;
static void set_window(struct beetle_bd4_window *w,uint32_t from){
 w->first_frame=from;w->count=128u;
 for(unsigned i=0;i<128u;i++)w->pcm[i]=(int16_t)((int32_t)(((from+i)*729u)%42000u)-21000);
 w->valid=1u;
}
int main(void){
 struct beetle_bd4_stream_voice *x=&a.voices[0];
 x->configured=1u;x->playing=1u;x->verified_header=1u;
 x->frame_count=384u;x->end_frame=384u;
 set_window(&x->windows[0],0u);set_window(&x->windows[1],128u);
 b=a;
 unsigned pending=0,frames=0;
 for(unsigned step=0;step<1500u;step++){
  if(step==525u){
   set_window(&a.voices[0].windows[0],256u);
   set_window(&b.voices[0].windows[0],256u);
  }
  int16_t old_pcm=0,new_pcm=0;
  int before=beetle_bd4_stream_ready_now(&a,0);
  int fast=beetle_bd4_stream_read_one_ready(&b,0,&new_pcm);
  assert(fast==before);
  if(before==1){
   beetle_bd4_stream_render(&a,0,&old_pcm,1u);
   assert(new_pcm==old_pcm);
   frames++;
  }else if(before==0)pending++;
  if(before>=0 && memcmp(&a.voices[0],&b.voices[0],sizeof(*x))!=0){fprintf(stderr,"MISMATCH step %u frames %u pos A %u B %u, pending %u\n",step,frames,a.voices[0].output_frames,b.voices[0].output_frames,pending);return 5;}
  if(before<0)break;
 }
 assert(frames==768u&&pending>0u);
 printf("PASS single-pass PCM matches legacy 768 frames, %u deferred frames, exact voice state\n",pending);
 return 0;
}

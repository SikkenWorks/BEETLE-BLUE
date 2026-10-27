#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "beetle_bd4_prefetch.h"
#define FRAMES 1024
#define NBL (FRAMES/256)
static uint8_t blob[16+NBL*134];
static int read_cb(void *ctx,uint8_t slot,uint32_t off,uint8_t *dst,uint16_t n){
 (void)ctx;(void)slot;if(off+n>sizeof(blob))return 0;
 memcpy(dst,blob+off,n);return 1;
}
static void make_blob(void){
 memset(blob,0,sizeof(blob));memcpy(blob,"BD4S",4);
 blob[4]=1;blob[5]=1;blob[6]=0x22;blob[7]=0x56; /* 22050 */
 blob[8]=0;blob[9]=4; /* 1024 */
 for(unsigned b=0;b<NBL;b++){
  unsigned at=16+b*134;
  int16_t val=(int16_t)(1000+b*1000);
  blob[at]=(uint8_t)val;blob[at+1]=(uint8_t)(val>>8);
  blob[at+2]=16;blob[at+4]=0;blob[at+5]=1; /* 256 frames */
 }
}
int main(void){
 make_blob();
 struct beetle_bd4_stream s;beetle_bd4_stream_reset(&s);
 assert(beetle_bd4_stream_prepare(&s,0,0,sizeof(blob))==0);
 beetle_bd4_stream_trim(&s,0,50,75);
 assert(beetle_bd4_prefetch_poll(&s,read_cb,0,2)==2);
 assert(s.voices[0].start_frame==512);
 assert(s.voices[0].end_frame==768);
 assert(s.voices[0].source_offset>=16+2*134);
 assert(beetle_bd4_stream_start(&s,0)==1);
 assert(s.voices[0].output_frames==1024);
 int16_t out[128];beetle_bd4_stream_render(&s,0,out,128);
 assert(out[0]==3000);
 for(unsigned i=0;i<7;i++){
  (void)beetle_bd4_prefetch_poll(&s,read_cb,0,2);
  beetle_bd4_stream_render(&s,0,out,128);
 }
 assert(beetle_bd4_stream_ready_now(&s,0)==-1); /* past END */
 puts("PASS: source seek to trim START and bounded END in PCM stream");
 return 0;
}

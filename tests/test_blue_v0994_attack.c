/* GPL-3.0-only. Actual Storage V2 -> BD4S -> decoded attack caching.
 * This exercises the live bridge, not a separate mock synthesis engine. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "beetle_bd4_live.h"
#define V2LEN (288u*1024u)
static uint8_t volume[V2LEN];
static uint32_t reads, read_during_dma;
static uint8_t in_dma;
static int io(void *ctx,uint32_t physical,uint8_t *out,uint32_t count){
 (void)ctx;
 if(in_dma)read_during_dma++;
 if(physical<BEETLE_V2_BASE||count>V2LEN||
    physical-BEETLE_V2_BASE>V2LEN-count)return -1;
 memcpy(out,volume+physical-BEETLE_V2_BASE,count);reads++;return 0;
}
static void lock(void){}static void unlock(void){}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 FILE *f=fopen(argv[1],"rb");if(!f)return 3;
 assert(fread(volume,1,V2LEN,f)==V2LEN);fclose(f);
 static uint8_t workspace[38000];
 beetle_bd4_live_bind(workspace,sizeof(workspace));
 beetle_bd4_live_poll(io,0,lock,unlock);
 for(unsigned i=0;i<13;i++)assert(beetle_bd4_live_available((uint8_t)i));
 /* Idle cache warming: all thirteen installed user WAVs become hot. */
 for(unsigned k=0;k<52;k++)beetle_bd4_live_warm_idle();
 uint32_t cold_reads=reads;assert(beetle_bd4_live_hot_mask()==0x1fffu);
 /* 6 voices x 13 slots, 1000 fast retriggers, no first-block read.
  * Playing 96 frames ensures the cached attack is real PCM, not silence. */
 uint32_t nonzero=0;
 for(unsigned n=0;n<1000;n++){
  uint8_t voice=(uint8_t)(n%6),slot=(uint8_t)(n%13);
  uint32_t seq=n+1u;
  beetle_bd4_live_request(voice,slot,seq);
  uint32_t before=reads;
  beetle_bd4_live_poll(io,0,lock,unlock);
  if(reads!=before){fprintf(stderr,"HOT MISS n=%u slot=%u voice=%u read=%u\n",n,slot,voice,reads-before);return 10;} /* no flash during hot retrigger */
  for(unsigned j=0;j<96;j++){
   int16_t sample=0;
   in_dma=1u;int state=beetle_bd4_live_next(voice,seq,&sample);in_dma=0u;
   assert(state==1);if(sample)nonzero++;
  }
 }
 assert(read_during_dma==0u && nonzero>10000u);
 assert(beetle_bd4_live_underruns()==0u);
 printf("PASS 13 resident BD4S attack caches, 1000 kit/slot changes, "
        "96000 PCM frames, 0 hot-trigger flash reads, 0 DMA reads, "
        "0 underruns; cache warm took %u flash callbacks\n",cold_reads);
 return 0;
}

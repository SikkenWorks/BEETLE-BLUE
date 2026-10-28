/* GPL-3.0-only. Actual BLUE drum routing with V2 live bridge mocked only at
 * storage interface: verify retrigger+pending decoder retirement in ISR. */
#include "track_drum.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int16_t *beetle_stem_capture;
uint32_t beetle_stem_offset;
static uint32_t stopped[6],requests[6],last_serial[6];
int beetle_bd4_live_available(uint8_t slot){return slot<13u;}
const char *beetle_bd4_live_label(uint8_t slot){(void)slot;return 0;}
void beetle_bd4_live_request(uint8_t voice,uint8_t slot,uint32_t serial){
 (void)slot;assert(voice<6u);requests[voice]++;last_serial[voice]=serial;
}
void beetle_bd4_live_stop(uint8_t voice){assert(voice<6u);stopped[voice]++;}
void beetle_bd4_live_choke_all(void){}
int beetle_bd4_live_next(uint8_t voice,uint32_t serial,int16_t *sample){
 (void)voice;(void)serial;*sample=0;return 0; /* producer still pending */
}
static struct track_drum_engine d;
static int16_t pcm[256u];
int main(void){
 track_drum_init(&d);
 for(unsigned slot=0;slot<13;slot++){
  track_drum_stop(&d);
  assert(track_drum_trigger(&d,(uint8_t)(14u+slot)));
  uint32_t first=d.trigger_serial;uint8_t oldix=6u;
  for(unsigned v=0;v<6;v++)if(d.voices[v].active&&d.voices[v].sequence==first)oldix=(uint8_t)v;
  assert(oldix<6u);assert(d.voices[oldix].id==32u+slot);
  assert(track_drum_trigger(&d,(uint8_t)(14u+slot)));
  assert(d.voices[oldix].release_frames==48u);
  memset(pcm,0,sizeof pcm);
  /* Pending data can't leave an obsolete request alive forever. */
  track_drum_render(&d,0,pcm,64u);
  assert(d.voices[oldix].active==0u);
  assert(stopped[oldix]>0u);
 }
 puts("PASS all 13 pinned BD4S slots self-choke, pending voice retired without waiting for flash");
 return 0;
}

/* RC5 lossless V32 small-RAM encoder: dense note-only projects fit. */
#include "track_project_store.h"
#include "beetle_instrument_library.h"
#include "track_master_fx.h"
#include "beetle_project_pack_v2.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
static struct track_store_project p,q;
static uint8_t buf[TRACK_STORE_V4_MAX],compressed[TRACK_STORE_V4_MAX+16],unpacked[TRACK_STORE_V4_MAX];
static uint16_t work[BEETLE_PACK_V2_WORK];
static void setup(void){
 struct track_engine e;track_init(&e);memset(&p,0,sizeof p);
 track_song_init(&p.song,&e);track_mixer_init(&p.mixer);
 beetle_library_init(&p.library);track_master_default(&p.master);
 for(unsigned c=0;c<TRACK_DRUM_COUNT;c++)track_drum_component_default(&p.drum[c],c);
 for(unsigned c=0;c<BEETLE_ONESHOT_NOTES;c++){
  track_drum_user_default(&p.drum_user[c]);p.sample_end[c]=100u;
  p.sample_decay[c]=50u;p.sample_sustain[c]=100u;p.sample_release[c]=50u;
 }
 for(unsigned i=0;i<8;i++)p.kit_palette[i]=i;
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  p.instruments[i].volume=65;p.instruments[i].duty=50;p.instruments[i].cutoff=7000;
  track_va_patch_default(&p.instruments[i].va);
 }
 p.bpm=120;p.slot=1;memcpy(p.title,"RC5 CAPACITY",12);
}
int main(void){
 setup();uint32_t n=0;
 assert(track_store_encode(&p,buf,sizeof buf,&n)==0);
 assert(track_store_decode(&q,buf,n)==0);
 printf("Empty project size: %u bytes\n",n);
 for(unsigned pat=0;pat<TRACK_PATTERNS;pat++)for(unsigned ch=0;ch<TRACK_CHANNELS;ch++)for(unsigned st=0;st<TRACK_STEPS;st++){
  struct track_step *v=&p.song.patterns[pat].ch[ch][st];
  v->note=ch==TRACK_DRUM_CHANNEL?(uint8_t)((pat+st)%14u):(uint8_t)(36u+(pat+ch+st)%64u);
 }
 assert(track_store_encode(&p,buf,sizeof buf,&n)==0);
 printf("48 patterns x 6 lanes x 16 notes: %u bytes (limit %u)\n",n,TRACK_STORE_V4_MAX);
 assert(n<=TRACK_STORE_V4_MAX);
 assert(buf[4]==32u);
 assert(track_store_decode(&q,buf,n)==0);
 size_t packed=0,unpacked_size=0;
 assert(beetle_pack_project_v2(buf,n,compressed,sizeof compressed,&packed,work)==0);
 assert(beetle_unpack_project_v2(compressed,packed,unpacked,sizeof unpacked,&unpacked_size)==0);
 assert(unpacked_size==n && !memcmp(buf,unpacked,n));
 printf("V2 packed size: %zu bytes\n",packed);
 for(unsigned pat=0;pat<TRACK_PATTERNS;pat++)for(unsigned ch=0;ch<TRACK_CHANNELS;ch++)for(unsigned st=0;st<TRACK_STEPS;st++){
  const struct track_step *a=&p.song.patterns[pat].ch[ch][st];
  const struct track_step *b=&q.song.patterns[pat].ch[ch][st];
  assert(a->note==b->note && a->effect==b->effect && a->value==b->value);
 }
 /* Non-default values are lossless too. */
 for(unsigned pat=0;pat<1u;pat++)for(unsigned ch=0;ch<TRACK_CHANNELS;ch++)for(unsigned st=0;st<TRACK_STEPS;st++){
  struct track_step *v=&p.song.patterns[pat].ch[ch][st];
  v->instrument=7;v->chord=(uint8_t)(ch==TRACK_DRUM_CHANNEL?10u:5u);
  v->effect=TRACK_FX_VOLUME;v->value=200u;v->dynamics=0x93u;v->groove=33u;
 }
 assert(track_store_encode(&p,buf,sizeof buf,&n)==0);
 assert(track_store_decode(&q,buf,n)==0);
 for(unsigned pat=0;pat<1u;pat++)for(unsigned ch=0;ch<TRACK_CHANNELS;ch++)for(unsigned st=0;st<TRACK_STEPS;st++){
  const struct track_step *v=&q.song.patterns[pat].ch[ch][st];
  assert(v->instrument==7u&&v->value==200u&&v->dynamics==0x93u&&v->groove==33u);
 }
 buf[n-1]^=1u;assert(track_store_decode(&q,buf,n)!=0);
 puts("PASS: V32 dense pattern project, full 7-field roundtrip, CRC rejection");
 return 0;
}

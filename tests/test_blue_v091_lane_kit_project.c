/* V0.91 BDR1 codec test. Compile with both V29 and V30 sources. GPL-3.0-only. */
#include "track_project_store.h"
#include "track_chip.h"
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_instrument_library.h"
#include "track_master_fx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static struct track_store_project a,b;
static uint8_t buf[TRACK_STORE_V4_MAX];
static void defaults(void){
 struct track_engine engine;track_init(&engine);
 memset(&a,0,sizeof(a));track_song_init(&a.song,&engine);track_mixer_init(&a.mixer);
 beetle_library_init(&a.library);track_master_default(&a.master);
 for(unsigned k=0;k<TRACK_DRUM_COUNT;k++)track_drum_component_default(&a.drum[k],(uint8_t)k);
 for(unsigned k=0;k<BEETLE_ONESHOT_NOTES;k++){
  track_drum_user_default(&a.drum_user[k]);
  a.sample_end[k]=100;a.sample_decay[k]=50;a.sample_sustain[k]=100;a.sample_release[k]=50;
 }
 for(unsigned k=0;k<8;k++)a.kit_palette[k]=k;
 for(unsigned k=0;k<TRACK_CHIP_SLOTS;k++){
  a.instruments[k].volume=65;a.instruments[k].duty=50;
  a.instruments[k].cutoff=7000;track_va_patch_default(&a.instruments[k].va);
 }
 a.bpm=120;a.slot=2;memcpy(a.title,"SIX LANE KIT",12);
 a.song.patterns[0].ch[0][0].note=0;
 a.song.patterns[0].ch[4][8].note=1;
}
int main(int argc,char **argv){
 assert(argc==2);defaults();uint32_t n=0;
#ifdef BUILD_OLD
 assert(TRACK_STORE_VERSION==29u);
 assert(track_store_encode(&a,buf,sizeof buf,&n)==0);
 FILE *f=fopen(argv[1],"wb");assert(f);assert(fwrite(buf,1,n,f)==n);fclose(f);
 printf("PASS legacy BDR1 V29 fixture (%u bytes)\n",n);
#else
 assert(TRACK_STORE_VERSION==30u);
 FILE *f=fopen(argv[1],"rb");assert(f);size_t m=fread(buf,1,sizeof buf,f);fclose(f);
 assert(m>50 && buf[4]==29);
 assert(track_store_decode(&b,buf,(uint32_t)m)==0);
 for(unsigned i=0;i<TRACK_CHANNELS;i++)assert(b.lane_kit[i]==0);
 const uint8_t kits[6]={0,1,2,5,10,13};
 memcpy(b.lane_kit,kits,6);
 assert(track_store_encode(&b,buf,sizeof buf,&n)==0);
 assert(n<=TRACK_STORE_V4_MAX && buf[4]==30);
 memset(&a,0,sizeof a);
 assert(track_store_decode(&a,buf,n)==0);
 assert(memcmp(a.lane_kit,kits,6)==0);
 assert(a.song.patterns[0].ch[0][0].note==0);
 assert(a.song.patterns[0].ch[4][8].note==1);
 buf[n-17]^=8;assert(track_store_decode(&a,buf,n)!=0);
 buf[n-17]^=8;b.lane_kit[3]=14;assert(track_store_encode(&b,buf,sizeof buf,&n)!=0);
 printf("PASS V29 migration, six lane-kit IDs, V30 CRC+range rejection, pattern integrity (%u bytes)\n",n);
#endif
 return 0;
}

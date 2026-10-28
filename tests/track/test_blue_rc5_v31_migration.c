/* Compile once against RC4(V31) with MAKE_LEGACY to create fixture;
 * compile against RC5(V32) to assert legacy decode and rewrite. */
#include "track_project_store.h"
#include "beetle_instrument_library.h"
#include "track_master_fx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static struct track_store_project a,b;
static uint8_t buffer[TRACK_STORE_V4_MAX];
static void defaults(void){
 struct track_engine e;track_init(&e);memset(&a,0,sizeof a);
 track_song_init(&a.song,&e);track_mixer_init(&a.mixer);
 beetle_library_init(&a.library);track_master_default(&a.master);
 for(unsigned c=0;c<TRACK_DRUM_COUNT;c++)track_drum_component_default(&a.drum[c],c);
 for(unsigned c=0;c<BEETLE_ONESHOT_NOTES;c++){
  track_drum_user_default(&a.drum_user[c]);a.sample_end[c]=100u;
  a.sample_decay[c]=50u;a.sample_sustain[c]=100u;a.sample_release[c]=50u;
 }
 for(unsigned i=0;i<8;i++)a.kit_palette[i]=i;
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  a.instruments[i].volume=65;a.instruments[i].duty=50;a.instruments[i].cutoff=7000;
  track_va_patch_default(&a.instruments[i].va);
 }
 a.bpm=132u;a.slot=2;memcpy(a.title,"LEGACY V31  ",12);
 a.lane_kit[0]=1u;a.lane_kit[1]=0u;a.lane_kit[5]=1u;
 a.song.patterns[1].ch[0][3].note=44u;
 a.song.patterns[1].ch[0][3].effect=TRACK_FX_VOLUME;
 a.song.patterns[1].ch[0][3].value=200u;
 a.song.patterns[4].ch[5][9].note=6u;
}
int main(int argc,char **argv){
 assert(argc==2);FILE *f;
#ifdef MAKE_LEGACY
 assert(TRACK_STORE_VERSION==31u);
 defaults();uint32_t n=0;
 assert(track_store_encode(&a,buffer,sizeof buffer,&n)==0);
 f=fopen(argv[1],"wb");assert(f);
 assert(fwrite(buffer,1,n,f)==n);fclose(f);
 printf("V31 fixture %u bytes\n",n);
#else
 assert(TRACK_STORE_VERSION==32u);
 f=fopen(argv[1],"rb");assert(f);size_t n=fread(buffer,1,sizeof buffer,f);fclose(f);
 assert(buffer[4]==31u);
 assert(track_store_decode(&a,buffer,(uint32_t)n)==0);
 assert(a.bpm==132u && a.lane_kit[0]==1u && a.lane_kit[5]==1u);
 assert(a.song.patterns[1].ch[0][3].note==44u);
 assert(a.song.patterns[1].ch[0][3].value==200u);
 assert(a.song.patterns[4].ch[5][9].note==6u);
 uint32_t used=0;
 assert(track_store_encode(&a,buffer,sizeof buffer,&used)==0);
 assert(buffer[4]==32u);
 assert(track_store_decode(&b,buffer,used)==0);
 assert(b.bpm==132u&&b.lane_kit[0]==1u&&b.lane_kit[5]==1u);
 assert(b.song.patterns[1].ch[0][3].value==200u);
 assert(b.song.patterns[4].ch[5][9].note==6u);
 puts("PASS V31 read + V32 save/reload, kit selection and FX/note integrity");
#endif
 return 0;
}

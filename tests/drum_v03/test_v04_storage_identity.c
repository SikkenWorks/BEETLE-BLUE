/* BEETLE DRUM V0.4 project identity and edited-kit persistence. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_project_store.h"
#include "beetle_instrument_library.h"
#include "track_master_fx.h"
static struct track_store_project a,b;
static uint8_t encoded[TRACK_STORE_V4_MAX];
int main(void){
 struct track_engine e;track_init(&e);
 track_song_init(&a.song,&e);track_mixer_init(&a.mixer);
 track_master_default(&a.master);beetle_library_init(&a.library);
 for(unsigned x=0;x<TRACK_DRUM_COUNT;x++)track_drum_timbre_default(&a.drum[x]);
 for(unsigned x=0;x<BEETLE_ONESHOT_NOTES;x++)track_drum_user_default(&a.drum_user[x]);
 for(unsigned x=0;x<TRACK_CHIP_SLOTS;x++){
  a.instruments[x].volume=65;a.instruments[x].duty=50;a.instruments[x].cutoff=7000;
  track_va_patch_default(&a.instruments[x].va);
 }
 a.bpm=124;a.slot=4;a.drum_kit=9;
 memcpy(a.title,"DRUM PROJECT",12);
 a.song.patterns[0].ch[0][0].note=0;
 a.song.patterns[0].ch[1][2].note=14;
 a.song.patterns[0].ch[5][4].note=10;
 a.song.patterns[0].note3[4]=26;
 a.drum[0].decay=83;a.drum[0].pitch=-8;
 uint32_t n=0;
 assert(track_store_encode(&a,encoded,sizeof(encoded),&n)==0);
 assert(n<TRACK_STORE_V4_MAX);
 assert(memcmp(encoded,"BDR1",4)==0);
 assert(encoded[20]==9 && memcmp(encoded+21,"DR4",3)==0);
 assert(track_store_decode(&b,encoded,n)==0);
 assert(b.slot==4 && b.drum_kit==9 && b.bpm==124);
 assert(b.drum[0].decay==83 && b.drum[0].pitch==-8);
 assert(b.song.patterns[0].ch[1][2].note==14);
 assert(b.song.patterns[0].note3[4]==26);
 /* Foreign vanilla BEETLE project with a perfect header CRC still refused. */
 encoded[0]='T';encoded[1]='R';encoded[2]='K';encoded[3]='1';
 assert(track_store_decode(&b,encoded,n)!=0);
 puts("PASS DRUM V0.4: BDR1 identity; kit/14+ user pads/NOTE3 persistence; foreign TRK1 blocked");
 return 0;
}

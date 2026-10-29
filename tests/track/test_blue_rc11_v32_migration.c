/* RC11 reads real RC10 V32 BDR1 records; no on-flash migration required. */
#include "track_project_store.h"
#include <assert.h>
#include <stdio.h>
static struct track_store_project old,newer;
static unsigned char data[TRACK_STORE_V4_MAX];
int main(int argc,char **argv){
 assert(argc==2);FILE*f=fopen(argv[1],"rb");assert(f);
 size_t n=fread(data,1,sizeof data,f);fclose(f);
 assert(data[4]==32u && n>100u);
 assert(track_store_decode(&old,data,(uint32_t)n)==0);
 assert(old.bpm==128u&&old.slot==1u);
 assert(old.song.patterns[2].length==8u);
 for(unsigned c=0;c<TRACK_CHANNELS;c++)assert(track_lane_length(&old.song.patterns[2],c)==8u);
 assert(old.song.patterns[2].ch[0][1].note==4u);
 assert(old.song.patterns[2].ch[5][7].note==7u);
 assert(old.song.patterns[2].note2_fx[2].value==65u);
 unsigned used=0u;assert(track_store_encode(&old,data,sizeof data,&used)==0);
 assert(data[4]==33u);
 assert(track_store_decode(&newer,data,used)==0);
 assert(newer.song.patterns[2].length==8u);
 for(unsigned c=0;c<TRACK_CHANNELS;c++)assert(track_lane_length(&newer.song.patterns[2],c)==8u);
 assert(newer.song.patterns[2].ch[5][7].note==7u);
 puts("PASS RC11: authentic RC10 V32 project loads, legacy lengths propagate to all lanes, saves V33, reloads intact");
}

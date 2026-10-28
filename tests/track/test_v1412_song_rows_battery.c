#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../firmware/fm1-native/beetle_song_rows.h"
#include "../../firmware/fm1-native/beetle_battery_policy.h"
int main(void){
 struct track_song_bank song={0};
 struct beetle_song_rows_clip clip={0};
 song.count=6u;
 for(unsigned i=0;i<6;i++){
  song.chain[i].repeats=(uint8_t)(i+1u);
  for(unsigned c=0;c<TRACK_CHANNELS;c++)song.chain[i].lane[c]=(uint8_t)(c==1?255u:(128+i));
 }
 assert(beetle_song_rows_in_range(2,4,2));
 assert(!beetle_song_rows_in_range(5,4,2));
 assert(beetle_song_rows_copy(&clip,&song,4u,2u)==3);
 assert(clip.count==3u && clip.rows[0].repeats==3u && clip.rows[2].repeats==5u);
 /* Copy 3 rows (old 3,4,5) and insert AFTER row 2, leaving it intact. */
 struct track_song_entry row2=song.chain[2];
 struct track_song_entry row5=song.chain[5];
 song.position=5;
 assert(beetle_song_rows_paste(&clip,&song,2u)==3);
 assert(song.count==9u && song.edit_row==3u && song.position==8u);
 assert(!memcmp(&song.chain[2],&row2,sizeof(row2)));
 assert(song.chain[3].repeats==3u&&song.chain[4].repeats==4u&&song.chain[5].repeats==5u);
 assert(song.chain[5].lane[1]==TRACK_SONG_SILENT);
 assert(!memcmp(&song.chain[8],&row5,sizeof(row5)));
 /* Inserting after the final position APPENDS, never overwrites it. */
 assert(beetle_song_rows_paste(&clip,&song,8u)==3);
 assert(song.count==12u && song.chain[8].repeats==6u);
 assert(song.chain[9].repeats==3u && song.chain[11].repeats==5u);
 /* Preflight failure on insufficient remaining space leaves all bytes intact. */
 struct track_song_bank *snap=(struct track_song_bank*)malloc(sizeof(song));
 assert(snap);memcpy(snap,&song,sizeof(song));
 assert(beetle_song_rows_paste(&clip,&song,12u)<0);
 assert(!memcmp(snap,&song,sizeof(song)));
 for(unsigned i=0;i<64;i++){
  song.chain[i].repeats=(uint8_t)((i%16u)+1u);
  song.chain[i].lane[0]=(uint8_t)(i%TRACK_PATTERNS);
 }
 song.count=64u;
 memcpy(snap,&song,sizeof(song));
 assert(beetle_song_rows_copy(&clip,&song,0,63)==64);
 assert(beetle_song_rows_paste(&clip,&song,0)<0);
 assert(!memcmp(snap,&song,sizeof(song)));
 free(snap);
 /* LCD must switch red only under threshold and recover with hysteresis. */
 assert(beetle_battery_filter(600,500,0)==500);
 assert(beetle_battery_filter(600,500,1)==588);
 assert(!beetle_battery_low20(800u,0u));
 assert(beetle_battery_low20(BEETLE_BATTERY_20_ADC_RAW,0u));
 assert(beetle_battery_low20((uint16_t)(BEETLE_BATTERY_20_ADC_RAW+10u),1u));
 assert(!beetle_battery_low20((uint16_t)(BEETLE_BATTERY_20_ADC_RAW+25u),1u));
 puts("PASS V1.0 RC1: song row insertion after cursor, preserved tails, capacity atomicity, battery policy");
 return 0;
}

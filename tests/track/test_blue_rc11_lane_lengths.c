/* BEETLE BLUE RC11: independent lane length, groove, song and BDR1 V33 codec.
 * Host regressions only; not evidence of physical FM1 safety. GPL-3.0-only. */
#include "track_project_store.h"
#include "track_master_fx.h"
#include "beetle_instrument_library.h"
#include "beetle_step_groove.h"
#include "beetle_pattern_edit.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static struct track_store_project a,b;
static uint8_t wire[TRACK_STORE_V4_MAX];
static void defaults(void){
 memset(&a,0,sizeof a);struct track_engine e;track_init(&e);
 track_song_init(&a.song,&e);track_mixer_init(&a.mixer);track_master_default(&a.master);
 beetle_library_init(&a.library);
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++)track_drum_component_default(&a.drum[i],i);
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++){
  track_drum_user_default(&a.drum_user[i]);a.sample_end[i]=100u;
  a.sample_decay[i]=50u;a.sample_sustain[i]=100u;a.sample_release[i]=50u;
 }
 for(unsigned i=0;i<8;i++)a.kit_palette[i]=i;
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  a.instruments[i].volume=65;a.instruments[i].duty=50;
  a.instruments[i].cutoff=7000;track_va_patch_default(&a.instruments[i].va);
 }
 a.bpm=125;a.slot=1;memcpy(a.title,"LANE LEN RC11",12);
}
int main(void){
 struct track_engine e;track_init(&e);
 assert(sizeof(struct track_step)==6u);
 assert(sizeof(struct track_pattern)==625u);
 track_set_channel(&e,0);assert(track_set_pattern_length(&e,12)==0);
 track_set_channel(&e,1);assert(track_set_pattern_length(&e,7)==0);
 track_set_channel(&e,5);assert(track_set_pattern_length(&e,1)==0);
 assert(e.active.length==16u); /* lanes 3/4/5 still 16 */
 assert(track_lane_length(&e.active,0)==12);
 assert(track_lane_length(&e.active,1)==7);
 assert(track_lane_length(&e.active,5)==1);
 assert(track_lane_row(&e.active,0,12)==0);
 assert(track_lane_row(&e.active,1,14)==0);
 assert(track_lane_row(&e.active,5,15)==0);
 e.active.ch[1][0].note=8;
 track_groove_set_micro(&e.active.ch[1][0],3);
 struct beetle_groove_clock clock={0};beetle_groove_reset(&clock);
 const uint32_t swing[6]={0};
 beetle_groove_schedule(&clock,&e.active,7u,1000u,swing);
 assert(clock.lane[1].at==46u); /* micro-delay repeats from row 0 */
 uint8_t patbytes[TRACK_PATTERN_BYTES];
 assert(track_pattern_encode(&e.active,patbytes,sizeof patbytes)==TRACK_PATTERN_BYTES);
 struct track_pattern restored;assert(patbytes[2]==6);
 assert(track_pattern_decode(&restored,patbytes,sizeof patbytes)==0);
 assert(track_lane_length(&restored,0)==12&&track_lane_length(&restored,1)==7);
 assert(track_lane_length(&restored,5)==1 && restored.ch[1][0].note==8);
 /* Changes within one lane never alter another lane's content or loop. */
 track_set_channel(&e,2);assert(track_set_pattern_length(&e,16)==0);
 assert(track_lane_length(&e.active,1)==7);
 /* All-short patterns must be extendable without lengthening other lanes. */
 {struct track_engine shorte;track_init(&shorte);
  for(unsigned c=0;c<6;c++){track_set_channel(&shorte,c);assert(track_set_pattern_length(&shorte,8)==0);}
  assert(shorte.active.length==8u);
  track_set_channel(&shorte,1);assert(track_set_pattern_length(&shorte,12)==0);
  assert(shorte.active.length==12u);
  for(unsigned c=0;c<6;c++)assert(track_lane_length(&shorte.active,c)==(c==1u?12u:8u));
  track_set_channel(&shorte,1);assert(track_set_pattern_length(&shorte,16)==0);
  assert(shorte.active.length==16u);
  for(unsigned c=0;c<6;c++)assert(track_lane_length(&shorte.active,c)==(c==1u?16u:8u));
 }
 defaults();
 a.song.patterns[4]=e.active;
 a.song.patterns[4].ch[0][0].note=0;
 a.song.patterns[4].ch[1][0].note=1;
 a.song.patterns[4].ch[5][0].note=2;
 a.song.patterns[4].note2_fx[0].effect=TRACK_FX_REVERSE;
 a.song.patterns[4].note2_fx[0].value=100;
 a.song.patterns[4].note3[0]=3;
 uint32_t bytes=0;assert(track_store_encode(&a,wire,sizeof wire,&bytes)==0);
 assert(bytes<TRACK_STORE_V4_MAX&&wire[4]==33u);
 assert(track_store_decode(&b,wire,bytes)==0);
 for(int c=0;c<6;c++)assert(track_lane_length(&b.song.patterns[4],c)==track_lane_length(&a.song.patterns[4],c));
 assert(b.song.patterns[4].ch[0][0].note==0);
 assert(b.song.patterns[4].ch[1][0].note==1);
 assert(b.song.patterns[4].note2_fx[0].effect==TRACK_FX_REVERSE);
 assert(b.song.patterns[4].note3[0]==3);
 /* Song composition must repeat each independent source lane, not the maximum. */
 b.song.enabled=1;b.song.chain[0].pattern=4;
 for(int c=0;c<6;c++)b.song.chain[0].lane[c]=4;
 track_song_start(&b.song,&e);
 assert(e.active.length==16);
 assert(track_lane_length(&e.active,0)==12);
 assert(track_lane_length(&e.active,1)==7);
 assert(track_lane_length(&e.active,5)==1);
 assert(e.active.ch[0][12].note==e.active.ch[0][0].note);
 assert(e.active.ch[1][7].note==e.active.ch[1][0].note);
 assert(e.active.note3[15]==e.active.note3[0]);
 assert(e.active.note2_fx[15].effect==e.active.note2_fx[0].effect);
 track_song_restore(&b.song,&e);
 /* Destructive block paste to row 0 keeps length, even if copied from another lane. */
 struct beetle_pattern_editor ed={0};struct beetle_pattern_image pending={0},whole={0};
 struct track_step block[TRACK_STEPS]={0};
 ed.pending=&pending;ed.whole_clip=&whole;ed.block=block;
 track_set_channel(&e,1);e.pattern_slot=4;e.active=b.song.patterns[4];
 ed.first=0;ed.last=0;ed.action=0;
 assert(beetle_edit_block_execute(&ed,&b.song,&e));
 ed.action=1;ed.first=0;ed.last=0;ed.block[0].note=11u;
 assert(beetle_edit_block_execute(&ed,&b.song,&e));
 assert(track_lane_length(&e.active,1)==7);
 puts("PASS RC11: 6 independent 1..16 lane lengths, note/groove wrap, standalone & BDR1 V33 codecs, song, NOTE2/3, block paste");
 return 0;
}

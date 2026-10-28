/* BEETLE BLUE RC6: test the actual legacy save function and correct error
 * classification. All NOR operations are mock flash, NEVER real hardware. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_journal.h"
#include "track_master_fx.h"
#include "beetle_instrument_library.h"
static uint8_t flash[2][2][TRACK_JOURNAL_BANK_BYTES];
static uint8_t scratch[TRACK_STORE_V4_MAX];
static struct track_store_project p, readback;
static unsigned erased,programmed;
static int rd(void *ctx,uint8_t slot,uint8_t bank,uint32_t off,uint8_t *out,uint32_t size){
 (void)ctx;
 if(slot>=2||bank>=2||off>TRACK_JOURNAL_BANK_BYTES||size>TRACK_JOURNAL_BANK_BYTES-off)return -1;
 memcpy(out,flash[slot][bank]+off,size);return 0;
}
static int er(void *ctx,uint8_t slot,uint8_t bank){
 (void)ctx;if(slot>=2||bank>=2)return -1;
 erased++;memset(flash[slot][bank],255,TRACK_JOURNAL_BANK_BYTES);return 0;
}
static int wr(void *ctx,uint8_t slot,uint8_t bank,uint32_t off,const uint8_t *in,uint32_t size){
 (void)ctx;if(slot>=2||bank>=2||off>TRACK_JOURNAL_BANK_BYTES||size>TRACK_JOURNAL_BANK_BYTES-off)return -1;
 for(uint32_t i=0;i<size;i++)if((flash[slot][bank][off+i]&in[i])!=in[i])return -1;
 for(uint32_t i=0;i<size;i++)flash[slot][bank][off+i]&=in[i];programmed++;return 0;
}
static const struct track_journal_io io={0,rd,er,wr};
static void new_project(uint8_t slot){
 struct track_engine e;track_init(&e);memset(&p,0,sizeof p);
 track_song_init(&p.song,&e);track_mixer_init(&p.mixer);track_master_default(&p.master);
 beetle_library_init(&p.library);
 for(unsigned j=0;j<TRACK_DRUM_COUNT;j++)track_drum_timbre_default(&p.drum[j]);
 for(unsigned j=0;j<BEETLE_ONESHOT_NOTES;j++){
  track_drum_user_default(&p.drum_user[j]);p.sample_end[j]=100;
  p.sample_decay[j]=50;p.sample_sustain[j]=100;p.sample_release[j]=50;
 }
 for(unsigned j=0;j<8;j++)p.kit_palette[j]=j;
 for(unsigned j=0;j<TRACK_CHIP_SLOTS;j++){
  p.instruments[j].volume=65;p.instruments[j].duty=50;
  p.instruments[j].cutoff=7000;track_va_patch_default(&p.instruments[j].va);
 }
 p.bpm=120;p.slot=slot;memcpy(p.title,"NEW RC6 TEST",12);
}
int main(void){
 memset(flash,255,sizeof flash);
 new_project(0);
 assert(track_journal_save(&io,0,&p,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(track_journal_load(&io,0,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.slot==0&&readback.bpm==120);
 unsigned first_erases=erased,first_programmed=programmed;
 new_project(1);
 assert(track_journal_save(&io,1,&p,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(track_journal_load(&io,1,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.slot==1&&readback.bpm==120);
 p.song.patterns[0].ch[0][0].note=62;p.bpm=133;
 assert(track_journal_save(&io,1,&p,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(track_journal_load(&io,1,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.slot==1&&readback.bpm==133&&readback.song.patterns[0].ch[0][0].note==62);
 assert(track_journal_load(&io,0,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.slot==0&&readback.bpm==120);
 unsigned before_er=erased,before_pr=programmed;
 p.bpm=0;
 assert(track_journal_save(&io,1,&p,scratch,sizeof scratch,0)==TRACK_JOURNAL_INVALID);
 assert(erased==before_er&&programmed==before_pr);
 p.bpm=133;
 assert(track_journal_load(&io,1,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.bpm==133);
 /* This is a REAL size error, not a data error; refusal must be atomic. */
 for(unsigned k=0;k<TRACK_PATTERNS;k++)for(unsigned c=0;c<TRACK_CHANNELS;c++)for(unsigned x=0;x<TRACK_STEPS;x++){
  struct track_step *st=&p.song.patterns[k].ch[c][x];
  st->note=(uint8_t)(c==TRACK_DRUM_CHANNEL?(x%14u):60u);
  st->instrument=7u;st->effect=TRACK_FX_VOLUME;st->value=200u;
  st->dynamics=(uint8_t)(100u+x);st->groove=(uint8_t)x;
 }
 before_er=erased;before_pr=programmed;
 assert(track_journal_save(&io,1,&p,scratch,sizeof scratch,0)==TRACK_JOURNAL_CAPACITY);
 assert(erased==before_er&&programmed==before_pr);
 assert(track_journal_load(&io,1,&readback,scratch,sizeof scratch,0)==TRACK_JOURNAL_OK);
 assert(readback.bpm==133&&readback.song.patterns[0].ch[0][0].note==62);
 printf("PASS legacy: slot 1 saved, slot 2 new+overwrite+reload, slot 1 intact; invalid not capacity; flash safe; erases=%u programs=%u\n",erased,programmed);
 assert(first_erases==1&&first_programmed>0);
 return 0;
}

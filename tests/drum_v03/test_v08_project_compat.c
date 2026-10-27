/* BEETLE DRUM V0.6 host regression: user timbres, sparse V28 codec,
 * V26/V27 migration and cross-family choke. This is not a hardware test. */
#include "track_chip.h"
#include "track_drum.h"
#include "track_mixer.h"
#include "track_project_store.h"
#include "beetle_instrument_library.h"
#include "track_master_fx.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static struct track_store_project p,q;
static struct track_drum_engine drum;
static struct track_mixer mixer;
static uint8_t pack[TRACK_STORE_V4_MAX];
static uint8_t samples[BEETLE_ONESHOT_FIRST_BYTES];
static int16_t stereo[2*256],stems[6*256];
static struct beetle_oneshot_bank bank;
static unsigned file_read(const char *file,uint8_t *dst){
 FILE *f=fopen(file,"rb");assert(f);assert(!fseek(f,0,SEEK_END));long n=ftell(f);
 assert(n>0 && n<TRACK_STORE_V4_MAX);rewind(f);
 assert(fread(dst,1,(size_t)n,f)==(size_t)n);fclose(f);return (unsigned)n;
}
static void project_init(void){
 memset(&p,0,sizeof(p));struct track_engine e;track_init(&e);
 track_song_init(&p.song,&e);track_mixer_init(&p.mixer);
 track_master_default(&p.master);beetle_library_init(&p.library);
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++)track_drum_component_default(&p.drum[i],i);
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++)track_drum_user_default(&p.drum_user[i]);
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  p.instruments[i].volume=65;p.instruments[i].duty=50;
  p.instruments[i].cutoff=7000;track_va_patch_default(&p.instruments[i].va);
 }
 for(unsigned k=0;k<8;k++)p.kit_palette[k]=(uint8_t)k;
 for(unsigned k=0;k<BEETLE_ONESHOT_NOTES;k++){
  p.sample_start[k]=0;p.sample_end[k]=100;
  p.sample_attack[k]=0;p.sample_decay[k]=50;
  p.sample_sustain[k]=100;p.sample_release[k]=50;
 }
 p.sample_start[0]=11;p.sample_end[0]=82;
 p.sample_attack[0]=4;p.sample_decay[0]=62;
 p.sample_sustain[0]=88;p.sample_release[0]=32;
 p.kit_palette[2]=13;
 p.bpm=120;p.slot=3;p.drum_kit=6;memcpy(p.title,"DRUM USER V6",12);
}
static int64_t audio_energy(uint8_t gain,uint8_t drive){
 track_drum_init(&drum);track_mixer_init(&mixer);
 drum.user[0].gain=gain;drum.user[0].drive=drive;
 track_drum_bind_oneshots(&drum,&bank);
 memset(stereo,0,sizeof(stereo));memset(stems,0,sizeof(stems));
 beetle_stem_capture=stems;beetle_stem_offset=0;
 assert(track_drum_trigger(&drum,14));drum.voices[0].source_lane=0;
 assert(drum.voices[0].gain_q8==(256u*gain)/100u);
 track_drum_render(&drum,&mixer,stereo,256);
 int64_t sum=0;for(unsigned i=0;i<256;i++){
  int32_t x=stems[i*6];sum+=(int64_t)x*x;
  for(unsigned ch=1;ch<6;ch++)assert(stems[i*6+ch]==0);
 }
 beetle_stem_capture=0;return sum;
}
int main(int argc,char**argv){
 assert(argc==3);
 project_init();
 uint32_t def_len=0,edited_len=0;
 assert(track_store_encode(&p,pack,sizeof(pack),&def_len)==0);
 assert(pack[4]==29 && pack[5]==0);
 assert(track_store_decode(&q,pack,def_len)==0);
 for(unsigned i=0;i<13;i++){
  assert(q.drum_user[i].gain==100 && q.drum_user[i].drive==0 && q.drum_user[i].choke_group==0);
 }
 p.drum_user[0].gain=33;p.drum_user[0].drive=71;p.drum_user[0].choke_group=3;
 p.drum_user[12].gain=92;p.drum_user[12].drive=15;p.drum_user[12].choke_group=7;
 assert(track_store_encode(&p,pack,sizeof(pack),&edited_len)==0);
 assert(edited_len==def_len+6);
 assert(track_store_decode(&q,pack,edited_len)==0);
 assert(q.drum_user[0].gain==33&&q.drum_user[0].drive==71&&q.drum_user[0].choke_group==3);
 assert(q.drum_user[12].gain==92&&q.drum_user[12].drive==15&&q.drum_user[12].choke_group==7);
 assert(q.drum_user[1].gain==100&&q.drum_user[1].drive==0);
 assert(q.kit_palette[2]==13 && q.sample_start[0]==11 && q.sample_end[0]==82);
 assert(q.sample_attack[0]==4 && q.sample_decay[0]==62 && q.sample_sustain[0]==88 && q.sample_release[0]==32);
 pack[edited_len-16]^=1u;assert(track_store_decode(&q,pack,edited_len)!=0);
 p.drum_user[0].choke_group=8;assert(track_store_encode(&p,pack,sizeof(pack),&edited_len)!=0);
 unsigned old=file_read(argv[1],pack);assert(pack[4]==27);
 assert(track_store_decode(&q,pack,old)==0);
 assert(q.drum_kit==7 && q.drum[0].drive==40 && q.drum[0].choke_group==2);
 assert(q.drum_user[0].gain==100 && q.drum_user[12].gain==100);
 assert(q.sample_start[0]==0 && q.sample_end[0]==100);
 old=file_read(argv[2],pack);assert(pack[4]==26);
 assert(track_store_decode(&q,pack,old)==0);
 assert(q.drum_user[0].gain==100 && q.drum_user[6].drive==0);
 for(unsigned i=0;i<512;i++)samples[i]=(uint8_t)(i%4==0?0x00:i%4==1?0x80:i%4==2?0x1F:0x9F);
 bank.first=samples;bank.length[0]=512;
 int64_t normal=audio_energy(100,0),soft=audio_energy(25,0),driven=audio_energy(100,70);
 assert(normal>0 && soft>0 && driven>0);
 assert(normal>soft && normal!=driven);
 track_drum_init(&drum);track_drum_bind_oneshots(&drum,&bank);
 drum.components[0].choke_group=4;drum.user[0].choke_group=4;
 assert(track_drum_trigger(&drum,0));assert(drum.voices[0].release_frames==0);
 assert(track_drum_trigger(&drum,14));assert(drum.voices[0].release_frames==48);
 assert(drum.voices[1].active && drum.voices[1].release_frames==0);
 assert(track_drum_trigger(&drum,0));assert(drum.voices[1].release_frames==48);
 track_drum_stop(&drum);beetle_stem_capture=0;
 printf("PASS V0.8: user gain & drive PCM, cross sample/factory choke, V29 palette & trim save/load, V26/V27 migrations; neutral=%lld soft=%lld driven=%lld\n",(long long)normal,(long long)soft,(long long)driven);
 return 0;
}

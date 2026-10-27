/* BEETLE DRUM V0.5: native per-pad drive/choke, persistence and defaults. */
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
static struct track_drum_engine d;
static struct track_mixer m;
static int16_t stereo[2048],stems[6144];
static struct track_store_project a,b;
static uint8_t packet[TRACK_STORE_V4_MAX];
static void setup_project(void){
 struct track_engine e;track_init(&e);track_song_init(&a.song,&e);
 track_mixer_init(&a.mixer);track_master_default(&a.master);
 beetle_library_init(&a.library);
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++)track_drum_component_default(&a.drum[i],i);
 for(unsigned i=0;i<BEETLE_ONESHOT_NOTES;i++)track_drum_user_default(&a.drum_user[i]);
 for(unsigned i=0;i<TRACK_CHIP_SLOTS;i++){
  a.instruments[i].volume=65;a.instruments[i].duty=50;
  a.instruments[i].cutoff=7000;track_va_patch_default(&a.instruments[i].va);
 }
 a.bpm=120;a.slot=3;a.drum_kit=5;
 memcpy(a.title,"BLUE KIT",9);
}
int main(int argc,char **argv){
 track_drum_init(&d);track_mixer_init(&m);
 assert(d.components[0].drive==0&&d.components[0].choke_group==0);
 beetle_stem_capture=stems;beetle_stem_offset=0;
 d.components[0].drive=0;
 assert(track_drum_trigger(&d,0));d.voices[0].source_lane=0;
 track_drum_render(&d,&m,stereo,1024);
 int64_t neutral=0;for(int i=0;i<1024;i++)neutral+=stems[i*6]*stems[i*6];
 memset(stereo,0,sizeof(stereo));memset(stems,0,sizeof(stems));
 track_drum_init(&d);d.components[0].drive=75;
 assert(track_drum_trigger(&d,0));d.voices[0].source_lane=0;
 track_drum_render(&d,&m,stereo,1024);
 int64_t driven=0;for(int i=0;i<1024;i++)driven+=stems[i*6]*stems[i*6];
 assert(neutral>0 && driven>0 && neutral!=driven);
 track_drum_init(&d);d.components[0].choke_group=2;d.components[9].choke_group=2;
 assert(track_drum_trigger(&d,0));assert(d.voices[0].release_frames==0);
 assert(track_drum_trigger(&d,9));
 assert(d.voices[0].release_frames==48);assert(d.voices[1].active);
 assert(d.voices[1].release_frames==0);
 setup_project();a.drum[0].drive=75;a.drum[0].choke_group=2;
 a.drum[9].choke_group=2;
 uint32_t size=0;assert(track_store_encode(&a,packet,sizeof(packet),&size)==0);
 assert(packet[4]==TRACK_STORE_VERSION && packet[5]==0 && size<TRACK_STORE_V4_MAX);
 assert(track_store_decode(&b,packet,size)==0);
 assert(b.drum[0].drive==75 && b.drum[0].choke_group==2);
 assert(b.drum[9].choke_group==2 && b.drum_kit==5);
 packet[32]^=0x20;assert(track_store_decode(&b,packet,size)!=0);
 if(argc>1){
  FILE *f=fopen(argv[1],"rb");assert(f);
  assert(fseek(f,0,SEEK_END)==0);long n=ftell(f);assert(n>0&&n<TRACK_STORE_V4_MAX);
  rewind(f);assert(fread(packet,1,(size_t)n,f)==(size_t)n);fclose(f);
  assert(packet[4]==26);
  assert(track_store_decode(&b,packet,(uint32_t)n)==0);
  assert(b.drum_kit==9 && b.drum[0].decay==83 && b.drum[0].pitch==-8);
  assert(b.drum[0].drive==0 && b.drum[0].choke_group==0);
 }
 beetle_stem_capture=0;
 printf("PASS V0.5: drive PCM output differs; choke only matching group; BDR1 current-schema CRC/persistence, V26 backward-load\n");
}

/* SPDX-License-Identifier: GPL-3.0-only. Host WAV references from exact native drum engine. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "track_drum.h"
#include "track_mixer.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0u;
static struct track_drum_engine eng;
static struct track_mixer mx;
static int16_t buf[128*2];
static void w16(FILE *f,uint16_t v){fputc(v&255,f);fputc((v>>8)&255,f);}
static void w32(FILE *f,uint32_t v){w16(f,(uint16_t)(v&65535u));w16(f,(uint16_t)(v>>16u));}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 const char *names[]={"kick","snare_1","hihat_1","open_hihat","tom_1","tom_2","tom_3","crash","ride","clap","perc_1","perc_2","perc_3","rim","snare_2","hihat_2"};
 for(unsigned k=0;k<16;k++){
  track_drum_init(&eng);track_mixer_init(&mx);
  if(!track_drum_trigger(&eng,(uint8_t)(64u+k)))return 3;
  unsigned frames=k==7?66150u:44100u;
  char path[512];snprintf(path,sizeof(path),"%s/%s.wav",argv[1],names[k]);
  FILE *f=fopen(path,"wb");if(!f)return 4;
  fwrite("RIFF",1,4,f);w32(f,36u+frames*2u);fwrite("WAVEfmt ",1,8,f);
  w32(f,16u);w16(f,1);w16(f,1);w32(f,44100u);w32(f,88200u);w16(f,2);w16(f,16);
  fwrite("data",1,4,f);w32(f,frames*2u);
  for(unsigned offset=0;offset<frames;){
   unsigned n=frames-offset;if(n>128u)n=128u;
   memset(buf,0,sizeof buf);track_drum_render(&eng,&mx,buf,n);
   for(unsigned j=0;j<n;j++){
    int32_t a=((int32_t)buf[2*j]+buf[2*j+1])/2;
    w16(f,(uint16_t)(int16_t)a);
   }
   offset+=n;
  }
  fclose(f);
 }
 puts("PASS ROM previews from real firmware track_drum_render: 16 WAV files @44100Hz");
 return 0;
}

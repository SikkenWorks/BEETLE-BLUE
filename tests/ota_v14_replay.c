/* SPDX-License-Identifier: GPL-3.0-only
 * Host replay of real FM1 OTA app protocol, using exact packaged loader bytes.
 * Flash is simulated, never accesses a physical device. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#define FELUCCA_ID "FM-1_2224"
#define FELUCCA_OTA_DRYRUN 0
#include "ota.c"
static uint8_t flash[OTA_AREA_LEN], reply[800];
static uint8_t *image;
static size_t image_len;
static uint32_t pending,clock_ms, erases, programs, commits, known=1;
static int ota_wire_send(const uint8_t *p,uint32_t n){
 uint8_t d[64], ans[528];uint32_t k=ota_unpack7(p+1,n-2,d,sizeof d);
 assert(k>=7);
 if(d[2]!=0x30)return 0;
 uint32_t addr=ota_rd32(d+7),len=ota_rd16(d+11),sum=0;
 assert(len<=512);
 memset(ans,0,sizeof ans);ans[1]=0x59;ans[2]=0x30;ota_wr32(ans+3,len+8);ans[6]=0;
 ota_wr32(ans+7,addr);ota_wr32(ans+11,len);
 if(addr==0xe0000000u)memcpy(ans+14,"success",8);
 else{assert(addr<=image_len&&len<=image_len-addr);memcpy(ans+14,image+addr,len);}
 for(uint32_t i=6;i<len+14;i++)sum+=ans[i];ans[len+14]=(uint8_t)~sum;
 pending=ota_pack7(ans,len+15,reply);return 0;
}
static int ota_frame_get(const uint8_t **p,uint32_t *n){if(!pending)return 0;*p=reply;*n=pending;return 1;}
static void ota_frame_done(void){pending=0;}
static uint32_t ota_now_ms(void){return clock_ms++;}
static void ota_idle(void){}
static int ota_aborted(void){return 0;}
static int ota_flash_ready(void){return known;}
static int ota_erase(uint32_t off){assert(off>=OTA_AREA&&off+4096<=OTA_AREA+OTA_AREA_LEN&&!(off&4095));memset(flash+off-OTA_AREA,255,4096);erases++;return 0;}
static int ota_prog(uint32_t off,const void *p,uint32_t n){assert(off>=OTA_AREA&&off+n<=OTA_AREA+OTA_AREA_LEN);const uint8_t *s=p;for(uint32_t i=0;i<n;i++){uint8_t *v=flash+off-OTA_AREA+i;assert((*v&s[i])==s[i]);*v&=s[i];}programs++;return 0;}
static int ota_fread(uint32_t off,void *p,uint32_t n){assert(off>=OTA_AREA&&off+n<=OTA_AREA+OTA_AREA_LEN);memcpy(p,flash+off-OTA_AREA,n);return 0;}
static void ota_show(uint32_t step,int32_t code){(void)step;(void)code;}
static void ota_commit(const uint8_t *p){assert(ota_rd16(p)==ota_crc16(p+2,78,0));commits++;}
static uint8_t *readfile(const char *p,size_t *n){FILE *f=fopen(p,"rb");assert(f);fseek(f,0,SEEK_END);*n=ftell(f);rewind(f);uint8_t *v=malloc(*n);assert(v&&fread(v,1,*n,f)==*n);fclose(f);return v;}
static void residue(const char *path){size_t n;uint8_t *v=readfile(path,&n);memset(flash,255,sizeof flash);assert(n>=64&&n<0x4000);assert(ota_prog(OTA_AREA+32,v+32,n-32)==0);assert(ota_stage_head(n-32)==0);/* Step-2 ldr_core erases final commit/owner sector only. */memset(flash+0x4000,255,4096);free(v);erases=programs=commits=clock_ms=pending=0;}
int main(int argc,char **argv){assert(argc>=6);uint8_t b[512];
 residue(argv[1]);assert(etch_ota_available(b));
 residue(argv[2]);
#ifdef BASELINE
 assert(!etch_ota_available(b));puts("V1.3.1 baseline refuses its own genuine loader residue");return 0;
#else
 assert(etch_ota_available(b));residue(argv[3]);assert(etch_ota_available(b));
 /* A modified loader and a random foreign sector must not be reclaimed. */
 flash[100]^=1;assert(!etch_ota_available(b));memset(flash,255,sizeof flash);flash[100]=0;assert(!etch_ota_available(b));
 for(int i=4;i<argc;i++){
   image=readfile(argv[i],&image_len);residue(argv[2]);known=0;assert(ota_session()==-25);assert(!erases&&!programs&&!commits);
   known=1;assert(ota_session()==0);assert(erases==5&&programs>0&&commits==1);
   printf("PASS full stage-1 SysEx replay to %s from V1.3.1 residue (%u erases)\n",argv[i],erases);free(image);
 }
 return 0;
#endif
}

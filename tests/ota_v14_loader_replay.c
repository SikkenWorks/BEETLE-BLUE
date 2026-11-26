/* SPDX-License-Identifier: GPL-3.0-only
 * Host-only replay of the real Felucca-lineage loader core. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define FELUCCA_ID "ota-FM-1_900"
#define FELUCCA_OTA_DRYRUN 0
#include "vendor/ota.c"
#include "ldr_core.c"
static uint8_t flash[0x100000], original[0x100000], reply[800];
static uint8_t *image;static size_t image_len;
static uint32_t pending,clock_ms,erases,programs,clears;
static int ota_wire_send(const uint8_t *p,uint32_t n){
 uint8_t d[64],ans[528];uint32_t k=ota_unpack7(p+1,n-2,d,sizeof d);assert(k>=7);
 if(d[2]!=0x30)return 0;
 uint32_t addr=ota_rd32(d+7),len=ota_rd16(d+11),sum=0;assert(len<=512);
 memset(ans,0,sizeof ans);ans[1]=0x59;ans[2]=0x30;ota_wr32(ans+3,len+8);
 ota_wr32(ans+7,addr);ota_wr32(ans+11,len);
 if(addr==0xf0000000u)memcpy(ans+14,"success",8);
 else{assert(addr<=image_len&&len<=image_len-addr);memcpy(ans+14,image+addr,len);}
 for(uint32_t i=6;i<len+14;i++)sum+=ans[i];ans[len+14]=(uint8_t)~sum;
 pending=ota_pack7(ans,len+15,reply);return 0;
}
static int ota_frame_get(const uint8_t **p,uint32_t *n){if(!pending)return 0;*p=reply;*n=pending;return 1;}
static void ota_frame_done(void){pending=0;}
static uint32_t ota_now_ms(void){return clock_ms++;}
static void ota_idle(void){}
static int ota_erase(uint32_t off){(void)off;return -1;}
static int ota_prog(uint32_t off,const void *p,uint32_t n){(void)off;(void)p;(void)n;return -1;}
static int ota_fread(uint32_t off,void *p,uint32_t n){return ldr_fread(off,p,n);}
static void ota_show(uint32_t step,int32_t code){(void)step;(void)code;}
static void ota_commit(const uint8_t *p){(void)p;}
static int ldr_fread(uint32_t off,void *p,uint32_t n){assert(off+n<=sizeof flash);memcpy(p,flash+off,n);return 0;}
static int ldr_erase(uint32_t off){assert(!(off&4095)&&((off>=0x4000&&off<0x93000)||off==0xe4000));memset(flash+off,255,4096);erases++;return 0;}
static int ldr_prog(uint32_t off,const void *p,uint32_t n){assert(off>=0x4000&&off+n<=0x93000&&n<=256&&((off&255)+n)<=256);const uint8_t *s=p;for(uint32_t i=0;i<n;i++){assert((flash[off+i]&s[i])==s[i]);flash[off+i]&=s[i];}programs++;return 0;}
static void ldr_record_clear(void){clears++;}
static void ldr_progress(uint32_t done,uint32_t total){assert(done<=total);}
static uint8_t *readfile(const char *p,size_t *n){FILE *f=fopen(p,"rb");assert(f);fseek(f,0,SEEK_END);*n=ftell(f);rewind(f);uint8_t *v=malloc(*n);assert(v&&fread(v,1,*n,f)==*n);fclose(f);return v;}
int main(int argc,char **argv){assert(argc>=4);size_t n;uint8_t *base=readfile(argv[1],&n);assert(n==0x93000);memset(original,255,sizeof original);memcpy(original,base,n);free(base);
 /* Populate user storage so a stray erase is observable. */
 for(uint32_t i=0x97000;i<0xe0000;i++)original[i]=(uint8_t)(i*17u+i/256u);
 /* Real valid update record, exactly like the first-stage commit. */
 uint8_t *p=original+0xe4f00;memset(p,0,112);ota_wr16(p+6,0x5441);ota_wr16(p,ota_crc16(p+2,78,0));
 for(int i=2;i<argc;i++){
  memcpy(flash,original,sizeof flash);image=readfile(argv[i],&image_len);uint8_t h[1024];uint32_t off,size,oo,ol;
  assert(!ota_ufw(h,&off,&size,&oo,&ol));assert(size==0x93000);
  /* Corrupt one byte in target application; preflight must refuse before erase. */
  image[off+0x5000]^=1;erases=programs=clears=0;assert(ldr_session()!=0);assert(!erases&&!programs&&!clears);assert(!memcmp(flash,original,sizeof flash));image[off+0x5000]^=1;
  assert(!ldr_session());assert(clears==1);assert(!memcmp(flash+0x4000,image+off+0x4000,0x8f000));assert(!memcmp(flash,original,0x4000));assert(!memcmp(flash+0x97000,original+0x97000,0x49000));
  for(uint32_t j=0xe4000;j<0xe5000;j++)assert(flash[j]==255);
  printf("PASS loader step-2 full replay to %s; app readback exact, boot and user storage unchanged (%u erases)\n",argv[i],erases);free(image);
 }
 return 0;
}

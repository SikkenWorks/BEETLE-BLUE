/* SPDX-License-Identifier: GPL-3.0-only */
#include "beetle_blue_kits.h"
#include "beetle_drum6_kits.h"
#include <string.h>
uint32_t blue_kit_crc(const uint8_t *p,uint32_t n){uint32_t c=0xffffffffu;
 for(uint32_t i=0;i<n;i++){c^=p[i];for(unsigned b=0;b<8;b++)c=(c>>1)^(0xedb88320u & -(c&1u));}return ~c;}
static void compact(struct blue_kit_tone *d,const struct track_drum_timbre *s){
 d->attack=s->attack;d->decay=s->decay;d->sustain=s->sustain;d->release=s->release;
 d->gain=s->gain;d->pitch=s->pitch;d->drive=s->drive;d->choke=s->choke_group;
}
void blue_kit_bank_init(struct blue_kit_bank *b){if(!b)return;memset(b,0,sizeof(*b));
 struct track_drum_timbre defaults[TRACK_DRUM_COUNT];
 bd6_kit_factory(0u,defaults);
 for(uint8_t n=0;n<BLUE_KIT_USER;n++)for(uint8_t k=0;k<TRACK_DRUM_COUNT;k++)
  compact(&b->user[n].drums[k],&defaults[k]);
}
void blue_kit_sound(const struct blue_kit_bank *b,uint8_t source,uint8_t component,
 const struct track_drum_timbre *fallback,struct track_drum_timbre *out){
 if(!out||!fallback)return;*out=*fallback;
 if(!b||component>=TRACK_DRUM_COUNT||source<2u||source>=14u||
    !(b->valid_mask&(1u<<(source-2u))))return;
 const struct blue_kit_tone *d=&b->user[source-2u].drums[component];
 out->attack=d->attack;out->decay=d->decay;out->sustain=d->sustain;out->release=d->release;
 out->gain=d->gain;out->pitch=d->pitch;out->drive=d->drive;out->choke_group=d->choke;
}
void blue_kit_store(struct blue_kit_bank *b,uint8_t dest,
 const struct track_drum_timbre drums[TRACK_DRUM_COUNT]){
 if(!b||!drums||dest>=12u)return;
 for(uint8_t i=0;i<TRACK_DRUM_COUNT;i++)compact(&b->user[dest].drums[i],&drums[i]);
 b->valid_mask|=(uint16_t)(1u<<dest);
}

/* Project-independent kit bank carried in the transactionally swapped V2
 * metadata sector footer. Existing B2SB readers skip this extension. */
int blue_kit_encode(const struct blue_kit_bank *b,uint8_t out[BLUE_KIT_WIRE_BYTES]){
 if(!b||!out)return -1;
 out[0]='B';out[1]='B';out[2]='K';out[3]='1';
 out[4]=(uint8_t)b->valid_mask;out[5]=(uint8_t)(b->valid_mask>>8);
 uint32_t at=6;
 for(unsigned kit=0;kit<12;kit++)for(unsigned part=0;part<TRACK_DRUM_COUNT;part++){
  const struct blue_kit_tone *d=&b->user[kit].drums[part];
  if(d->attack>100||d->decay>100||d->sustain>100||d->release>100||
     d->gain>100||d->pitch<-36||d->pitch>36||d->drive>100||d->choke>7)return -1;
  out[at++]=d->attack;out[at++]=d->decay;out[at++]=d->sustain;out[at++]=d->release;
  out[at++]=d->gain;out[at++]=(uint8_t)d->pitch;out[at++]=d->drive;out[at++]=d->choke;
 }
 if(at!=BLUE_KIT_WIRE_BYTES-4u)return -1;
 uint32_t crc=blue_kit_crc(out,at);
 out[at++]=(uint8_t)crc;out[at++]=(uint8_t)(crc>>8);
 out[at++]=(uint8_t)(crc>>16);out[at++]=(uint8_t)(crc>>24);
 return 0;
}
int blue_kit_decode(struct blue_kit_bank *b,const uint8_t in[BLUE_KIT_WIRE_BYTES]){
 if(!b||!in||in[0]!='B'||in[1]!='B'||in[2]!='K'||in[3]!='1')return -1;
 uint32_t crc=blue_kit_crc(in,BLUE_KIT_WIRE_BYTES-4u);
 if(in[BLUE_KIT_WIRE_BYTES-4u]!=(uint8_t)crc||
    in[BLUE_KIT_WIRE_BYTES-3u]!=(uint8_t)(crc>>8)||
    in[BLUE_KIT_WIRE_BYTES-2u]!=(uint8_t)(crc>>16)||
    in[BLUE_KIT_WIRE_BYTES-1u]!=(uint8_t)(crc>>24))return -1;
 /* Verify ALL fields before changing the live bank. No 1.4KiB stack copy on FM1. */
 uint16_t mask=(uint16_t)(in[4]|((uint16_t)in[5]<<8));
 if(mask&0xf000u)return -1;
 uint32_t at=6u;
 for(unsigned k=0;k<12u;k++)for(unsigned part=0;part<TRACK_DRUM_COUNT;part++){
  uint8_t aa=in[at],dd=in[at+1u],ss=in[at+2u],rr=in[at+3u];
  uint8_t gg=in[at+4u],drive=in[at+6u],choke=in[at+7u];
  int8_t pitch=(int8_t)in[at+5u];
  if(aa>100u||dd>100u||ss>100u||rr>100u||gg>100u||
     drive>100u||choke>7u||pitch<-36||pitch>36)return -1;
  at+=8u;
 }
 b->valid_mask=mask;
 at=6u;
 for(unsigned k=0;k<12u;k++)for(unsigned part=0;part<TRACK_DRUM_COUNT;part++){
  struct blue_kit_tone *d=&b->user[k].drums[part];
  d->attack=in[at++];d->decay=in[at++];d->sustain=in[at++];d->release=in[at++];
  d->gain=in[at++];d->pitch=(int8_t)in[at++];d->drive=in[at++];d->choke=in[at++];
 }
 return 0;
}

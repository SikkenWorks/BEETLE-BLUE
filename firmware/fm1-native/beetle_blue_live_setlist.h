/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE Live Mode: independent project-slot setlist, NOT a pattern chain.
 * V2 setlist metadata uses the existing A/B journal; no new flash partition.
 * Project loads are foreground operations; audio ISR only raises a completion flag.
 */
#ifndef BEETLE_BLUE_LIVE_SETLIST_H
#define BEETLE_BLUE_LIVE_SETLIST_H
#include <stdint.h>
#define BLUE_LIVE_MAX 24u
#define BLUE_LIVE_NONE 255u
#define BLUE_LIVE_WIRE_BYTES 64u
/* Metadata footer in B2SB A/B index; no independent erase sector. */
#define BLUE_LIVE_INDEX_OFFSET 2610u
struct blue_live_setlist {
 uint8_t slots[BLUE_LIVE_MAX];
 uint8_t count, cursor, position, menu, stop_choice;
 uint8_t active, paused, ready, finished, error;
};
static inline void blue_live_init(struct blue_live_setlist *s){
 if(!s)return;
 for(uint8_t i=0;i<BLUE_LIVE_MAX;i++)s->slots[i]=BLUE_LIVE_NONE;
 s->count=s->cursor=s->position=s->menu=s->stop_choice=0u;
 s->active=s->paused=s->ready=s->finished=s->error=0u;
}
static inline uint8_t blue_live_insert(struct blue_live_setlist *s,uint8_t slot){
 if(!s||s->count>=BLUE_LIVE_MAX||slot>=12u)return 0u;
 uint8_t at=s->count?s->cursor+1u:0u;
 if(at>s->count)at=s->count;
 for(uint8_t i=s->count;i>at;i--)s->slots[i]=s->slots[i-1u];
 s->slots[at]=slot;s->count++;s->cursor=at;return 1u;
}
static inline uint8_t blue_live_delete(struct blue_live_setlist *s){
 if(!s||!s->count||s->cursor>=s->count)return 0u;
 for(uint8_t i=s->cursor;i+1u<s->count;i++)s->slots[i]=s->slots[i+1u];
 s->slots[--s->count]=BLUE_LIVE_NONE;
 if(s->cursor>=s->count)s->cursor=s->count?s->count-1u:0u;
 return 1u;
}
static inline uint8_t blue_live_move(struct blue_live_setlist *s,int delta){
 if(!s||!s->count||!delta)return 0u;
 int at=(int)s->cursor+delta;
 if(at<0)at=0;
 if(at>=s->count)at=s->count-1u;
 if(at==s->cursor)return 0u;
 uint8_t item=s->slots[s->cursor];
 if(at<s->cursor)for(int i=s->cursor;i>at;i--)s->slots[i]=s->slots[i-1];
 else for(int i=s->cursor;i<at;i++)s->slots[i]=s->slots[i+1];
 s->slots[at]=item;s->cursor=(uint8_t)at;return 1u;
}
static inline uint8_t blue_live_next(struct blue_live_setlist *s){
 if(!s||!s->active||s->position>=s->count)return 0u;
 s->paused=s->ready=0u;
 if(s->position+1u>=s->count){s->finished=1u;s->active=0u;return 0u;}
 s->position++;return 1u;
}
/* Versioned, CRC-checked independent setlist footer. 64-byte record.
 * Only one copy is updated in each normal V2 index A/B transaction. */
static inline uint32_t blue_live_crc(const uint8_t *buf,uint8_t count){
 uint32_t c=0xffffffffu;
 for(uint8_t i=0u;i<count;i++){
  c^=buf[i];for(uint8_t j=0;j<8u;j++)c=(c>>1u)^(0xedb88320u&-(c&1u));
 }
 return ~c;
}
static inline void blue_live_encode(const struct blue_live_setlist *s,uint8_t out[BLUE_LIVE_WIRE_BYTES]){
 for(uint8_t i=0;i<BLUE_LIVE_WIRE_BYTES;i++)out[i]=0xffu;
 out[0]='B';out[1]='L';out[2]='S';out[3]='T';out[4]=1u;out[5]=s->count;
 out[6]=out[7]=0u;
 for(uint8_t i=0;i<BLUE_LIVE_MAX;i++)out[8u+i]=s->slots[i];
 uint32_t crc=blue_live_crc(out,32u);
 for(uint8_t k=0;k<4u;k++)out[32u+k]=(uint8_t)(crc>>(8u*k));
}
static inline uint8_t blue_live_decode(struct blue_live_setlist *s,const uint8_t in[BLUE_LIVE_WIRE_BYTES]){
 if(!s||in[0]!='B'||in[1]!='L'||in[2]!='S'||in[3]!='T'||in[4]!=1u||in[5]>BLUE_LIVE_MAX)return 0u;
 uint32_t crc=blue_live_crc(in,32u),saved=0u;
 for(uint8_t k=0;k<4u;k++)saved|=(uint32_t)in[32u+k]<<(8u*k);
 if(crc!=saved)return 0u;
 for(uint8_t i=0;i<in[5];i++)if(in[8u+i]>=12u)return 0u;
 blue_live_init(s);s->count=in[5];
 for(uint8_t i=0;i<s->count;i++)s->slots[i]=in[8u+i];
 return 1u;
}
#endif

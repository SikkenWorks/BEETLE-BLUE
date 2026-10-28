/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "beetle_blue_live_setlist.h"
int main(void){
 struct blue_live_setlist s;uint8_t rec[BLUE_LIVE_WIRE_BYTES];
 blue_live_init(&s);
 assert(!s.count&&!s.active&&!blue_live_next(&s));
 assert(blue_live_insert(&s,0));
 assert(blue_live_insert(&s,4));
 assert(blue_live_insert(&s,11));
 assert(s.count==3&&s.slots[0]==0&&s.slots[1]==4&&s.slots[2]==11);
 s.cursor=2;assert(blue_live_move(&s,-2));
 assert(s.slots[0]==11&&s.slots[1]==0&&s.slots[2]==4);
 assert(blue_live_delete(&s));assert(s.count==2&&s.slots[0]==0);
 blue_live_encode(&s,rec);
 assert(!memcmp(rec,"BLST",4));
 struct blue_live_setlist rest;blue_live_init(&rest);
 assert(blue_live_decode(&rest,rec));assert(rest.count==2);
 assert(rest.slots[0]==0&&rest.slots[1]==4);
 rec[9]^=1u;assert(!blue_live_decode(&rest,rec));rec[9]^=1u;
 rec[4]=2u;assert(!blue_live_decode(&rest,rec));rec[4]=1u;
 assert(blue_live_decode(&rest,rec));
 rest.active=1u;rest.position=0u;rest.ready=1u;
 assert(blue_live_next(&rest));assert(rest.position==1u&&!rest.ready);
 assert(!blue_live_next(&rest));assert(rest.finished&&!rest.active);
 blue_live_init(&s);
 for(unsigned i=0;i<BLUE_LIVE_MAX;i++)assert(blue_live_insert(&s,(uint8_t)(i%12)));
 assert(!blue_live_insert(&s,0));assert(s.count==BLUE_LIVE_MAX);
 for(unsigned i=0;i<BLUE_LIVE_MAX;i++)assert(blue_live_delete(&s));
 assert(!s.count&&!blue_live_delete(&s));
 puts("PASS BLUE live setlist 24 slots, reorder, deletion, CRC, end-of-set");
}

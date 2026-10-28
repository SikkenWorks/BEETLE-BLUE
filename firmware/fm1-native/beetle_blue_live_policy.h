/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE V1.0 RC1: pure, testable Setlist input/slot policy.
 * Only fully recognized saved projects (status == 1) are selectable.
 * The UI scans metadata incrementally; a still-unscanned slot is never
 * silently treated as an existing project.
 */
#ifndef BEETLE_BLUE_LIVE_POLICY_H
#define BEETLE_BLUE_LIVE_POLICY_H
#include <stdint.h>
#include "beetle_blue_live_setlist.h"
static inline uint8_t blue_live_first_saved(const uint8_t *status,uint8_t count){
 if(!status)return BLUE_LIVE_NONE;
 for(uint8_t i=0;i<count;i++)if(status[i]==1u)return i;
 return BLUE_LIVE_NONE;
}
static inline uint8_t blue_live_step_saved(const uint8_t *status,uint8_t count,
                                            uint8_t current,int32_t steps){
 if(!status||!count)return BLUE_LIVE_NONE;
 uint8_t found=blue_live_first_saved(status,count);
 if(found==BLUE_LIVE_NONE)return found;
 if(current>=count||status[current]!=1u)return found;
 if(!steps)return current;
 int direction=steps>0?1:-1;
 /* Clamp the number of detents: each traversal over a complete cycle returns
  * to the same saved slot, even if the encoder floods the poll with steps. */
 uint32_t n=steps>0?(uint32_t)steps:(uint32_t)(-(int64_t)steps);
 if(n>255u)n=255u;
 while(n--){
  uint8_t test=current;
  for(uint8_t j=0;j<count;j++){
   test=(uint8_t)((test+count+direction)%count);
   if(status[test]==1u){current=test;break;}
  }
 }
 return current;
}
/* A UI back press must never be repurposed as 'create entry', even when
 * the list is empty or SEL was pressed on the same polling frame. */
static inline uint8_t blue_live_can_add(const uint8_t *status,uint8_t count){
 return blue_live_first_saved(status,count)!=BLUE_LIVE_NONE;
}
#endif

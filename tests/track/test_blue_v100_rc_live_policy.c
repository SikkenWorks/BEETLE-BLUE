/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include "beetle_blue_live_policy.h"
#include "beetle_blue_live_nav.h"
#include "beetle_blue_live_autosave.h"
static unsigned writes;
static int failing;
static uint8_t write_list(void *ctx){(void)ctx;writes++;return (uint8_t)!failing;}
int main(void){
 uint8_t none[12]={2,2,2,2,2,2,2,2,2,2,2,2};
 struct blue_live_setlist s;blue_live_init(&s);
 /* V0.99.8 regression: no project must mean no phantom P01 & no flash write. */
 assert(!blue_live_can_add(none,12));
 assert(blue_live_first_saved(none,12)==BLUE_LIVE_NONE);
 assert(blue_live_step_saved(none,12,0,1)==BLUE_LIVE_NONE);
 uint8_t dirty=0,status=0;
 assert(blue_live_autosave_dirty(&dirty,&status,write_list,0));
 assert(writes==0 && !s.count);
 assert(blue_live_home_target(BLUE_LIVE_SETLIST_PAGE)==BLUE_LIVE_MENU_PAGE);
 assert(blue_live_home_target(BLUE_LIVE_MENU_PAGE)==BLUE_HOME_PROJECTS);
 uint8_t saved[12]={2,0,1,2,1,3,2,2,2,2,2,2};
 assert(blue_live_first_saved(saved,12)==2);
 assert(blue_live_step_saved(saved,12,2,1)==4);
 assert(blue_live_step_saved(saved,12,4,1)==2);
 assert(blue_live_step_saved(saved,12,2,-1)==4);
 assert(blue_live_step_saved(saved,12,11,1)==2);
 assert(blue_live_step_saved(saved,12,2,0)==2);
 assert(blue_live_step_saved(saved,12,2,2)==2);
 assert(blue_live_step_saved(saved,12,2,3)==4);
 assert(blue_live_insert(&s,blue_live_first_saved(saved,12)));
 assert(s.count==1 && s.slots[0]==2);
 dirty=1;assert(blue_live_autosave_dirty(&dirty,&status,write_list,0));
 assert(!dirty && writes==1);
 dirty=1;failing=1;
 assert(!blue_live_autosave_dirty(&dirty,&status,write_list,0));
 assert(dirty && status==2 && writes==2);
 /* Back-navigation after a failure keeps RAM edits; only successful save
  * clears dirty. Caller must keep prominent UNSAVED indicator. */
 assert(s.slots[0]==2 && dirty);
 failing=0;
 assert(blue_live_autosave_dirty(&dirty,&status,write_list,0));
 assert(!dirty && !status && writes==3);
 assert(blue_live_step_saved(0,12,0,1)==BLUE_LIVE_NONE);
 assert(blue_live_first_saved(saved,0)==BLUE_LIVE_NONE);
 return 0;
}

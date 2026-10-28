/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include "beetle_blue_live_nav.h"
#include "beetle_blue_live_autosave.h"
static int writes,fail;
static uint8_t writer(void *ctx){(void)ctx;writes++;return !fail;}
int main(void){
 assert(blue_live_home_target(BLUE_LIVE_MENU_PAGE)==BLUE_HOME_PROJECTS);
 assert(blue_live_home_target(BLUE_LIVE_SETLIST_PAGE)==BLUE_LIVE_MENU_PAGE);
 assert(blue_live_home_target(BLUE_LIVE_PERFORM_PAGE)==BLUE_LIVE_MENU_PAGE);
 assert(blue_live_home_target(BLUE_LIVE_STOP_PAGE)==BLUE_LIVE_MENU_PAGE);
 /* All five autosave cases retain the already released behaviour. */
 uint8_t dirty=0,status=0;
 assert(blue_live_autosave_dirty(&dirty,&status,writer,0)&&!writes);
 dirty=1;assert(blue_live_autosave_dirty(&dirty,&status,writer,0));
 assert(!dirty&&writes==1);
 dirty=1;fail=1;
 assert(!blue_live_autosave_dirty(&dirty,&status,writer,0));
 assert(dirty&&status==2&&writes==2);
 fail=0;assert(blue_live_autosave_dirty(&dirty,&status,writer,0));
 assert(!dirty&&writes==3);
 dirty=1;assert(!blue_live_autosave_dirty(&dirty,&status,0,0));
 assert(dirty&&status==2);
 return 0;
}

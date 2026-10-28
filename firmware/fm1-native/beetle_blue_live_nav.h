/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE: a single, explicit HOME back-stack for independent Live Mode.
 * Start screen (29) and ordinary HOME/PROJECTS menu (1) are distinct.
 */
#ifndef BEETLE_BLUE_LIVE_NAV_H
#define BEETLE_BLUE_LIVE_NAV_H
#include <stdint.h>
#define BLUE_HOME_PROJECTS 1u
#define BLUE_LIVE_MENU_PAGE 43u
#define BLUE_LIVE_SETLIST_PAGE 44u
#define BLUE_LIVE_PERFORM_PAGE 45u
#define BLUE_LIVE_STOP_PAGE 46u
#define BLUE_HOME_LIVE_INDEX 7u
/* All Live child views return to Live Mode. Live Mode itself returns to
 * PROJECTS, even if opened directly from the power-on NEW/LOAD chooser. */
static inline uint8_t blue_live_home_target(uint8_t page){
 return page==BLUE_LIVE_MENU_PAGE ? BLUE_HOME_PROJECTS : BLUE_LIVE_MENU_PAGE;
}
#endif

/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE Live Mode setlist leave gate. No writes in audio ISR.
 * A failed commit MUST keep the editor open and RAM edits untouched.
 */
#ifndef BEETLE_BLUE_LIVE_AUTOSAVE_H
#define BEETLE_BLUE_LIVE_AUTOSAVE_H
#include <stdint.h>
static inline uint8_t blue_live_autosave_dirty(uint8_t *dirty,uint8_t *status,
 uint8_t (*commit)(void *),void *context){
 if(!dirty||!status)return 0u;
 if(!*dirty)return 1u; /* no flash writes, avoids needless wear */
 if(!commit || !commit(context)){
  *status=2u;return 0u; /* no silent loss; caller must stay in editor */
 }
 *dirty=0u;
 *status=0u; /* success is intentionally invisible to the musician */
 return 1u;
}
#endif

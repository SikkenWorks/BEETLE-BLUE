/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE: explicit per-lane kit is authoritative for all notes.
 * Never infer a kit override from an old step's INST field; that field
 * is shared with saved pattern data and is not a reliable override flag.
 */
#ifndef BEETLE_BLUE_LANE_SOURCE_H
#define BEETLE_BLUE_LANE_SOURCE_H
#include <stdint.h>
static inline uint8_t beetle_blue_lane_source(const uint8_t kits[6],uint8_t lane){
 return kits && lane<6u && kits[lane]<14u?kits[lane]:0u;
}
#endif

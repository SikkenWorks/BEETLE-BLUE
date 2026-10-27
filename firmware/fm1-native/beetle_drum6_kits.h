/* Twelve lightweight BEETLE DRUM kit starting points.
 * GPL-3.0-only. Uses the existing factory ROM one-shots (NOT licensed third-party recordings).
 * Selected preset and edited timbres can be saved in a BDR1 project.
 */
#ifndef BEETLE_DRUM6_KITS_H
#define BEETLE_DRUM6_KITS_H
#include "track_drum.h"
#define BD6_KIT_COUNT 12u
extern const char *const bd6_kit_names[BD6_KIT_COUNT];
/* Rebuild the 14 factory ROM timbres using current BEETLE defaults, with
 * bounded, deterministic character offsets; preserves each instrument ID. */
void bd6_kit_factory(uint8_t kit,struct track_drum_timbre out[TRACK_DRUM_COUNT]);
#endif

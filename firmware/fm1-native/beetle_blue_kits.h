/* BEETLE BLUE V0.8: 2 factory + 12 user kit definitions. GPL-3.0-only.
 * Source IDs are project-independent. A project merely records eight mappings.
 */
#ifndef BEETLE_BLUE_KITS_H
#define BEETLE_BLUE_KITS_H
#include <stdint.h>
#include "track_drum.h"
#define BLUE_KIT_TOTAL 14u
#define BLUE_KIT_USER 12u
#define BLUE_KIT_PALETTE 8u
#define BLUE_KIT_WIRE_BYTES 1354u
#define BLUE_KIT_INDEX_OFFSET (4092u-BLUE_KIT_WIRE_BYTES)
struct blue_kit_tone {uint8_t attack,decay,sustain,release,gain;int8_t pitch;uint8_t drive,choke;};
struct blue_kit {struct blue_kit_tone drums[TRACK_DRUM_COUNT];};
struct blue_kit_bank {struct blue_kit user[BLUE_KIT_USER];uint16_t valid_mask;};
void blue_kit_bank_init(struct blue_kit_bank *bank);
void blue_kit_sound(const struct blue_kit_bank *bank,uint8_t source,uint8_t component,
 const struct track_drum_timbre *fallback,struct track_drum_timbre *out);
void blue_kit_store(struct blue_kit_bank *bank,uint8_t dest,
 const struct track_drum_timbre drums[TRACK_DRUM_COUNT]);
int blue_kit_encode(const struct blue_kit_bank *bank,uint8_t dst[BLUE_KIT_WIRE_BYTES]);
int blue_kit_decode(struct blue_kit_bank *bank,const uint8_t src[BLUE_KIT_WIRE_BYTES]);
uint32_t blue_kit_crc(const uint8_t *p,uint32_t n);
#endif

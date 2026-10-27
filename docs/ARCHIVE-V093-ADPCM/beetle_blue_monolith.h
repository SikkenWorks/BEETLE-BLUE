/* Embedded, read-only MONOLITH sample kit. */
#ifndef BEETLE_BLUE_MONOLITH_H
#define BEETLE_BLUE_MONOLITH_H
#include <stdint.h>
#define BLUE_MONOLITH_COUNT 16u
#define BLUE_MONOLITH_RATE 8000u
#define BLUE_MONOLITH_BYTES 50800u
extern const uint8_t blue_monolith_adpcm[BLUE_MONOLITH_BYTES];
extern const uint32_t blue_monolith_offset[BLUE_MONOLITH_COUNT+1u];
#endif

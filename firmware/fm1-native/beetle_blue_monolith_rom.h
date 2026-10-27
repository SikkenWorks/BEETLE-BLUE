/* SPDX-License-Identifier: GPL-3.0-only
 * MONOLITH ROM percussion: 16 deterministic digital-synthesis patches.
 * Modelled from the supplied MONOLITH WAVs' spectral content and envelope
 * profiles; NOT the original audio and not sample playback. All data are ROM.
 */
#ifndef BEETLE_BLUE_MONOLITH_ROM_H
#define BEETLE_BLUE_MONOLITH_ROM_H
#include <stdint.h>
#define BLUE_MONOLITH_COUNT 16u
#define BLUE_MONOLITH_ROM_RATE 44100u
struct blue_rom_patch {
 uint32_t phase_inc, phase2_inc, sweep_inc;
 uint16_t length_22050;
 uint8_t noise_mix, hp_shift, env_shift;
};
extern const struct blue_rom_patch blue_rom_patches[16];
extern const int16_t blue_rom_sine[256];
#endif

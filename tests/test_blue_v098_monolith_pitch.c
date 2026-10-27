/* SPDX-License-Identifier: GPL-3.0-only
 * MONOLITH NOTE PITCH FX changes the procedural oscillator phase directly.
 * Previously only its PCM playback cursor changed (different duration, same tone).
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_core.h"
#include "track_drum.h"
#include "track_mixer.h"
#include "beetle_blue_monolith_rom.h"
int16_t *beetle_stem_capture=0;
uint32_t beetle_stem_offset=0;
static struct track_drum_engine d;
static struct track_mixer mix;
static int16_t buf[2];
static uint32_t advance(uint8_t value){
 track_drum_init(&d);track_mixer_init(&mix);
 assert(track_drum_trigger(&d,64u));
 struct track_drum_voice *v=&d.voices[0];assert(v->active);
 track_drum_step_fx(&d,v,0u,TRACK_FX_DRUM_PITCH,value,120u);
 uint32_t expected=((blue_rom_patches[0].phase_inc+v->kit_sweep)>>8u)*(v->phase_step>>7u);
 memset(buf,0,sizeof buf);track_drum_render(&d,&mix,buf,1u);
 assert(v->kit_phase==expected);
 return v->kit_phase;
}
int main(void){
 const uint32_t low=advance(0u),normal=advance(12u),high=advance(24u);
 assert(low>0u && low<normal && normal<high);
 assert((uint64_t)high>=(uint64_t)normal*19u/10u);
 printf("PASS V0.98: MONOLITH audible oscillator pitch follows FX PITCH, low=%u normal=%u high=%u\n",low,normal,high);
 return 0;
}

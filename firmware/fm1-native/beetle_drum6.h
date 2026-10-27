/* BEETLE DRUM V0.2 - independent six-lane sample-accurate event scheduler.
 * Based on BEETLE V1.7.14. GPL-3.0-only. No I/O, allocation or floating point.
 * Integration with FM1 render/flash is intentionally NOT claimed here.
 */
#ifndef BEETLE_DRUM6_H
#define BEETLE_DRUM6_H
#include <stdint.h>
#define BD6_CHANNELS 6u
#define BD6_STEPS 16u
#define BD6_PATTERNS 48u
#define BD6_MAX_RATCHET 4u
#define BD6_MAX_EVENTS (BD6_CHANNELS * BD6_MAX_RATCHET)
#define BD6_FACTORY_PADS 14u
#define BD6_TOTAL_PADS 27u
struct bd6_step {
 uint8_t active,velocity,probability,ratchet;
 uint8_t micro; /* 0..7, 1/64 step increments */
 int8_t pitch_lock; /* -24..24 semitones */
};
struct bd6_lane {
 uint8_t pad,mute,level,choke_group;
 int8_t pan;
};
struct bd6_pattern {struct bd6_step lane[BD6_CHANNELS][BD6_STEPS];};
struct bd6_song_position {uint8_t pattern,repeats;};
struct bd6_project {
 uint16_t bpm;
 uint8_t swing;
 struct bd6_lane lanes[BD6_CHANNELS];
 struct bd6_pattern patterns[BD6_PATTERNS];
 struct bd6_song_position song[64];
 uint8_t song_count;
};
struct bd6_event {
 uint32_t frame_offset;
 uint8_t channel,pad,velocity,choke_group;
 int8_t pitch_lock;
};
struct bd6_batch {struct bd6_event events[BD6_MAX_EVENTS];uint8_t count;uint32_t step_frames;};
/* Randomness is owned by caller, so test renders are deterministic. */
void bd6_project_init(struct bd6_project *p);
int bd6_project_validate(const struct bd6_project *p);
/* Returns count of scheduled drum hits; all offsets < step_frames.
 * Accumulated audio sample time is solely owned by the host DMA timeline. */
uint8_t bd6_schedule_step(const struct bd6_project *p,uint8_t pattern,uint8_t row,
                         uint32_t *rng_state,struct bd6_batch *out);
#endif

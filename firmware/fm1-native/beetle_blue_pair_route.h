/* SPDX-License-Identifier: GPL-3.0-only
 * Source-correct NOTE1/NOTE2 classification for the six-channel drum tracker.
 * Voice IDs 64..79 denote MONOLITH ROM patches, while BEETLE PCM uses
 * physical variation IDs that intentionally differ from note components.
 * This pure helper is also compiled and exercised in host regression tests.
 */
#ifndef BEETLE_BLUE_PAIR_ROUTE_H
#define BEETLE_BLUE_PAIR_ROUTE_H
#include <stdint.h>
#define BLUE_NOTE_RANGE 27u
static inline uint8_t beetle_blue_pair_is_note2(
  uint8_t first, uint8_t second, uint8_t voice_id,
  uint8_t voice_component, uint32_t voice_serial, uint32_t prior_serial){
 const uint8_t has_first=(uint8_t)(first<BLUE_NOTE_RANGE || (first>=64u&&first<80u));
 const uint8_t has_second=(uint8_t)(second<BLUE_NOTE_RANGE || (second>=64u&&second<80u));
 if(!has_second)return 0u;
 if(!has_first)return 1u;
 if(first==second)return (uint8_t)(voice_serial!=prior_serial+1u);
 /* MONOLITH IDs cannot be compared to note numbers or vice versa. */
 if(first>=64u || second>=64u){
  if(voice_id==second)return 1u;
  if(voice_id==first)return 0u;
 }else{
  /* Factory kit can choose alternate physical snare/hat PCM IDs.
   * The logical component, not the physical PCM slot, identifies the row. */
  if(voice_component==second)return 1u;
  if(voice_component==first)return 0u;
 }
 /* If a future voice backend cannot report its source, retain the actual
  * trigger order rather than assigning every MONOLITH voice to NOTE2. */
 return (uint8_t)(voice_serial!=prior_serial+1u);
}
#endif

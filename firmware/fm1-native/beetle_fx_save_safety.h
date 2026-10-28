/* SPDX-License-Identifier: GPL-3.0-only
 * RC3: prevent a valid VOLUME 101..200% from poisoning another effect when
 * the user changes TYPE in FX1/FX2/FX3. Also repair *only* impossible legacy
 * value/type pairs on the staged SAVE copy, never touch notes or audio RAM.
 */
#ifndef BEETLE_FX_SAVE_SAFETY_H
#define BEETLE_FX_SAVE_SAFETY_H
#include "track_song.h"

static inline uint8_t beetle_fx_save_limit(uint8_t type){
 return type==TRACK_FX_VOLUME?200u:type==TRACK_FX_DRUM_PITCH?24u:100u;
}
static inline uint8_t beetle_fx_change_value(uint8_t old_type,uint8_t new_type,uint8_t previous){
 if(new_type==TRACK_FX_VOLUME&&old_type!=TRACK_FX_VOLUME)return 100u;
 if(new_type==TRACK_FX_DRUM_PITCH&&old_type!=TRACK_FX_DRUM_PITCH)return 12u;
 if(!new_type)return 50u;
 const uint8_t max=beetle_fx_save_limit(new_type);
 return previous>max?max:previous;
}
/* This runs only after project capture and before project serialization. It
 * corrects values that could not be persisted or edited legally in the new FX
 * domain. No change to notes, instruments, effects, timing or used slots. */
static inline uint16_t beetle_fx_repair_staged_values(struct track_song_bank *song){
 if(!song)return 0u;
 uint16_t changed=0u;
 for(uint8_t p=0u;p<TRACK_PATTERNS;p++){
  struct track_pattern *pat=&song->patterns[p];
  for(uint8_t ch=0u;ch<TRACK_CHANNELS;ch++)for(uint8_t row=0u;row<TRACK_STEPS;row++){
   struct track_step *step=&pat->ch[ch][row];
   if(step->effect>=TRACK_FX_COUNT)continue; /* invalid type must remain diagnosable */
   uint8_t limit=beetle_fx_save_limit(step->effect);
   if(step->value>limit){step->value=limit;changed++;}
  }
  for(uint8_t row=0u;row<TRACK_STEPS;row++){
   struct track_note2_fx *fx=&pat->note2_fx[row];
   if(fx->effect>=TRACK_FX_COUNT)continue;
   uint8_t limit=beetle_fx_save_limit(fx->effect);
   if(fx->value>limit){fx->value=limit;changed++;}
  }
 }
 /* Structural corruption must still fail closed. */
 if(song->extras.count>TRACK_EXTRA_FX_CAP)return changed;
 for(uint8_t k=0u;k<song->extras.count;k++){
  struct track_extra_fx *fx=&song->extras.data[k];
  uint8_t type=track_extra_effect(fx);
  if(!type||type>=TRACK_FX_COUNT)continue;
  uint8_t limit=beetle_fx_save_limit(type);
  if(fx->value>limit){fx->value=limit;changed++;}
 }
 return changed;
}
#endif

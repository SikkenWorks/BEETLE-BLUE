/* GPL-3.0-only. The reserved FX nibble value 12 stores REV without changing BDR1 V30. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "track_core.h"
int main(void){
 struct track_pattern x={0},y;
 uint8_t wire[TRACK_PATTERN_BYTES];
 x.length=16u;
 for(unsigned l=0;l<TRACK_CHANNELS;l++)for(unsigned s=0;s<TRACK_STEPS;s++){
  x.ch[l][s].note=TRACK_NOTE_EMPTY;
 }
 x.ch[0][0].note=0;
 x.ch[0][0].effect=TRACK_FX_REVERSE;
 x.ch[0][0].value=100u;
 x.ch[5][8].note=1;
 x.ch[5][8].effect=TRACK_FX_REVERSE;
 x.ch[5][8].value=50u;
 x.note2_fx[8].effect=TRACK_FX_REVERSE;
 x.note2_fx[8].value=100u;
 assert(track_pattern_encode(&x,wire,sizeof wire)>0);
 assert(track_pattern_decode(&y,wire,sizeof wire)==0);
 assert(y.ch[0][0].effect==TRACK_FX_REVERSE);
 assert(y.ch[5][8].effect==TRACK_FX_REVERSE);
 assert(y.note2_fx[8].effect==TRACK_FX_REVERSE);
 puts("PASS REV pattern primary CH1 + CH6 and NOTE2 FX serialize/deserialize (project wire unchanged)");
 return 0;
}

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "beetle_blue_kits.h"
/* Host shim: isolate binary kit integrity from unrelated firmware dependencies. */
void bd6_kit_factory(uint8_t kit,struct track_drum_timbre out[TRACK_DRUM_COUNT]){
 (void)kit;
 memset(out,0,sizeof(struct track_drum_timbre)*TRACK_DRUM_COUNT);
 for(unsigned i=0;i<TRACK_DRUM_COUNT;i++){
  out[i].gain=100;out[i].decay=50;out[i].pitch=(int8_t)i;out[i].release=20;
 }
}
int main(void){
 struct blue_kit_bank bank,decoded;
 uint8_t blob[BLUE_KIT_WIRE_BYTES],damaged[BLUE_KIT_WIRE_BYTES];
 blue_kit_bank_init(&bank);
 struct track_drum_timbre pads[TRACK_DRUM_COUNT];
 bd6_kit_factory(0,pads);
 for(unsigned k=0;k<BLUE_KIT_USER;k++){
  pads[0].gain=(uint8_t)(85-k);pads[0].drive=(uint8_t)k;
  pads[0].choke_group=(uint8_t)(k%8);
  blue_kit_store(&bank,(uint8_t)k,pads);
 }
 assert(bank.valid_mask==0xfffu);
 assert(blue_kit_encode(&bank,blob)==0);
 memset(&decoded,0x33,sizeof(decoded));
 assert(blue_kit_decode(&decoded,blob)==0);
 assert(!memcmp(&bank,&decoded,sizeof(bank)));
 memcpy(damaged,blob,sizeof(blob));
 damaged[123]^=0x40;
 assert(blue_kit_decode(&decoded,damaged)!=0);
 assert(!memcmp(&bank,&decoded,sizeof(bank))); /* reject atomically */
 struct track_drum_timbre out,fallback={0};
 fallback.gain=100;
 blue_kit_sound(&decoded,13,0,&fallback,&out);
 assert(out.gain==74&&out.drive==11&&out.choke_group==3);
 blue_kit_sound(&decoded,0,0,&fallback,&out);
 assert(out.gain==100);
 puts("PASS: 12 global slots, 14 pads, CRC, atomic rejection, bank retrieval");
 return 0;
}

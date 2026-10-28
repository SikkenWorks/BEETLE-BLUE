/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE on-device help: fixed-width, <=36 glyphs at 5x7+1 pixel.
 * Chapters are delimited by # headings. K2 selects chapter; K1 scrolls only within it.
 */
#ifndef BEETLE_HELP_H
#define BEETLE_HELP_H
#include <stdint.h>
#define BEETLE_HELP_VISIBLE 9u
static const char *const beetle_help_lines[] = {
 "# BEETLE BLUE / START",
 "6 DRUM LANES / 48 PATTERNS",
 "SEQ: TRACKER   PLAY: TRANSPORT",
 "SELECT: CH1-CH6   ALG: PATTERN",
 "K1 STEP K2 CELL K3 VALUE",
 "K4 BPM / SEQ+K4 PAT LEN",
 "KEYS: DRUM HITS / REC TO WRITE",
 "# TRACKER / CH6",
 "CH1-5: NOTE INST FX / VALUE",
 "CH6: THREE INDEPENDENT HITS",
 "CH6: KIT FIELD IS NOW RAT",
 "RAT: HIT REPEATS PER STEP 1-4",
 "CH6 SEL+FX: NOTE3 / FX MODE",
 "CH6 STP CAN STOP ONE HIT",
 "SEQ+SELECT: CHANGE LANE KIT",
 "# STEP FX",
 "FX CELL + TAP FX: EDIT STEP FX",
 "FX1 / FX2 / FX3 PER STEP",
 "K1 FX SLOT K2 TYPE K3 VALUE",
 "VOLUME: 0-200 PERCENT",
 "REV: REVERSE DRUM HIT",
 "FX ARE SAVED PER PATTERN/LANE",
 "OCT+: NEXT SLOT OCT-: CLEAR",
 "# PUNCH IN FX",
 "TAP FX TWICE: PUNCH IN PAGE",
 "HOLD FX: TEMPORARY PUNCH",
 "SEQ+WHITE KEYS: LIVE PUNCH FX",
 "FX SCREEN: HOLD KEY, TURN K1",
 "BLEND: DRY/WET ALL PUNCH FX",
 "PITCH: K1 SEMITONES -24/+24",
 "K1 CHANGES HELD FX PARAMETER",
 "REC: RECORD PUNCH AUTOMATION",
 "# KITS / DRUM EDIT",
 "EDIT: DRUM ENGINE / TIMBRES",
 "EDIT AGAIN: KIT DESIGNER",
 "BEETLE: SAMPLE-BASED ROM KIT",
 "MONOLITH: SYNTHESIZED ROM KIT",
 "12 GLOBAL USER KIT PRESETS",
 "DRUM MIXER: GLO TWICE",
 "USER WAV SLOTS: U01-U13",
 "USER WAV: STORAGE V2 REQUIRED",
 "# PROJECTS / STORAGE",
 "HOME: PROJECTS MENU",
 "LOAD: K1 SLOT SEL LOAD",
 "LOAD: EDIT DELETE SELECTED SLOT",
 "DELETE: SELECT NO/YES THEN SEL",
 "MAKE BACKUP BEFORE UPDATING",
 "V31 READS EXISTING V30 PROJECTS",
 "V30 CANNOT OPEN SAVED V31 DATA",
 "GLOBAL KITS NEED SEPARATE BACKUP",
 "# SONG / PERFORMANCE",
 "ARP: SONG CHAINS / CH1-CH6",
 "GLO: LANE MIX / DRUM MIX",
 "LFO: MASTER AUDIO FX",
 "SEQ+OP1/2: COPY / PASTE",
 "SEQ+OP5/6: UNDO / REDO",
 "# SAFETY / HELP",
 "HOME+PLAY HOLD: AUDIO RECOVERY",
 "USE RECOVERY BEFORE REBOOT",
 "STORAGE / OTA: BACK UP FIRST",
 "USB AUDIO IS EXPERIMENTAL",
 "USB-C: 44.1KHZ/16BIT AUDIO",
 "FIRMWARE: V1.0 PRE-RC EXPERIMENT",
 };
#define BEETLE_HELP_COUNT ((uint8_t)(sizeof(beetle_help_lines)/sizeof(beetle_help_lines[0])))
/* Chapter indexing is derived from source lines. Update the native HELP
 * and simulator HELP together; enforce parity in tests/track/test_help_contract.py.
 * These display-only helpers do not modify saved project data. */
static inline uint8_t beetle_help_chapter_count(void){
 uint8_t count=0u;
 for(uint8_t i=0u;i<BEETLE_HELP_COUNT;i++)if(beetle_help_lines[i][0]=='#')count++;
 return count;
}
static inline uint8_t beetle_help_chapter_heading(uint8_t chapter){
 uint8_t seen=0u;
 for(uint8_t i=0u;i<BEETLE_HELP_COUNT;i++)if(beetle_help_lines[i][0]=='#'){
  if(seen==chapter)return i;
  seen++;
 }
 return 0u;
}
static inline uint8_t beetle_help_chapter_end(uint8_t chapter){
 uint8_t start=beetle_help_chapter_heading(chapter);
 for(uint16_t i=(uint16_t)start+1u;i<BEETLE_HELP_COUNT;i++)
  if(beetle_help_lines[i][0]=='#')return (uint8_t)i;
 return BEETLE_HELP_COUNT;
}
static inline uint8_t beetle_help_chapter_length(uint8_t chapter){
 uint8_t first=beetle_help_chapter_heading(chapter);
 uint8_t last=beetle_help_chapter_end(chapter);
 return last>first?(uint8_t)(last-first-1u):0u;
}
static inline uint8_t beetle_help_chapter_maxscroll(uint8_t chapter){
 uint8_t n=beetle_help_chapter_length(chapter);
 return n>BEETLE_HELP_VISIBLE?(uint8_t)(n-BEETLE_HELP_VISIBLE):0u;
}
static inline uint8_t beetle_help_chapter_shift(uint8_t chapter,int32_t detents){
 int32_t result=(int32_t)chapter+detents;
 int32_t last=(int32_t)beetle_help_chapter_count()-1;
 if(result<0)result=0;
 if(result>last)result=last;
 return (uint8_t)result;
}
static inline uint8_t beetle_help_chapter_scroll(uint8_t chapter,uint8_t current,int32_t detents){
 int32_t result=(int32_t)current+detents;
 int32_t last=beetle_help_chapter_maxscroll(chapter);
 if(result<0)result=0;
 if(result>last)result=last;
 return (uint8_t)result;
}
#endif

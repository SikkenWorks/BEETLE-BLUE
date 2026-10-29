/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE BLUE RC12 HELP: three readable cards per page on the FM1's 240x240 LCD.
 * K2 changes topic; K1 changes page. Display-only, never touches project data.
 */
#ifndef BEETLE_HELP_H
#define BEETLE_HELP_H
#include <stdint.h>
#define BEETLE_HELP_CARDS_PER_PAGE 3u
struct beetle_help_card { const char *title; const char *detail; };
struct beetle_help_topic { const char *title; uint8_t first; uint8_t count; };
static const struct beetle_help_card beetle_help_cards[] = {
 /* QUICK START */
 {"OPEN TRACKER","PRESS SEQ"},
 {"CHOOSE LANE","TURN SELECT: CH1-CH6"},
 {"CHOOSE PATTERN","TURN ALGORITHM: 01-48"},
 {"EDIT A STEP","K1 STEP / K2 CELL / K3 VALUE"},
 {"SET TEMPO","TURN K4: BPM"},
 {"PLAY OR STOP","PRESS PLAY/STOP"},
 /* LANE LENGTHS */
 {"SELECT THE LANE","TURN SELECT: CH1-CH6"},
 {"CHANGE ITS LENGTH","HOLD SEQ + TURN K4"},
 {"ALLOWED LENGTH","01 THROUGH 16 STEPS"},
 {"SIX INDEPENDENT LOOPS","EACH PATTERN SAVES ALL SIX"},
 {"SHORTEN SAFELY","HIDDEN LATER NOTES STAY SAVED"},
 {"SONG AND LIVE","USE THESE SAVED LANE LOOPS"},
 /* TRACKER / CH6 */
 {"ENTER DRUM HITS","USE KEYS / REC WRITES STEPS"},
 {"CHANGE LANE KIT","HOLD SEQ + TURN SELECT"},
 {"CHANNEL 6","THREE INDEPENDENT HITS"},
 {"RAT FIELD","RETRIGGERS PER STEP: 1-4"},
 {"NOTE3 / STEP FX","CH6: SEL + FX TO SWITCH"},
 {"STP","STOPS ONE CH6 HIT ONLY"},
 /* STEP EFFECTS */
 {"OPEN STEP FX","FOCUS FX CELL; TAP FX"},
 {"THREE FX PER STEP","FX1, FX2 AND FX3"},
 {"SELECT EFFECT","K1 SLOT / K2 TYPE"},
 {"CHANGE AMOUNT","K3 ADJUSTS THE VALUE"},
 {"NEXT OR REMOVE","OCT+ NEXT / OCT- CLEAR"},
 {"VOLUME AND REV","0-200% / REVERSE THE HIT"},
 /* PUNCH-IN FX */
 {"OPEN PUNCH-IN","DOUBLE TAP FX"},
 {"TEMPORARY FX","HOLD FX TO ACTIVATE"},
 {"LIVE KEY FX","SEQ + WHITE KEYS"},
 {"CHANGE EFFECT","HOLD KEY + TURN K1"},
 {"GLOBAL BLEND","K1: DRY/WET"},
 {"PITCH AND REC","-24..+24 / RECORD AUTOMATION"},
 /* KITS / SAMPLES */
 {"BUILT-IN BEETLE","PCM ROM DRUM SOUNDS"},
 {"BUILT-IN MONOLITH","PROCEDURAL ROM DRUM SOUNDS"},
 {"EDIT A DRUM","EDIT OPENS DRUM EDIT"},
 {"KIT DESIGNER","PRESS EDIT AGAIN"},
 {"GLOBAL USER KITS","12 PRESETS; SAVE FROM EDIT"},
 {"DRUM MIXER","PRESS GLO TWICE"},
 {"USER WAV SLOTS","U01-U13 NEED STORAGE V2"},
 /* PROJECT STORAGE */
 {"PROJECT MENU","PRESS HOME"},
 {"LOAD A PROJECT","K1 SLOT / SEL LOAD"},
 {"DELETE A SLOT","EDIT ON OCCUPIED SLOT"},
 {"CONFIRM DELETE","CHOOSE NO/YES THEN SEL"},
 {"PROJECT FORMAT","BDR1 V33 SAVES LANE LENGTHS"},
 {"OLD PROJECTS","SUPPORTED V30-V32 CAN LOAD"},
 {"BACK UP FIRST","OLDER FW CANNOT READ V33"},
 {"GLOBAL USER KITS","BACK UP SETTINGS SEPARATELY"},
 /* LIVE MODE */
 {"OPEN LIVE MODE","HOME > LIVE MODE"},
 {"CHOOSE A VIEW","SETLIST OR PERFORM"},
 {"SETLIST CAPACITY","24 SONGS / 12 PROJECT SLOTS"},
 {"EDIT SETLIST","K1 ROW / K2 PROJECT SLOT"},
 {"REORDER OR EDIT","K4 MOVE / OCT+/- ADD/DEL"},
 {"LEAVE SETLIST","HOME AUTOSAVES CHANGES"},
 {"PERFORM","PROJECT LOADS WITH PROGRESS"},
 {"START NEXT SONG","PRESS PLAY WHEN READY"},
 {"STOP OPTIONS","CONTINUE OR START AGAIN"},
 /* SONG / MIXING */
 {"SONG MODE","PRESS ARP"},
 {"LANE MIXER","PRESS GLO"},
 {"DRUM MIXER","PRESS GLO AGAIN"},
 {"MASTER FX","PRESS LFO"},
 {"COPY / PASTE","SEQ + OP1 / OP2"},
 {"UNDO / REDO","SEQ + OP5 / OP6"},
 /* SAFETY / ABOUT */
 {"RECOVER AUDIO","HOLD HOME + PLAY"},
 {"BEFORE UPDATING","BACK UP PROJECTS AND SAMPLES"},
 {"FLASH OPERATIONS","DO NOT INTERRUPT WRITES"},
 {"USB AUDIO","EXPERIMENTAL 44.1KHZ/16BIT"},
 {"VERSION","BEETLE BLUE V1.0 RC12"},
 {"CREDITS","HOME > CREDITS FOR AUTHORS"},
};
static const struct beetle_help_topic beetle_help_topics[] = {
 {"QUICK START",0u,6u},
 {"LANE LENGTHS",6u,6u},
 {"TRACKER / CH6",12u,6u},
 {"STEP EFFECTS",18u,6u},
 {"PUNCH-IN FX",24u,6u},
 {"KITS / SAMPLES",30u,7u},
 {"PROJECT STORAGE",37u,8u},
 {"LIVE MODE",45u,9u},
 {"SONG / MIXING",54u,6u},
 {"SAFETY / ABOUT",60u,6u},
};
#define BEETLE_HELP_TOPIC_COUNT ((uint8_t)(sizeof(beetle_help_topics)/sizeof(beetle_help_topics[0])))
#define BEETLE_HELP_CARD_COUNT ((uint8_t)(sizeof(beetle_help_cards)/sizeof(beetle_help_cards[0])))
static inline uint8_t beetle_help_page_count(uint8_t topic){
 if(topic>=BEETLE_HELP_TOPIC_COUNT)return 1u;
 uint8_t count=beetle_help_topics[topic].count;
 return (uint8_t)((count+BEETLE_HELP_CARDS_PER_PAGE-1u)/BEETLE_HELP_CARDS_PER_PAGE);
}
static inline uint8_t beetle_help_topic_shift(uint8_t topic,int32_t detents){
 int32_t n=(int32_t)topic+detents;
 if(n<0)n=0;
 if(n>=(int32_t)BEETLE_HELP_TOPIC_COUNT)n=(int32_t)BEETLE_HELP_TOPIC_COUNT-1;
 return (uint8_t)n;
}
static inline uint8_t beetle_help_page_shift(uint8_t topic,uint8_t page,int32_t detents){
 int32_t n=(int32_t)page+detents;
 if(n<0)n=0;
 if(n>=(int32_t)beetle_help_page_count(topic))n=(int32_t)beetle_help_page_count(topic)-1;
 return (uint8_t)n;
}
#endif

/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE Song Chain row range clipboard. No allocation or flash I/O.
 * Independent from the tracker pattern/step clipboard.
 */
#ifndef BEETLE_SONG_ROWS_H
#define BEETLE_SONG_ROWS_H
#include "track_song.h"
struct beetle_song_rows_clip {
 struct track_song_entry rows[TRACK_SONG_ENTRIES];
 uint8_t count,valid;
};
static inline uint8_t beetle_song_rows_in_range(uint8_t row,uint8_t a,uint8_t b){
 return (uint8_t)(row>=(a<b?a:b)&&row<=(a>b?a:b));
}
static inline int beetle_song_rows_copy(struct beetle_song_rows_clip *clip,
 const struct track_song_bank *song,uint8_t first,uint8_t last){
 if(!clip||!song||first>=song->count||last>=song->count)return -1;
 uint8_t lo=first<last?first:last,hi=first>last?first:last;
 clip->valid=0u;clip->count=(uint8_t)(hi-lo+1u);
 for(uint8_t k=0u;k<clip->count;k++)clip->rows[k]=song->chain[(uint8_t)(lo+k)];
 clip->valid=1u;return clip->count;
}
/* Insert the copied rows AFTER the selected Song Chain position. Never
 * overwrite an existing song row. Reject a full chain/invalid cursor before
 * modifying anything, so no partial paste or tail truncation is possible.
 * The source clipboard is independent from the destination chain, including
 * when the copied range comes from that same chain. */
static inline int beetle_song_rows_paste(const struct beetle_song_rows_clip *clip,
 struct track_song_bank *song,uint8_t after){
 if(!clip||!song||!clip->valid||!clip->count ||
    clip->count>TRACK_SONG_ENTRIES || !song->count ||
    song->count>TRACK_SONG_ENTRIES || after>=song->count ||
    (unsigned)song->count+(unsigned)clip->count>TRACK_SONG_ENTRIES)return -1;
 const uint8_t at=(uint8_t)(after+1u);
 /* Shift existing song positions, including the last one, to make room. */
 for(int i=(int)song->count-1;i>=(int)at;i--)
  song->chain[i+(int)clip->count]=song->chain[i];
 for(uint8_t k=0u;k<clip->count;k++)
  song->chain[(uint8_t)(at+k)]=clip->rows[k];
 if(song->position>after)
  song->position=(uint8_t)(song->position+clip->count);
 song->count=(uint8_t)(song->count+clip->count);
 song->edit_row=at; /* Cursor follows the FIRST pasted row. */
 song->play_range_active=0u; /* No stale loop indices after structural edits. */
 return clip->count;
}

#endif

/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE live DPCM4 bridge. Six sample voices REUSE the six drum voices;
 * no seventh-to-twelfth drum voices are silently added. Main-loop prefetch
 * verifies a full V2 record once per voice/sector. DMA renders RAM only.
 */
#include "beetle_bd4_live.h"
#include "beetle_bd4_prefetch.h"
#include <string.h>
#define BD4_CHUNK 4068u
/* 6 * 4068-byte immutable sector caches; every cache retains its sample
 * generation. The PCM voice windows are separately bounded. */
struct live_chunk {uint32_t generation;uint16_t part;uint8_t slot,valid;};
struct live_note {volatile uint32_t serial;volatile uint8_t pending,slot;uint8_t starting,trim_start,trim_end;};
/* Keep a fully decoded, CRC-generation-pinned 256-frame ATTACK snapshot for
 * each user slot. The entire decoder cursor (not just the first PCM pages)
 * is captured, so a warm retrigger resumes without skipping later blocks.
 * Rebuilt on demand if trim or sample generation changes. All cache writes
 * are main-context operations; the audio ISR never sees this array. */
struct live_attack { struct beetle_bd4_stream_voice prime;
 uint32_t generation;uint8_t valid,trim_start,trim_end; };
/* The existing project staging region is used only when NO project load,
 * save, preview or sample-transfer operation is in flight. This ownership
 * contract is enforced by the caller; the audio DMA is paused on reuse. */
struct live_runtime {
 struct beetle_bd4_stream stream;
 struct live_chunk chunks[BEETLE_BD4_STREAM_VOICES];
 struct live_note notes[BEETLE_BD4_STREAM_VOICES];
 struct live_attack attack[BEETLE_BD4_STREAM_SLOTS];
 struct beetle_bd4_stream warm_stream;
 uint8_t warm_slot,warm_active;
 struct beetle_v2_sample slots[BEETLE_BD4_STREAM_SLOTS];
 char labels[BEETLE_BD4_STREAM_SLOTS][4];
 struct beetle_v2_device device;
 uint8_t index[4096];
 uint8_t chunk_scratch[4068];
 uint8_t valid,refresh,cached_voice,cached_valid;
 uint32_t failed_reads;
};
static struct live_runtime *live_rt;
#define live_stream (live_rt->stream)
#define live_chunks (live_rt->chunks)
#define live_notes (live_rt->notes)
#define live_slots (live_rt->slots)
#define live_device (live_rt->device)
#define live_index (live_rt->index)
#define live_chunk_scratch (live_rt->chunk_scratch)
#define live_valid (live_rt->valid)
#define live_refresh (live_rt->refresh)
#define live_failed_reads (live_rt->failed_reads)
void beetle_bd4_live_bind(void *workspace,uint32_t bytes){
 live_rt=(workspace&&bytes>=sizeof(struct live_runtime))?
       (struct live_runtime *)workspace:0;
 if(live_rt){memset(live_rt,0,sizeof(*live_rt));live_rt->refresh=1u;}
}
void beetle_bd4_live_init(void){
 if(live_rt)beetle_bd4_live_bind(live_rt,sizeof(*live_rt));
}
void beetle_bd4_live_invalidate(void){if(live_rt){live_refresh=1u;live_valid=0u;live_rt->cached_valid=0u;live_rt->warm_active=0u;
  memset(live_rt->attack,0,sizeof(live_rt->attack));
  memset(live_chunks,0,sizeof(live_chunks));}}
const char *beetle_bd4_live_label(uint8_t slot){
 return live_rt&&slot<BEETLE_BD4_STREAM_SLOTS&&live_valid&&live_rt->labels[slot][0]?live_rt->labels[slot]:0;
}
int beetle_bd4_live_available(uint8_t slot){
 return live_rt&&slot<BEETLE_BD4_STREAM_SLOTS && live_valid && live_slots[slot].generation!=0u;
}
void beetle_bd4_live_request(uint8_t voice,uint8_t slot,uint32_t serial){
 if(!live_rt||voice>=BEETLE_BD4_STREAM_VOICES||!beetle_bd4_live_available(slot))return;
 struct live_note *n=&live_notes[voice];
 n->serial=serial;n->slot=slot;n->trim_start=0u;n->trim_end=100u;
 n->pending=1u;n->starting=1u;
}
void beetle_bd4_live_set_trim(uint8_t voice,uint8_t start,uint8_t end,uint32_t serial){
 if(!live_rt||voice>=BEETLE_BD4_STREAM_VOICES||start>=end||end>100u)return;
 struct live_note *n=&live_notes[voice];
 if(n->serial==serial && n->pending){n->trim_start=start;n->trim_end=end;}
}
int beetle_bd4_live_next(uint8_t voice,uint32_t serial,int16_t *sample){
 if(!live_rt||!sample||voice>=BEETLE_BD4_STREAM_VOICES)return -1;
 *sample=0;
 struct live_note *n=&live_notes[voice];
 if(n->serial!=serial)return -1;
 if(n->pending||n->starting)return 0;
 /* Combined window availability + PCM read, with no duplicated per-frame
  * window scan. A cache miss holds the musical cursor until foreground
  * prefetch fills RAM; neither decoding nor flash I/O runs in the DMA ISR. */
 return beetle_bd4_stream_read_one_ready(&live_stream,voice,sample);
}
void beetle_bd4_live_stop(uint8_t voice){
 if(!live_rt||voice>=BEETLE_BD4_STREAM_VOICES)return;
 /* An audio IRQ can interrupt the main-loop DPCM decoder halfway
  * through filling a PCM window. Never mutate that decoder from IRQ.
  * Mark the note stopped; main-loop poll performs the stream choke. */
 live_notes[voice].serial=0u;
 live_notes[voice].pending=0u;
 live_notes[voice].starting=0u;
}
void beetle_bd4_live_choke_all(void){
 if(!live_rt)return;
 for(unsigned i=0;i<BEETLE_BD4_STREAM_VOICES;i++){
  live_notes[i].serial=0u;
  live_notes[i].pending=0u;live_notes[i].starting=0u;
  /* Deferred decoder cleanup occurs in the background poll. */
 }
}
uint8_t beetle_bd4_live_start_pending(void){
 if(!live_rt)return 0u;
 for(unsigned i=0u;i<BEETLE_BD4_STREAM_VOICES;i++){
  const struct live_note *n=&live_notes[i];
  if(n->serial && (n->pending||n->starting))return 1u;
 }
 return 0u;
}
uint32_t beetle_bd4_live_underruns(void){
 if(!live_rt)return 0u;
 uint32_t n=live_failed_reads;
 for(unsigned i=0;i<BEETLE_BD4_STREAM_VOICES;i++)n+=live_stream.voices[i].underruns;
 return n;
}
/* main-loop ONLY. The index and sector CRCs are checked outside audio IRQ.
 * This callback cannot silently follow a replaced U-slot generation. */
static int live_read(void *unused,uint8_t voice,uint8_t slot,uint32_t generation,
 uint32_t offset,uint8_t *dst,uint16_t size){
 (void)unused;
 if(!live_valid||voice>=6u||slot>=13u||!dst||!size||
    !live_slots[slot].generation||generation!=live_slots[slot].generation||
    offset>live_slots[slot].byte_length||size>live_slots[slot].byte_length-offset)return 0;
 struct live_chunk *c=&live_chunks[voice];
 while(size){
  uint16_t part=(uint16_t)(offset/BD4_CHUNK);
  uint16_t within=(uint16_t)(offset%BD4_CHUNK);
  uint16_t count=(uint16_t)(BD4_CHUNK-within);
  if(count>size)count=size;
  /* There is only ONE shared 4068-byte cache, not six independent sector
   * buffers. Each voice does, however, remember which immutable, pinned
   * sector was already CRC-verified. When switching between six samples,
   * re-verifying entire sectors every 2.9ms starves audio prefetch. */
  const uint8_t verified=(uint8_t)(c->valid && c->part==part &&
                                   c->slot==slot && c->generation==generation);
  uint8_t cached=(uint8_t)(verified && live_rt->cached_valid &&
                            live_rt->cached_voice==voice);
  if(!verified){
   c->valid=0u;live_rt->cached_valid=0u;
   uint16_t verified_length=0u;
   if(beetle_v2_sample_chunk_read(&live_device,slot,&live_slots[slot],part,
                                  live_chunk_scratch,&verified_length,live_index)){
    live_failed_reads++;return 0;
   }
   c->part=part;c->slot=slot;c->generation=generation;c->valid=1u;
   live_rt->cached_voice=voice;live_rt->cached_valid=1u;
   cached=1u;
  }
  uint32_t q=(uint32_t)live_slots[slot].index_offset+(uint32_t)part*12u;
  uint32_t rel=(uint32_t)live_index[q] | ((uint32_t)live_index[q+1]<<8) |
               ((uint32_t)live_index[q+2]<<16)|((uint32_t)live_index[q+3]<<24);
  uint32_t bytes=(uint32_t)live_index[q+4] | ((uint32_t)live_index[q+5]<<8) |
               ((uint32_t)live_index[q+6]<<16)|((uint32_t)live_index[q+7]<<24);
  if(bytes>BD4_CHUNK||within>bytes||count>bytes-within||
     rel<8192u||rel>BEETLE_V2_BYTES-24u-bytes){
    c->valid=0u;live_rt->cached_valid=0u;return 0;
  }
  if(cached){
   /* Cache hit: absolutely no flash transfer or CRC work. */
   memcpy(dst,live_chunk_scratch+within,count);
  }else{
   /* Different voice now owns shared cache. This sector was already
    * verified under a PINNED generation; read only 6..134 encoded bytes.
    * Transfers cannot mutate that generation while playback is active. */
   if(live_device.read(live_device.context,BEETLE_V2_BASE+rel+24u+within,
                       dst,count)){
    c->valid=0u;live_failed_reads++;return 0;
   }
  }
  offset+=count;dst+=count;size=(uint16_t)(size-count);
 }
 return 1;
}
/* Poll is intentionally bounded: one window per voice per invocation.
 * Audio interrupt never accesses V2 storage, USB or DPCM4 decode. */
void beetle_bd4_live_poll(beetle_v2_read read,void *ctx,void (*lock)(void),void (*unlock)(void)){
 if(!read||!live_rt||!lock||!unlock)return;
 if(live_refresh){
  /* Installation/migration required: refuses the legacy layout. */
  live_valid=0u;live_rt->cached_valid=0u;live_rt->warm_slot=0u;live_rt->warm_active=0u;
  memset(live_rt->attack,0,sizeof(live_rt->attack));memset(live_chunks,0,sizeof(live_chunks));memset(live_slots,0,sizeof(live_slots));memset(live_rt->labels,0,sizeof(live_rt->labels));
  if(beetle_v2_open(&live_device,read,ctx,live_index)==0){
   live_valid=1u;
   for(uint8_t i=0;i<13u;i++){
    struct beetle_v2_sample info;
    if(beetle_v2_sample_info(&live_device,i,&info,live_index)==1){
     live_slots[i]=info;
     if(info.byte_length>=30u){
      uint8_t footer[8];
      if(beetle_v2_sample_read(&live_device,i,info.byte_length-8u,footer,8u,live_index)==0 &&
         footer[0]=='B'&&footer[1]=='N'&&footer[2]=='M'&&footer[3]=='1'&&
         (uint8_t)(footer[4]^footer[5]^footer[6]^0xA5u)==footer[7]){
       uint8_t good=1u;
       for(unsigned j=0;j<3u;j++)if(footer[4u+j]<32u||footer[4u+j]>126u)good=0u;
       if(good){for(unsigned j=0;j<3u;j++)live_rt->labels[i][j]=(char)footer[4u+j];live_rt->labels[i][3]=0;}
      }
     }
    }
   }
  }
  live_refresh=0u;
 }
 if(!live_valid)return;
 /* Finish stopped voices on the producer side. This avoids the IRQ
  * interrupting prefetch in the middle of modifying a stream/window. */
 for(unsigned i=0u;i<6u;i++){
  lock();
  if(!live_notes[i].serial && !live_notes[i].pending &&
     live_stream.voices[i].configured){
   beetle_bd4_stream_choke_voice(&live_stream,i);
   /* A stopped stream must not consume background flash bandwidth. */
   live_stream.voices[i].configured=0u;
   live_stream.voices[i].playing=0u;
  }
  unlock();
 }
 for(unsigned i=0;i<6u;i++){
  struct live_note *n=&live_notes[i];
  /* Snapshot and acknowledge a request atomically. An ISR retrigger
   * between reading pending and clearing it previously lost the NEW note.
   * Never leave IRQs masked across storage reads or DPCM decode. */
  lock();
  const uint8_t pending=n->pending;
  const uint32_t seq=n->serial;
  const uint8_t slot=n->slot;
  if(pending)n->pending=0u;
  unlock();
  if(pending){
   /* Rapid retrigger of an unchanged sample MUST NOT invalidate the
    * pinned/CRC-verified sector: repeated 4KiB reads and full CRCs
    * can starve the cooperative prefetch path while audio DMA runs. */
   if(!beetle_bd4_live_available(slot) || !live_chunks[i].valid ||
      live_chunks[i].slot!=slot ||
      live_chunks[i].generation!=live_slots[slot].generation){
    live_chunks[i].valid=0u;
    if(live_rt->cached_voice==i)live_rt->cached_valid=0u;
   }
   if(!beetle_bd4_live_available(slot)){
    n->starting=0u;n->serial=0u;continue;
   }
   /* ISR is gated by starting=1 or pending=1 for this sequence. Restore
    * the attack outside the IRQ-masked section; 748-byte copies must not
    * monopolize DMA scheduling. Re-check sequence before publication. */
   struct live_attack *a=&live_rt->attack[slot];
   const uint8_t trim_start=n->trim_start,trim_end=n->trim_end;
   const uint8_t hot=(uint8_t)(a->valid &&
     a->generation==live_slots[slot].generation &&
     a->trim_start==trim_start && a->trim_end==trim_end);
   if(hot){
    memcpy(&live_stream.voices[i],&a->prime,sizeof(a->prime));
   }else{
    int prepare=beetle_bd4_stream_prepare_pinned(&live_stream,i,slot,
      live_slots[slot].byte_length,live_slots[slot].generation);
    if(prepare){n->starting=0u;n->serial=0u;continue;}
    beetle_bd4_stream_trim(&live_stream,i,trim_start,trim_end);
   }
   lock();
   if(n->serial!=seq || n->pending){unlock();continue;}
   if(hot && beetle_bd4_stream_start(&live_stream,i))n->starting=0u;
   unlock();
   /* An IRQ may have retriggered the same drum voice during this setup.
    * Its newer serial/pending flag must win. */
   if(n->serial!=seq||n->pending)continue;
  }
 }
 (void)beetle_bd4_prefetch_poll_pinned(&live_stream,live_read,0,6u);
 for(unsigned i=0;i<6u;i++){
  struct live_note *n=&live_notes[i];
  if(n->starting&&!n->pending){
   /* Cache copy is FOREGROUND-only and is deliberately OUTSIDE the IRQ
    * critical section. The producer has not published starting=0 yet. */
   if(n->pending || !n->serial)continue;
   struct beetle_bd4_stream_voice *v=&live_stream.voices[i];
   if(v->configured && v->verified_header &&
      v->windows[0].valid &&
      (v->windows[1].valid||v->frame_count<=BEETLE_BD4_WINDOW_FRAMES) &&
      v->slot<BEETLE_BD4_STREAM_SLOTS && v->generation==live_slots[v->slot].generation){
    struct live_attack *a=&live_rt->attack[v->slot];
    /* Snapshot is never read from IRQ. Retain the immutable first two
     * windows and decoder continuation state before starting playback. */
    a->valid=0u;
    memcpy(&a->prime,v,sizeof(*v));
    a->generation=v->generation;
    a->trim_start=v->trim_start_percent;a->trim_end=v->trim_end_percent;
    a->valid=1u;
   }
   lock();
   /* The IRQ can retrigger at any point during the foreground cache copy.
    * Start only the serial that survived the entire snapshot operation. */
   if(!n->pending && n->serial){
    if(beetle_bd4_stream_start(&live_stream,i))n->starting=0u;
    else if(!live_stream.voices[i].configured || live_stream.voices[i].invalid_blocks){
     n->starting=0u;n->serial=0u;
    }
   }
   unlock();
  }
 }
}

/* PRE-RC V0.99.4: Prewarm immutable start windows in foreground while STOPPED.
 * All thirteen installed U-slots are scanned incrementally (at most ONE
 * 128-frame window per invocation), never in DMA and never during playback.
 * Thereafter any lane/voice can instantaneously restore its cached first
 * 256 frames and the exact ADPCM decoder continuation cursor. The 13 caches
 * live in the existing V2 project staging workspace; no new .bss/.pool RAM.
 */
void beetle_bd4_live_warm_idle(void){
 if(!live_rt || !live_valid || live_refresh)return;
 for(unsigned v=0u;v<BEETLE_BD4_STREAM_VOICES;v++)
  if(live_notes[v].pending || live_notes[v].starting ||
     live_stream.voices[v].playing)return;
 while(live_rt->warm_slot<BEETLE_BD4_STREAM_SLOTS){
  uint8_t slot=live_rt->warm_slot;
  if(!live_slots[slot].generation){live_rt->warm_slot++;live_rt->warm_active=0u;continue;}
  struct live_attack *a=&live_rt->attack[slot];
  if(a->valid && a->generation==live_slots[slot].generation){
   live_rt->warm_slot++;live_rt->warm_active=0u;continue;
  }
  if(!live_rt->warm_active){
   beetle_bd4_stream_reset(&live_rt->warm_stream);
   if(beetle_bd4_stream_prepare_pinned(&live_rt->warm_stream,0u,slot,
       live_slots[slot].byte_length,live_slots[slot].generation)){
    live_rt->warm_slot++;continue;
   }
   beetle_bd4_stream_trim(&live_rt->warm_stream,0u,0u,100u);
   live_rt->warm_active=1u;
  }
  (void)beetle_bd4_prefetch_poll_pinned(&live_rt->warm_stream,live_read,0,1u);
  struct beetle_bd4_stream_voice *v=&live_rt->warm_stream.voices[0];
  if(v->configured && v->verified_header && v->windows[0].valid &&
     (v->windows[1].valid||v->frame_count<=BEETLE_BD4_WINDOW_FRAMES)){
   a->valid=0u;memcpy(&a->prime,v,sizeof(*v));
   a->generation=v->generation;a->trim_start=0u;a->trim_end=100u;
   a->valid=1u;live_rt->warm_active=0u;live_rt->warm_slot++;
  }else if(!v->configured || v->invalid_blocks){
   live_rt->warm_active=0u;live_rt->warm_slot++;
  }
  break; /* strictly one decode/fetch window per cooperative foreground pass */
 }
}
/* Read-only diagnostics: one bit per resident sample attack cache. */
uint16_t beetle_bd4_live_hot_mask(void){
 if(!live_rt)return 0u;
 uint16_t bits=0u;
 for(unsigned i=0u;i<BEETLE_BD4_STREAM_SLOTS;i++)
  if(live_rt->attack[i].valid &&
     live_rt->attack[i].generation==live_slots[i].generation)
   bits|=(uint16_t)(1u<<i);
 return bits;
}

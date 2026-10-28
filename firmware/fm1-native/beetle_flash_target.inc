/* SPDX-License-Identifier: GPL-3.0-only
 * BEETLE standalone V0.34 physical flash backend. Included by bringup.c after
 * fm1_flash.h via wave_store_target.c. All flash operations are outside audio.
 * No writes happen until a manual hold-to-format, or in an owned TRKJ journal.
 */
#include "beetle_flash_layout.h"
#if !defined(TRACK_FIRMWARE) || !BEETLE_NATIVE_STORAGE || ETCH_NATIVE_STORAGE
#error "Standalone BEETLE storage cannot be linked into an ETCH storage build"
#endif
static uint8_t beetle_write_permit;
/* Only the physical on-device double-YES PROJECTS flow may set this. */
static uint8_t beetle_full_wipe_confirmed;
static int beetle_storage_last_audit=BEETLE_CLAIM_INVALID;
static int beetle_hal_abs_read(void *ctx,uint32_t off,uint8_t *dst,uint32_t n){
 (void)ctx;
 if(!dst||!n||!((FL_IN(off,n,BEETLE_FLASH_BASE,BEETLE_FLASH_END))||
                   FL_IN(off,n,BEETLE_FLASH_MARKER,BEETLE_FLASH_MARKER_END)))return -1;
 while(n){
  uint32_t k=n>256u?256u:n;
  fm1_irq_off();int rc=FL_FAR(fl_read_ram)(off,dst,k);fm1_irq_enable_all();
  fm1_wdt_feed();if(rc)return rc;
  off+=k;dst+=k;n-=k;
 }
 return 0;
}
static int beetle_hal_probe_ownership(void){
 int rc=beetle_flash_claim_probe(beetle_hal_abs_read,0);
 beetle_storage_last_audit=rc;
 return rc;
}
/* BEETLE V1.4.20: V2 migration uses an explicitly versioned marker.
 * Verify the EXACT marker and entire protected marker sector. A V2 marker
 * is NOT equivalent to the legacy journal marker: their writers must NEVER
 * operate on each other's storage layout. */
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
#include "beetle_storage_v2_reader.h"
static int beetle_hal_v2_marker_prefix(void){
 static const uint8_t magic[8]={'B','T','L','S','T','V','2',1u};
 uint8_t hdr[8];
 if(beetle_hal_abs_read(0,BEETLE_FLASH_MARKER,hdr,sizeof(hdr)))return -1;
 return memcmp(hdr,magic,sizeof(magic))==0?1:0;
}
static int beetle_hal_probe_v2_ownership(void){
 uint8_t marker[32],expected[32];
 if(beetle_hal_abs_read(0,BEETLE_FLASH_MARKER,marker,sizeof(marker)))return BEETLE_CLAIM_IO;
 beetle_v2_make_marker(expected);
 if(memcmp(marker,expected,sizeof(expected)))return BEETLE_CLAIM_FOREIGN;
 uint8_t block[256];
 for(uint32_t at=BEETLE_FLASH_MARKER+32u;at<BEETLE_FLASH_MARKER_END;){
  uint32_t n=BEETLE_FLASH_MARKER_END-at;if(n>sizeof(block))n=sizeof(block);
  if(beetle_hal_abs_read(0,at,block,n))return BEETLE_CLAIM_IO;
  for(uint32_t i=0;i<n;i++)if(block[i]!=0xffu)return BEETLE_CLAIM_FOREIGN;
  at+=n;
 }
 return BEETLE_CLAIM_EXISTING;
}
static int beetle_hal_probe_ownership_ui(void){
 int is_v2=beetle_hal_v2_marker_prefix();
 int rc=is_v2<0?BEETLE_CLAIM_IO:is_v2?beetle_hal_probe_v2_ownership():beetle_hal_probe_ownership();
 beetle_storage_last_audit=rc;
 return rc;
}
#else
static int beetle_hal_probe_ownership_ui(void){return beetle_hal_probe_ownership();}
#endif
static int beetle_hal_journal_read(void *ctx,uint8_t slot,uint8_t bank,
 uint32_t offset,uint8_t *dst,uint32_t n){
 uint32_t addr;(void)ctx;
 if(beetle_flash_map(slot,bank,offset,n,&addr))return -1;
 return beetle_hal_abs_read(0,addr,dst,n);
}
static int beetle_hal_journal_erase(void *ctx,uint8_t slot,uint8_t bank){
 uint32_t addr;(void)ctx;
 if(!beetle_write_permit||beetle_hal_probe_ownership()!=BEETLE_CLAIM_EXISTING||
    beetle_flash_map(slot,bank,0u,TRACK_JOURNAL_BANK_BYTES,&addr))return -1;
 for(uint32_t at=0u;at<TRACK_JOURNAL_BANK_BYTES;at+=BEETLE_FLASH_SECTOR){
  uint32_t took=0u;
  fm1_irq_off();int rc=FL_FAR(fl_erase4k_ram)(addr+at,&took);fm1_irq_enable_all();
  fm1_wdt_feed();if(rc)return rc;
 }
 return 0;
}
static int beetle_hal_journal_program(void *ctx,uint8_t slot,uint8_t bank,
 uint32_t offset,const uint8_t *src,uint32_t n){
 uint32_t addr;(void)ctx;
 if(!src||!n||!beetle_write_permit||beetle_hal_probe_ownership()!=BEETLE_CLAIM_EXISTING||
    beetle_flash_map(slot,bank,offset,n,&addr))return -1;
 while(n){
  uint32_t k=256u-(addr&255u);if(k>n)k=n;
  uint32_t took=0u;
  fm1_irq_off();int rc=FL_FAR(fl_prog_ram)(addr,src,k,&took);fm1_irq_enable_all();
  fm1_wdt_feed();if(rc)return rc;
  addr+=k;src+=k;n-=k;
 }
 return 0;
}
static const struct track_journal_io beetle_hal_journal_io={
 0,beetle_hal_journal_read,beetle_hal_journal_erase,beetle_hal_journal_program
};
/* Explicitly repair ONLY a missing/CRC-corrupt marker, NEVER a foreign one.
 * The ENTIRE A/B area must first be audited read-only for ownership. */
static int beetle_hal_marker_audit(void){
 int ownership=beetle_hal_probe_ownership();
 if(ownership==BEETLE_CLAIM_EXISTING)return 0; /* nothing to repair */
 if(ownership==BEETLE_CLAIM_IO)return -2;
 int marker=beetle_flash_marker_repairable(beetle_hal_abs_read,0);
 if(marker<0)return -2;
 if(marker!=1)return -1;
 int banks=track_journal_audit_owned_volume(&beetle_hal_journal_io);
 if(banks==TRACK_JOURNAL_IO)return -2;
 if(banks==TRACK_JOURNAL_NO_PROJECT)return -3;
 if(banks!=TRACK_JOURNAL_OK)return -1;
 return 1; /* repair is eligible, but still requires explicit MIDI consent */
}
static int beetle_hal_restore_marker(void){
 if(!beetle_write_permit||native_safe||native_tripped||native_updating)return -1;
 if(beetle_hal_marker_audit()!=1)return -1;
 uint32_t took=0u;
 fm1_irq_off();int rc=FL_FAR(fl_erase4k_ram)(BEETLE_FLASH_MARKER,&took);fm1_irq_enable_all();
 fm1_wdt_feed();if(rc)return -2;
 uint8_t block[256];
 for(uint32_t at=BEETLE_FLASH_MARKER;at<BEETLE_FLASH_MARKER_END;at+=sizeof(block)){
  if(beetle_hal_abs_read(0,at,block,sizeof(block)))return -2;
  for(uint16_t j=0;j<sizeof(block);j++)if(block[j]!=0xFFu)return -2;
 }
 uint8_t marker[32];beetle_flash_make_marker(marker);
 fm1_irq_off();rc=FL_FAR(fl_prog_ram)(BEETLE_FLASH_MARKER,marker,sizeof(marker),&took);fm1_irq_enable_all();
 fm1_wdt_feed();if(rc)return -2;
 return beetle_hal_probe_ownership()==BEETLE_CLAIM_EXISTING?0:-2;
}

/* RC1.0.5: explicitly consented factory reinitialization of the fixed
 * BEETLE PROJECT PARTITION, INCLUDING any older ETCH/Felucca user records
 * within that same range. NO firmware/header/OTA sectors are addressed.
 * A separate physical double-YES gesture is required even if BEETLE's
 * marker is absent or corrupt. This does NOT enable normal journal writes
 * to foreign/unknown data; only this whole-volume reinitialization can
 * erase it. Any flash read error or unknown runtime mode is rejected. */
/* RC7: physical double-YES Wipe All Storage ALWAYS formats Storage V2
 * when compiled with BEETLE_NATIVE_V2, including recognized legacy TRKJ.
 * This is a DESTRUCTIVE FORMAT, never an automatic in-place migration.
 * Foreign/unknown ownership is still rejected, and boot/OTA are untouched. */
static int beetle_hal_wipe_projects(void (*progress)(uint8_t,uint8_t)){
 if(!beetle_write_permit||!beetle_full_wipe_confirmed||
    native_safe||native_tripped||native_updating)return -1;
 int use_v2=0;
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 /* Output format is always V2. Input ownership must be recognized first. */
 use_v2=1;
 int has_v2_marker=beetle_hal_v2_marker_prefix();
 if(has_v2_marker<0)return -1;
 if(has_v2_marker){
  /* Corrupt superblocks can be reformatted, but the MARKER itself must
   * still prove BEETLE ownership before touching any sector. */
  if(beetle_hal_probe_v2_ownership()!=BEETLE_CLAIM_EXISTING)return -1;
 }else
#endif
 {
  int owner=beetle_hal_probe_ownership();
  /* Only a valid legacy marker or a completely blank area may be taken over.
   * NEVER erase foreign, damaged-marker, or unreadable storage blindly. */
  if(owner!=BEETLE_CLAIM_EXISTING&&owner!=BEETLE_CLAIM_BLANK)return -1;
 }
 const uint8_t total=(uint8_t)((BEETLE_FLASH_MARKER_END-BEETLE_FLASH_BASE)/BEETLE_FLASH_SECTOR);
 uint8_t checked[128];uint8_t done=0u;
 /* The marker sector is erased LAST. If power fails before the commit,
  * recovery requires an explicit new wipe and never claims successful save. */
 for(uint32_t off=BEETLE_FLASH_BASE;off<BEETLE_FLASH_MARKER_END;off+=BEETLE_FLASH_SECTOR){
  uint32_t took=0u;
  fm1_irq_off();int rc=FL_FAR(fl_erase4k_ram)(off,&took);fm1_irq_enable_all();
  fm1_wdt_feed();if(rc)return -2;
  for(uint32_t k=0u;k<BEETLE_FLASH_SECTOR;k+=sizeof(checked)){
   if(beetle_hal_abs_read(0,off+k,checked,sizeof(checked)))return -3;
   for(uint8_t j=0u;j<sizeof(checked);j++)if(checked[j]!=0xFFu)return -3;
  }
  done++;
  if(progress&&(done==total||(done%4u)==0u))progress(done,total);
 }
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 if(use_v2){
  /* Build the canonical EMPTY B2SB superblock with the same payload and
   * CRC as Storage V2's read-only parser and Python migration tooling.
   * The 37K non-realtime staging RAM is free while transport is paused. */
  uint8_t *index=(uint8_t *)(void *)&beetle_staged,*crcwork=index+4096u;
  const uint32_t payload=12u*32u+13u*14u;
  memset(index,0xff,4096u);memset(crcwork,0,16u+payload);
  /* CRITICAL RC7: B2SB parser requires zero upper bytes of version,
   * flags, generation and length. RC4-RC6 left those at FF after the
   * erased-sector fill, so no reader could ever mount the formatted V2! */
  memset(index+4u,0,16u);
  memcpy(index,"B2SB",4u);index[4]=1u;index[8]=1u;
  index[12]=(uint8_t)payload;index[13]=(uint8_t)(payload>>8u);
  memset(index+20u,0,payload);
  memcpy(crcwork,index,16u);
  uint32_t crc=track_store_crc32(crcwork,16u+payload);
  for(uint8_t k=0u;k<4u;k++)index[16u+k]=(uint8_t)(crc>>(8u*k));
  memcpy(index+4092u,"B2OK",4u);
  /* Write full verified superblock BEFORE claiming the volume marker. */
  for(uint16_t off=0u;off<4096u;off+=256u){
   uint32_t took=0u;
   fm1_irq_off();int rc=FL_FAR(fl_prog_ram)(BEETLE_V2_BASE+off,index+off,256u,&took);fm1_irq_enable_all();
   fm1_wdt_feed();if(rc)return -4;
  }
  for(uint16_t off=0u;off<4096u;off+=128u){
   if(beetle_hal_abs_read(0,BEETLE_V2_BASE+off,checked,sizeof(checked))||
      memcmp(checked,index+off,sizeof(checked)))return -5;
  }
  struct beetle_v2_device v2;
  if(beetle_v2_open(&v2,beetle_hal_abs_read,0,crcwork)||v2.generation!=1u)return -5;
 }
#endif
 uint8_t marker[32];
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 if(use_v2)beetle_v2_make_marker(marker);
 else
#endif
 beetle_flash_make_marker(marker);
 uint32_t took=0u;
 fm1_irq_off();int rc=FL_FAR(fl_prog_ram)(BEETLE_FLASH_MARKER,marker,sizeof(marker),&took);fm1_irq_enable_all();
 fm1_wdt_feed();if(rc)return -6;
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 if(use_v2){
  if(beetle_hal_probe_v2_ownership()!=BEETLE_CLAIM_EXISTING)return -7;
  uint8_t *scratch=(uint8_t *)(void *)&beetle_staged;
  struct beetle_v2_device d;
  if(beetle_v2_open(&d,beetle_hal_abs_read,0,scratch))return -7;
  for(uint8_t slot=0u;slot<TRACK_PROJECTS;slot++){
   uint8_t *packed=scratch+8192u;uint32_t decoded=0u;
   if(beetle_v2_project_read(&d,slot,beetle_codec_buffer,sizeof(beetle_codec_buffer),
       &decoded,scratch,packed,12272u)!=0)return -7;
  }
  beetle_storage_last_audit=BEETLE_CLAIM_EXISTING;
  return 0;
 }
#endif
 if(beetle_hal_probe_ownership()!=BEETLE_CLAIM_EXISTING)return -7;
 for(uint8_t slot=0u;slot<TRACK_PROJECTS;slot++){
  char title[13];
  if(track_journal_preview(&beetle_hal_journal_io,slot,title)!=TRACK_JOURNAL_NO_PROJECT)return -7;
 }
 return 0;
}
/* Destructive provisioning allowed ONLY from TRACK's explicit five-second
 * SELECT+SAVE consent screen. The mark is written LAST, so any interrupted
 * formatting stays unowned and can never accidentally enable journal writes.
 * This standalone partition DESTROYS any prior ETCH/Felucca user data in it.
 */
static int beetle_hal_init_exclusive(void (*progress)(uint8_t,uint8_t)){
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 if(beetle_hal_v2_marker_prefix()!=0)return -1;
#endif
 if(native_safe||native_tripped||native_updating)return -1;
 int prev=beetle_hal_probe_ownership();
 if(prev==BEETLE_CLAIM_EXISTING)return 0; /* never erase saved BEETLE files */
 if(prev==BEETLE_CLAIM_IO||prev==BEETLE_CLAIM_INVALID)return -1;
 /* Never touch boot header, app, 0xE0000 OTA, globals or 0xE5000 autosave. */
 const uint8_t total=(uint8_t)((BEETLE_FLASH_MARKER_END-BEETLE_FLASH_BASE)/BEETLE_FLASH_SECTOR);
 uint8_t done=0u;
 for(uint32_t off=BEETLE_FLASH_BASE;off<BEETLE_FLASH_MARKER_END;off+=BEETLE_FLASH_SECTOR){
  uint32_t took=0u;
  fm1_irq_off();int rc=FL_FAR(fl_erase4k_ram)(off,&took);fm1_irq_enable_all();
  fm1_wdt_feed();if(rc)return -2;
  uint8_t verify[256];
  for(uint32_t k=0;k<BEETLE_FLASH_SECTOR;k+=sizeof(verify)){
   if(beetle_hal_abs_read(0,off+k,verify,sizeof(verify)))return -3;
   for(uint32_t j=0;j<sizeof(verify);j++)if(verify[j]!=0xFFu)return -4;
  }
  ++done;
  if(progress&&(done==total || (done%4u)==0u))progress(done,total);
 }
 uint8_t marker[32];beetle_flash_make_marker(marker);
 uint32_t took=0u;
 fm1_irq_off();int rc=FL_FAR(fl_prog_ram)(BEETLE_FLASH_MARKER,marker,sizeof(marker),&took);fm1_irq_enable_all();
 fm1_wdt_feed();if(rc)return -5;
 return beetle_hal_probe_ownership()==BEETLE_CLAIM_EXISTING?0:-6;
}

#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
/* Storage V2 absolute-address SPI NOR adapter. Project operations are in
 * main context with transport PAUSED. No flash reads or writes in DMA ISR.
 * Storage ownership: EXACT 0x97000..0xDEFFF, with legacy marker preserved.
 * The B2SB volume must already have been migrated by a verified recovery
 * image; unknown/legacy bytes are never auto-reformatted here. */
static int beetle_v2_hal_read(void *ctx,uint32_t address,uint8_t *data,uint32_t n){
 (void)ctx;return beetle_hal_abs_read(0,address,data,n);
}
/* BLUE PRE-RC V0.99.4: audio sample prefetch shares the XIP flash bus
 * with interrupt instruction fetch. The general 256-byte transaction above
 * masks IRQs for too long when hit repeatedly by six BD4S streams. Bound
 * EACH IRQ-masked flash transaction to 32 bytes (8x smaller). The caller
 * runs only in foreground, never inside ALNK0/DMA; it performs no writes. */
#define BEETLE_BLUE_LIVE_FLASH_BURST 32u
static int beetle_v2_live_read(void *ctx,uint32_t address,uint8_t *dst,uint32_t n){
 (void)ctx;
 if(!dst || !n || !FL_IN(address,n,BEETLE_FLASH_BASE,BEETLE_FLASH_END))return -1;
 while(n){
  uint32_t k=n>BEETLE_BLUE_LIVE_FLASH_BURST?BEETLE_BLUE_LIVE_FLASH_BURST:n;
  fm1_irq_off();
  int rc=FL_FAR(fl_read_ram)(address,dst,k);
  fm1_irq_enable_all();
  if(rc)return rc;
  address+=k;dst+=k;n-=k;
 }
 fm1_wdt_feed();return 0;
}
static int beetle_v2_hal_program(void *ctx,uint32_t address,const uint8_t *data,uint32_t n){
 (void)ctx;
 if(!data||!n||!beetle_write_permit||native_safe||native_tripped||native_updating||
    address<BEETLE_FLASH_BASE||address>BEETLE_FLASH_END||
    n>BEETLE_FLASH_END-address||((address&255u)+n)>256u)return -1;
 uint32_t took=0;
 fm1_irq_off();int rc=FL_FAR(fl_prog_ram)(address,data,n,&took);fm1_irq_enable_all();
 fm1_wdt_feed();return rc;
}
static int beetle_v2_hal_erase(void *ctx,uint32_t address){
 (void)ctx;
 if(!beetle_write_permit||native_safe||native_tripped||native_updating||
    (address&4095u)||address<BEETLE_FLASH_BASE||
    address>BEETLE_FLASH_END-4096u)return -1;
 uint32_t took=0;
 fm1_irq_off();int rc=FL_FAR(fl_erase4k_ram)(address,&took);fm1_irq_enable_all();
 fm1_wdt_feed();return rc;
}
#endif

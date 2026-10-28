/* GPL-3.0-only. Read/write NOR simulator of the actual FM1 wipe function.
 * Verifies legacy -> V2 conversion, preservation of protected flash,
 * unknown-marker rejection, post-wipe V2 writer save/load and crash behavior. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#define TRACK_FIRMWARE 1
#define BEETLE_NATIVE_STORAGE 1
#define BEETLE_NATIVE_V2 1
#define ETCH_NATIVE_STORAGE 0
#define FL_IN(off,n,start,end) ((off)>=(start) && (off)<=(end) && (n)<=(end)-(off))
#define FL_FAR(f) f
#include "beetle_flash_layout.h"
#include "beetle_storage_v2_reader.h"
#include "beetle_storage_v2_writer.h"
#include "beetle_project_pack_v2.h"
static uint8_t nor[1024*1024],beetle_staged[40000],beetle_codec_buffer[16384];
static uint8_t native_safe,native_tripped,native_updating;
static unsigned erases,programs,fail_erase_at;
static void fm1_irq_off(void){}
static void fm1_irq_enable_all(void){}
static void fm1_wdt_feed(void){}
static int fl_read_ram(uint32_t address,uint8_t *dst,uint32_t n){
 if(address>sizeof(nor)||n>sizeof(nor)-address)return -1;
 memcpy(dst,nor+address,n);return 0;
}
static int fl_erase4k_ram(uint32_t address,uint32_t *took){
 if(address%4096||address<BEETLE_FLASH_BASE||address>BEETLE_FLASH_MARKER_END-4096)return -1;
 erases++;if(fail_erase_at&&erases==fail_erase_at)return -1;
 memset(nor+address,0xff,4096);*took=1;return 0;
}
static int fl_prog_ram(uint32_t address,const uint8_t *src,uint32_t n,uint32_t *took){
 if(address<BEETLE_FLASH_BASE||address>BEETLE_FLASH_MARKER_END||n>BEETLE_FLASH_MARKER_END-address)return -1;
 programs++;for(uint32_t i=0;i<n;i++)nor[address+i]&=src[i];*took=1;return 0;
}
uint32_t track_store_crc32(const uint8_t *p,uint32_t n){uint32_t c=~0u;
 for(uint32_t k=0;k<n;k++){c^=p[k];for(int bit=0;bit<8;bit++)c=(c>>1)^((c&1)?0xEDB88320u:0u);}return ~c;}
int track_journal_audit_owned_volume(const struct track_journal_io *io){(void)io;return TRACK_JOURNAL_NO_PROJECT;}
int track_journal_preview(const struct track_journal_io *io,uint8_t slot,char title[13]){
 (void)io;(void)slot;(void)title;return TRACK_JOURNAL_NO_PROJECT;
}
#include "beetle_flash_target.inc"
static void reset_legacy(void){
 memset(nor,0x4c,sizeof(nor));
 memset(nor+BEETLE_FLASH_BASE,0xff,BEETLE_FLASH_MARKER_END-BEETLE_FLASH_BASE);
 uint8_t mark[32];beetle_flash_make_marker(mark);memcpy(nor+BEETLE_FLASH_MARKER,mark,32);
 nor[BEETLE_FLASH_BASE+44]=0x38; /* occupied legacy project data */
 erases=programs=fail_erase_at=0;beetle_write_permit=beetle_full_wipe_confirmed=1;
}
static void assert_outside_preserved(void){
 for(unsigned x=0;x<BEETLE_FLASH_BASE;x++)assert(nor[x]==0x4c);
 for(unsigned x=BEETLE_FLASH_MARKER_END;x<sizeof(nor);x++)assert(nor[x]==0x4c);
}
int main(void){
 reset_legacy();
 assert(beetle_hal_v2_marker_prefix()==0);
 {int rc=beetle_hal_wipe_projects(0);fprintf(stderr,"WIPE result=%d erase=%u program=%u owner=%d v2prefix=%d\n",rc,erases,programs,beetle_hal_probe_v2_ownership(),beetle_hal_v2_marker_prefix());if(rc){uint8_t *i=nor+BEETLE_V2_BASE;fprintf(stderr,"Index bytes: %.4s v%u gen%u len%u tail %.4s crc written %08x\n",i,i[4],i[8],(unsigned)(i[12]|(i[13]<<8)),i+4092,(unsigned)(i[16]|(i[17]<<8)|(i[18]<<16)|(i[19]<<24)));struct beetle_v2_device dd;uint8_t wk[4096];fprintf(stderr,"v2_open=%d calc-crc=%08x header-6=%02x%02x records pos=%u\n",beetle_v2_open(&dd,beetle_hal_abs_read,0,wk),track_store_crc32(i,16),i[6],i[7],20+384+13*14);}assert(rc==0);}
 assert(beetle_hal_v2_marker_prefix()==1);
 assert(beetle_hal_probe_v2_ownership()==BEETLE_CLAIM_EXISTING);
 assert_outside_preserved();
 assert(erases==(BEETLE_FLASH_MARKER_END-BEETLE_FLASH_BASE)/4096);
 uint8_t a[4096],b[4096],work[256];
 struct beetle_v2_writer w;
 assert(beetle_v2_writer_open(&w,beetle_v2_hal_read,beetle_v2_hal_program,
       beetle_v2_hal_erase,0,a,b)==0);
 uint8_t original[300];for(int k=0;k<300;k++)original[k]=(uint8_t)(k%47);
 uint8_t packed[350];size_t n=0;uint16_t dict[BEETLE_PACK_V2_WORK];
 assert(beetle_pack_project_v2(original,sizeof(original),packed,sizeof(packed),&n,dict)==0);
 beetle_write_permit=1;
 assert(beetle_v2_write_project(&w,1,packed,n,a,b,beetle_staged+24576)==0);
 beetle_write_permit=0;
 struct beetle_v2_device d;assert(beetle_v2_open(&d,beetle_v2_hal_read,0,a)==0);
 uint32_t outlen=0;
 assert(beetle_v2_project_read(&d,1,beetle_codec_buffer,sizeof(beetle_codec_buffer),&outlen,
   a,beetle_staged+8192,12272)==1);
 assert(outlen==sizeof(original)&&memcmp(original,beetle_codec_buffer,outlen)==0);
 assert_outside_preserved();
 /* Reformat V2 also clears valid project index and leaves V2 mounted. */
 beetle_write_permit=beetle_full_wipe_confirmed=1;
 assert(beetle_hal_wipe_projects(0)==0);
 assert(beetle_hal_v2_marker_prefix()==1);
 assert(beetle_v2_open(&d,beetle_v2_hal_read,0,a)==0);
 assert(beetle_v2_project_read(&d,1,beetle_codec_buffer,sizeof(beetle_codec_buffer),&outlen,
   a,beetle_staged+8192,12272)==0);
 struct beetle_v2_domains domains;
 assert(beetle_v2_audit_domains(&d,&domains,a)==0);
 /* A valid BEETLE V2 marker with corrupted B2SB index is explicitly
  * reformattable; this was the RC4 failure users could not clear. */
 nor[BEETLE_V2_BASE+13]^=0x01;
 assert(beetle_v2_open(&d,beetle_v2_hal_read,0,a)!=0);
 assert(beetle_hal_wipe_projects(0)==0);
 assert(beetle_v2_open(&d,beetle_v2_hal_read,0,a)==0);
 assert_outside_preserved();
 /* A truly blank BEETLE partition can also be provisioned as V2. */
 reset_legacy();
 memset(nor+BEETLE_FLASH_BASE,0xff,BEETLE_FLASH_MARKER_END-BEETLE_FLASH_BASE);
 assert(beetle_hal_wipe_projects(0)==0);
 assert(beetle_hal_probe_v2_ownership()==BEETLE_CLAIM_EXISTING);
 reset_legacy();beetle_full_wipe_confirmed=0;
 assert(beetle_hal_wipe_projects(0)==-1&&erases==0);
 reset_legacy();nor[BEETLE_FLASH_MARKER]=0x11;
 assert(beetle_hal_wipe_projects(0)==-1&&erases==0);
 reset_legacy();fail_erase_at=3;
 assert(beetle_hal_wipe_projects(0)<0);
 assert(beetle_hal_v2_marker_prefix()==0);
 assert_outside_preserved();
 puts("PASS RC7: explicit legacy->V2 wipe, genuine V2 save/read, reformat, ownership gate, protected sectors, interrupted wipe");
 return 0;
}

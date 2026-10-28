#!/usr/bin/env python3
"""Exercise actual runtime SAVE wrappers with the actual aliased live cache.
Run --runtime /path/to/old/track_runtime.h to reproduce the RC7 failure.
NOR and hardware callbacks are host emulation only.
"""
from pathlib import Path
import subprocess,tempfile,sys,argparse
ROOT=Path(__file__).resolve().parents[2]
SRC=ROOT/'firmware/fm1-native'
parser=argparse.ArgumentParser()
parser.add_argument('--runtime',type=Path,default=SRC/'track_runtime.h')
parser.add_argument('--mode',choices=['both','legacy','v2'],default='both')
args=parser.parse_args()
runtime=args.runtime
s=runtime.read_text()
def function(name):
 start=s.index('static int '+name+'(')
 end=s.index('\n}',start)+2
 return s[start:end]
# Reuse valid project initialization and strict legacy NOR callbacks.
fixture=(ROOT/'tests/track/test_blue_rc6_real_save_paths.c').read_text().split('int main(void)')[0]
fixture=fixture.replace('flash[2][2]', 'flash[TRACK_PROJECTS][2]').replace('slot>=2', 'slot>=TRACK_PROJECTS')
extra=r'''
#include "beetle_bd4_live.h"
#include "beetle_storage_v2_reader.h"
#include "beetle_storage_v2_writer.h"
#include "beetle_project_pack_v2.h"
#define beetle_staged p
#define beetle_codec_buffer scratch
#define beetle_hal_journal_io io
static uint8_t track_save_capacity_reason,track_save_data_reason,track_save_is_legacy,beetle_write_permit;
static struct track_journal_status beetle_last_journal;
static int mode;
static uint8_t nor[0x49000],saved_nor[0x49000];
static unsigned v2writes;
static uint8_t *beetle_v2_workspace(void){return (uint8_t *)&p;}
static int beetle_v2_mode(void){return mode;}
static int beetle_v2_hal_read(void *ctx,uint32_t a,uint8_t *out,uint32_t n){
 (void)ctx;if(a<0x97000||a>0xe0000||n>0xe0000-a)return -1;
 memcpy(out,nor+a-0x97000,n);return 0;
}
static int beetle_v2_hal_program(void *ctx,uint32_t a,const uint8_t *in,uint32_t n){
 (void)ctx;if(!beetle_write_permit||a<0x97000||a>0xdf000||n>0xdf000-a)return -1;
 for(unsigned j=0;j<n;j++)if((nor[a-0x97000+j]&in[j])!=in[j])return -1;
 for(unsigned j=0;j<n;j++)nor[a-0x97000+j]&=in[j];v2writes++;return 0;
}
static int beetle_v2_hal_erase(void *ctx,uint32_t a){
 (void)ctx;if(!beetle_write_permit||a%4096||a<0x97000||a>=0xdf000)return -1;
 memset(nor+a-0x97000,255,4096);v2writes++;return 0;
}
'''
main=r'''
static void stage(uint8_t slot){
 /* Actual runtime layout, bound exactly as after boot/preview/previous SAVE.
  * Fill the project AFTER binding, retaining the RC7 stale alias. */
 beetle_bd4_live_bind(&p,sizeof p);new_project(slot);
 for(unsigned k=0;k<TRACK_PATTERNS;k++)for(unsigned c=0;c<TRACK_CHANNELS;c++){
  p.song.patterns[k].ch[c][0].note=(c==TRACK_DRUM_CHANNEL)?2:60+k%12;
 }
 p.song.patterns[0].ch[0][0].value=200;
 p.song.patterns[0].ch[0][0].effect=TRACK_FX_VOLUME;
}
static void reload(uint8_t slot){
 if(!mode){assert(track_journal_load(&io,slot,&readback,scratch,sizeof scratch,0)==0);return;}
 uint8_t ix[4096],packed[12272];struct beetle_v2_device d;uint32_t n;
 assert(beetle_v2_open(&d,beetle_v2_hal_read,0,ix)==0);
 assert(beetle_v2_project_read(&d,slot,scratch,sizeof scratch,&n,ix,packed,sizeof packed)==1);
 assert(track_store_decode(&readback,scratch,n)==0);
}
int main(int argc,char **argv){
 assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);
 assert(fread(nor,1,sizeof nor,f)>=0x48000);fclose(f);
 for(mode=0;mode<=1;mode++){
  memset(flash,255,sizeof flash);
  for(unsigned iteration=0;iteration<24;iteration++){
   uint8_t slot=iteration%TRACK_PROJECTS;stage(slot);p.bpm=120+iteration;
   uint8_t expected[TRACK_STORE_MAX];uint32_t size;
   assert(track_store_encode(&p,expected,sizeof expected,&size)==0);
   /* Alternate direct capture and decoded Project Manager imports. */
   if(iteration&1){
    memset(&p,0,sizeof p);
    assert(track_store_decode(&p,expected,size)==0);
   }
   int rc=beetle_project_save_staged();
   printf("mode=%d save=%u rc=%d reason=%u\n",mode,iteration,rc,track_save_data_reason);fflush(stdout);
   assert(rc==TRACK_JOURNAL_OK);assert(!beetle_write_permit);
   reload(slot);uint32_t n;
   assert(track_store_encode(&readback,scratch,sizeof scratch,&n)==0);
   assert(n==size&&!memcmp(expected,scratch,n));
  }
  for(unsigned slot=0;slot<TRACK_PROJECTS;slot++){reload(slot);assert(readback.bpm==132+slot);}
  stage(1);p.bpm=0;
  unsigned writes=programmed+erased+v2writes;memcpy(saved_nor,nor,sizeof nor);
  assert(beetle_project_save_staged()==TRACK_JOURNAL_INVALID);
  assert(track_save_data_reason==1);assert(writes==programmed+erased+v2writes);
  assert(!memcmp(saved_nor,nor,sizeof nor));reload(1);assert(readback.bpm==133);
 }
 puts("PASS actual SAVE wrapper: legacy + V2, all 12 slots, capture/import/overwrite/reload, exact encoded equality, invalid refusal without writes");
}
'''
if args.mode=='v2':main=main.replace('for(mode=0;mode<=1;mode++)','for(mode=1;mode<=1;mode++)')
if args.mode=='legacy':main=main.replace('for(mode=0;mode<=1;mode++)','for(mode=0;mode<=0;mode++)')
with tempfile.TemporaryDirectory() as td:
 t=Path(td)
 sys.path.insert(0,str(ROOT/'tools/storage_v2'))
 from storage_v2_sim import Nor,Storage
 nor=Nor(authorized=True);volume=Storage(nor,None)
 volume._commit(volume.current.projects,volume.current.samples)
 (t/'nor.bin').write_bytes(nor.bytes)
 (t/'test.c').write_text(fixture+extra+'\n'+function('beetle_v2_save_staged')+'\n'+function('beetle_project_save_staged_internal')+'\n'+function('beetle_project_save_staged')+'\n'+main)
 sources=['track_core','track_song','track_chip','track_drum','beetle_drum_rom','beetle_blue_monolith_rom','track_mixer','track_project_store','track_journal','beetle_bd4_live','beetle_storage_v2_reader','beetle_storage_v2_writer','beetle_project_pack_v2']
 subprocess.run(['cc','-std=c11','-O1','-g','-fsanitize=undefined','-fno-sanitize-recover=all','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DBEETLE_DRUM_EDITION=1','-DBEETLE_NATIVE_V2=1','-I'+str(SRC),str(t/'test.c'),*[str(SRC/(x+'.c')) for x in sources],'-o',str(t/'test')],check=True)
 subprocess.run([str(t/'test'),str(t/'nor.bin')],check=True)

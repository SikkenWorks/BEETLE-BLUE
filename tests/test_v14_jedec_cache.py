#!/usr/bin/env python3
"""Execute the actual cached JEDEC predicate with a mocked SPI read, not hardware."""
from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[1];hal=r/'firmware/fm1-native/hal'
s=(hal/'fm1_flash.h').read_text();src=s.split('static uint8_t fl_known_chip;',1)[1].split('#endif',1)[0]
code='''#include <stdint.h>
#include <assert.h>
#include "fm1_flash_id.h"
#define RAMINL static inline
static unsigned calls;
static uint32_t samples[3];
static uint32_t fl_read_jedec_spi(void){assert(calls<3);return samples[calls++];}
static uint8_t fl_known_chip;
'''+src+'''
static void reset(uint32_t a,uint32_t b,uint32_t c){fl_known_chip=0;calls=0;samples[0]=a;samples[1]=b;samples[2]=c;}
int main(void){
 reset(0x856014,0,0);assert(fl_chip_known_spi()&&calls==1);assert(fl_chip_known_spi()&&calls==1);
 reset(0xB36014,0,0);assert(fl_chip_known_spi()&&calls==1);assert(fl_chip_known_spi()&&calls==1);
 reset(0,0xffffff,0xB36014);assert(fl_chip_known_spi()&&calls==3);assert(fl_chip_known_spi()&&calls==3);
 reset(0,0x123456,0xffffff);assert(!fl_chip_known_spi()&&calls==3&&!fl_known_chip);
 calls=0;samples[0]=0x856014;assert(fl_chip_known_spi()&&calls==1);
 return 0;
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'cache.c';p.write_text(code);e=Path(d)/'cache';subprocess.run(['cc','-std=c11','-Wall','-Werror','-I'+str(hal),str(p),'-o',str(e)],check=True);subprocess.run([str(e)],check=True)
print('PASS actual cache predicate: both IDs, single read, retry, refusal, no failure cached')

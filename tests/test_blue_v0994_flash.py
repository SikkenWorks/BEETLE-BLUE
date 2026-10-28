#!/usr/bin/env python3
"""Compile the EXACT native streaming flash helper with a simulated FM1 NOR HAL.
Tests bounds, transaction lengths, IRQ masking and PCM staging headroom.
No device-flash claim is made.
"""
from pathlib import Path
import subprocess,tempfile
r=Path(__file__).resolve().parents[1]
t=(r/'firmware/fm1-native/beetle_flash_target.inc').read_text()
start=t.index('#define BEETLE_BLUE_LIVE_FLASH_BURST')
end=t.index('static int beetle_v2_hal_program',start)
func=t[start:end]
assert '#define BEETLE_BLUE_LIVE_FLASH_BURST 32u' in func
h='''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BEETLE_FLASH_BASE 0x97000u
#define BEETLE_FLASH_END 0xdf000u
#define FL_IN(o,n,b,e) ((o)>=(b) && (n)<=(e)-(b) && (o)<=(e)-(n))
#define FL_FAR(x) x
static uint8_t flash[0xdf000u];
static int irq_masked;static unsigned transactions,max_bytes,errors,wdt;
static void fm1_irq_off(void){if(irq_masked)errors++;irq_masked=1;}
static void fm1_irq_enable_all(void){if(!irq_masked)errors++;irq_masked=0;}
static void fm1_wdt_feed(void){wdt++;}
static int fl_read_ram(uint32_t a,uint8_t *dst,uint32_t n){
 if(!irq_masked)errors++;
 if(n>max_bytes)max_bytes=n;
 transactions++;
 if(a<BEETLE_FLASH_BASE||a+n>BEETLE_FLASH_END)return -1;
 memcpy(dst,flash+a,n);return 0;
}
'''
main='''
int main(void){
 for(unsigned k=0;k<sizeof(flash);k++)flash[k]=(uint8_t)(k^0xb5u);
 uint8_t out[4068];
 if(beetle_v2_live_read(0,BEETLE_FLASH_BASE+4096u,out,sizeof(out)))return 1;
 if(max_bytes!=32u||transactions!=128u||errors||irq_masked||wdt!=1u)return 2;
 for(unsigned k=0;k<sizeof(out);k++)if(out[k]!=flash[BEETLE_FLASH_BASE+4096u+k])return 3;
 if(!beetle_v2_live_read(0,BEETLE_FLASH_BASE-1u,out,1u))return 4;
 if(!beetle_v2_live_read(0,BEETLE_FLASH_END-1u,out,2u))return 5;
 if(!beetle_v2_live_read(0,BEETLE_FLASH_BASE,out,0u))return 6;
 printf("PASS exact native sample-reader: max 32 B/IRQ-off, 128 bounded transactions for 4068 B, exact payload, invalid addresses rejected\\n");
 return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='blue0994-flash-') as d:
 c=Path(d)/'native.c';c.write_text(h+func+main)
 exe=Path(d)/'flash'
 subprocess.run(['cc','-std=c11','-O2','-Wall','-Werror','-fsanitize=undefined','-fno-sanitize-recover=all',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)

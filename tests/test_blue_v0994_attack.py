#!/usr/bin/env python3
"""Compile and test the real V2 BD4S attack cache on a 13-slot NOR fixture."""
from pathlib import Path
import math,subprocess,tempfile,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools/storage_v2'))
sys.path.insert(0,str(ROOT/'tools/dpcm4'))
from storage_v2_sim import Storage,Nor
import beetle_dpcm4 as bd4
SRC=ROOT/'firmware/fm1-native'
with tempfile.TemporaryDirectory(prefix='blue0994-') as td:
    out=Path(td)
    flash=Nor(authorized=True)
    vol=Storage(flash,None)
    for slot in range(13):
        pcm=[int(9000*math.sin(2*math.pi*(140+slot*37)*i/22050)) for i in range(4096)]
        vol.replace_sample(slot,bd4.encode(pcm))
    (out/'volume.bin').write_bytes(flash.bytes)
    cc=['cc','-std=c11','-O2','-fsanitize=address,undefined',
        '-fno-sanitize-recover=all','-I',str(SRC),
        str(ROOT/'tests/test_blue_v0994_attack.c'),
        str(SRC/'beetle_bd4_live.c'),str(SRC/'beetle_bd4_prefetch.c'),
        str(SRC/'beetle_storage_v2_reader.c'),str(SRC/'beetle_project_pack_v2.c'),
        '-o',str(out/'hot')]
    subprocess.run(cc,check=True)
    subprocess.run([str(out/'hot'),str(out/'volume.bin')],check=True)

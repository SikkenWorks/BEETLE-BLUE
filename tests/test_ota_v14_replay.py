#!/usr/bin/env python3
"""Use exact release bytes to replay the actual application OTA protocol."""
from pathlib import Path
import tempfile,sys,subprocess,argparse
ap=argparse.ArgumentParser();ap.add_argument("--baseline",type=Path);ap.add_argument("--extra",type=Path,nargs="*",default=[]);ap.add_argument("--stock",type=Path,help="Exact original Hub V15 .fwsc (optional)");a=ap.parse_args()
r=Path(__file__).resolve().parents[1];sys.path.insert(0,str(r/'tools'));import fm1_fwsc as fw
src=r/'firmware/fm1-native'
with tempfile.TemporaryDirectory() as t:
 d=Path(t);args=[]
 for name in ('reference/v1.3-reference.fwsc','reference/v1.3.1-reference.fwsc','BEETLE-BLUE-V1.4.fwsc'):
  raw=(r/name).read_bytes();p=d/(Path(name).stem+'.ota');p.write_bytes(next(e['data'] for e in fw.parse_ufw(raw)['entries'] if e['name']=='ota.bin'));args.append(str(p))
 for name in ('reference/v1.3-reference.fwsc','BEETLE-BLUE-V1.4.fwsc'):
  p=d/(Path(name).stem+'.ufw');p.write_bytes(fw.decode_fwsc((r/name).read_bytes())[1]);args.append(str(p))
 for idx,extra in enumerate(a.extra):
  p=d/(str(idx)+'-'+extra.stem+'.ufw');p.write_bytes(fw.decode_fwsc(extra.read_bytes())[1]);args.append(str(p))
 exe=d/'replay';subprocess.run(['cc','-std=c11','-O2','-I'+str(src),str(r/'tests/ota_v14_replay.c'),str(src/'native_digest.c'),'-o',str(exe)],check=True)
 subprocess.run([str(exe),*args],check=True)
 if a.stock:
  subprocess.run([sys.executable,str(r/'tests/test_stock_v15_exit.py'),str(a.stock)],check=True)
  stock=d/'official-v15.ufw';stock.write_bytes(fw.decode_fwsc(a.stock.read_bytes())[1]);subprocess.run([str(exe),*args[:5],str(stock)],check=True)
 # Execute the actual second-stage loader, not only its format checker.
 base=d/'baseline.flash';base.write_bytes(next(e['data'] for e in fw.parse_ufw((r/'reference/v1.3.1-reference.fwsc').read_bytes())['entries'] if e['name']=='flash.bin'))
 loader_exe=d/'loader';subprocess.run(['cc','-std=c11','-O2','-I'+str(r/'firmware/fm1-loader'),str(r/'tests/ota_v14_loader_replay.c'),'-o',str(loader_exe)],check=True)
 subprocess.run([str(loader_exe),str(base),*args[3:]],check=True)
 if a.baseline:
  baseline=a.baseline;subprocess.run(['cc','-std=c11','-O2','-DBASELINE','-I'+str(baseline),str(r/'tests/ota_v14_replay.c'),str(src/'native_digest.c'),'-o',str(exe)],check=True);subprocess.run([str(exe),*args],check=True)

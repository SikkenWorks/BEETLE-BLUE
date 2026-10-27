#!/usr/bin/env python3
"""Reproducible V0.98 source and direct-release artifact assembly; never flashes hardware."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib, shutil, re,base64
r=Path(__file__).resolve().parents[1]; out=r.parent
fw=r/'BEETLE-BLUE-V0.98-FM1-EXPERIMENTAL.fwsc';app=r/'build/track-fm1-native-beetle-storage-lab-updater-uac/app.bin'
sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest()
fwsha=sha(fw);appsha=sha(app)
assert fw.stat().st_size==610086
assert 'simulator' not in (r/'BEETLE-BLUE-Manager.html').read_text().lower()
for obsolete in ('BEETLE-BLUE-V0.97-FM1-EXPERIMENTAL.fwsc','BEETLE-BLUE-V0.97-WebInstaller.html'):
 (r/obsolete).unlink(missing_ok=True)
report=f'''# BEETLE BLUE V0.98 — Verified software checks, remaining FM1 risks

## Native build and firmware package

- Native JieLi build with drum engine, Storage V2, updater and USB audio options: **PASS**.
- Native application size: **{app.stat().st_size:,} bytes**; designated application-space headroom **{581564-app.stat().st_size:,} bytes** (limit 581,564). **This is not CPU or RAM headroom**.
- Application SHA-256: `{appsha}`.
- Firmware `.fwsc` SHA-256: `{fwsha}`; size **{fw.stat().st_size:,} bytes**.
- Firmware package static audit **PASS**: UFW parse, app payload round-trip, protected 16-KiB header and original OTA loader preserved.
- Separate read-only diagnostic build: **PASS**, linker symbol audit prevents eleven forbidden write entries. It is not a physical safety certificate.

## Automated host coverage

- PCM buffer source-initialization contract, BEETLE factory PCM and all 16 MONOLITH procedural ROM patches: **PASS**.
- All twelve native drum step-effect types, FX1/FX2/FX3 composition, 2,500 mixed-FX retriggers: **PASS**.
- REV both built-in kits (literal reverse on BEETLE PCM; reverse-style synthesizer on MONOLITH), 3,000 retriggers: **PASS**.
- Project-local sample START/END and reverse playback window, one-frame sample, 3,000 retriggers: **PASS**.
- **New V0.98:** BEETLE and MONOLITH hi-hat release/choke including alternate hat; CH6 NOTE1/NOTE2 source/STOP classification; **4,096** rapid triggers: **PASS**.
- **New V0.98:** procedural MONOLITH oscillator actually follows the PITCH effect, not merely PCM playback length; modulo Q8 32-bit rate verified: **PASS**.
- V29→V30 project migration, six per-lane kit IDs, CRC corruption rejection and REV step serialization: **PASS**.
- Separate no-write native diagnostic build; native 240×240 LCD capture functions, 17 captured PNG files enlarged with exactly 8× nearest-neighbor: **PASS**.
- WebInstaller JS parser, Project Manager JS parser, two-tool Manager JS parser: **PASS**. Exact installer firmware and matching manager embedded pages: **PASS**.

## Unverified RC blockers

- **No physical FM1 testing was possible:** the previously reported double-kick `DMA DEADLINE / AUDIO STOP` is not proven solved, even with 4,096 successful simulated host triggers.
- FM1 output peak/clip measurements, actual USB audio timing and six stems, WebMIDI transfers, NOR flash endurance, destructive migration, power-loss recovery and hardware OTA/recovery are unverified.
- Streamed Storage V2 WAV samples still lack literal REV and PCM-history-based DELAY/REVERB/CHORUS; MONOLITH's REV is reverse-inspired synthesis, not reverse PCM.
- The static build and host test suite do **not** prove that every UI/input combination or DSP function works on the device. Treat V0.98 as **EXPERIMENTAL**, **not** as V1.0 RC or certified safe to flash.
'''
(r/'validation/V0.98-RESULTS.md').write_text(report)
(r/'STATUS.md').write_text('BEETLE BLUE V0.98 EXPERIMENTAL — native build and host suite pass; physical FM1 audio DMA, OTA, recovery and peak safety unverified. See validation/V0.98-RESULTS.md.\n')
ui=out/'BEETLE-BLUE-V0.98-NATIVE-UI-1920x1920.zip'
with ZipFile(ui,'w',ZIP_DEFLATED,compresslevel=7) as z:
 for p in sorted((r/'UI-CAPTURES-NATIVE-1920').glob('*.png')):z.write(p,p.name)
assert len(ZipFile(ui).namelist())==17
source=out/'BEETLE-BLUE-V0.98-COMPLETE-SOURCE.zip'
with ZipFile(source,'w',ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(r.rglob('*')):
  if p.is_file() and not p.name.startswith('.'):
   z.write(p,str(Path(r.name)/p.relative_to(r)))
assert ZipFile(source).testzip() is None
assert ZipFile(source).read(r.name+'/'+fw.name)==fw.read_bytes()
for src,name in [(fw,fw.name),(r/'WebInstaller.html','BEETLE-BLUE-V0.98-WebInstaller.html'),(r/'BEETLE-BLUE-Manager.html','BEETLE-BLUE-V0.98-Manager.html'),(r/'BEETLE-BLUE-Project-Manager.html','BEETLE-BLUE-V0.98-Project-Manager.html'),(r/'README.md','BEETLE-BLUE-V0.98-README.md'),(r/'CHANGELOG.md','BEETLE-BLUE-V0.98-CHANGELOG.md'),(r/'UI-CAPTURES-NATIVE-1920/TRACKER-NATIVE-EXACT.png','BEETLE-BLUE-V0.98-TRACKER-1920x1920.png'),(r/'UI-CAPTURES-NATIVE-1920/STEP-FX-EDITOR.png','BEETLE-BLUE-V0.98-STEP-FX-EDITOR-1920x1920.png')]:shutil.copyfile(src,out/name)
bundle=out/'BEETLE-BLUE-V0.98-ALL-DOWNLOADS.zip'
with ZipFile(bundle,'w',ZIP_DEFLATED,compresslevel=6) as z:
 for f in [out/fw.name, source,ui,out/'BEETLE-BLUE-V0.98-WebInstaller.html',out/'BEETLE-BLUE-V0.98-Manager.html',out/'BEETLE-BLUE-V0.98-Project-Manager.html',out/'BEETLE-BLUE-V0.98-README.md',out/'BEETLE-BLUE-V0.98-CHANGELOG.md',out/'BEETLE-BLUE-V0.98-TRACKER-1920x1920.png',out/'BEETLE-BLUE-V0.98-STEP-FX-EDITOR-1920x1920.png']:
  z.write(f,f.name)
assert ZipFile(bundle).testzip() is None
installer=(r/'WebInstaller.html').read_text();manager=(r/'BEETLE-BLUE-Manager.html').read_text()
def payload(html,key):
 m=re.search(rf'<script id="{key}" type="application/octet-stream">([^<]*)</script>',html)
 assert m,key
 return base64.b64decode(m.group(1))
assert payload(installer,'firmware-embedded')==fw.read_bytes()
assert payload(manager,'source-installer')==installer.encode()
assert payload(manager,'source-projectmanager')==(r/'BEETLE-BLUE-Project-Manager.html').read_bytes()
print('PASS: release',bundle,'SHA',fwsha,'source entries',len(ZipFile(source).namelist()),'17 UI captures', 'no simulator')

#!/usr/bin/env python3
"""Prepare two-tool BEETLE BLUE release (no simulator), source docs and embedded firmware."""
from pathlib import Path
import re,base64,hashlib
r=Path(__file__).resolve().parents[1]
fw=(r/'BEETLE-BLUE-V0.98-FM1-EXPERIMENTAL.fwsc').read_bytes()
sha=hashlib.sha256(fw).hexdigest()
app=r/'build/track-fm1-native-beetle-storage-lab-updater-uac/app.bin'
size=app.stat().st_size;assert size<581564
pm=r/'BEETLE-BLUE-Project-Manager.html'
install=r/'WebInstaller.html'
manager=r/'BEETLE-BLUE-Manager.html'
# Preserve existing project transfer protocol and Storage V2 settings verbatim.
pmtext=pm.read_text().replace('V0.97','V0.98')
pm.write_text(pmtext)
installtext=install.read_text().replace('V0.97','V0.98')
def replace_payload(html,key,content):
 pattern=rf'(<script id="{re.escape(key)}" type="application/octet-stream">)[^<]*(</script>)'
 html,n=re.subn(pattern,lambda m:m.group(1)+base64.b64encode(content).decode('ascii')+m.group(2),html,count=1)
 assert n==1,key
 return html
installtext=replace_payload(installtext,'firmware-embedded',fw)
installtext,n=re.subn(r"(HASH_ET: ')[a-f0-9]{64}(')",lambda m:m.group(1)+sha+m.group(2),installtext,count=1)
assert n==1
install.write_text(installtext)
managertext=manager.read_text().replace('V0.97','V0.98')
assert 'source-emulator' not in managertext and "const ids=['projectmanager','installer']" in managertext
managertext=replace_payload(managertext,'source-projectmanager',pm.read_bytes())
managertext=replace_payload(managertext,'source-installer',install.read_bytes())
manager.write_text(managertext)
for rel,p in [('web/BEETLE-BLUE-Project-Manager.html',pm),('web/WebInstaller.html',install),('web/BEETLE-BLUE-Manager.html',manager),('web/BEETLE-BLUE-MANAGER.html',manager),('BEETLE-BLUE-V0.98-WebInstaller.html',install)]:
 q=r/rel;q.parent.mkdir(parents=True,exist_ok=True);q.write_bytes(p.read_bytes())
def unpack(html,key):
 match=re.search(rf'<script id="{key}" type="application/octet-stream">([^<]*)</script>',html)
 assert match,key
 return base64.b64decode(match.group(1))
assert unpack(installtext,'firmware-embedded')==fw
assert unpack(managertext,'source-projectmanager')==pm.read_bytes()
assert unpack(managertext,'source-installer')==install.read_bytes()
# Keep user-requested README structure; maintain detailed release notes in CHANGELOG only.
readme=r/'README.md';s=readme.read_text()
s=s.replace('V0.97 EXPERIMENTAL','V0.98 EXPERIMENTAL').replace('V0.97 has been compiled','V0.98 has been compiled').replace('BEETLE BLUE V0.97 |','BEETLE BLUE V0.98 |')
s=s.replace('in V0.97. Delay/reverb','since V0.97. Delay/reverb')
s=s.replace('with the V0.97 `.fwsc`','with the V0.98 `.fwsc`')
s=s.replace('for V0.97. Do not','for V0.98. Do not')
s=s.replace('validation/V0.97-RESULTS.md','validation/V0.98-RESULTS.md')
s=s.replace('V0.97 build, host tests','V0.98 build, host tests')
s=s.replace('### Software validation reported for V0.97','### Software validation reported for V0.98')
s=s.replace('The V0.97 native app is **524,796 bytes**, with **56,768 bytes**',f'The V0.98 native app is **{size:,} bytes**, with **{581564-size:,} bytes**')
s=s.replace('**V0.97 `.fwsc` SHA-256:** `8093aec43c3ba718626931eb3e55a4293cb534ffcc1740fcf9ef8fb33a1fef20`',f'**V0.98 `.fwsc` SHA-256:** `{sha}`')
s=s.replace('package_beetle_blue_v097.py','package_beetle_blue_v098.py').replace('render_blue_v094.py','render_blue_v098.py').replace('the V0.97 embedded firmware hash','the V0.98 embedded firmware hash')
s=s.replace('The current release includes two browser tools','The current release includes two browser tools')
s=s.replace('| 3,000 repeated kick triggers and 4,338-event regression | Host tests PASS |','| 3,000 repeated kick triggers and 4,096 hat/kick retriggers | Host tests PASS |')
s=s.replace('| Webtool JS syntax, embedded firmware and matching tool versions | PASS |','| MONOLITH hat choke, CH6 NOTE1/NOTE2 selection and oscillator PITCH | Host tests PASS |\n| Webtool JS syntax, embedded firmware and matching tool versions | PASS |')
s=s.replace('| `UI-CAPTURES-NATIVE-1920/` | 16 native-code-rendered screen states','| `UI-CAPTURES-NATIVE-1920/` | 17 native-code-rendered screen states')
assert 'simulator.html' not in s.lower() and 'WEB_SIMULATOR' not in s
readme.write_text(s)
changelog=r/'CHANGELOG.md';notes='''## V0.98 — Drum trigger correctness and CPU cleanup (EXPERIMENTAL)

- Fixed hi-hat choke across **both** built-in drum kits: BEETLE PCM HHC/HHO and MONOLITH ROM closed/open/alternate hats now share the correct release interaction. Previously MONOLITH's voices used IDs 66/67/79, which were not recognized when checking *existing* voices for a choke.
- Fixed CH6 note-column assignment for MONOLITH: mapped procedural voice IDs 64–79 no longer masquerade as missing NOTE1 hits and inherit NOTE2 FX or STOP state. A small pure NOTE1/NOTE2 classifier uses the voice's actual source ID (MONOLITH) or logical component ID (BEETLE's randomized PCM variations).
- Fixed the MONOLITH **PITCH** step effect: changes the actual `kit_phase` oscillator increment rather than merely altering an unrelated PCM cursor or note duration. Implemented with a bounded modular 32-bit multiplication rather than a 64-bit multiply inside the audio callback.
- Reduced drum render work for inactive voices by skipping EQ, FX, ROM-pointer and trim preparation for unused polyphony slots.
- Added host regression for both-kit choke, CH6 NOTE1/NOTE2 effect routing, 4,096 rapid retriggers and procedural PITCH frequency. Re-ran existing PCM, MONOLITH, FX1–FX3, REV, trim, V29→V30 migration and PCM-initialization tests.
- Firmware remains BDR1 **V30**, with 48 patterns, 12 project slots, 6 drum lanes, 12 BBK1 User Kits and no WebSimulator. Project/Storage V2 layouts are unchanged. Updated two-tool WebInstaller/Manager and regenerated 17 native LCD renders using 8× nearest-neighbor scaling.
- **Hardware limitations:** no physical FM1 DMA timing, output-peak measurement, USB audio, flash/recovery or long-run flash journal verification has been performed. This is experimental and must not be advertised as V1.0 RC ready.

'''
c=changelog.read_text();assert '## V0.98' not in c;c=c.replace('# BEETLE BLUE CHANGELOG\n\n','# BEETLE BLUE CHANGELOG\n\n'+notes,1);changelog.write_text(c)
rc=r/'docs/V1.0-RC-CHECKLIST.md';t=rc.read_text();t=t.replace('- [ ] FM1 hardware repeated-kick DMA DEADLINE', '- [x] Host: both-kit hi-hat choke, CH6 MONOLITH NOTE1/NOTE2 routing, and actual oscillator PITCH (V0.98)\n- [ ] FM1 hardware repeated-kick DMA DEADLINE');t=t.replace('- [ ] WebSimulator step FX model aligned with native UI and audible results','- [x] WebSimulator removed from release, Manager and README as requested');rc.write_text(t)
print('PASS V0.98: embedded exact FWSC',len(fw),'sha256',sha,'app',size,'headroom',581564-size,'two-tool manager')

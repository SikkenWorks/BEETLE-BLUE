## BEETLE BLUE V1.0 RC2 — safer project save, V2 orphan cleanup and exact error reasons

- Distinguishes **PROJECT DATA TOO LARGE** (BDR1 buffer), **PROJECT PACK TOO LARGE** (12-KiB flash record) and **FLASH SPACE LOW** (physical page/guard capacity). Empty slots do not bypass per-project bounds.
- On confirmed Storage V2 physical capacity failure, attempts a CRC/A/B-audited orphan-sector cleanup, then retries exactly once. Does **not** erase referenced projects/samples, reformat, drop data or change the BDR1 V31 schema.
- Invalid BDR1 input is no longer mislabeled as an ordinary storage-capacity error.
- Native writable and read-only builds, V2 writer/reader, power-loss, Song Chain and project format regressions passed in software. Hardware, DMA stress, OTA/recovery remain unverified.
- This is **not** an unlimited-storage fix. A project over the 12-KiB serialization limit still cannot be saved on the FM1; no save success is falsely reported.

# BEETLE BLUE CHANGELOG

## V1.0.0 RC1 — Setlist HOME fix, non-destructive Song Chain insertion, safety audit

- Fixed **HOME from empty Setlist** by prioritizing HOME input, scanning project ownership before permitting insert/selection and refusing placeholder project slots. On an unsaved write error, no data is mislabelled as saved; deliberate second HOME can leave the editor with a visible UNSAVED warning.
- Fixed Song Chain clipboard: **SEQ+OP2 inserts after the current row**, moves existing rows down, keeps all lane assignments/repeats/auto/silent references, selects the first inserted row and refuses overflow *without modifying the chain*. A short on-screen warning appears on failure.
- Fixed non-BLUE host regression compilation: a reversed drum-voice field was incorrectly referenced in a build where that field is excluded. BLUE's own reverse behavior is unaffected.
- Rebased one legacy PCM fingerprint test to the **independently verified unchanged V0.99.8 output** (`dd0f3681`) rather than keeping a stale V1.2 fingerprint; this is test maintenance, not DSP retuning.
- Added unit regressions for no saved projects, clipboard insertion/tail preservation, end-of-chain appends, full-chain atomic refusal and user-facing screen capture for no projects. Native writable / read-only JieLi firmware builds and selected storage, audio and song-chain host tests were run.
- Source audio timing and DMA scheduling are NOT changed. **Outstanding RC blocker: user-observed `DMA DEADLINE / AUDIO STOP` under heavy FM1 playback, physical USB capture/OTA and safe stage playback have NOT been proven.** This is a candidate for device acceptance testing, not a certified stable V1.0 release.


## V0.99.8 — HOME menu Live Mode entry and nested HOME navigation (EXPERIMENTAL)

- Added **LIVE MODE** as a selectable entry in the normal **HOME / PROJECTS** menu, between SETTINGS and DIAGNOSTICS, without changing the boot NEW/LOAD/LIVE chooser.
- Repaired Live Mode HOME routing: **SETLIST → LIVE MODE → HOME/PROJECTS** with one HOME press per level. The **PERFORM** and **PERFORMANCE STOP** HOME routes also lead first to LIVE MODE, not immediately to the main menu.
- Preserved V0.99.7 silent SETLIST autosave on leaving via HOME or SEL, and its fail-closed behaviour on flash write errors.
- Reflowed all 10 HOME menu items with 14-pixel line pitch and separate bottom status/footers on the native 240×240 LCD. Saved native C-rendered 1920×1920 UI captures, 8× nearest-neighbour.
- Added dedicated C navigation/autosave regression, rebuilt native writable and read-only JieLi images, and reverified experimental `.fwsc` protected loader identity and byte-matched WebInstaller.
- No changes to drum DSP, patterns, song transport, samples, BDR1 V31 project codec, V2 setlist record, kit engines or DMA scheduling. **No physical FM1 validation. DMA DEADLINE/AUDIO STOP under heavy use remains unresolved.**



## V0.99.7 — Live Setlist autosave, safer Perform gates, BLUE battery HUD (EXPERIMENTAL)

- **Silent automatic Setlist save:** leaving Setlist via HOME or the SEL navigation path commits changes to Storage V2's existing versioned/CRC32 A/B metadata footer. Unchanged entries do not write flash; success shows no toast. A failed commit **blocks the exit**, keeps the full in-RAM setlist and displays a clear retry error rather than losing changes. SAVE remains an optional retry. New failure-injection unit tests verify all five state paths.
- **Perform safety:** PLAY is ignored unless an actual CRC-verified project is ready, Live Mode is active, the audio subsystem is healthy and there is no pending song-completion flag. A manual stop confirmation is accepted only for an actual paused transport. The interface explicitly reports an AUDIO STOP condition instead of claiming PLAYING/READY. Full project loads remain foreground-only; no flash I/O is introduced into the DMA callback.
- **Battery indicator:** native 240×240 pixel HUD in TRACKER above the PAT counter and in LIVE / PERFORM above the setlist count. Four coarse ADC-derived bars, no fake battery percentage. A small blue bolt denotes a USB **data link**, not verified charging. No real charger-status/charging-current sensor is exposed by the available FM1 source. **Actual charging detection and percentage accuracy remain unverified.**
- **Layout:** reserved non-overlapping HUD rectangles; regenerated actual-source C Live Mode captures at 1920×1920 with 8× nearest-neighbor scaling. Broader complete-device UI regression and physical display confirmation remain outstanding.
- **Tests:** writable/read-only JieLi builds, complete `.fwsc` checksum/payload/bootloader verification, A/B footer torn-write isolation, autosave fail/retry, battery policy thresholds, and existing user-sample/PCM regressions. **No physical FM1 verification or guarantee of stage safety**. The older `DMA DEADLINE / AUDIO STOP` failure under heavy playback remains an active hardware regression risk.


## V0.99.6 — LIVE MODE: project setlists and stage transport (EXPERIMENTAL)

- Startup menu now presents **NEW PROJECT**, **LOAD PROJECT** and a separate **LIVE MODE** entry. LIVE MODE has **SETLIST** and **PERFORM** pages, all implemented in native FM1 C with 240×240 screen renders.
- A global 24-position setlist chains references to 12 independently saved projects. K1 selects row, K2 assigns project, K4 moves row, OCT+ inserts, OCT− removes. Each project retains its own BPM, kits, effects and Song Chain. Repeat slots are allowed.
- **PERFORM** loads the next entire CRC-checked project only after the previous project transport has stopped; the audio ISR merely flags completion. The next project is READY but never auto-started. Existing SONG arrangement plays once in Live Mode; SONG OFF plays one pattern cycle.
- Pressing PLAY/STOP while performing opens **CONTINUE CURRENT TRACK** or **START CURRENT TRACK AGAIN**. Continue resumes the saved step and Song Chain position; restart returns to the beginning. The existing ordinary Song Chain mode is untouched.
- **V2 persistence:** explicit SAVE from Setlist stores a 64-byte versioned/CRC32-protected independent setlist footer in the existing 4096-byte Storage V2 metadata A/B transaction, outside the bounded project/sample indexes and global kit bank. All other V2 writes preserve the footer. No new NOR sector or new project version is required. Legacy storage remains RAM-only and reports save failure.
- Added host state-machine, CRC rejection, metadata torn-write recovery and project-isolation tests; native JieLi lab build and exact native LCD C captures. Physical FM1 stability, storage writes, OTA and live audio safety remain **unverified**. This is not stage-certified.



## V0.99.5 — Per-slot self-choke and faster BD4S audio ISR (EXPERIMENTAL)

- **User-sample default self-choke:** all global U01–U13 Storage V2 samples and project-local user samples are now monophonic **per sample slot**, even when CHOKE=0. Retriggering the same slot from any lane applies the existing 48-frame voice fade. Other U slots are still independent; explicit shared choke groups retain their original behavior. Project format stays **BDR1 V31**.
- **Cross-kit hi-hat priority:** BEETLE closed/open and MONOLITH closed/open/alternate hats share unconditional 48-frame choke, independent of kit/lane and configurable group setting. The existing cross-kit logic was retained and given an additional regression.
- **Pending stream retirement:** when a choked Storage V2 voice has no resident PCM frame, retire that pending voice/request without waiting for the flash refill. Avoids stranded decoder work and unbounded stale prefetch after rapid retriggers.
- **Reduced DMA ISR work:** replace two sequential 128-frame window scans (`ready_now` then `render(1)`) with a single-pass `beetle_bd4_stream_read_one_ready`. At even frames locate the source pair once; at odd frames reuse it. Never advance source clock, age or envelope on a missing PCM frame; no flash read, decode, dynamic allocation or blocking in audio ISR.
- **Tests:** exact per-frame PCM and full stream voice state matched the previous renderer for a 768-frame sample with 15 artificially delayed polls. Tested independent/global user-slot choke, 13 warm BD4S attacks / 1000 fast retriggers / 96,000 rendered PCM frames, hot-cache flash read absence, cross-ROM hats, compiled writable + no-write native firmware.
- **Known RC blocker:** physical M-VAVE FM1 still exhibited `DMA DEADLINE / AUDIO STOP` with rapid user samples on **V0.99.4**. V0.99.5 reduces ISR and decoder pressure but its fix **cannot be certified without a new physical FM1 stress run**. Long sample tails remain flash-streamed; complete glitch elimination is NOT claimed.
- README remains a concise operational guide; CHANGELOG contains development details. Two-tool Manager retains only WebInstaller and Project Manager; WebSimulator stays removed. Protected boot header/OTA loader are unchanged.

## V0.99.4 — Prewarmed user-sample attacks / bounded flash bursts (EXPERIMENTAL)

- **Usersample latency:** prewarm the first 256 source frames of every installed Storage V2 sample into immutable per-slot PCM/ADPCM attack snapshots while the transport is stopped and no key is held. Cache uses the existing project staging workspace, no extra permanently allocated audio-RAM section.
- **Hot retrigger:** any of six drumvoices can restore a cached attack and the exact ADPCM continuation state from RAM, instead of re-reading and re-decoding the first two PCM windows. Preserves the selected sample generation and START/END trim; changed trim falls back to the normal path and learns a matching attack cache.
- **DMA/flash latency:** restrict IRQ-masked SPI NOR transactions for **active sample prefetch** to 32 bytes (formerly 256 bytes per call), with each transaction independently interruptible. No flash read, storage write, PCM decoding or memory allocation is introduced into the audio ISR. Project/installer flash operations remain on the existing separate path.
- **Idle warming:** one 128-frame decode window per foreground poll, only with stopped transport and no held physical notes; no bulk decoding inside a single DMA frame. The initial 13-slot cache scan is discarded on sample-bank refresh, metadata mutation or rebind.
- **Correctness:** 13 real Storage V2 ADPCM sample fixtures, 1,000 quick cross-slot retriggers over six voices, 96,000 PCM frames, zero read during audio callback, zero warm-trigger flash reads and zero decoder underruns (host test). Existing six-voice streaming, PCM exactness, rev/FX, project V31 and kit-regression tests repeated.
- **Limitations:** only first 256 source frames are resident; long sample tails still stream from flash. Cache warms when idle, so a sample struck immediately after boot may still need its first foreground prefetch. Physical hardware DMA DEADLINE and true simultaneous flash/LCD/audio behavior are **not yet measured**. Do not claim never-glitch or hardware-certified stability.
- README kept a concise product guide; detailed version changes belong here. The WebSimulator remains excluded.

## V0.99.3 — Deterministic per-lane kits and sample handoff audit (EXPERIMENTAL)

- **Fixed kit confusion on CH1–CH6:** previously `s->instrument == lane` gated whether the lane's selected kit played; any other saved INST value silently selected a source from the project palette. Playback now consistently uses the lane's selected kit for NOTE1, NOTE2 and NOTE3, including Song Mode. All existing INST/palette bytes remain stored, but **per-step source overrides are intentionally suspended** until a distinct user-editable override flag exists.
- Corrected a rapid-user-sample **lost-request race**: main-loop `pending` acknowledgement now snapshots voice serial/slot while briefly holding the IRQ lock. The lock is never held across flash I/O, sector CRC or DPCM decoding. A superseding request cannot be overwritten by a stale prepare/start.
- Moved **DPCM decoder stop/choke** out of the audio ISR. The ISR only invalidates the active voice request; the main-loop performs stream cleanup. Prevents an IRQ from interrupting producer-side decoder/window changes during a fast retrigger.
- Verified **14 of 14 corresponding BEETLE and MONOLITH components** render different actual native PCM sequences (20×256 stereo frames each), with nonzero peaks. MONOLITH's two additional alternate sounds remain present. Digital difference is not a subjective listening or DAC hardware validation.
- Added 672 lane/source/legacy-step combinations and the full V0.99.2 host regression suite. Native JieLi writable-image build and independent read-only build passed. Protected boot header and OTA loader remain inherited unchanged, BDR1 V31 unchanged.
- The All-in-One Manager still contains only Project Manager and WebInstaller; no simulator. README remains a guide; all implementation changes are documented here.
- **RC blocker:** the reported FM1 `DMA DEADLINE / AUDIO STOP` from rapid user samples has **not** been physically reproduced or eliminated. No guarantee of zero glitches, underruns, slowdowns, or reboot-free recovery is possible from software-host tests alone. Check actual FM1 flash bandwidth, ISR deadlines, DAC output and USB multi-stem operation before claiming V1.0 RC.

## V0.99.2 — Punch-In FX Mixer header collision (EXPERIMENTAL)

- Fixed the **LANE 0 / REC FX** Punch-In recording mixer: the horizontal page-header rule intersected `STEP`, `FX` and `ON/OFF` labels (line y=42 versus 5×7 glyphs y=38–44). Placed divider at native LCD y=37, column labels at y=43 and first event at y=54.
- Audited all 16 Punch-In event rows: last entry ends at y=195, leaving clearance before the footer at y=201; no removal of recorded Punch-In events or changed navigation semantics.
- Fixed the same header-rule-over-label condition on the existing read-only RC diagnostic page (divider y=36, subheading remains at y=40).
- Added a **native-source-extracted framebuffer regression** verifying the actual pixels, line/column/header spacing, all 16 rows and the bottom instructions. Captures use 240×240 native RGB565 followed by exactly 8× nearest-neighbor to 1920×1920.
- No change to audio callbacks, DMA, storage, BDR1 V31, step FX, user samples, USB, OTA loader, kits or WebInstaller workflows; no WebSimulator reintroduced. Native JieLi and read-only binaries rebuilt from the modified source.
- **Not V1.0 RC:** physical FM1 screen, DMA DEADLINE, flashed update/recovery and analogue audio still require hardware verification.

## V0.99.1 — FM1 user-reported bugs, Punch-In routing and pre-RC stability (EXPERIMENTAL)

- LOAD PROJECT individual delete: use **EDIT** on selected occupied project; opens NO-default confirmation with explicit YES/NO. Legacy EDIT+SEL chord remains, but no longer required.
- FX **double short press** opens Punch-In FX menu; single FX press on step FX column opens its three-slot parameter editor. Long FX hold remains the live Punch-In modifier.
- Live Punch-In overlay: suppress partial playhead redraw while live Punch FX is active; permit SEQ + white-key overlay and use current **BLEND/PITCH** names.
- Punch-In: replace OCT UP with **BLEND** 0–100% dry/wet applied to all Punch-In audio processing, replace OCT DOWN with **PITCH** K1 ±24 semitones. ROM / procedural / local PCM drum voices retune. **BD4S streamed user samples still play at fixed rate**, so this is not yet universal pitch support.
- Reduce Punch-In per-sample work: compute Q8 global blend coefficient once per DMA block rather than repeated divides per stereo sample.
- CH6: rename tracker KIT field to **RAT**, whose K3 controls 1–4 drum ratchets; retain existing per-step kit route for backward compatibility. Song Mode now labels it **CH6**, not DR.
- CH6 NOTE3 mode no longer disables the otherwise saved per-step FX for NOTE1 and NOTE2. Step FX playback uses per-lane identifiers, independent of which lane is displayed.
- Primary FX **VOLUME 0–200%**: widened runtime field to 8 bits and new **BDR1 V31** packed project codec (42 bits per populated step). Updated project validator, encoder, decoder and Project Manager V31 compatibility. Older V26–V30 decodes remain supported; V30 firmware will not understand V31 saves. New regression round-trips FX value 200 with groove/dynamics intact.
- MONOLITH synthesizer: additional distinct harmonic shapes for hats, snares, toms, cymbals, percussion/rim while BEETLE µ-law drum ROM remains unchanged. Both kits still require subjective FM1 listening validation.
- User-sample repeated retrigger: preserve previously CRC-verified, immutable, pinned BD4S sector metadata across repeat triggers of the same slot/generation; avoid unnecessary 4 KiB re-verification and flash I/O in foreground prefetch. PCM ISR remains flash-free. **DMA DEADLINE is NOT confirmed fixed on physical hardware**.
- Replaced obsolete synthesizer/Scale Guide native HELP chapters with BLUE drum, kit, FX, Punch-In, load/delete, safety controls.
- Preserved existing 6 lanes, 48 patterns, 12 project slots, kits, storage/OTA loader, USB code and two-tool Manager. No WebSimulator.
- **Tests:** full retained host FX/ROM/retrigger/project suite PASS, BDR1 V31 200% serialization PASS, native JieLi release and read-only symbol-audited builds PASS, protected header and OTA container verified. Physical DMA/DAC/flash/recovery/USB and streamed pitch remain unvalidated; **NOT V1.0 RC**.


## V0.99 — Stereo overload protection and lightweight per-voice math (EXPERIMENTAL)

- Fixed a mix-headroom weakness in the BLUE six-drum-lane output: individually soft-limited lanes could still sum to a hard-clipped stereo peak. The final stereo sample now uses a **symmetric bounded soft knee** above +/-24,000, capped at +/-31,500, with unchanged values throughout the linear region. This is a deterministic software safeguard, not an analogue SPL or hardware-clip certification.
- Simplified the 73-value pitch lookup algebra to remove software-emulated 64-bit multiplication on every drum trigger / retune. Added an **exact equivalence test** to the old Q16 formula for every tuning value.
- Replaced two additional 64-bit per-sample multiplications with 32-bit fixed-point equivalents where input ranges prove the products fit; retained wide arithmetic for actual PCM sample interpolation where it is required.
- New host test runs **3,900 mixed BEETLE/MONOLITH triggers** across six lanes and **166,400 rendered audio frames**, with changing pan, simultaneous FX and a maximum stereo magnitude below 31,500; all V0.98 regression suites re-run successfully.
- Preserved BDR1 **V30**, 48 patterns, 12 project slots, six lane kit selections, BBK1 saved kits, all existing FX identifiers and the protected OTA loader. No change to non-audio control semantics, NOR storage layout, DMA memory buffers or USB configuration.
- Rebuilt actual JieLi image and matching WebInstaller; regenerated **17 native-C LCD captures at 1920x1920, 8x nearest-neighbor**. Manager embeds only Project Manager and WebInstaller; no WebSimulator.
- **RC blockers remain:** physical FM1 DMA DEADLINE behavior, analogue output peak measurements, physical flash/recovery, streamed user-sample REV/delay, USB stems, and BBK1 kit-bank backup/restore. V0.99 is experimental, not verified V1.0 RC.

## V0.98 — Drum trigger correctness and CPU cleanup (EXPERIMENTAL)

- Fixed hi-hat choke across **both** built-in drum kits: BEETLE PCM HHC/HHO and MONOLITH ROM closed/open/alternate hats now share the correct release interaction. Previously MONOLITH's voices used IDs 66/67/79, which were not recognized when checking *existing* voices for a choke.
- Fixed CH6 note-column assignment for MONOLITH: mapped procedural voice IDs 64–79 no longer masquerade as missing NOTE1 hits and inherit NOTE2 FX or STOP state. A small pure NOTE1/NOTE2 classifier uses the voice's actual source ID (MONOLITH) or logical component ID (BEETLE's randomized PCM variations).
- Fixed the MONOLITH **PITCH** step effect: changes the actual `kit_phase` oscillator increment rather than merely altering an unrelated PCM cursor or note duration. Implemented with a bounded modular 32-bit multiplication rather than a 64-bit multiply inside the audio callback.
- Reduced drum render work for inactive voices by skipping EQ, FX, ROM-pointer and trim preparation for unused polyphony slots.
- Added host regression for both-kit choke, CH6 NOTE1/NOTE2 effect routing, 4,096 rapid retriggers and procedural PITCH frequency. Re-ran existing PCM, MONOLITH, FX1–FX3, REV, trim, V29→V30 migration and PCM-initialization tests.
- Firmware remains BDR1 **V30**, with 48 patterns, 12 project slots, 6 drum lanes, 12 BBK1 User Kits and no WebSimulator. Project/Storage V2 layouts are unchanged. Updated two-tool WebInstaller/Manager and regenerated 17 native LCD renders using 8× nearest-neighbor scaling.
- **Hardware limitations:** no physical FM1 DMA timing, output-peak measurement, USB audio, flash/recovery or long-run flash journal verification has been performed. This is experimental and must not be advertised as V1.0 RC ready.

## V0.97 — Drum step-FX engine, FX cursor editing and realtime optimization (EXPERIMENTAL)

- Release packaging cleanup: removed the WebSimulator, its README button and all emulator UI/payloads from the All-in-One Manager. The Manager now contains only the Project Manager and WebInstaller. Updated the two-tool Manager build script. This is a web/documentation-only repack; the native V0.97 `.fwsc` is unchanged.

- Audited the six-lane drum audio callbacks: V0.96 only applied step VOLUME, PITCH and REV to many BLUE drum voices, leaving FILTER, DRIVE, DELAY, REVERB, CHORUS, VIB, ARP, CUT and TREMOLO absent from the drum DSP path.
- Introduced a fixed 3-slot transient per-voice FX state. FX1, FX2 and FX3 are dispatched in slot order, never allocated or saved during the audio interrupt. No V30 project-format change, no FX ID renumbering.
- Added native drum effect processing: FILTER as low-pass blend; DRIVE bounded saturation; PCM ROM short-tap DELAY, REVERB and CHORUS; oscillator/pitch LFO VIB and CHORUS; 3-tone ARP; gate-based CUT; amplitude TREMOLO; plus existing VOLUME, PITCH and REV. MONOLITH's procedural oscillator now responds to its own ARP/VIB/CHORUS modulation (separate from PCM cursor rate).
- Avoids heavy long-delay buffers or flash reads in the audio callback. Effects use bounded arithmetic, the existing PCM ROM data, a small 36-byte transient state and power-of-two LFO timing. The dry/no-FX path bypasses extra processing.
- Short FX release on a focused FX cell opens PER STEP FX for that selected step and lane. Repeated short FX while editing preserves editor focus; long FX still controls Punch-In. REV displays ON rather than a misleading numeric parameter.
- Host tests: 12 effects alter BEETLE and MONOLITH audio relative to dry; 3-FX combinations differ from their component effects; 2,500 overlapping retriggers; FX 12 storage codec and V30 compatibility retained. Application build and container tests in validation/V0.97-RESULTS.md.
- **Current limitations:** short ROM taps and MONOLITH tonal approximations are not equivalent to long buffered echo/convolution. Streaming Storage V2 samples have no backwards/PCM-history read for REV/DELAY/REVERB/CHORUS. Hardware FM1 DMA deadlines, real output peaks, and OTA recovery remain unverified, so V0.97 is **NOT** a V1.0 Release Candidate.

## V0.96 — Project-sample REV trim correction and CH6 consistency (EXPERIMENTAL)

- Corrected project-local µ-law sample START/END in the native drum renderer. Previously the output age was treated as an absolute source endpoint and REV could read the wrong interval. Both directions now address the same source-frame window.
- Applied the local sample window to all three CH6 hits (NOTE1, NOTE2 and NOTE3) as well as the other five drum lanes. The existing Storage V2 streaming trim request remains separate and forward-only.
- Added a bounded 32-frame release ramp to reversed PCM hits, limiting clicks where reversed audio ends on the original transient. Mono procedural REV continues to be reverse-style synthesis, not a time-reversed waveform.
- Used existing per-block voice metadata to pack local window start/span without enlarging the critical FM1 RAM pool or reducing DMA/USB buffers. Never allocates in the audio interrupt.
- Native JieLi build and readonly build PASS. Host tests: 14 BEETLE/16 MONOLITH REV variants, 3,000 repeated reverse triggers, local sample cropping and a one-frame sample, V30 REV step codec.
- Preserved existing 48 patterns, 12 project slots, 12 global User Kits, BDR1 V30 format, ROM engines, native mixers and protected update components.
- WebInstaller updated with SHA-pinned V0.96 firmware; Manager regenerated from the exact separate tools. Simulator remains an approximation and does not yet fully reproduce native per-step REV/trim.
- **Unverified:** physical FM1 DMA deadlines, real analogue peaks, USB six-stem timing, persistent NOR wear, OTA write/recovery. Not an RC-certified firmware.


## V0.95 — Reverse step FX implementation and 1.0 RC preparation (EXPERIMENTAL)

- Audited native step-FX definitions: V0.94 had no `REV` step effect; reverse key detection and Punch-In reverse were unrelated.
- Added FX code **12 = REV** to the existing 4-bit FX field; old effect codes 0–11 and BDR1 V30 binary layout are unchanged.
- Implemented literal reversed per-voice PCM indexing for the BEETLE factory ROM kit and project-local PCM one-shots, including NOTE2 and sparse step FX where supported.
- Implemented a bounded reverse-inspired rising envelope and descending oscillator phase for procedural MONOLITH. **This is not literal time-reversed PCM.**
- Guarded Storage V2's streaming BD4S path against pretending to reverse: it still only supports forward reads.
- Added native host tests covering BEETLE and MONOLITH output differences, 3,000 reverse retriggers and REV pattern serialization. Existing MONOLITH/BEETLE and PCM-clear contract tests remain passing.
- Retained project V30, 48 patterns, 12 project slots, 12 global kits, user samples, native mixer, USB routing and protected OTA components.
- Next RC blockers: FM1 real-hardware DMA and output-level validation; streamed sample reverse; synth/legacy dependency audit; simulator equivalence for per-step REV; backup of global user-kit presets.
- **Not a hardware-verified Release Candidate.**


## V0.94 — Audio PCM initialization fix and MONOLITH procedural ROM kit (EXPERIMENTAL)

- Fixed the missing `native_stereo` zero-initialization introduced when BLUE disabled synth render, preventing stale PCM from being re-added by the drum mixer every ALNK0 block.
- MONOLITH: replaced the built-in 50,800-byte, 8 kHz ADPCM bank with 16 fixed-patch 44.1 kHz ROM drum synthesis models. BEETLE original 14-pad PCM ROM remains unchanged.
- Optimized inactive-drum render, precomputed EQ coefficient gains per block, and reduced repeated wide-integer operations in the per-sample drum loop.
- Kept BDR1 V30 project storage, BBK1 global kits, 48 patterns, 12 project slots, user samples and six drum lanes.
- Refreshed WebSimulator using 16 previews rendered by the real C drum engine; WebInstaller packages exact rebuilt V0.94 `.fwsc`; all-in-one embeds matching tools.
- Added 16 source-derived 1920x1920 LCD renders including tracker, nearest-neighbor 8x.
- FM1 actual flashing, DMA deadlines, acoustic quality and output peak safety **NOT physically verified**.

# BEETLE BLUE Changelog

## V0.93 — MONOLITH built directly into firmware

- Integrated **all 16** supplied MONOLITH WAV drum sounds as compiled, read-only FM1 application ROM data. No external sample-bank upload or U01–U13 allocation is needed to play them.
- Implemented 8 kHz mono 4-bit IMA ADPCM sample packing (50,800 compressed bytes) with bounded per-voice on-demand decoding and PCM interpolation. Explicit audio quality/trim disclosure in README and generated manifest.
- Kit source `MONOLITH` now routes KIK..RIM to real embedded samples, with U01 / U02 serving secondary SNR2 and HHT2 while in MONOLITH mode. Other U03–U13 retain their independent user-bank handling. CH6 pair playback updated for embedded kit identifiers.
- Updated Drum Mixer to list MONOLITH's sixteen entries, not thirteen uploaded sample slots.
- Project format **BDR1 V30**, 48 patterns, 12 project slots and original BEETLE ROM kit are unchanged.
- Rebuilt firmware `.fwsc` with preserved OTA loader / 16 KiB boot header; verified native app round-trip.
- Rebuilt WebInstaller with V0.93 firmware hash, simulator with decoded embedded audio source buffers, Project Manager without MONOLITH upload requirement, and All-in-One Manager with byte-identical embeds.
- Generated UI renders from extracted native C drawing paths and 8× nearest neighbor scaling, including tracker and MONOLITH mixer.
- Host test: all 16 embedded sounds distinct and audible, 2000 kick retriggers and CH6 dual-sample pair pass. **Hardware DMA / audio and OTA safety remain unverified.**

## Older development

See `docs/BLUE-V0.92-CHANGELOG-ARCHIVE.md` for the previous release history. No previous firmware release is overwritten by these changes.

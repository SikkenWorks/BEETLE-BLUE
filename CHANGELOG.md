## V1.0.0 RC12 — Readable, paginated on-device HELP (EXPERIMENTAL)

- Replaced dense HELP paragraphs with 66 concise two-line action cards in ten topics, showing a maximum of three separated cards per page.
- K2 selects a topic, K1 pages through its cards. HOME and SEQ retain prior navigation. No project or storage format changes.
- Updated on-device version/credits string, README controls and native HELP test harness. RC12 flashable firmware remains gated on release/recovery proof; native builds and reference rendering are not hardware tests.
- Current project root contains no historical release binaries. Older histories remain in CHANGELOG and `validation/archive/`.

## Documentation maintenance — RC11 operator guide / HELP source sync

- Replaced the accumulated RC4–RC10 paragraphs in README with a single up-to-date, English user guide. Historical release notes remain in CHANGELOG and validation/archive.
- Corrected native FM1 HELP for **SEQ+K4 independent lane length (01–16)**, six lengths per pattern, V33 project compatibility and the Live Mode Setlist/Perform workflow.
- Recompiled writable/read-only native APP binaries and passed fixed-width HELP, lane-length and V32→V33 regression tests. Original RC11 flashable WebInstaller has **not** been modified; rebuilt image is distributed as a non-flashable lab container because the project's release gate lacks verified recovery proof.
- Removed obsolete firmware/installer/manager duplicates and old manifests from the current project root. Added a reusable release-hygiene script and operating rules.

---

## V1.0.0 RC11 | Per-pattern six-lane loop length (experimental)

- Added **independent 1–16 step length for CH1–CH6 in each of the 48 patterns**; default 16. Hold **SEQ and rotate K4** on the tracker to set the selected lane's length. `LEN xx` shows its length. K1 step navigation respects the selected lane.
- Each lane independently loops modulo its own length, including the additional CH6 drum hits, drum FX, groove/microtiming, ratchets and Song Mode source lanes. The overall pattern advances on the longest active lane; no >16-step transport added.
- Updated clipboard/block operations and undo/redo to retain the selected lane-length metadata stored in reserved row-0 bits. **No increase in step (6 B) or pattern (625 B) RAM footprint.**
- New **BDR1 V33** serialized project format: three packed lane-length bytes per pattern, along with a version 6 standalone pattern file. V32 projects and other supported older versions continue to load. Downgrading to earlier BEETLE BLUE firmware after saving V33 is unsupported; back up before update.
- Validated native writable/read-only builds; BDR1 V33 project save/load, authentic RC10 V32 migration, independent 1/7/12/16 loops, drum FX, groove, Song Mode, block editing, original BEETLE Song Chain regression, and existing physical-workspace storage tests on host. **Real FM1 hardware timing and installation are not tested.**
- Original RC10 credits and upstream licence headers, RC9 Live Mode warm audio changes and Storage V2 policy are preserved. No auto-format or destructive migration.

---

## V1.0.0 RC10 | Credits and battery attribution

- Explicitly credit Leo Kuroshita / Hügelton Instruments (Felucca, GPL-3.0-only) for the 531/561/591 battery-voltage ADC thresholds used by the BLUE indicator. BLUE's filtering, fallback/invalid-read behavior and pixel presentation are SikkenWorks additions.
- Update the on-device CREDITS page, README, full source credit documents, browser Manager, Project Manager and WebInstaller. Preserve BEETLE, ETCH, Felucca, JieLi, reSID/Nes_Snd_Emu reference distinctions and all required upstream copyright/licence notices.
- No change to the RC9 audio, persistence, DSP, project schema or Live Mode algorithms. Firmware recompiled only for on-device credit text and version. Experimental: hardware testing still required.

# V1.0 RC9 — BATTERY HUD / SILENT LIVE PROJECT LOADING / USER KIT SELECTION

- PB1 battery indicator now follows Felucca's FM1 raw ADC thresholds 531 / 561 / 591, has an invalid-reading guard and keeps USB connection separate from verified charger state; no invented percentage.
- Live Perform and Setlist autosave now use an in-place warm DMA quiesce with a 256-sample fade, DAC buffer drain, digital zero during flash operations and fade-in on resume, instead of switching PC6 off/on at every transition. Existing realtime progress overlay retained.
- Fixed the unsent stereo PCM buffer tail on end-of-pattern playback when Live Mode has no per-project Song Chain.
- After Kit Designer stores a global User Kit successfully, that kit is automatically selected for the current lane (save your project to persist the lane reference). Existing BBK1 format, 12 global kit slots and 14 timbre components are unchanged.
- No changes to Storage V2 allocation, project schemas, USB protocol, protected header or OTA-loader. Physical FM1 charging indication, analog pop-free switching and sustained USB recording are not certified.

---

# BEETLE BLUE V1.0.0 RC8 — PROJECT DATA ERROR fix

Gebruik **BEETLE-BLUE-Manager.html** of **BEETLE-BLUE-V1.0.0-RC8-WebInstaller.html**.
De nieuwe firmware is **BEETLE-BLUE-V1.0.0-RC8-PROJECT-SAVE-FIX.fwsc**.
Oudere bestanden zijn behouden als referentie; gebruik voor deze fix RC8.

## Oorzaak en correctie

De samplecache en het te bewaren project delen hetzelfde RAM. RC7 riep na
project-capture/decoding `beetle_bd4_live_invalidate()` aan. Die functie schrijft
cacheflags en wist chunks/attack-snapshots via de oude pointer. Daarmee werd de
klaargezette song beschadigd voordat de encoder hem controleerde. De fout is
op de originele RC7 SAVE-wrapper gereproduceerd: `TRACK_JOURNAL_INVALID`,
diagnose 4 (PATTERN / NOTES), bij de eerste opslag op lege legacy-opslag.
Hetzelfde gedeelde pad wordt gebruikt voor Storage V2 en projectimports.

RC8 ontkoppelt de cache met `beetle_bd4_live_bind(0,0u)` zonder projectbytes te
wijzigen. Na de transactie wordt de cache opnieuw opgebouwd. Capture ontkoppelt
eerst; eerste opslag na initialisatie maakt een verse capture na de bevestiging
in plaats van oude scratchdata te vertrouwen. Validatie, capaciteitcontroles,
CRC, schrijfautorisatie en opslagformaat blijven behouden. Geen extra grote
RAM-buffer; geen automatische wipe, migratie of projectnormalisatie toegevoegd.


Zie RC8-VALIDATION.md voor tests en gebruik.

---

## V1.0.0 RC7 — Fix Legacy-to-V2 wipe and invalid V2 index creation

- **Root cause:** previously Wipe All Storage chose its output layout from the existing marker. Legacy devices remained on 12K TRKJ even after wipe.
- **Root cause:** invalid B2SB header bytes left as FF made newly formatted V2 storage unreadable; a host flash test reproduced the failure.
- Double-confirmed wipe now creates and verifies a clean V2 index and V2 owner marker. Existing BEETLE legacy projects/samples/setlists are all erased; NO automatic migration or default formatting.
- V2 status is independently checked before showing STORAGE READY. Flash bus reads remain outside realtime audio.
- Real V2 writer save and subsequent readback tested on simulated flash. Real FM1 hardware not validated. Other DATA ERROR causes remain possible.

## V1.0.0 RC6 - Legacy storage error and new-session state fixes

- Fixed legacy TRKJ invalid data being mislabeled as PROJECT TOO FULL.
- Reset all transient serialization staging memory on NEW project.
- Report active legacy/V2 backend and differentiate actual capacity from invalid project data, with no flash writes on refusal.
- Added real journal save/overwrite/load regression test for slots 1 and 2.
- Experimental hardware status unchanged. No silent formatting, migration or erasure.

## BEETLE BLUE V1.0 RC5 — Project capacity / kit source routing

- New V32 BDR1 sparse step codec eliminates 42-bit fixed cost per populated note when properties are defaults, while retaining V31 and older decode support. Older firmware cannot open V32 projects.
- Drums: switching KIT from DRUM MIXER changes the currently selected channel, and all stopped preview paths honor that channel source.
- Added host tests for dense 48-pattern projects, original V31-to-V32 migration, and distinct native PCM from all 14 BEETLE/MONOLITH components.
- Verified native writable/read-only builds and V2 writer regression tests. Real device flashing, savings and DAC audio remain unverified.

## BEETLE BLUE V1.0 RC4 — NEW defaults, V2 format, Live loading UI

- Reset BLUE sample trims, sample ADSR, MASTER settings and palettes on NEW.
- Allow explicitly confirmed Wipe All Storage to reformat recognized Storage V2 volumes; fully verify erases and CRC-checked empty index.
- Refresh ownership after wipe; preserve flash boundary protections.
- Real V2 read-progress overlay in Perform; right-aligned tracker PAT label.
- Existing RC3 FX validation/repair kept. Hardware flash/OTA recovery and DMA stability unverified.

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

## V1.0.0 RC3 — Save DATA ERROR when switching STEP FX types (EXPERIMENTAL)

- Reproduced `PROJECT DATA ERROR` when an FX TYPE change retained a VOLUME value of 200 on an effect whose maximum is 100. Native host test reproduced serializer rejection and confirms clean encoding after correction.
- Fix FX1, NOTE2 FX, FX2/FX3 type-change values at edit time, with a strictly limited compatibility repair of invalid FX value/type pairs in the **staged SAVE copy**. No mutation to the active song, notes, effect IDs or audio, and no changed flash layout or project schema.
- If another invalid project field exists, save fails closed and reports a diagnostic field group. This repair cannot prove the specific unsaved hardware project has no additional invalid data.
- Preserve RC2 safe Storage V2 reclamation, RC1 Setlist/Song fixes, protected header and OTA-loader, all 12 project slots and sample banks.
- Native writable JieLi app build / audit, `.fwsc` integrity and embedded installer identity, user-data regressions and UBSAN test for FX transitions passed. Physical hardware, DMA and recovery **not tested**.
- **Do not reboot, power off or flash the FM1 while its current project is unsaved.**


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

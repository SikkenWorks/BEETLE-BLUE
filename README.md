<p align="center">
  <img src="BEETLE-BLUE.png" alt="BEETLE BLUE pixel-art boot image" width="320">
</p>

<h1 align="center">BEETLE BLUE</h1>
<p align="center">
  <strong>An open-source, six-channel drum tracker and ROM-based drum workstation for the M-VAVE FM1.</strong><br>
  SikkenWorks · Born out of passion.
</p>

<p align="center">
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/WebInstaller.html"><img src="https://img.shields.io/badge/%E2%96%B2_INSTALL_BEETLE_BLUE-006EFF?style=for-the-badge" alt="Install BEETLE BLUE"></a>
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/BEETLE-BLUE-Project-Manager.html"><img src="https://img.shields.io/badge/%E2%96%B6_PROJECT_MANAGER-000000?style=for-the-badge" alt="Open BEETLE BLUE Project Manager"></a>
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/BEETLE-BLUE-Manager.html"><img src="https://img.shields.io/badge/%E2%96%B6_ALL--IN--ONE_MANAGER-000000?style=for-the-badge" alt="Open BEETLE BLUE Manager"></a>
  <a href="https://thefirmwarehub.com/?q=sikkenworks"><img src="https://img.shields.io/badge/%E2%96%B6_THE_FIRMWARE_HUB-000000?style=for-the-badge" alt="The Firmware Hub"></a>
</p>

<p align="center">
  <sub>Development release: <strong>V1.0 RC2 (EXPERIMENTAL)</strong> · <a href="CHANGELOG.md">Release history</a> · <a href="validation/RC2-STORAGE-FIX.md">Software validation</a></sub>
</p>

---

<h2 align="center">A DRUM MACHINE. A TRACKER. A POCKET-SIZED WORKSTATION.</h2>
<p align="center">
  <strong>Build beats across six dedicated drum lanes, shape your own kits, arrange songs and perform with live effects.</strong><br>
  <sub>Two built-in drum kits. Twelve global User Kits. A blue pixel-art interface. Hands-on FM1 controls.</sub>
</p>

## What is BEETLE BLUE?

**BEETLE BLUE** is the drum-focused fork of [BEETLE](https://github.com/SikkenWorks/BEETLE) for the **M-VAVE FM1**. Instead of BEETLE's five synth lanes and one drum lane, BLUE routes its six sequencer lanes to drums. BEETLE BLUE is derived from the BEETLE V1.7.14 code base; its retained project, storage, effects, USB and OTA code should not be confused with features independently validated on real hardware.

| Feature | BEETLE BLUE V1.0 RC2 |
| :-- | :-- |
| **Drum lanes** | 6 independent sequencer lanes; each has an authoritative project-saved kit assignment |
| **Patterns** | 48 patterns, up to 16 steps per pattern |
| **Song Mode** | Up to 64 chain positions |
| **Built-in kit: BEETLE** | Original 14-sound PCM/µ-law ROM drum kit |
| **Built-in kit: MONOLITH** | 16 distinct procedural ROM drum patches at 44.1 kHz, inspired by the supplied MONOLITH WAVs |
| **Global User Kits** | 12 saved timbre-preset slots (not 12 separate WAV banks) |
| **Kit Designer** | 8 saved project kit references (`INST 1–8`); kit selection per lane is authoritative in playback |
| **Global user samples** | U01–U13; optional and independent of the two built-in ROM kits |
| **Usersample playback** | Per-slot RAM-prewarmed attacks, self-choke and faster DMA ISR window reads; long tails still stream |
| **Sample editing** | START / END, ADSR, GAIN, DRIVE and CHOKE for user samples |
| **Mixing** | Lane Mixer and kit-specific Drum Mixer |
| **Performance** | Punch-In FX, groove controls, mute groups, snapshots, REV step FX and Song Mode |
| **Project storage** | 12 device project slots; `BDR1` schema **V31** (reads V30 and earlier supported versions); `.bdrum` backup files |
| **USB audio** | Inherited experimental stereo and six-mono-stem capture paths |
| **UI** | Blue native LCD theme and C-rendered 1920 × 1920 reference captures |

### V1.0 RC1: corrected Setlist and Song Chain clipboard

- **Empty Setlist:** HOME returns to Live Mode even when there are **no saved projects**. SELECT and OCT+ cannot create phantom P01 slots. Project selection requires a recognized saved slot, scanned incrementally. A failed write never silently marks an edited setlist as saved; a second HOME deliberately exits with an **UNSAVED** notice visible in Live Mode, keeping its edits in RAM for retry. Unchanged empty setlists never write flash.
- **Song Chain copy/paste:** on the SONG CHAIN page, hold SEQ and press OP1 to copy the selected row or an inclusive SEL+K1 range. Put the cursor on the desired existing song position and press **SEQ+OP2** to **insert the copied rows immediately AFTER the cursor**. Existing rows move down and remain unchanged; a capacity failure (64 positions maximum) rejects the entire operation and briefly shows `CHAIN FULL - PASTE CANCELLED`. The first newly inserted row is selected. Undo/Redo remains available through existing editing history.
- **Compatibility:** BDR1 V31, persistent 12 project slots, 24-entry Live Setlist and separate Song Chain remain unchanged. No new storage partition, project migration or automatic formatting is introduced.

### V1.0 RC2: PROJECT TOO FULL diagnostic + safe V2 cleanup

**The number of free project slots is not a measure of project capacity.** Each
project has its own bounded 12-KiB encoded data/workspace limit, and all 12
project slots share the storage volume with global sample banks. A large
arrangement or project-owned sample data can exceed the per-project limit
although other slots are empty. RC2 intentionally does not change the BDR1 V31
schema, expand the physical partition or remove any notes/samples to make a
save appear successful.

On save to an initialized **Storage V2** volume, RC2 now identifies the failing
stage: `PROJECT DATA TOO LARGE` (raw BDR1 serializer); `PROJECT PACK TOO LARGE`
(packed record); or `FLASH SPACE LOW` (actual physical V2 allocation/guard).
For physical-space refusals only, it checks both A/B metadata generations,
collects **only unreferenced orphan sectors** and retries the same project once.
This cannot fix a genuinely oversized project or a volume filled with valid
sample data. No project slot or live sample sector is sacrificed. Storage V1
still has its unchanged journal bank limit.

**If RC2 still reports `PROJECT DATA TOO LARGE`, stop trying different slots.**
Back up the project through the existing Project Manager or compatible transfer
route first, then reduce the project's notes/FX or project-owned sample content.
Do not delete project 1, format the device, or power off during a write. The
physical FM1 behavior and the safe restore path are not certified.

### Live Mode: project setlists (V1.0 RC1)

At power-on the FM1 offers three independent choices: **NEW PROJECT**, **LOAD PROJECT**, and **LIVE MODE**. You can also open **HOME → LIVE MODE** at any time from the normal Projects menu (between SETTINGS and DIAGNOSTICS). LIVE MODE opens **SETLIST** or **PERFORM**; it is separate from the existing per-project 64-position Song Chain.

- **SETLIST** holds up to **24 song positions**, each referencing one of the existing **12 project slots** (the same project can appear more than once). Turn **K1** to select a setlist position, **K2** to assign a project slot, **K4** to move that position up/down, **OCT+** to insert a position, **OCT−** to delete one. **HOME** goes back to the LIVE MODE menu (with silent autosave when changed). Press HOME again to return to the normal HOME/PROJECTS menu. This same nested HOME route applies whether LIVE MODE was opened at boot or through HOME. The setlist **automatically saves on leaving SETLIST** with HOME or SEL-to-Perform. Successful saves are visually silent. Unmodified setlists do not incur needless NOR writes. If saving fails, SETLIST stays open, keeps all RAM edits and displays an error. SAVE can also manually retry. The setlist is never saved as part of an individual BDR1 project.
- **PERFORM** loads the first project's complete settings, including its own BPM, instruments, drum kits, patterns and Song Chain. Press **PLAY/STOP** to start. The current song runs once, ignoring its saved Song Chain loop flag only for this performance. On the last Song Chain position, or after the selected pattern if SONG is off, playback stops, and the next project is CRC-loaded in the foreground. **The next song remains stopped until you press PLAY/STOP again.** No project loading or flash access occurs in the audio interrupt.
- If **PLAY/STOP** is pressed while a live performance is playing, BEETLE BLUE stops and displays **CONTINUE CURRENT TRACK** or **START CURRENT TRACK AGAIN**. Turn **K1** to select; **SEL** resumes at the saved step / Song Chain position or restarts the whole song. **HOME** returns to the LIVE MODE menu; a second HOME returns to the normal HOME/PROJECTS menu.
- **Persistence requirement:** a setlist can be saved/recalled across restarts on an existing, valid **Storage V2** volume. Older journal-only volumes can edit in RAM, but automatic save fails and the editor stays open with an error instead of exiting and silently losing changes; this release does **not** auto-provision V2 storage. Back up projects and samples before any firmware update. There is no automatic storage migration.
- **Caveat:** the full Live Mode workflow and NOR A/B saves are software-tested only. Real FM1 audio transitions, project-load latency, power-loss behaviour and loudness must be validated on hardware before using Live Mode on stage.

### Battery HUD (V1.0 RC1)

A small **BLUE pixel battery symbol** is drawn at the lower-right of the TRACKER, above the PAT counter, and in its own upper-right band in LIVE / PERFORM. The fill has four broad stages from the FM1 battery ADC (PB1), read only every four seconds in the foreground. It **does not show an invented percentage**; the real FM1 ADC divider and lithium-battery discharge curve are not calibrated. A blue bolt beside the battery indicates an active **enumerated USB data connection**, **not confirmed battery charging**. Some USB wall chargers will not be detected and an enumerated USB connection does not prove current is flowing into the cell. Accurate charging/full indication requires a verified charging-status or VBUS hardware signal, which is unavailable in this source. This limitation is deliberate: do not trust the bolt as an indication of charging or battery capacity during a concert.

The HUD uses reserved LCD rectangles: TRACKER `x=204..235, y=208..217`, PERFORM `x=204..235, y=53..62`. Those do not cover PAT text, lane dots, chord hint, saved-toast caption or Perform titles. New Live Mode native C screen captures are provided. This is **not** an exhaustive real-device visual proof across every screen.

### Audio engine and efficiency

BEETLE BLUE has an independent six-lane native drum mixer, including BEETLE PCM and MONOLITH procedural instruments. The final stereo bus uses a bounded soft knee to reduce hard-clipping risk. Idle voices skip unnecessary DSP. Native PCM comparisons verify that all 14 shared BEETLE and MONOLITH components render different audio, although their perceived similarity still needs listening evaluation. For uploaded U01–U13 samples, the initial attack is predecoded into RAM while stopped, and hot retriggers restore the attack without fresh flash reads. Each uploaded U-slot now self-chokes when retriggered, with a short voice fade; BEETLE/MONOLITH hats choke one another. Long samples still stream from Storage V2 and must be hardware-stress-tested. Sample requests use interrupt-safe acknowledgement, deferred decoder stop and a single-pass resident-frame read; real FM1 DAC peak levels, sample streaming glitches and DMA deadline margins **still require hardware measurement**. V0.99.4 exhibited a confirmed DMA DEADLINE with rapid sample triggers; V0.99.5 is not yet proven to solve it. Version-by-version implementation notes are recorded only in the [CHANGELOG](CHANGELOG.md).

### Step FX controls

From the tracker, select a drum step, focus its **FX** column with K2, and **press/release FX** to open **PER STEP FX** for that exact lane and step. **Double-tap FX** to open the Punch-In FX menu. A long FX hold remains the Punch-In modifier. In the editor: **K1** selects FX1/FX2/FX3, **K2** selects the effect type, **K3** adjusts its value, **OCT+** advances to the next effect slot, and **OCT−** deletes the selected effect. FX1–FX3 can be combined in order. REV is an ON/OFF action, not a numeric control. **VOLUME ranges from 0% to 200%.**

The selected source kit is authoritative per lane on CH1–CH6, including Song Mode. Historic step `INST` values remain stored for compatibility, but no longer silently override the lane kit. The Kit Designer palette remains persisted; per-step source overrides are suspended until they have an explicit, unambiguous opt-in mechanism.

The drum-oriented effects are FILTER, DRIVE, DELAY, REVERB, CHORUS, VIB, ARP, CUT, TREMOLO, VOLUME, PITCH and REV. All twelve effect identifiers now reach the native BEETLE/MONOLITH drum engine since V0.97. Delay/reverb/chorus are **short, bufferless ROM-tap effects for PCM** and **lightweight tonal approximations for procedural MONOLITH**, not a long-delay or convolution reverb. Storage V2 streaming samples have no PCM random-access history and do **not** support literal delay/reverb/chorus taps or streamed REV. For these sources full FX parity remains a future RC gate.

### Step FX: REV

Select **REV** in a drum step's FX column to reverse only that step's triggered hit. BEETLE's built-in PCM drum samples are read backwards. MONOLITH uses a reverse-style rising procedural envelope and backwards oscillator motion because its sounds are generated, not stored as PCM. The optional Storage V2 streamed U01–U13 WAV decoder remains forward-only; REV is not supported on those streamed samples. The FM1 hardware behaviour is not yet verified.

## The Two Built-in Drum Kits

### BEETLE

**BEETLE** is the original 14-component PCM/µ-law ROM kit carried over from BEETLE. Its ROM audio assets remain in firmware and its drum settings can still be edited. The previous UI name `DEFAULT / ROM` has been changed to **BEETLE**.

### MONOLITH

**MONOLITH is built into V0.94.** No WAV import, `.beetlebank`, Project Manager upload or Storage V2 sample slot is required to hear its 16 drum sounds. Its audio is generated by its own **procedural ROM-based drum engine**, using distinct oscillator shapes, tone/noise settings and a small wavetable. This is **not** the earlier V0.93 8 kHz ADPCM samplebank, and it is not bit-identical to the 16 source WAVs.

| Components | MONOLITH character | Reference WAV(s) |
| :-- | :-- | :-- |
| KIK | Sweeping, deep sub kick | `kick.wav` |
| SNR / SNR2 | Two noise-and-shell snare characters | `snare_1.wav`, `snare_2.wav` |
| HHC / HHT2 / HHO | Bright closed, alternative and open hats | `hihat_1.wav`, `hihat_2.wav`, `open_hihat.wav` |
| TM1 / TM2 / TM3 | Tonal pitch-swept toms | `tom_1.wav`–`tom_3.wav` |
| CRS / RDE | Longer, brighter cymbal-type patches | `crash.wav`, `ride.wav` |
| CLP / RIM | Staggered clap and resonant rim | `clap.wav`, `rim.wav` |
| PC1 / PC2 / PC3 | Three pitched/percussive variants | `perc_1.wav`–`perc_3.wav` |

The 16 original MONOLITH WAVs are retained in the source archive as development/reference assets. They are **not** copied into the device by V0.94. The ROM engine has a 44.1 kHz render path instead of V0.93's 8 kHz ADPCM source bandwidth, but higher sample rate does **not** guarantee a more faithful or subjectively better result. Listening on the real hardware is still necessary.

## Get Started on the FM1

1. **Open the tracker:** press **SEQ**.
2. **Choose a lane:** turn the **SELECT** rotary encoder to switch between lanes 1–6.
3. **Assign a kit to that lane:** **hold SEQ and turn SELECT**. A temporary blue overlay shows the selected lane and kit. Every lane defaults to **BEETLE** in a new project.
4. **Choose a pattern:** turn **ALGORITHM** in the tracker to select pattern 01–48 while stopped.
5. **Edit steps:** **K1** moves between steps, **K2** changes the selected field and **K3** adjusts its value. Use the FM1 keyboard for drum entry/audition as supported by the current page.
6. **Set tempo and play:** turn **K4** in the tracker to adjust BPM, then press **PLAY/STOP**.
7. **Edit drums:** press **EDIT** to open **DRUM EDIT**; press EDIT again for **KIT DESIGNER**. Use the context labels on the FM1 display.
8. **Mix:** press **GLO** for the Lane Mixer, then **GLO again** for the Drum Mixer.
9. **CH6 extra hits:** NOTE3 mode retains step-FX playback. The former KIT tracker field is now **RAT** (ratchet/retrigger count).
10. **Arrange and save:** press **ARP** for Song Mode and use **SAVE / HOME** for project operations. Back up before changing firmware or storage.

> **Control terminology:** **SELECT** means the rotary encoder; **SEL** means pressing that encoder. The behavior of a knob can vary by screen. Holding **SEQ** while turning SELECT is different from pressing SEL.

## Kit Designer and Global User Kits

BEETLE BLUE has three different ways to select sounds and settings. These are intentionally separate:

| Concept | Quantity | Storage | Meaning |
| :-- | --: | :-- | :-- |
| **BEETLE / MONOLITH** | 2 | Firmware ROM | Immediately available built-in drum sound sources |
| **Global User Kits** | 12 | Storage V2 `BBK1` metadata | Factory-ROM drum timbre presets, shared across projects |
| **Active palette (`INST 1–8`)** | 8 per project | `BDR1` project | References to one of the two built-in sources or a global User Kit |
| **Lane kit assignment** | 6 per project | `BDR1` project | Default kit source for each drum lane, changed with SEQ + SELECT |
| **Legacy factory timbre variations** | 12 | Firmware | Starting parameter variations in DRUM EDIT; distinct from the 12 globally saved User Kits |

### Kit Designer controls

From **DRUM EDIT**, press **EDIT** to enter **KIT DESIGNER**.

| Control | Action |
| :-- | :-- |
| **K1** | Choose palette entry `INST 1–8` |
| **K3** | Choose BEETLE, MONOLITH or USER KIT 01–12 for the selected palette entry |
| **SEL** | Store an eligible USER KIT's current timbre settings globally, when playback is stopped and Storage V2 is writable |
| **EDIT** | Return to DRUM EDIT |
| **SEQ** | Return to the tracker |

A global User Kit stores settings such as Attack, Decay, Sustain, Release, Gain, Pitch, Drive and Choke for the factory drum components. It **does not** include 16 copied MONOLITH WAV files or its own sample bank. The Kit Designer palette remains stored for project compatibility, but native playback uses the explicitly selected kit of each lane. Legacy per-step `INST` values do not silently replace that kit. CH6 retains its independent NOTE2/NOTE3 drum hits.

### Global-kit backup limitation

The twelve global kits live in a CRC-checked `BBK1` bank in Storage V2 metadata. These are **not** saved inside ordinary `.bdrum` project exports or `.beetlebank` global sample libraries. The current Project Manager **does not** export/import the `BBK1` bank. A downgrade, storage migration or destructive reformat may lose those presets. Keep critical settings documented separately and do not reformat the shared device storage without a complete, verified backup and recovery plan.

## Drum Editor and Sample Editor

The native Drum Editor lets you edit built-in drum timbre settings and optional user samples. The global U01–U13 sample slots remain available regardless of whether you play BEETLE or MONOLITH.

| User-sample control | Function |
| :-- | :-- |
| **START** | Playback start, 0–99% |
| **END** | Playback end, 1–100%; later than START |
| **ATTACK / DECAY / SUSTAIN / RELEASE** | Envelope stages, each 0–100 |
| **GAIN** | Sample level |
| **DRIVE** | Saturation |
| **CHOKE** | Choke-group assignment |

For Storage V2 `BD4S` streamed samples, START/END use block-aware prefetch and an anti-click trim end. Project-local short µ-law samples use the same START/END window in forward and REV playback, including CH6 NOTE1/NOTE2/NOTE3, and apply a brief reverse tail fade. Editing these settings does **not** place copies of the original imported WAVs inside each project.

## Mixing and Performance

### Lane Mixer and Drum Mixer

Pressing **GLO** cycles through **Lane Mixer → Drum Mixer → Tracker**.

- **Lane Mixer:** control the six drum lanes as channels, with their own volume, pan and mute/solo-related settings.
- **Drum Mixer:** rotate **SELECT** to switch between **BEETLE**, **MONOLITH**, and **USER KIT 01–12**. **K1** chooses the component/pad, **K2** chooses a parameter, and **K3** adjusts it. Available columns include Gain, Pitch, Drive and Choke where supported. **SEL** saves eligible edits to a global User Kit. MONOLITH's 16 ROM components are represented separately from the U01–U13 global sample library.

### FX and Punch-In

- **Short FX press/release:** opens the focused step FX editor. **Double-tap FX:** opens Punch-In FX.
- **Hold FX and press keyboard keys:** activates momentary **Punch-In FX** without navigating away to the menu.
- **Punch-In BLEND:** K1 controls global 0–100% dry/wet. **PITCH:** K1 adjusts −24 to +24 semitones on the native ROM/PCM drum voices. Storage V2 streamed BD4S user samples currently remain fixed-pitch.
- **Overlay behavior:** shows the last activated effect that is still held. Releasing that effect reveals the previously pressed effect if it remains active.
- Punch-In FX, Song Mode, groove/dynamics editors, mute groups, snapshots and other retained BEETLE performance tools are available where implemented in native BLUE. Timing and physical key combinations still need hardware validation.

## Projects, Storage and Compatibility

- **Capacity:** 48 patterns, **12** device project slots and 64 Song Mode chain positions. This release does **not** add 64 patterns or 16 project slots, because safely doing so requires verified RAM, serialization and flash-layout changes.
- **Current format:** `BDR1` project **schema V31**, including the six lane-kit settings. V29 and V30 decoders remain supported; **older firmware cannot read V31 saves**. Preserve backups when upgrading.
- **Backups:** `.bdrum` holds BLUE project data, whereas `.beetlebank` is for the optional global sample library. Neither fully backs up the separate `BBK1` global kit bank.
- **Not interchangeable:** BLUE `BDR1` and the original BEETLE `TRK1` projects should not be treated as equivalent or imported by renaming a file extension.
- **Shared flash:** projects and global user samples share device storage. Never overwrite, migrate, format or repair storage merely because a different firmware does not recognize its contents.

### Project Manager

The BLUE Project Manager (`BEETLE-BLUE-Project-Manager.html`) supports project slots and optional global U01–U13 sample operations where the firmware's protocol supports them. It is **not required to play the built-in MONOLITH ROM kit**. USB project/sample transfer and global-kit-bank backup have not been physically validated for this build. Storage migration and marker-repair features inherited from BEETLE may be restricted to avoid accidental loss of the BLUE kit bank.

## Web Tools and UI Renders

The current release includes two browser tools, individually or together in the Manager:

| File | Purpose |
| :-- | :-- |
| [`WebInstaller.html`](WebInstaller.html) | USB-MIDI firmware installer with the V0.99.5 `.fwsc` embedded and verified by hash |
| [`BEETLE-BLUE-Project-Manager.html`](BEETLE-BLUE-Project-Manager.html) | Backup, restore and optional user-sample management |
| [`BEETLE-BLUE-Manager.html`](BEETLE-BLUE-Manager.html) | All-in-One Manager combining the Project Manager and WebInstaller |

The WebInstaller and Project Manager are the only supported browser tools in this release. WebInstaller firmware/hash checks validate package identity and structure; they do not guarantee successful or safe physical OTA installation.

`UI-CAPTURES-NATIVE-1920/` includes **17** reference captures produced using native C LCD drawing code with seeded interface states, including the tracker and Drum Mixer. They are captured at **240×240** and enlarged **8× with nearest-neighbor** to **1920×1920**. They are **not** photographs of the FM1 LCD, and not an exhaustive capture of every UI state.

> **GitHub Pages note:** the links above assume the corresponding HTML files are published in the repository root. `BEETLE-BLUE.png` must also exist alongside this README if the top logo is to render correctly; that logo file is not included in the source archive.

## USB-C Audio Recording (Experimental)

BLUE retains the inherited BEETLE USB Audio Class paths for stereo master audio and **six mono stems at 44.1 kHz / 16-bit**, subject to the supported device configuration. This captures **FM1 → computer** audio. It does not provide USB screen recording, and it is not computer-to-FM1 audio playback.

USB enumeration, sustained recording, sync, channel routing and stability have **not been validated on a physical FM1** for V0.99.5. Do not rely on an experimental build for irreplaceable recordings.

## Installation, Safety and Validation

1. Export and separately verify all important `.bdrum` projects and optional global sample libraries. Record global User Kit settings because `BBK1` export is not provided.
2. Confirm an **independent, working FM1 recovery path** before installing the experimental firmware.
3. Review [`validation/V0.99.5-RESULTS.md`](validation/V0.99.5-RESULTS.md) and verify the `.fwsc` hash if distributing the package.
4. Start hardware tests conservatively. Avoid headphones and powered monitoring at unmeasured output levels; use suitable level-limited test equipment.
5. Do not interrupt a flash write or trigger storage provisioning/formatting without an established backup and recovery procedure.

### Software validation reported for V0.99.5

| Check | Result |
| :-- | :-- |
| JieLi native production-options build | PASS |
| Read-only build / write-symbol audit | PASS |
| 14 BEETLE ROM components + 16 MONOLITH procedural patches | Host tests PASS |
| 3,000 repeated kick triggers and 4,096 hat/kick retriggers | Host tests PASS |
| PCM buffer clearing and project V29→V31 migration checks | PASS |
| REV: factory reverse playback and MONOLITH reverse-style output | Host tests PASS |
| REV FX pattern round-trip for CH1, CH6 and NOTE2 | PASS |
| Local sample START/END + REV and 3,000 retriggers | Host test PASS; CH6 integration compiled but not tested on-device |
| Firmware payload / CRC / header / OTA-loader comparison | PASS |
| MONOLITH hat choke, CH6 NOTE1/NOTE2 selection and oscillator PITCH | Host tests PASS |
| V0.99.5 exact 73-value pitch table, 3900 dense triggers and bounded stereo soft knee | Host tests PASS |
| Webtool JS syntax, embedded firmware and matching tool versions | PASS |
| Self-choke all 13 user slots (project-local and Storage V2) | Host tests PASS |
| Fastpath vs previous renderer: 768 frames, delayed refill | Exact PCM and state PASS |
| Actual FM1 flashing, DMA DEADLINE, analogue peaks, USB/OTA recovery | **NOT VERIFIED; V0.99.4 USER-REPORTED DMA DEADLINE** |

The V0.99.5 native app is **522,132 bytes**, with **59,432 bytes** of stated application-space headroom (of 581,564 bytes). The `.fwsc` is **610,086 bytes**.

**V0.99.5 `.fwsc` SHA-256:** `281300758b5308ca471524809dd62e0e206a0f76c1edce08b988b30d80d8f98d`

Those figures describe the software build, **not** CPU headroom, safe sound pressure, successful hardware flashing or proven absence of a DMA failure.

## Development and Source Layout

| Directory / file | Content |
| :-- | :-- |
| `firmware/fm1-native/` | Native sequencer, BEETLE/MONOLITH drum engines, LCD, storage, USB and OTA code |
| `assets/MONOLITH-Kit/` | Original 16 reference WAVs and historical mapping; not required for ROM playback |
| `web/` | Web tools, including historical references |
| `tests/` | Native host tests and software regression scripts |
| `validation/V0.99.5-RESULTS.md` | V0.99.5 build, host tests, checksums and hardware limitations |
| `UI-CAPTURES-NATIVE-1920/` | 18 native-code-rendered screen states, enlarged with nearest-neighbor |
| `docs/` | Licensing, source attributions and archived development references |
| `CHANGELOG.md` | Version-by-version changes |

### Build from Source

Obtain a compatible JieLi Linux toolchain separately. The toolchain is **not** part of the release archive.

```bash
export JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1
python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio
python tools/package_beetle_blue_v0995.py
# Optional: regenerate native LCD capture gallery
python tools/render_blue_v0991.py
# Web tools for this release are included in the ZIP; preserve the V0.99.5 embedded firmware hash
```

See the [`CHANGELOG.md`](CHANGELOG.md) for release history and [`validation/V0.99.5-RESULTS.md`](validation/V0.99.5-RESULTS.md) for the original software test evidence. The README is an operating and technical guide, not a substitute for hardware verification.

## Reporting Issues

When reporting a defect, include the firmware version, the on-device page, the kit and component, whether playback was stopped or running, the exact control sequence (especially repeated keyboard hits), and whether global U-samples or User Kits were involved. On the LOAD PROJECT screen, press **EDIT** on an occupied slot for individual deletion and confirm YES; default is NO. The project file format is **BDR1 V31**: this build reads earlier supported versions, but previous firmware may not read a V31 project saved by this build. Back up before installing.

For `DMA DEADLINE / AUDIO STOP` failures, include how many rapid kick triggers were pressed and whether the issue also occurs with the plain BEETLE kit. Keep private projects or complete flash dumps private unless intentionally sanitized.

---

## Credits and Licenses

**BEETLE BLUE** is a SikkenWorks drum-focused derivative of **BEETLE**, based on the BEETLE V1.7.14 development line. It is **not** a clean-room reimplementation. The native firmware is distributed under **GNU GPL-3.0-only**. Keep the associated `LICENSE`, `LICENSES/`, source-level notices and third-party credits.

| Project / contributor | Attribution |
| :-- | :-- |
| **[BEETLE](https://github.com/SikkenWorks/BEETLE)** · SikkenWorks | Original FM1 tracker, sequencing, project storage, performance, DSP, USB and OTA foundation |
| **[Felucca](https://github.com/hugelton/Felucca)** · Leo Kuroshita / Hügelton Instruments | FM1 hardware abstraction, LCD/input, startup, audio and OTA foundation |
| **[ETCH](https://github.com/SikkenWorks/ETCH)** · SikkenWorks | Earlier FM1 firmware and part of the protected firmware/OTA packaging lineage |
| **JieLi Technology** | Target-chip and toolchain manufacturer |

The visible on-device BLUE credits no longer include a reSID label. This does **not** authorize removal of any copyright or license notices that are legally required for inherited source files. The original MONOLITH WAVs were user-supplied and their public redistribution rights have **not** been independently confirmed. Check those rights before publishing the WAV assets.

<p align="center"><strong>BEETLE BLUE · SikkenWorks · Born out of passion.</strong></p>

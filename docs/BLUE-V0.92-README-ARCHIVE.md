# BEETLE BLUE V0.92 — Six-lane drum tracker for the M-VAVE FM1

**SikkenWorks · Born out of passion.** Experimental native JieLi firmware derived from BEETLE V1.7.14, ETCH and Felucca. The full source remains GPL-3.0-only; preserve notices in `LICENSE`, `LICENSES/`, `docs/` and native source.

> **EXPERIMENTAL.** A successful native build, `.fwsc` package verification, host tests or browser tests do **not** prove that the firmware is safe to flash, works without audio peaks/DMA failures, or can be recovered on a physical FM1. Avoid testing the experimental audio output with headphones, speakers or an amplifier until independently measured. A verified backup and independent firmware recovery route are required before any device experiment.

## New in V0.92: MONOLITH actual WAV drum kit

- **Native firmware:** MONOLITH source 1 now resolves KIK–PC3 to the corresponding **global WAV slots U01–U13**. If a WAV is missing, that drum is SILENT instead of silently substituting another built-in drum sound. The 14th default RIM component still uses a ROM rim, because the hardware/global sample-bank layout has only 13 U-slots. Per-note kit override and independent per-lane kit assignment are preserved, as is the V30 project codec.
- **Browser WebSimulator:** its MONOLITH mode now plays **real decoded WAV assets from all 16 original files** provided by the author rather than synthesized sine/noise approximations. Other simulated kits still use a browser approximation. Unlike the native firmware, the simulator embeds the WAV data; the browser is not a verified FM1 DSP emulator.
- **Ready-to-load sample library:** `BEETLE-BLUE-MONOLITH-DRUM-KIT.beetlebank` contains a 13-slot PC library with converted **22,050 Hz mono PCM16 WAV** data. Its checksum/index validation and encode/decode round-trip were verified using the actual Project Manager JavaScript codec. Originals stay untouched under `assets/MONOLITH-Kit/`. The default U-slot mapping is in the asset folder. Three original samples are optional alternatives (`rim.wav`, `snare_2.wav`, `hihat_2.wav`). No MONOLITH sample is flashed automatically with the firmware; upload the bank separately using the Project Manager.
- **Native tracker screenshot:** `UI-CAPTURES-NATIVE-1920/TRACKER-NATIVE-EXACT.png` was generated from the verbatim, compiled `track_fm1_grid()` drawing function of this release. It draws a *test-state pattern*, not a saved user project or physical LCD capture. The 240×240 RGB565 framebuffer is enlarged to 1920×1920 using nearest-neighbour (8×). The capture C harness and other 15 UI states are included.
- **Webtools:** WebInstaller is pinned to this V0.92 firmware; Manager embeds the matching simulator/Project Manager/installer. Normal project SAVE/LOAD and Storage V2 sample handling were not intentionally removed.

### Import the MONOLITH drumkit onto the FM1

1. **Back up your existing U01–U13 sample library** and BLUE projects before attempting any USB transfer. Project backups do not contain global WAVs or the independent BBK1 global user-kit settings.
2. Open `BEETLE-BLUE-Project-Manager.html` using a supported browser with HTTPS/localhost for Web MIDI, then select **OPEN .BEETLEBANK** and choose `BEETLE-BLUE-MONOLITH-DRUM-KIT.beetlebank`.
3. Verify all 13 PC WAV slots and the connected FM1's existing U-slot state. Choose upload only if you intend to replace the corresponding existing samples. Storage V2 is required and physical transfer behavior remains unverified.
4. On the tracker, hold **SEQ**, rotate **SELECT** to choose `MONOLITH` for a lane, and release. The imported WAVs are then used by the native drum engine. The normal ROM-kit remains independently selectable. Missing samples remain silent to avoid giving an incorrect impression of the imported kit.

**Critical limitations:** There is not enough verified app ROM space to embed all WAVs in the `.fwsc`. This release retains **48 patterns and 12 project slots**. The earlier **DMA DEADLINE / AUDIO STOP after rapid double kick triggers has not been independently solved or tested on an FM1**. The firmware, updater, output levels and flash writes remain experimental. Never test against irreplaceable data or connected listening equipment without independent recovery and output safety measurements.

## What's new in V0.91 (previous release)

1. **Independent lane kit assignments:** hold **SEQ** and rotate the physical **SELECT encoder** while viewing the Tracker. The currently selected lane CH1–CH6 receives a kit-source assignment `0..13`. A temporary **blue LCD overlay** shows the lane number and current kit name. Normal SELECT (without SEQ) still changes the focused lane. SEQ+SELECT detents are consumed and do not accidentally change the selected lane or navigate on SEQ release.
2. **All lanes start on DEFAULT KIT** (`source 0`, BEETLE ROM). The selection is saved **per project** and restored with SAVE/LOAD. The separate kit *definitions* in the twelve USER KIT slots are stored globally via the existing BBK1 Storage V2 metadata, not inside each project.
3. **14 kit-source choices:** BEETLE ROM, MONOLITH sample routing and USER KIT 01–12. MONOLITH depends on imported U01–U13 global WAVs; the 16 supplied original MONOLITH WAVs cannot all be installed simultaneously in thirteen slots.
4. **Per-step instrument overrides retained:** the normal lane-default `INST` in each lane follows that lane's kit. An explicitly different `INST` selects the existing eight-slot kit palette instead. For the inherited tracker default, lanes 1–5 use `INST=channel index`, and CH6 uses `INST=3`. This behavior preserves existing notes and explicitly customized instrument cells.
5. **Project data upgraded to BDR1 V30.** Six lane-kit bytes are serialized, validated and migrated from earlier BLUE versions. V29 real-fixture migration and V30 CRC/validation tests are included; older BLUE project bytes do not receive arbitrary kit IDs on load.
6. **Four BEETLE BLUE webtools:** `web/simulator.html`, `web/BEETLE-BLUE-Project-Manager.html`, `web/WebInstaller.html`, `web/BEETLE-BLUE-Manager.html`. These are adapted from the four BEETLE webtools supplied for this release; untouched originals and corresponding README/CHANGELOG are under `reference/UPLOADED-BEETLE-WEBTOOLS/`. A matching firmware image is embedded inside the WebInstaller and All-in-One Manager.
7. **Native renderer captures:** 15 chosen LCD/UI states are rendered by source-extracted C drawing code, at 240×240 pixels and exactly 8× nearest-neighbor to 1920×1920 PNG. The kit-aware Drum Mixer now includes DEFAULT, MONOLITH and USER KIT states. These are *selected source-renderer states*, not a complete inventory of firmware screens or a physical FM1 capture. Kit overlays are isolated over black rather than composited over a tracker image.

### Drum controls

| Control | Behavior |
|---|---|
| Turn SELECT | Focus CH1–CH6 |
| **Hold SEQ + turn SELECT** | Assign source kit **for the currently focused lane**; view temporary overlay |
| Turn ALGORITHM | Select one of 48 patterns (existing behavior) |
| K1 / K2 / K3 in Tracker | Navigate steps / fields / edit values |
| EDIT then EDIT | Drum Editor, then Kit Designer for eight active palette references and global preset storing |
| GLO (once / twice) | Lane Mixer / kit-aware Drum Mixer. In DRUM MIXER SELECT chooses the kit, K1 component, K2 column, K3 value. USER KIT changes are saved globally by SEL while stopped. |
| FX | Short release opens FX menu; long-hold permits Punch-In notes and latest-held-effect overlay (carried from V0.7) |
| HOME / SAVE | Project management: 12 existing project slots |

The existing song chains, microtiming, pattern edits, six-lane audio routing, user sample START/END and ADSR editing, DRIVE/CHOKE, punch effects and storage facilities are retained in the source. `BDR1` drum projects are **not** interchangeable with original BEETLE `TRK1` synth projects.

## Storage and backups

- **12 project slots**, **48 patterns**, **8 active per-step kit-palette references**, **6 project-local lane defaults**, **12 global user kit timbre presets**, **13 global streamed user WAV sample slots**.
- All six lane-kit selections are stored in each **BDR1 V30** project; save the project to make changes permanent.
- USER KIT 01–12 settings are stored in the separate `BBK1` global footer, and WAVs in Storage V2's separate global sample bank. **Exporting `.bdrum` alone does not back up BBK1 or the global WAV library**. The supplied Project Manager cannot yet export BBK1; record/backup those presets separately. **Destructive Storage V2 volume migration and owner-marker repair are disabled in the BLUE Project Manager** to protect the independent kit bank.
- The 12 project slots share physical NOR flash with BEETLE. Original BEETLE and BLUE use different project data identities. Never assume installing the other firmware gives you another 12 isolated slots. Storage operations and firmware flashing remain experimental.
- Raising capacity to 64 patterns / 16 slots has **not** been done: existing storage A/B allocation and project-bank size make a constant-only change unsafe. V0.91 retains 48 / 12.

## Webtools and practical limitations

`BEETLE-BLUE-Manager.html` is a self-contained all-in-one page with embedded simulator, Project Manager and installer. Standalone tools are available in `web/` and also at repository root for GitHub Pages deployment. The simulator is a **browser-based six-lane drum prototype**, not a bit-identical FM1 LCD/DSP emulator. It supports the lane-kit control, but its generated audio does not validate native sound. The Project Manager retains BLUE `.bdrum` management and existing project/sample flow, and accepts V30 project files in its local one-shot validator. Full storage migration/marker repair is disabled while BBK1 cannot be backed up. The WebInstaller includes the exact V0.92 experimental firmware and retains the original two-stage USB-MIDI updater flow; **its flashing path has not been validated on physical FM1 hardware**. Be especially careful of V2 migration, erasure and hardware marker repair routes. Browser USB support requires a compatible browser and HTTPS/localhost.

The firmware retains USB-C audio/stems source pathways, but their stability and real-device enumeration are not confirmed. Prior reports of **DMA DEADLINE / AUDIO STOP after two quick kick triggers** are **not proven resolved on hardware**. V0.9 reduced IRQ-off editor work and made drum DSP optimizations, but V0.91 is not a physical audio safety certification.

All provided web pages block right-click/context menus and text selection. They contain no deliberate remote transfer of user project/sample contents, but native USB device operations may write to FM1 when explicitly invoked.

## Build and tests

Requires the **separately provided JieLi Linux toolchain**:

```sh
export JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1
python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio
python tools/package_beetle_blue_v092.py
sh tests/run_blue_v091.sh
```

See `V0.92-RELEASE-MANIFEST.json` for V0.92 results (and `validation/V0.91-RESULTS.md` for the previous release), firmware hash and known exclusions. The WebInstaller contains the exact built `.fwsc`, with unchanged protected 16 KiB header and OTA loader bytes relative to the reference. These integrity properties **do not** guarantee bootability, safe device programming, audio levels or recovery.

## Credits

BEETLE BLUE is developed by **SikkenWorks** and derives from [BEETLE](https://github.com/SikkenWorks/BEETLE), [Felucca](https://github.com/hugelton/Felucca) and [ETCH](https://github.com/SikkenWorks/ETCH). JieLi supplies the target hardware platform and compiler toolchain. Historical licences, source-level credits and relevant third-party notices remain in the archive. The **original MONOLITH WAV redistribution rights have not been independently verified**; do not publicly redistribute those sounds without permission. Visible credits pages are drum-focused and omit the reSID reference as requested, but preserving applicable upstream source attributions remains essential.

**BEETLE BLUE · Born out of passion.**

## Source audit

See `docs/V0.91-SYNTH-AUDIT.md`. Inherited synth rendering is already compiled out of BLUE audio output, while some legacy synth code and serialized patch structures remain pending a safe codec migration. No extra CPU savings are claimed without hardware timing.

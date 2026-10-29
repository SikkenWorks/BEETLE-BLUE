# BEETLE BLUE | Credits, copyright and source provenance

BEETLE BLUE is a six-lane drum-focused derivative developed by **SikkenWorks (Joppe Sikken)**. It is **not** a clean-room FM1 firmware. The distributed native source is licensed **GPL-3.0-only**, subject to retained notices and the specific third-party terms included under `LICENSES/` and `firmware/fm1-native/LICENSES/`. Do not remove upstream source-file copyright headers.

## Project lineage and hardware platform

| Component | Original author / attribution | Relationship to BEETLE BLUE |
| :-- | :-- | :-- |
| BEETLE BLUE | **SikkenWorks / Joppe Sikken** | Drum-focused user experience, BLUE drum engines, native tracker UI, sequencing and subsequent changes |
| [BEETLE](https://github.com/SikkenWorks/BEETLE) | **SikkenWorks / Joppe Sikken** | Tracker, project/storage, DSP, USB, live-performance and firmware foundations |
| [ETCH](https://github.com/SikkenWorks/ETCH) | **SikkenWorks** | Earlier FM1 firmware, boot/OTA lineage and some preserved support code |
| [Felucca](https://github.com/hugelton/Felucca) | **Leo Kuroshita (@kurogedelic), Hügelton Instruments**, copyright (C) 2026 | FM1 hardware abstraction, startup, display/input, audio/USB and OTA foundations; **original ADC battery thresholds** |
| JieLi AC79 hardware and SDK | **JieLi Technology** | Hardware/toolchain and third-party vendor package data, as applicable; do not attribute BEETLE source authorship to JieLi |
| M-VAVE FM1 hardware | **M-VAVE** | Physical hardware platform, not a claim that M-VAVE authored BLUE firmware |

## Battery indicator: explicit upstream credit

**Original thresholds:** Felucca `firmware/src/ui_draw.c`, function `batt_level()`, by **Leo Kuroshita / Hügelton Instruments** (GPL-3.0-only), maps ADC channel 3 readings to bands at **531, 561, 591**. Confirmed against the supplied `Felucca-main.zip` source. In upstream Felucca a configured, non-suspended USB interface is used as an external-power indicator; it is *not* a battery charge-current measurement.

**BEETLE BLUE implementation:** `firmware/fm1-native/beetle_battery_policy.h` carries these threshold values. BLUE's own C/UI code handles four-bar styling, foreground ADC polling, smoothing and invalid-read behavior. Thus the battery-voltage thresholds should be **credited to Felucca**, while BLUE's adaptation is attributed to **SikkenWorks**. The UI does not establish a percentage-accurate state of charge, a charge-complete signal or measured battery charging. `HOME > CREDITS` includes a short upstream attribution; the full details are here and in the source header.

## Built-in drums, kits and audio assets

- **BEETLE factory kit:** derived from the project's original BEETLE PCM/µ-law ROM drum sources; see the retained `beetle_drum_rom.c` and generation/reference tooling for provenance.
- **MONOLITH:** BEETLE BLUE's independent procedural drum rendering (not an incorporated MONOLITH WAV player). The reference WAVs were supplied by the project user. **Public redistribution rights for those WAV files have not been independently verified.** Do not treat user-supplied recordings as automatically CC0 or royalty-free.
- **Global User Kits** and BLUE drum editing: BLUE project-specific code and data handling; no claim of authorship over the underlying Felucca HAL.

## BEETLE-era reference projects retained in the source

- **reSID:** originally written by **Dag Lem**, MOS 6581 SID emulation; upstream GPL-2.0-or-later and [source](https://github.com/libsidplayfp/resid). BEETLE's SID-inspired C implementation is **not** the original reSID C++ emulator. BLUE's six-lane drum build does not expose those BEETLE SID synth voices.
- **Nes_Snd_Emu:** originally **Shay Green (blargg)**, with the linked fork maintained by **James Athey** ([source](https://github.com/jamesathey/Nes_Snd_Emu), LGPL-2.1, text in `LICENSES/NES-SND-EMU-LGPL-2.1.txt`). Its C++ code is **not** embedded or linked; retained BEETLE NES C code is a separate implementation and not part of BLUE's active drum voice selection.

These are specific provenance acknowledgements, not assertions that all technical references or engines are actively compiled in BEETLE BLUE.

## Preserved third-party licensing notices

`LICENSES/FELUCCA-LICENSING.txt` documents upstream code and dependency provenance, including DaisySP (Electrosmith / Emilie Gillet), Rings (Emilie Gillet), msfa/Dexed (Google / Pascal Gauthier), CrispyZebra, Fukiai, Inter Tight, DotGothic16, Versilian Studios VSCO-2, klattsch and X0X. The specific MIT, Apache-2.0 and OFL notices remain under `firmware/fm1-native/LICENSES/` where included. These notices concern lineage and applicable files, **not** a claim that every listed third-party library, font or audio asset ships or runs in BLUE.

The FM1 WebInstaller's MIDI/OTA transport derives from Felucca's open-source updater. Preserve the GPL-3.0-only attribution in its script, plus the original copyright in `firmware/fm1-native/lcd.c` and other inherited source files. The experimental firmware package also retains protected loader/SDK data from the established ETCH/Felucca packaging lineage; changing the app's CREDITS screen does not transfer ownership of those components.

## Licence, redistribution and limitations

Keep complete corresponding source, `LICENSE`, `LICENSES/`, the retained source-level notices and this document with redistributions of the firmware. Do not mistake additional attributions for sublicensing third-party recordings or vendor firmware. The source-code audit and successful build are **not** a physical FM1 test, nor proof of safe OTA recovery, analog-output levels or accurate battery charging.

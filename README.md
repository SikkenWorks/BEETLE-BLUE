<p align="center">
  <img src="BEETLE-BLUE.png" alt="BEETLE BLUE pixel-art boot image" width="320">
</p>

<h1 align="center">BEETLE BLUE</h1>
<p align="center">
  <strong>An open-source, six-channel drum tracker and ROM-based drum workstation for the M-VAVE FM1.</strong><br>
  SikkenWorks · Born out of passion.
</p>

<p align="center">
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/WebInstaller.html">Install BEETLE BLUE</a> ·
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/BEETLE-BLUE-Project-Manager.html">Project Manager</a> ·
  <a href="https://sikkenworks.github.io/BEETLE-BLUE/BEETLE-BLUE-Manager.html">All-in-One Manager</a>
</p>

<p align="center"><sub>Current source: V1.0 RC12 (experimental; see STATUS.md for installation status) · <a href="CHANGELOG.md">Release history</a> · <a href="validation/">Validation reports</a></sub></p>

## Overview

BEETLE BLUE is the drum-focused fork of [BEETLE](https://github.com/SikkenWorks/BEETLE), with six drum sequencer lanes and two built-in ROM drum engines. **BEETLE BLUE has been extensively tested by its developer on two physical M-VAVE FM1 units.** Its main capabilities are:

| Feature | Capability |
| :-- | :-- |
| Drum lanes | Six; independently selected drum source and loop length per pattern |
| Patterns | 48 patterns; each of six lanes is **1–16 steps** |
| Song Mode | Up to 64 chain positions, with individual source patterns per lane |
| Live Mode | 24-position Setlist; manual-start Perform with loading progress overlay |
| Built-in kits | BEETLE (14 PCM/µ-law components), MONOLITH (16 procedural ROM patches) |
| User Kits | 12 global timbre-preset slots |
| User samples | U01–U13 shared slots, requiring compatible Storage V2 support |
| Drum processing | Per-step FX, Punch-In FX, groove/ratchets, mute groups, snapshots and mixer |
| Storage | 12 project slots, `.bdrum` exports, `BDR1 V33` project format |
| USB capture | Experimental stereo master or six mono stems at 44.1 kHz / 16-bit |

## On-device HELP

Press **HOME → HELP** for a compact, on-device reference. Each screen presents **up to three separate cards** with a control and its action. Turn **K2** to choose one of ten topics and turn **K1** to move between short pages in that topic. **HOME** returns to the menu and **SEQ** returns to the tracker. These pages are available without altering your project.

## Getting started

1. Press **SEQ** to open the Tracker and turn the **SELECT** encoder to choose CH1–CH6. **SEL** means pressing the encoder; it is not the same as turning SELECT.
2. Turn **ALGORITHM** to select PAT 01–48 while stopped. **K1** selects a step, **K2** a field and **K3** adjusts its value; the keyboard enters and auditions drum hits on supported pages.
3. Turn **K4** for BPM. Press **PLAY/STOP** to start or stop playback.
4. Hold **SEQ** while turning **SELECT** to assign BEETLE, MONOLITH or an available User Kit to the selected lane.
5. Press **EDIT** for Drum Edit and **EDIT** again for Kit Designer. Press **GLO** for Lane Mixer, then **GLO** again for Drum Mixer.
6. Press **ARP** for Song Mode; use **HOME** to open project, Live Mode, settings and storage operations.

## Independent lane lengths — 1 to 16 steps

**Each of the 48 patterns stores six separate loop lengths**, one for each lane. For example, in one pattern, CH1 can be 16 steps, CH2 12, CH3 7, CH4 16, CH5 8 and CH6 4. Every lane independently restarts at step 1 when it reaches its own length. The pattern transport cycle never exceeds 16 steps.

**To change a lane's length:** open the Tracker (**SEQ**), rotate **SELECT** to choose the lane, then **hold SEQ and turn K4**. The screen displays its current length, such as `LEN 07`. Only the selected lane changes. The normal K4 action (without SEQ) still changes BPM.

Shortening a lane **does not delete** notes or FX past the new end. Lengthening it again restores access to them. Lane lengths and the notes they contain survive SAVE/LOAD, block editing, Song Mode and Live Mode. CH6's extra hits, step FX, ratchets and groove follow their lane's loop length.

**Project compatibility:** this firmware saves `BDR1 V33`. Supported older project versions, including V32, are loaded with their previously shared pattern length applied to all six lanes. Older firmware cannot read V33 saves. Back up before downgrading; no storage wipe is needed for lane lengths.

## Built-in drums and User Kits

**BEETLE** is the factory 14-component PCM/µ-law drum ROM kit. **MONOLITH** provides 16 distinct, procedurally generated drum sounds, including alternative snares, hats and percussion. MONOLITH is not a copy of the development WAV reference files.

Kit selection is **per lane**, saved with the project. In Drum Mixer, turning SELECT changes the kit for the selected lane; use K1/K2/K3 to choose a drum component, parameter and value. The global U01–U13 user-sample library is separate from factory ROM kits.

### Editing and storing global User Kits

A Global User Kit contains **14 factory drum-component timbre presets** (including ADSR, gain, pitch, drive and choke). It is not a 14- or 16-WAV sample bank; two additional MONOLITH variants are not included.

1. In **Drum Edit**, choose or adjust the settings for your drum components.
2. Press **EDIT** again for **Kit Designer**. Use **K1** for palette entry `INST 1–8`, **K3** to select the source or USER KIT 01–12, and **SEL** to store the chosen User Kit when storage is writable and playback is stopped.
3. After a successful store, that User Kit becomes active on the currently edited lane. **Also SAVE the project** to retain its lane-to-kit assignment.

Global User Kits live in Storage V2 `BBK1` metadata, **outside** normal `.bdrum` project exports. Later edits to a global kit affect projects referring to it. The current Project Manager does not export/import this global kit bank. Document critical settings before a destructive format or downgrade.

### User samples

Imported U01–U13 samples can be edited with START, END, Attack/Decay/Sustain/Release, GAIN, DRIVE and CHOKE. Their attacks can be prewarmed in RAM; longer tails still stream from flash. These settings do not copy the original WAV into each project. Long-sample streaming and worst-case DMA timing may still benefit from targeted stress testing.

## Patterns, step FX and Song Mode

Use **K1/K2/K3** in the Tracker to navigate and edit. On a focused FX cell, tap **FX** to edit up to three step FX slots; double-tap FX for Punch-In, or hold FX for a momentary effect. In the step editor, K1 chooses FX slot, K2 effect type, K3 value, OCT+ next slot, OCT− clear. The VOLUME effect supports 0–200%. REV reverses PCM ROM hits; MONOLITH uses a reverse-style procedural envelope. Streamed U-sample REV is not supported.

Song Mode is a **64-position chain inside one project**, distinct from Live Mode's inter-project Setlist. On the Song Chain page, **SEQ+OP1** copies selected position(s) and **SEQ+OP2** inserts them after the cursor where space permits. Song Mode uses each source pattern's independent lane lengths.

## Live Mode — Setlist and Perform

At boot choose **LIVE MODE**, or open **HOME → LIVE MODE**. Choose **SETLIST** to arrange projects or **PERFORM** to play them live. A Live Setlist is global and separate from any project's Song Chain.

### Setlist

The Setlist contains up to **24 positions** referring to the existing 12 saved project slots. The same project can appear at several positions. Use **K1** to select a setlist position, **K2** to assign a saved project, **K4** to move the position up or down, **OCT+** to insert and **OCT−** to remove.

Leaving Setlist with **HOME** or **SEL to Perform** silently saves changes when the storage volume permits it. Unchanged setlists do not create unnecessary writes. If a save fails, the editor reports an error and preserves the unsaved edits in RAM. **HOME** returns to the Live Mode menu, and a second HOME returns to the ordinary HOME/Projects menu.

### Perform

Perform opens the first assigned project, including its own tempo, selected kits, patterns, Song Chain and mix settings. A **loading overlay with real progress** appears while project data is read. Press **PLAY/STOP** to start; the next song is prepared only when the current one finishes, and **waits for your next PLAY**. This avoids unexpectedly starting the next track.

When you stop during a performance, choose **CONTINUE CURRENT TRACK** or **START CURRENT TRACK AGAIN** using K1 and confirm with SEL. A warm digital audio transition is designed to mute output while the next project loads without turning off the analogue output on normal transitions. As with any live audio system, completely pop-free operation cannot be guaranteed for every playback state and external audio setup. No project flash load happens in the audio interrupt.

## Battery indicator

The Tracker and Perform show a four-level pixel battery-voltage indicator. Its voltage bands are based on **Felucca by Leo Kuroshita / Hügelton Instruments** (ADC thresholds 531/561/591); BLUE adds display filtering and an invalid-reading indication. This is **not** a calibrated percentage or an independently verified charge-current measurement. An enumerated USB connection does not prove that the battery is charging.

## Projects, storage and backup

BEETLE BLUE supports **12 project slots**. Project files use `BDR1 V33`; `.bdrum` backups are not interchangeable with original BEETLE `TRK1` projects. The optional `.beetlebank` library is separate from both project files and global User Kit presets.

Project slots and imported samples share finite flash storage. `WIPE ALL STORAGE` is a **destructive Storage V2 format**, not a repair for unsaved edits: it deletes all saved projects, setlists, imported samples and global User Kits. Only perform it after a verified, complete backup and explicit confirmation; never wipe simply to fix another feature.

## Web tools and development

| File | Purpose |
| :-- | :-- |
| [`WebInstaller.html`](WebInstaller.html) | USB-MIDI firmware installer |
| [`BEETLE-BLUE-Project-Manager.html`](BEETLE-BLUE-Project-Manager.html) | Project/sample backup and transfer |
| [`BEETLE-BLUE-Manager.html`](BEETLE-BLUE-Manager.html) | All-in-One web interface |

**Installation status:** the native HELP changes are included in the source build, not in the currently published WebInstaller. Check [STATUS.md](STATUS.md) for the exact installed-firmware version and release-gate status. No new flashable firmware is provided with this source-only update.

The USB audio interface can expose stereo or six-channel mono capture. USB capture remains experimental; compatibility and recording stability may vary with the connected computer and audio setup. Native LCD reference images under `UI-CAPTURES-NATIVE-1920/` are screenshots of the C display renderer, not photos of hardware.

### Build from source

Provide the compatible JieLi Linux toolchain separately, then from the source directory run:

```bash
export JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1
python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio
```

The toolchain is not distributed in the release archive. Native build checks complement the extensive testing on two physical FM1 units; they do not establish OTA recovery safety or replace a verified backup. Detailed version-by-version changes belong only in [CHANGELOG.md](CHANGELOG.md); technical checks are in `validation/`.

## Reporting issues

Include the firmware version, page, exact control sequence, kit and component, whether transport was stopped, and any `DMA DEADLINE / AUDIO STOP`, storage or audible pop errors. Keep personal project exports/flash dumps private unless deliberately sanitized.

## Credits and licenses

BEETLE BLUE is developed by **SikkenWorks** and derived from **BEETLE** and **ETCH**. The [Felucca project](https://github.com/hugelton/Felucca) by **Leo Kuroshita (@kurogedelic) / Hügelton Instruments** contributes FM1 platform foundations and the battery ADC voltage thresholds. **JieLi Technology** produces the target chipset/toolchain. Source-level licence notices, `LICENSE`, `LICENSES/`, and [full attributions](docs/CREDITS-AND-LICENSES.md) remain part of this source release.

<p align="center"><strong>BEETLE BLUE · SikkenWorks · Born out of passion.</strong></p>

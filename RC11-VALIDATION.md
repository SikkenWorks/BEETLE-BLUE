# BEETLE BLUE V1.0 RC11 — Native/host validation

**Status:** experimental firmware build. No physical M-VAVE FM1, analogue audio or real USB-OTA/recovery test was performed.

## Feature and controls

CH1–CH6 each store a **1–16 step length per pattern (48 patterns)**. Default is 16. In Tracker, select a lane with SELECT, then **hold SEQ and rotate K4** to set its `LEN` value. The overall pattern cycle is the maximum lane length, never more than 16. Shorter lanes wrap step-by-step independently. Song Mode composes each lane with its source length. When length decreases, off-range step data is retained and reappears upon lengthening.

## Findings and regressions

| Test | Result / evidence |
| :-- | :-- |
| Writable native JieLi FM1 build | PASS; application **538,172 B**, free space **43,392 B** (581,564 B app maximum) |
| Read-only native JieLi build and write-symbol audit | PASS; **464,784 B**; forbidden-write symbol audit passed |
| RAM: `track_step` / `track_pattern` unchanged sizes | PASS: **6 / 625 bytes**. Lengths encoded in reserved step-0 bits. No added RAM array. |
| Lane lengths 1, 7, 12, 16 and independently changing selected lane | PASS native-host UBSAN test |
| All lanes 8; extend only CH2 to 12, then 16 | PASS; other lanes remain at 8 |
| Step, groove, ratchet, NOTE2/NOTE3 repeat behavior | PASS native-host UBSAN test |
| BDR1 V33 project save and exact reload of lengths, notes and drum FX | PASS |
| Standalone pattern V6 write/read | PASS; V1–V5 decoding retained |
| Authentic RC10 V32 fixture → RC11 decoder → V33 save/reload | PASS; all 6 lanes receive legacy shared length |
| Song Mode composition and block-edit copying row 0 | PASS; source and lane metadata preserved |
| Original BEETLE Song Chain regression (non-BLUE) | PASS; its original shared-length path preserved |
| Existing RC1.0.10 storage-workspace, CRC and simulated power cuts | PASS |
| Firmware packaging: protected first 0x4000 of flash and OTA payload | PASS; byte-identical to RC10 reference |
| Firmware plaintext matches native audited APP | PASS |
| WebInstaller embedded `.fwsc`, SHA-256 and Manager nested webtools | PASS byte-identical; JavaScript syntax checked |
| Physical FM1 playback/navigation, OTA and project persistence | **NOT TESTED** |

## Source-reproducible tests

Run from repository root: `bash tests/track/run_blue_rc11_lane_lengths.sh`; `bash tests/track/run_rc1010_storage_workspace.sh`; `bash tests/track/run_rc1011_song_chains.sh`.

Firmware: `JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1 python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio`.

Read-only firmware: `JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1 python firmware/fm1-native/build.py --drum-edition --hardware-test`.

## Compatibility and safety

- Device storage partition, Storage V2 reader/writer, User Kits, audio output and credits remain unchanged.
- New saves are **BDR1 V33**; pre-RC11 builds cannot read V33. RC11 reads V32 and supported historical projects and maps their global pattern length to every lane.
- Saving is not an automatic destructive migration; **do not wipe storage** to activate lane lengths.
- Firmware updates can erase unsaved RAM state. Back up existing projects, global kits and samples and verify independent recovery BEFORE flashing experimental builds.
- Host tests do not establish safe physical analogue levels, DMA performance or OTA recovery. Flash only with a recovery method and controlled output levels.

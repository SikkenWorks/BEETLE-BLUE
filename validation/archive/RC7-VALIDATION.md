# BEETLE BLUE V1.0 RC7 — Storage V2 format and mount repair

## Reproduced root causes

- RC6 `STORAGE LEGACY / 12K` is real: the FM1 is using the older TRKJ journal. RC5 V32 compression is only exercised when the flash is mounted as Storage V2. Wipe All Storage in RC4–RC6 picked `use_v2` by the previous marker. A legacy volume therefore stayed legacy after erasing.
- CRITICAL: `beetle_hal_wipe_projects()` initialized the new B2SB 4 KiB index with FF bytes and only set the low byte of version, generation and length. Fields at offsets 5–7, 9–11 and 14–15 remained FF, which violates V2 index parser requirements. Reproduced in a real compiled host flash simulator: the old format returned -5 and `beetle_v2_open()` returned -2. RC7 zeroes the whole 4..19 header region before setting those fields.
- RC7 explicitly formats valid legacy, blank, and recognized BEETLE-V2 volumes as canonical Storage V2 after the existing TWO physical confirmations. Corrupt V2 index with valid BEETLE V2 marker can be reformatted. Unknown/foreign/corrupt markers are rejected; no bypass of flash ownership safeguards, no automatic conversion on boot/save.
- Success is followed by actual V2 marker and index readback plus `beetle_v2_mode()==1` verification before `STORAGE READY`. Readback happens with audio paused to avoid flash contention.

## Measured tests — all SOFTWARE ONLY

- `tests/storage_v2/test_blue_rc7_wipe_converts_legacy.c`: PASS. Uses mocked 1 MiB NOR and the ACTUAL firmware wipe implementation, V2 reader, V2 writer and compressor. Valid legacy with data -> V2 format -> writer opens -> save slot 2 -> decode equals source -> reformat -> empty slots, CRC. Also validates blank volume, V2 reformat with corrupt index, double confirmation, foreign marker rejection, interrupted erase, and preservation of all bytes outside `0x97000..0xDFFFF`.
- Native writable JieLi (`--beetle-storage-lab --beetle-v2 --updater --usb-audio --drum-edition`): PASS; app **536,084 bytes**, headroom **45,480 bytes**.
- Native read-only JieLi (`--hardware-test --drum-edition`): PASS; forbidden write symbol audit PASS.
- Existing V2 writer test: PASS (all 12 projects/13 samples, 71 flash interruption points, preserving old data under failed transactions).
- Existing journal/workspace test: PASS.
- Firmware `.fwsc` integrity, protected boot header (first 16 KiB), OTA loader, app decryption match and embedded installer bytes: PASS. Firmware SHA256: `d51c605d2a9d1ed48e2e2c0fb8f4129b019f46072c27b2012b1fa9b6198d70f4`.
- **Physical M-VAVE FM1:** NOT TESTED. Repeated `PROJECT DATA ERROR` has other potential causes, particularly invalid project structures. Do NOT promise all save paths fixed based only on formatted-volume host tests. No audio/DMA reliability claim.

## CRITICAL DATA LOSS / FLASHING NOTE

- RC7 does NOT automatically migrate legacy projects. The corrected Wipe All Storage destroys **all** BEETLE projects, global sample banks, setlists and user kits in the BEETLE-owned 0x97000–0xDFFFF region. It cannot rescue the current unsaved project from RAM across firmware installation or restart. Make/verify a complete backup and ensure independent recovery before flashing/formatting.
- The legacy-to-V2 conversion is ONLY triggered by Wipe All Storage with both confirmations, never by regular project SAVE or power-on. Corrupted/foreign markers, I/O faults or unexpected data remain hard-fail and cannot be “fixed” by unsafe blind erasing.
- Power loss during wipe can leave storage unusable; do not interrupt or unplug during erasure, and do not attempt wipe if the battery/power supply is unstable.
- Format may fix the storage backend selection; it does NOT prove that an invalid project serializer or specific hardware flash fault has been corrected. If RC7 after wipe still fails saving a brand-new project on physical hardware, do NOT repeat wipes: capture the exact error, firmware identity and reproduction steps, then investigate read-only diagnostics.

# BEETLE BLUE V1.0.0 RC3 — FX SAVE Fix: validation

## Reproduced defect

A step effect with `TRACK_FX_VOLUME` and value `200` changes to an effect allowing <=100 while retaining the old 200 value. `track_store_encode` then returns `TRACK_JOURNAL_INVALID`, surfaced as `PROJECT DATA ERROR` by the UI. An empty project slot does not affect this validation.

## Patch

- `firmware/fm1-native/beetle_fx_save_safety.h` defines FX domain transitions and staged-only repair.
- `track_runtime.h` applies the new edit-time transition to FX TYPE, and repairs known-invalid FX values in a staging copy before serialization. Live song data, notes and effect IDs are not changed by this repair.
- `track_project_store.c` + `.h` return an informative category for other invalid data; no unknown data is force-repaired.
- BDR1 V31 and V2 flash allocation, A/B journaling and sample bank layouts unchanged.

## Software checks performed

- Native BLUE writable FM1 PIE32v2 compilation and manual link; independent application-image validation. App size 533,380 bytes, within the configured 581,564-byte app limit (48,184 bytes headroom).
- `bash tests/track/run_rc3_fx_save.sh`: FX1, NOTE2 FX, FX2/FX3 old-value migration, type-change bounds, no note loss and successful encode. UBSAN enabled.
- `bash tests/track/run_rc1010_storage_workspace.sh`
- `bash tests/track/run_rc1011_song_chains.sh`
- `python tests/storage_v2/test_native_v2_writer.py`
- `python tests/storage_v2/test_native_v2_reader.py`
- `.fwsc` container identity FM-1_2124, package checksum, exact app roundtrip, original first 16 KiB and OTA loader unchanged.
- WebInstaller embedded `.fwsc` byte-identical to standalone RC3 file, Manager embedded WebInstaller byte-identical to standalone RC3 WebInstaller.

## Not tested or guaranteed

- The particular unsaved project on the user's physical FM1 was not available, so other unrelated data errors cannot be ruled out.
- Physical NOR hardware save/reload, OTA installation, hardware recovery, USB audio and live-performance DMA timing were not tested.
- Read-only variant did not finish within the time budget; do not claim a read-only build passed.
- No guarantee of data recovery after reboot/flash with unsaved RAM state. Any firmware update risks loss and requires verified independent recovery.

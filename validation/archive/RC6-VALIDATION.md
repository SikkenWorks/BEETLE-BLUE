# BEETLE BLUE V1.0 RC6: legacy save-path correction and investigation

## Confirmed source defects

1. `track_journal_save()` treated BOTH codec errors `-1` (invalid project) and `-2` (buffer exhaustion) as `TRACK_JOURNAL_CAPACITY`. Thus a bad parameter was shown as `PROJECT TOO FULL` on the legacy TRKJ path. This was a real bug and is fixed. All rejected writes remain atomic and non-destructive.
2. RC5 compression only activates if flash carries a valid Storage V2 (`BTLSTV2`) marker. A device with an old TRKJ marker falls back to legacy A/B saves. The previous release did not explain that distinction on the error screen. RC6 explicitly identifies legacy versus V2 storage during save failures.
3. NEW cleared many individual controls but did not clear the full project staging object. RC6 resets it during the already destructive-to-RAM NEW operation before reassigning sample pointers. This eliminates stale serializer state leaking across new sessions.
4. The legacy firmware save path now separates invalid project (`PROJECT DATA ERROR`), serializer overflow (`PROJECT DATA TOO LARGE`), and a real 12,256-byte legacy bank overflow (`PROJECT PACK TOO LARGE`). This does NOT increase the finite legacy bank size nor change the on-flash format.

## Tests executed

- Native writable JieLi build with V2 support: PASS, app 535,888 bytes, remaining headroom 45,676 bytes.
- Native read-only diagnostic JieLi build plus forbidden-writes audit: PASS.
- Legacy writer host test: save slot 1, new save slot 2, overwrite and reload slot 2; slot 1 unchanged: PASS.
- Legacy invalid-data injection: correctly returns INVALID with no flash erase/program: PASS.
- Legacy oversized project: correctly returns CAPACITY with no flash erase/program: PASS.
- Existing V0.74 legacy preflight behavior tests (adapted to correct INVALID result): PASS.
- Native V2 writer tests: all 12 projects and 13 sample slots, CRC and power interruption after 71 mutations: PASS.
- Historical `run_v074.sh` recursively invokes an old V0.73 static-screen check which failed because current BEETLE has more screen IDs; this is unrelated to storage and has NOT been marked as passing.
- Actual FM1 save operation with the user's stored data: NOT TESTED / ROOT CAUSE STILL NOT CONFIRMED ON HARDWARE.

## Safety and remaining limitations

Do not flash/reset the device while the current project is unsaved. RC6 is an **experimental diagnostic-and-correction release**, not a claimed universal fix for every project. The user may still see a TRUE project size limit if the encoded project exceeds 12,256 bytes on legacy and 12,256 bytes of raw encoding for V2. Migration from legacy TRKJ to V2 must NOT be automatic without a verifiable full backup: it can erase existing songs and samples. Wipe All Storage is destructive. Physical flash, OTA recovery, audio/DMA stability and exact affected user-project contents remain unverified. No automatic storage format or migration is performed.

## How to interpret RC6 SAVE error

- STORAGE LEGACY / 12K + PROJECT DATA TOO LARGE or PROJECT PACK TOO LARGE: true legacy buffer/bank limit.
- STORAGE LEGACY / 12K + PROJECT DATA ERROR + category: failed project validation, NOT flash fullness.
- STORAGE V2 + FLASH SPACE LOW: V2 physical free/garbage-collection reserve limit.
- New project (~1.5KiB in test) saving to empty slot 2 succeeds in the host test; this is NOT proof it will succeed on a physical device with different RAM or flash state.

Release firmware SHA256: e5cba64d5c217de7774820f87dd392478b9f3e7d608e8cbfd60fc6a76bc546c6

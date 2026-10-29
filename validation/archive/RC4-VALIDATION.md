# BEETLE BLUE V1.0 RC4 — Storage recovery, Live Mode load and UI

## Confirmed source-level root causes

1. **NEW PROJECT did not initialize all serialized BLUE state.** The previous session's global sample trim START/END, per-sample ADSR, kit palette and MASTER FX configuration persisted into the new project. An invalid trim pair (START >= END) triggers the V31 serializer preflight and returns `PROJECT DATA ERROR / NO DATA WRITTEN` *before flash writes*. RC4 resets these to the documented defaults on NEW, matching the boot initialization.
2. **V2 Wipe All Storage never formatted.** The previous wipe function explicitly returned an error when the Storage V2 marker was present, so malformed project records were not cleared. RC4 adds an explicitly confirmed full BEETLE V2 format within 0x97000–0xDFFFF, validates erased pages, writes a new CRC-checked B2SB index and its ownership marker, then verifies the 12 empty project slots. The OTA/firmware flash region is outside that range. The ownership audit cache is refreshed after success.
3. **SAVE error category was ambiguous.** Invalid current project data remains distinct from a corrupt destination slot. Existing invalid step-FX repair from RC3 stays in place. Saving a corrupted project does not silently discard notes or normalize unknown parameters.

### IMPORTANT remaining unknown

**These findings do not prove they explain the exact corruption on a particular physical FM1.** We have no live RAM capture, project export, flash dump or hardware recording. A new or existing project may still fail for a separate invalid parameter, flash read/write hardware failure, insufficient capacity, or old firmware. RC4 is a software-tested experimental candidate, not a confirmed fix on any real FM1. **Never install firmware or wipe storage while holding an unsaved project in RAM.**

## New experience

- Setlist navigation and autosave remain unchanged; entering Perform shows a BLUE loading modal with a progress bar derived from completed V2 flash reads and full CRC/decode completion, not a time-based fake animation. Legacy flash may remain in a non-percentage loading state until completion.
- Tracker PAT footer is right aligned with the battery HUD; 240x240 layout needs hardware review.
- Existing songs, drums, synths, samples, MIDI and effects remain in source, no project-schema change (BDR1 V31).

## Validation

- Native writable JieLi firmware, linked APP payload and protected header/OTA roundtrip: PASS.
- Host RC3 FX save test: PASS.
- Native V2 writer project / sample coexistence, power-loss points, capacity refusal, orphan collection: PASS.
- V2 empty index parity test: PASS.
- Separate read-only build: not verified (test exceeded execution time).
- Physical FM1 audio, flash write/erase, battery charging, extended Live Mode tests and OTA recovery: NOT VERIFIED.
- No claim of exhaustive UI overlap or long-session glitch-free Perform operation.

### Hardware caution

Full Wipe is deliberately destructive: it deletes **ALL twelve projects, thirteen sample slots, Live Mode setlists and Storage V2 indices**. The two confirmation pages remain required. Flash marker damage or foreign ownership may be refused rather than erased to avoid destroying unrelated firmware data. Obtain a verified full backup and working recovery path first.

Firmware version: **V1.0 RC4 EXPERIMENTAL**, product `FM-1_2124`. Compiled app **533,888 bytes**, application headroom **47,676 bytes**. Firmware SHA-256 `99687dc9a84c5151dc3d41f45ccbdfab9b09fd970c2fc6d3004a2be0949c0d5a`.

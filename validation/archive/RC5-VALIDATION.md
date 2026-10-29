# BEETLE BLUE V1.0 RC5 — project capacity and drum kit routing

## Confirmed source-level root causes

1. **BDR1 V31 packed every populated step in 42 bits**, even with default instrument, FX, dynamics and groove. The V2 flash layer cannot overcome the 12,256-byte uncompressed codec scratch limit, even on an empty project slot. RC5 introduces **BDR1 V32**, a sparse, lossless, per-step encoding storing only fields differing from documented defaults. No project slot or flash partition layout change. Old V31/older project decoders are retained; modified projects are written in V32. Older firmware cannot read V32 project files: export backups before updating/downgrading.
2. **Drum Mixer kit selection updated the mixer browser, not a selected lane's active kit ID.** RC5 now applies mixer source selection to the currently selected channel; entering the mixer synchronizes the displayed kit with that channel. Direct stopped-transport audition routes use the same source-kit selection as running playback, instead of always playing original BEETLE PCM.
3. **BEETLE and MONOLITH native engines are different**: BEETLE ROM PCM samples versus the built-in MONOLITH procedural digital synthesis engine. On host, every one of 14 matching note IDs has a distinct PCM hash. This is not an ear-tested claim or hardware measurement.

## Capacity measurements and remaining physical limit

- New, empty test project: **1,502 bytes** V32 versus **1,514 bytes** V31 for the separate migration fixture (not the same session).
- 48 full patterns × 6 lanes × 16 note events using otherwise default step values: **10,718 bytes** raw V32, **1,544 bytes** BPK1 compressed. The decoded project equals the encoded input. This workload would exceed 12,256 bytes in V31. No events or settings were omitted.
- Explicit non-default volume FX at 200%, instrument, chord, dynamics and groove: V32 roundtrip test passes (one complete pattern with all such overrides).
- **Not an unlimited project format.** The project wire image is still limited to 12,256 bytes before BPK1 compression and one V2 project record. Complex fully populated projects with many non-default FX and/or locally embedded one-shots can still be too large. V32 fixes the demonstrated representational waste but cannot abolish finite hardware limits. A genuinely unlimited fix would require a larger, backward-incompatible streaming storage architecture plus physical verification.

## Tests performed

- Native writable JieLi/AC79 firmware compilation, app/linker audit: **PASS**, app 535,712 bytes, flash application headroom 45,852 bytes.
- Native no-write hardware diagnostic build and forbidden-symbol audit: **PASS**.
- V31 raw fixture generated using RC4 and read with RC5 then re-saved as V32: **PASS**, including BPM, notes, kits and 200% step volume.
- V32 dense 48-pattern encode/decode, V2 BPK1 roundtrip, non-default step settings, CRC rejection: **PASS** (UBSan).
- Native BEETLE/MONOLITH 14-component PCM comparisons: **PASS** (UBSan).
- V2 transactional storage writer: slot replacement, twelve projects + thirteen sample slots, power cut after 71 flash mutations: **PASS** (host simulation).
- V2 reserve-sector capacity rules: **PASS** (host simulation).
- Installer FWSC/APP/header/OTA byte checks: **PASS**.
- Actual FM1 NOR save/load, real kit audition over its DAC, heavy DSP/DMA stress, OTA recovery: **NOT TESTED**.

## Hardware and project safety

**Do NOT flash or reboot while an unsaved project is active.** A firmware update cannot recover data that only exists in RAM. Before testing, make a verified project backup and establish an independent recovery path. No automatic Wipe All Storage is performed. Wipe is destructive (twelve projects, user samples and Live Mode setlists). Experimental firmware is not proven suitable for critical performances.

Firmware SHA-256: `3fff1397d12b4ac552050c96fae6d68c91e929366df3ef2b2b2cae4e5c23e856`. Product: `FM-1_2124`. Storage ownership, bootloader and flash range are unchanged.

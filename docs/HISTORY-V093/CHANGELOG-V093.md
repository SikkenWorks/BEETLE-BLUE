# BEETLE BLUE Changelog

## V0.93 — MONOLITH built directly into firmware

- Integrated **all 16** supplied MONOLITH WAV drum sounds as compiled, read-only FM1 application ROM data. No external sample-bank upload or U01–U13 allocation is needed to play them.
- Implemented 8 kHz mono 4-bit IMA ADPCM sample packing (50,800 compressed bytes) with bounded per-voice on-demand decoding and PCM interpolation. Explicit audio quality/trim disclosure in README and generated manifest.
- Kit source `MONOLITH` now routes KIK..RIM to real embedded samples, with U01 / U02 serving secondary SNR2 and HHT2 while in MONOLITH mode. Other U03–U13 retain their independent user-bank handling. CH6 pair playback updated for embedded kit identifiers.
- Updated Drum Mixer to list MONOLITH's sixteen entries, not thirteen uploaded sample slots.
- Project format **BDR1 V30**, 48 patterns, 12 project slots and original BEETLE ROM kit are unchanged.
- Rebuilt firmware `.fwsc` with preserved OTA loader / 16 KiB boot header; verified native app round-trip.
- Rebuilt WebInstaller with V0.93 firmware hash, simulator with decoded embedded audio source buffers, Project Manager without MONOLITH upload requirement, and All-in-One Manager with byte-identical embeds.
- Generated UI renders from extracted native C drawing paths and 8× nearest neighbor scaling, including tracker and MONOLITH mixer.
- Host test: all 16 embedded sounds distinct and audible, 2000 kick retriggers and CH6 dual-sample pair pass. **Hardware DMA / audio and OTA safety remain unverified.**

## Older development

See `docs/BLUE-V0.92-CHANGELOG-ARCHIVE.md` for the previous release history. No previous firmware release is overwritten by these changes.

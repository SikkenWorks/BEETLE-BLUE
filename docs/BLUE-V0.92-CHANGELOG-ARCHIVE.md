## V0.92 — Correct MONOLITH WAV drum source (experimental)

- Fixed MONOLITH drum playback routing to prefer global U01–U13 WAVs and leave missing sample pads silent instead of falling back to misleading factory ROM sounds; CH6 trigger/restore logic corrected to use original note identifiers. RIM remains a documented ROM fallback due to the 13-slot U-bank limit.
- WebSimulator MONOLITH sound now uses original imported WAV audio, not browser-synthesized oscillator drums. Six lane kit assignments and existing BLUE features remain.
- Supplied a prebuilt, CRC-validated `BEETLE-BLUE-MONOLITH-DRUM-KIT.beetlebank` and preserved all sixteen original WAVs. FM1 upload remains an explicit, separate, potentially destructive choice.
- Added an actual source-extracted native tracker C function capture and regenerated 16 source-based UI PNGs at 1920×1920 using nearest-neighbour 8×.
- Updated installer embedded binary/hash and rebuilt All-in-One Manager with matching web tools.
- Build passes and existing V29→V30 project migration regression passes. **Physical FM1, hardware DMA audio safety and live USB transfer still not validated.**

# BEETLE BLUE Changelog

## V0.91 — Lane Kits, Kit-aware Drum Mixer, BLUE Webtools, V30 Project Format (experimental)

- Hold **SEQ** and turn **SELECT** to change the focused CH1–CH6 drum lane's kit source, using a temporary blue overlay. Kit sources 0–13: BEETLE ROM (default), MONOLITH, USER KIT 01–12.
- Every lane defaults to source 0 in new projects. Each lane has its own persisted `lane_kit` field in `BDR1 V30`. Explicitly overridden per-step `INST` cells continue to select the existing palette; default cells follow the selected lane kit.
- DRUM MIXER now uses SELECT to choose BEETLE ROM, MONOLITH or one of twelve global USER KIT sources. K1 selects individual sound, K2 parameter and K3 changes values. On USER KIT, SEL explicitly commits the global timbre bank with transport stopped; errors are reported.
- Fixed same-poll SEQ-down + SELECT detent collision: releasing SEQ after a kit change does not navigate away.
- Tested actual V29 project fixture migration to V30 and six-source round-trip, with invalid kit values and damaged CRC rejected.
- Added adapted BLUE-specific `WebInstaller.html`, `BEETLE-BLUE-Manager.html`, `BEETLE-BLUE-Project-Manager.html` and `simulator.html`. Original BEETLE files are retained under `reference/UPLOADED-BEETLE-WEBTOOLS/`. The installer embeds the **actual new firmware** with strict local hash checking and the inherited updater protocol.
- Rebuilt 15 source-extracted native C painter states including 4 kit-scoped Drum Mixer states. Exact 240×240 framebuffer to 1920×1920 nearest-neighbor export. Not every firmware screen or combined overlay state is captured.
- Disabled destructive volume migration and marker repair from BLUE Project Manager until global BBK1 kit backups are available. Added static source audit `docs/V0.91-SYNTH-AUDIT.md`.
- Native JieLi build and UFW/fwsc integrity checks passed. Web JS parser checks and Chromium no-device interaction smoke tests passed. **Physical FM1 flashing, DMA/audio safety, USB/OTA recovery are not validated.**
- Retained 48 patterns/12 project slots; no unsafe storage repartitioning. Global `BBK1` presets cannot yet be exported from the Project Manager.

## V0.9 — Interface and Audio Safeguards (experimental)

- Native LCD overlap corrections, blue header divider, drum-focused visible credits and blue application updater matrix.
- Skipped inherited synth waveform rendering in BLUE audio paths while retaining required code/data for compatibility.
- Reduced interrupt-disabled work when committing tracker edits and performed native host repeated kick trigger tests. **DMA DEADLINE not yet reproduced/cleared on physical FM1.**
- Added nine C-renderer-based 1920×1920 image captures. Read-only diagnostic and updater builds compiled.

## V0.8 — Kit Designer, Global Kit Bank & Sample Editing (experimental)

- Twelve global kit timbre preset slots and two source types, with eight editable project-local kit palette references.
- `BDR1 V29` project schema with per-user-sample START/END and ADSR, streamed sample seek and end fade.
- MONOLITH kit's sixteen source WAVs included; native U01–U13 library maps thirteen active sample slots.
- `BBK1` global kit footer with CRC and alternating generation commits; simulated interrupted-write tests performed.
- BLUE V0.8 original reference UI images were *design references*, not native LCD captures; replaced by native renderer captures beginning V0.9.

The preserved `docs/` and `reference/` directories contain upstream BEETLE history. The listed tests are offline software results, not proof of real FM1 hardware behavior.

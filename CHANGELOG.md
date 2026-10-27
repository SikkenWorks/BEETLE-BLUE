# BEETLE BLUE CHANGELOG

## V0.99 — Stereo overload protection and lightweight per-voice math (EXPERIMENTAL)

- Fixed a mix-headroom weakness in the BLUE six-drum-lane output: individually soft-limited lanes could still sum to a hard-clipped stereo peak. The final stereo sample now uses a **symmetric bounded soft knee** above +/-24,000, capped at +/-31,500, with unchanged values throughout the linear region. This is a deterministic software safeguard, not an analogue SPL or hardware-clip certification.
- Simplified the 73-value pitch lookup algebra to remove software-emulated 64-bit multiplication on every drum trigger / retune. Added an **exact equivalence test** to the old Q16 formula for every tuning value.
- Replaced two additional 64-bit per-sample multiplications with 32-bit fixed-point equivalents where input ranges prove the products fit; retained wide arithmetic for actual PCM sample interpolation where it is required.
- New host test runs **3,900 mixed BEETLE/MONOLITH triggers** across six lanes and **166,400 rendered audio frames**, with changing pan, simultaneous FX and a maximum stereo magnitude below 31,500; all V0.98 regression suites re-run successfully.
- Preserved BDR1 **V30**, 48 patterns, 12 project slots, six lane kit selections, BBK1 saved kits, all existing FX identifiers and the protected OTA loader. No change to non-audio control semantics, NOR storage layout, DMA memory buffers or USB configuration.
- Rebuilt actual JieLi image and matching WebInstaller; regenerated **17 native-C LCD captures at 1920x1920, 8x nearest-neighbor**. Manager embeds only Project Manager and WebInstaller; no WebSimulator.
- **RC blockers remain:** physical FM1 DMA DEADLINE behavior, analogue output peak measurements, physical flash/recovery, streamed user-sample REV/delay, USB stems, and BBK1 kit-bank backup/restore. V0.99 is experimental, not verified V1.0 RC.

## V0.98 — Drum trigger correctness and CPU cleanup (EXPERIMENTAL)

- Fixed hi-hat choke across **both** built-in drum kits: BEETLE PCM HHC/HHO and MONOLITH ROM closed/open/alternate hats now share the correct release interaction. Previously MONOLITH's voices used IDs 66/67/79, which were not recognized when checking *existing* voices for a choke.
- Fixed CH6 note-column assignment for MONOLITH: mapped procedural voice IDs 64–79 no longer masquerade as missing NOTE1 hits and inherit NOTE2 FX or STOP state. A small pure NOTE1/NOTE2 classifier uses the voice's actual source ID (MONOLITH) or logical component ID (BEETLE's randomized PCM variations).
- Fixed the MONOLITH **PITCH** step effect: changes the actual `kit_phase` oscillator increment rather than merely altering an unrelated PCM cursor or note duration. Implemented with a bounded modular 32-bit multiplication rather than a 64-bit multiply inside the audio callback.
- Reduced drum render work for inactive voices by skipping EQ, FX, ROM-pointer and trim preparation for unused polyphony slots.
- Added host regression for both-kit choke, CH6 NOTE1/NOTE2 effect routing, 4,096 rapid retriggers and procedural PITCH frequency. Re-ran existing PCM, MONOLITH, FX1–FX3, REV, trim, V29→V30 migration and PCM-initialization tests.
- Firmware remains BDR1 **V30**, with 48 patterns, 12 project slots, 6 drum lanes, 12 BBK1 User Kits and no WebSimulator. Project/Storage V2 layouts are unchanged. Updated two-tool WebInstaller/Manager and regenerated 17 native LCD renders using 8× nearest-neighbor scaling.
- **Hardware limitations:** no physical FM1 DMA timing, output-peak measurement, USB audio, flash/recovery or long-run flash journal verification has been performed. This is experimental and must not be advertised as V1.0 RC ready.

## V0.97 — Drum step-FX engine, FX cursor editing and realtime optimization (EXPERIMENTAL)

- Release packaging cleanup: removed the WebSimulator, its README button and all emulator UI/payloads from the All-in-One Manager. The Manager now contains only the Project Manager and WebInstaller. Updated the two-tool Manager build script. This is a web/documentation-only repack; the native V0.97 `.fwsc` is unchanged.

- Audited the six-lane drum audio callbacks: V0.96 only applied step VOLUME, PITCH and REV to many BLUE drum voices, leaving FILTER, DRIVE, DELAY, REVERB, CHORUS, VIB, ARP, CUT and TREMOLO absent from the drum DSP path.
- Introduced a fixed 3-slot transient per-voice FX state. FX1, FX2 and FX3 are dispatched in slot order, never allocated or saved during the audio interrupt. No V30 project-format change, no FX ID renumbering.
- Added native drum effect processing: FILTER as low-pass blend; DRIVE bounded saturation; PCM ROM short-tap DELAY, REVERB and CHORUS; oscillator/pitch LFO VIB and CHORUS; 3-tone ARP; gate-based CUT; amplitude TREMOLO; plus existing VOLUME, PITCH and REV. MONOLITH's procedural oscillator now responds to its own ARP/VIB/CHORUS modulation (separate from PCM cursor rate).
- Avoids heavy long-delay buffers or flash reads in the audio callback. Effects use bounded arithmetic, the existing PCM ROM data, a small 36-byte transient state and power-of-two LFO timing. The dry/no-FX path bypasses extra processing.
- Short FX release on a focused FX cell opens PER STEP FX for that selected step and lane. Repeated short FX while editing preserves editor focus; long FX still controls Punch-In. REV displays ON rather than a misleading numeric parameter.
- Host tests: 12 effects alter BEETLE and MONOLITH audio relative to dry; 3-FX combinations differ from their component effects; 2,500 overlapping retriggers; FX 12 storage codec and V30 compatibility retained. Application build and container tests in validation/V0.97-RESULTS.md.
- **Current limitations:** short ROM taps and MONOLITH tonal approximations are not equivalent to long buffered echo/convolution. Streaming Storage V2 samples have no backwards/PCM-history read for REV/DELAY/REVERB/CHORUS. Hardware FM1 DMA deadlines, real output peaks, and OTA recovery remain unverified, so V0.97 is **NOT** a V1.0 Release Candidate.

## V0.96 — Project-sample REV trim correction and CH6 consistency (EXPERIMENTAL)

- Corrected project-local µ-law sample START/END in the native drum renderer. Previously the output age was treated as an absolute source endpoint and REV could read the wrong interval. Both directions now address the same source-frame window.
- Applied the local sample window to all three CH6 hits (NOTE1, NOTE2 and NOTE3) as well as the other five drum lanes. The existing Storage V2 streaming trim request remains separate and forward-only.
- Added a bounded 32-frame release ramp to reversed PCM hits, limiting clicks where reversed audio ends on the original transient. Mono procedural REV continues to be reverse-style synthesis, not a time-reversed waveform.
- Used existing per-block voice metadata to pack local window start/span without enlarging the critical FM1 RAM pool or reducing DMA/USB buffers. Never allocates in the audio interrupt.
- Native JieLi build and readonly build PASS. Host tests: 14 BEETLE/16 MONOLITH REV variants, 3,000 repeated reverse triggers, local sample cropping and a one-frame sample, V30 REV step codec.
- Preserved existing 48 patterns, 12 project slots, 12 global User Kits, BDR1 V30 format, ROM engines, native mixers and protected update components.
- WebInstaller updated with SHA-pinned V0.96 firmware; Manager regenerated from the exact separate tools. Simulator remains an approximation and does not yet fully reproduce native per-step REV/trim.
- **Unverified:** physical FM1 DMA deadlines, real analogue peaks, USB six-stem timing, persistent NOR wear, OTA write/recovery. Not an RC-certified firmware.


## V0.95 — Reverse step FX implementation and 1.0 RC preparation (EXPERIMENTAL)

- Audited native step-FX definitions: V0.94 had no `REV` step effect; reverse key detection and Punch-In reverse were unrelated.
- Added FX code **12 = REV** to the existing 4-bit FX field; old effect codes 0–11 and BDR1 V30 binary layout are unchanged.
- Implemented literal reversed per-voice PCM indexing for the BEETLE factory ROM kit and project-local PCM one-shots, including NOTE2 and sparse step FX where supported.
- Implemented a bounded reverse-inspired rising envelope and descending oscillator phase for procedural MONOLITH. **This is not literal time-reversed PCM.**
- Guarded Storage V2's streaming BD4S path against pretending to reverse: it still only supports forward reads.
- Added native host tests covering BEETLE and MONOLITH output differences, 3,000 reverse retriggers and REV pattern serialization. Existing MONOLITH/BEETLE and PCM-clear contract tests remain passing.
- Retained project V30, 48 patterns, 12 project slots, 12 global kits, user samples, native mixer, USB routing and protected OTA components.
- Next RC blockers: FM1 real-hardware DMA and output-level validation; streamed sample reverse; synth/legacy dependency audit; simulator equivalence for per-step REV; backup of global user-kit presets.
- **Not a hardware-verified Release Candidate.**


## V0.94 — Audio PCM initialization fix and MONOLITH procedural ROM kit (EXPERIMENTAL)

- Fixed the missing `native_stereo` zero-initialization introduced when BLUE disabled synth render, preventing stale PCM from being re-added by the drum mixer every ALNK0 block.
- MONOLITH: replaced the built-in 50,800-byte, 8 kHz ADPCM bank with 16 fixed-patch 44.1 kHz ROM drum synthesis models. BEETLE original 14-pad PCM ROM remains unchanged.
- Optimized inactive-drum render, precomputed EQ coefficient gains per block, and reduced repeated wide-integer operations in the per-sample drum loop.
- Kept BDR1 V30 project storage, BBK1 global kits, 48 patterns, 12 project slots, user samples and six drum lanes.
- Refreshed WebSimulator using 16 previews rendered by the real C drum engine; WebInstaller packages exact rebuilt V0.94 `.fwsc`; all-in-one embeds matching tools.
- Added 16 source-derived 1920x1920 LCD renders including tracker, nearest-neighbor 8x.
- FM1 actual flashing, DMA deadlines, acoustic quality and output peak safety **NOT physically verified**.

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

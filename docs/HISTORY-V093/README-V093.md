# BEETLE BLUE V0.93

**SikkenWorks · M-VAVE FM1 · Experimental Drum Firmware**

A six-lane drum-only fork of BEETLE V1.7.14, with an electric-blue LCD theme, native tracker, programmable drum kits, project storage, global user timbre kits, performance FX, six-stem USB audio support and a native built-in MONOLITH drumkit.

## Key change: MONOLITH IS INCLUDED IN THE FIRMWARE

**No WebInstaller sample transfer, no `.beetlebank` import, no U01–U13 overwrite, and no Storage V2 sample upload is required to play MONOLITH.** Every V0.93 firmware build contains all 16 separate MONOLITH one-shots compiled into read-only application flash. Kit source 1 chooses the MONOLITH drum bank. The firmware's independent user-sample slots are preserved.

### MONOLITH routing

| MONOLITH component | Embedded original source |
|---|---|
| KIK | `kick.wav` |
| SNR | `snare_1.wav` |
| HHC | `hihat_1.wav` |
| HHO | `open_hihat.wav` |
| TM1 / TM2 / TM3 | `tom_1.wav` / `tom_2.wav` / `tom_3.wav` |
| CRS / RDE | `crash.wav` / `ride.wav` |
| CLP | `clap.wav` |
| PC1 / PC2 / PC3 | `perc_1.wav` / `perc_2.wav` / `perc_3.wav` |
| RIM | `rim.wav` |
| U01 while using MONOLITH | `snare_2.wav` (alternate snare) |
| U02 while using MONOLITH | `hihat_2.wav` (alternate hat) |

U03–U13 retain access to the existing global user sample bank; their data are not touched by MONOLITH. The separate BEETLE ROM factory drumkit also remains intact.

### Compression trade-off

The provided WAV files total about **3.18 MB** uncompressed and cannot fit unmodified in the existing fixed 581,564-byte FM1 application budget, which already contains BEETLE BLUE itself. V0.93 stores **50,800 bytes** of **8,000 Hz mono IMA ADPCM4**, derived from every WAV. Individual hits were shortened to approximately **0.30–1.50 seconds**, with a brief ending fade to avoid clicks. This is a lossy, lower-bandwidth drum-kit adaptation, **not lossless or original-rate playback**. The complete original 44.1 kHz stereo WAVs are retained in `assets/MONOLITH-Kit/`. The source archive contains a sample-by-sample provenance, duration and encoding manifest in `assets/MONOLITH-Kit/BEETLE-BLUE-V0.93-EMBED-MANIFEST.json`.

The native engine reads the compiled 4-bit ROM stream via a per-voice predictor and interpolates to the FM1 audio sample clock. It does **not** wait for the Storage V2 BD4S prefetcher when playing MONOLITH. The six-voice polyphony limit is unchanged. Host rendering checks pass, but **FM1 CPU/DMA latency and audio-peak safety are not yet hardware-validated**.

## FM1 controls

- Rotate **SELECT** in TRACKER to change the six drum lanes.
- While holding **SEQ**, rotate **SELECT** to select the kit for the current lane. A blue overlay shows which kit is selected; a new project defaults all six lanes to the built-in BEETLE ROM kit.
- Select kit `MONOLITH` to use the 16 embedded samples directly. No global sample bank connection is necessary.
- Press **GLO** to open lane mixer, then press it again for the **DRUM MIXER**; rotate SELECT to choose Default / MONOLITH / one of the 12 global user timbre kits. K1 selects a drum component, K2 a parameter, K3 edits and SEL saves global user kits when allowed.
- In the MONOLITH Drum Mixer, the 16 sample entries are available including SNR2 and HHT2. The second samples use the alternate sample triggers.
- The separate Kit Designer includes 12 global drum **timbre** presets, not 12 new audio WAV banks. Eight active kit references are available per project and individual steps can override the selected lane kit using INST.
- Press **FX** briefly and release to open the FX screen. Long press FX plus keyboard to trigger live Punch-In effects (last held effect shown in the overlay).

## Persistence & backward compatibility

- Project format remains **BDR1 V30**, storing six lane kit selections, patterns, mixer settings and drum parameters. V29 compatibility is retained.
- Twelve project slots and forty-eight patterns remain; safe expansion requires storage migration.
- The twelve global user-timbre kits remain stored as `BBK1` in Storage V2 metadata. Ordinary `.bdrum` files do **not** include that separate global kit bank, and the current Manager cannot export it. Back up or record these presets separately before downgrading.
- Global user-sample slots U01–U13 remain available for imported samples, independent of the embedded MONOLITH kit.
- No destructive volume migration, sample overwrite, or project format migration is performed by the embedded MONOLITH feature itself.

## Installation (experimental)

The supplied `.fwsc` is a freshly compiled native JieLi firmware package. Source-built app integrity, UFW container, protected 16 KiB header, preserved OTA loader, ROM data inclusion, and CRC project tests can be checked offline. **There has been no real FM1 flash / audio / recovery validation in this environment.** The previously reported rapid double-kick `DMA DEADLINE / AUDIO STOP` has **not been proven fixed** on real hardware. Never test with connected headphones or active loudspeakers, and only test flashing on a dedicated device after you have verified an independent recovery method and backed up projects, global user samples and important global timbre kits.

## Web tools

- `WebInstaller.html` — V0.93 `.fwsc` embedded and SHA-256 pinned; actual USB flashing unverified.
- `simulator.html` — the 16 browser audio buffers correspond to the **decoded embedded ADPCM samples**, not synth oscillator approximations. The simulator's audio engine and scheduling remain a WebAudio approximation, not native FM1 DSP.
- `BEETLE-BLUE-Project-Manager.html` — project and custom U01–U13 sample workflow, but MONOLITH installation is *not needed*.
- `BEETLE-BLUE-Manager.html` — all three tools byte-identical and embedded for offline distribution.

## Source, tests and UI captures

- `firmware/fm1-native/beetle_blue_monolith.{h,c}` contains the 16 embedded sample assets.
- `tests/test_blue_v093_monolith.c` checks distinct PCM outputs and fast-retrigger behavior on a host computer.
- `tools/package_beetle_blue_v093.py` reconstructs the `.fwsc` and checks header/loader/app round-trip.
- `UI-CAPTURES-NATIVE-1920/` includes selected screens, including the TRACKER captured from the C draw path, rasterized at 240×240 and enlarged **8× nearest-neighbor** to 1920×1920. These are source-extracted LCD drawing snapshots, not photos of the hardware.

Build from source:

```bash
export JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1
python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio
python tools/package_beetle_blue_v093.py
```

**Credits:** BEETLE BLUE is a BEETLE derivative by SikkenWorks. Source/license credits for Felucca and other retained third-party code must remain in `LICENSE`, `LICENSES/` and `docs/`. Original MONOLITH WAV redistribution rights should be verified before public GitHub release. GPL-3.0-only firmware lineage applies.

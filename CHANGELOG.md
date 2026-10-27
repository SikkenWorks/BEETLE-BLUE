# BEETLE BLUE Changelog

Version history for **BEETLE BLUE**, a drum-focused M-VAVE FM1 firmware derived from SikkenWorks BEETLE. **Newest verified source release first.** Release-specific technical changes belong here; controls, setup, backups and attributions are explained in [README.md](README.md).

> **Validation standard:** "PASS" below refers to the software tests documented in the supplied archive. The physical FM1, audio safety, USB-C capture, NOR endurance and OTA/recovery paths **have not been verified** for BLUE V0.8.

## V0.8 — Kit Designer, Global Kit Bank & Sample Editing (experimental)

### Kit architecture

- Added a native **KIT DESIGNER** screen with **eight active kit references** (`INST 1–8`) stored in each project. Each primary drum step selects one of the eight references through its existing 3-bit INST field.
- Added **14 selectable kit-source IDs**: `0` BEETLE ROM, `1` MONOLITH sample routing, and `2–13` representing **12 global user-saved drum timbre kits**.
- Each global user kit stores eight parameters for each of **14 factory ROM drum components**: attack, decay, sustain, release, gain, pitch, drive and choke. These are **preset settings, not independent WAV sample banks**.
- In KIT DESIGNER, K1 chooses INST 1–8, K3 chooses a source, and SEL stores the current component settings into a selected user-kit source when audio is stopped and safe V2 ownership is established.
- Existing **12 factory timbre variation kits** remain accessible with K4 in DRUM EDIT. They are distinct from the 12 global user-kit sources.

### Persistent global kits and flash safeguards

- Added a 1,354-byte **`BBK1`** global kit-bank footer in the **existing Storage V2 B2SB metadata sector**, with CRC validation and alternating generation commits. No new flash partition or automatic destructive migration was added.
- Global writes require a verified V2 owner/index and a stopped, healthy audio engine. The native code suspends audio DMA for the metadata write and refuses unsafe or unowned writes.
- Added host tests for kit CRC encoding/decoding, invalid-record rejection and journal preservation across **17 deliberately interrupted global-kit writes**. Simulated project data remained recoverable.
- **Not yet backed up by the Project Manager:** `.bdrum` files and `.beetlebank` libraries do not capture the separate `BBK1` global preset footer. Older firmware or a destructive reformat may discard these kits.

### User samples and project format

- Extended drum-user sample editing with **START**, **END**, **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE**, while retaining gain, drive and choke settings.
- Added start/end positions for the streamed **BD4S** path, including block-aligned background prefetch, exact trim-start handling and bounded anti-click end fading. Project-local µ-law sample sources also support trim.
- Added **BDR1 project schema V29** for eight active palette source IDs and per-sample trim/envelope settings. Decoding includes migration defaults for BLUE V26–V28 project data; separate V28 historical fixture regression coverage is not yet provided.
- Preserved the **13 global U01–U13 sample-slot limit**. The project sample bank and global kit-preset bank remain separate.
- The supplied V0.8 Project Manager understands V29 projects and the existing global sample workflow; it **does not** yet export/import `BBK1` kit presets.

### MONOLITH sample assets

- Included **all 16 supplied MONOLITH WAV source files** in `assets/MONOLITH-Kit/`.
- Documented the 13-slot native default mapping in `assets/MONOLITH-Kit/MONOLITH-FM1-SLOT-MAP.json`. `rim.wav`, `snare_2.wav` and `hihat_2.wav` remain optional replacement assets; the default RIM sound falls back to ROM.
- MONOLITH WAVs are **not embedded or automatically transferred by firmware flashing**; users must import the desired WAVs through the global sample workflow.
- Permission to redistribute the bundled WAVs publicly has **not been independently established**.

### UI, release files and software verification

- Added Kit Designer and drum editing UI pages while retaining the drum mixer, tracker, project tools, Punch-In FX and inherited performance flow.
- Provided **42 native-page reference artworks and two boot frames**, generated from 240 × 240 designs at exactly 8× nearest-neighbor scaling (1920 × 1920). They are **reference images, not proven pixel-identical LCD screenshots**.
- Native JieLi Storage V2 / updater / USB-audio build reported **PASS**: application **523,420 bytes**, **58,144 bytes** of designated app-space headroom; `.fwsc` length **610,086 bytes**.
- The packaged `.fwsc` was audited for exact application payload, preserved protected 16 KiB header and preserved OTA loader. This is **firmware-container integrity, not hardware flashing certification**.
- Host tests reported PASS for BBK1 storage round-trips and failed writes; existing NOR sample-writer suite (including 71 interrupted sample transactions); V29 project serialization; V26/V27 migration; and BD4S sample trim frame boundaries.
- Five Project Manager JavaScript code blocks passed parser checks. Browser USB operation and real FM1 transfers were **not** tested.
- See [`validation/V0.8-RESULTS.md`](validation/V0.8-RESULTS.md) for the original result list, artifact hashes and caveats.

### Known limitations and unverified behavior

- **No physical FM1 validation:** DSP/audio peak safety, live playing, flash recovery/endurance, safe real NOR commits, USB host enumeration and OTA recovery remain unproven.
- **Primary drum-hit only:** the new kit palette is not independently applied to CH6 NOTE2/NOTE3 subhits.
- **No arbitrary per-pad WAV routing in a saved USER KIT:** global user-kit slots currently describe factory-ROM timbres. MONOLITH is a distinct fixed source mapping, limited to the current 13 sample slots.
- **No full V0.8 WebSimulator or certified WebInstaller** in the source archive. `web/BEETLE-DRUM-V0.2-WebPrototype.html` is an older prototype.
- **Global user kits cannot be exported** with the current Project Manager, so downgrading or reformatting may discard them even when `.bdrum` projects are backed up.
- **Source-package metadata cleanup still required:** the supplied original ZIP contains a stale `BUILD-MANIFEST.json` marked V0.7, an outdated `STATUS.md` describing features as unfinished, and an inherited on-device HELP version line showing V0.7. Refer to the actual V0.8 native source, firmware hashes and `validation/V0.8-RESULTS.md` rather than treating those stale files as V0.8 release validation. The firmware/HELP binaries were **not changed** by this documentation update.

### Distributed V0.8 firmware identifiers

| Artifact | Value |
| :-- | :-- |
| Native app SHA-256 | `87c6e4ce17c1b92af8a6185296c053cce7e22ab7c9fe0f8a8ed75992dc1c428d` |
| `.fwsc` SHA-256 | `dd1d38f122a8d8f97bce16a5e5abcbbfb77b2c1015058b7117fb09827e3255aa` |
| Project magic / write version | `BDR1` / V29 |
| Global kit metadata magic | `BBK1` |
| Hardware status | Experimental, not physically validated |

## Earlier BEETLE BLUE Development

The V0.8 archive includes prior BLUE drum-engine development sources and regression scripts (including V0.3–V0.6 test stages), together with the upstream BEETLE V1.7.14 documentation. **It does not include a complete, independently verified per-release BLUE V0.1–V0.7 changelog**, so specific earlier release claims have not been reconstructed or invented here.

The earlier code base already contains the six-drum-lane concept, factory drum sounds and timbre kits, user-sample support, Song Chains, lane mixing and performance features that V0.8 builds upon. For upstream BEETLE history, see [`docs/UPSTREAM-CHANGELOG.md`](docs/UPSTREAM-CHANGELOG.md); those entries are **not** BEETLE BLUE release notes.

---

**BEETLE BLUE · SikkenWorks · Born out of passion.**

# BEETLE BLUE V1.3 — Release Status

- **Firmware:** V1.3 complete `.fwsc` with native on-device version and matching WebInstaller.
- **Identity:** `FM-1_2224` / `AC791N`, unchanged from V1.2.
- **Storage / project format:** Storage V2 and `BDR1 V33`, unchanged; no format or data migration.
- **Official stock return:** existing OTA policy accepts the verified M-VAVE V15 package using its pinned SHA-256 and vendor loader metadata. USB OTA retains priority over unfinished project/sample transfers and a bounded 15-minute session. No permissive bypass for unverified firmware.
- **Bootloader:** immutable OTA-loader and first 16 KiB flash boot bytes from V1.2 remain preserved.
- **Hardware testing:** The developer reports extensive BEETLE BLUE testing on two physical FM1 units. A physical flash and stock V15 rollback of this exact V1.3 build is not independently recorded.
- **Docs:** README is the current operating manual; version history remains in CHANGELOG.
- **Release checks:** see `validation/V1.3-VALIDATION.md` and `FIRMWARE-MANIFEST.json`.

**V1.3 firmware SHA-256:** `c699bcf0685815d60692540d04033fad7732c7e4a001b646756cddd536a00102`  
**V1.3 native app SHA-256:** `9a0c292d76644288778aef052d5ebf88d17d14de1eb7a853f0e418b32508d1f8`

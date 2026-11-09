# BEETLE BLUE V1.3.1 — Release Status

- **Firmware:** V1.3.1 complete `.fwsc` with native on-device version and matching WebInstaller.
- **Identity:** `FM-1_2224` / `AC791N`, unchanged from V1.2.
- **Storage / project format:** Storage V2 and `BDR1 V33`, unchanged; no format or data migration.
- **Official stock return:** existing OTA policy accepts the verified M-VAVE V15 package using its pinned SHA-256 and vendor loader metadata. USB OTA retains priority over unfinished project/sample transfers and a bounded 15-minute session. No permissive bypass for unverified firmware.
- **Bootloader:** the first 16 KiB flash boot bytes remain preserved. The bundled RAM OTA-loader is rebuilt from Felucca GPL source with the two-vendor JEDEC write whitelist; this changes `ota.bin` intentionally, without changing the SPL, stock-V15 source policy or storage layout.
- **Hardware testing:** The developer reports extensive BEETLE BLUE testing on two physical FM1 units. A physical flash and stock V15 rollback of this exact V1.3.1 build is not independently recorded.
- **Flash-chip support:** Both Puya `0x856014` and UC25HQ80 `0xB36014` explicitly accepted before program/erase in the app and the bundled OTA loader. Unknown IDs refused.
- **New-chip test status:** Tested in software only; no physical UC25HQ80 FM1 available here.
- **Docs:** README is the current operating manual; version history remains in CHANGELOG.
- **Release checks:** see `validation/V1.3.1-VALIDATION.md` and `FIRMWARE-MANIFEST.json`.

**V1.3.1 firmware SHA-256:** `78de3d0ddd47ffdf98795b4a75c033c33ad96c5c8431cb7e8225e98580f3455d`  
**V1.3.1 native app SHA-256:** `5521434cf9a7b40c09e9abee986bb12dc53197bef7815dc13ea34d2817a4e373`

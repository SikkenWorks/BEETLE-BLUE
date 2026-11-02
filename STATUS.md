# BEETLE BLUE V1.1 — Release Status

- **Release:** BEETLE BLUE **V1.1**. Full `.fwsc` firmware package with the native V1.1 application, `flash.bin` and `ota.bin`; not a LAB-only build.
- **Hardware testing:** The developer reports extensive testing of BEETLE BLUE on **two physical M-VAVE FM1 units**. The exact newly rebranded V1.1 binary has not been separately recorded in an on-device OTA/recovery validation log in this archive.
- **Package identity:** `FM-1_2124` on `AC791N`, unchanged from the preceding BLUE release. Project format `BDR1 V33`, Storage V2 partition and updater remain unchanged.
- **Firmware checks:** Package CRC/layout, decryption, native application audit, app fit, bootheader preservation and byte-for-byte OTA-loader parity have passed offline validation.
- **On-device HELP:** Ten topics, 66 short cards, three per screen maximum; K2 topic, K1 page. HELP displays V1.1.
- **Project features:** Six independent drum lanes, 1–16 steps per lane, 48 patterns, Song Mode and Live Mode. No project migration or storage wipe required solely for this renaming.
- **Web tools:** `WebInstaller.html`, `BEETLE-BLUE-Project-Manager.html` and `BEETLE-BLUE-Manager.html` are pinned to the exact V1.1 firmware where applicable.
- **Bootloader:** Inherited `uboot.boot` CRC discrepancy is documented. Header and OTA bytes are deliberately left unchanged, not silently rewritten.
- **Release hygiene:** README is the current operating manual only. Historical versions and bug fixes are documented in CHANGELOG and the archived source history.

**V1.1 `.fwsc` SHA-256:** `b96f70294c44ed5f7b44e0cc66dc35319c9de9744222ea92f48b5b94129b3a47`  
**V1.1 native app SHA-256:** `80a9be06d8a88d38fc8816853419cb3aa2eecf5d9711babe8d1e5da0b8cdf833`

Before installation, back up projects, user sample banks and User Kit settings; keep a verified recovery path available. Offline package validation does not substitute for a device-specific flash/recovery test.

# BEETLE BLUE RC11 — HELP source and README validation

**Source revision:** operator guide + FM1 on-device HELP only; pattern/audio/storage behavior unchanged. The current flashable RC11 WebInstaller still embeds the original firmware and therefore still displays the old HELP.

- Writable native JieLi build: **PASS**, application 538,712 bytes; SHA256 `3314b8b1a7dd3cabb13fc6fa6cd91e1497787cb7a763595f1a89bc3155d85484`.
- Read-only native JieLi build and write-symbol audit: **PASS**.
- Device HELP: 1–16 steps, six lengths per pattern, Live Mode, BDR1 V33 and compatibility guidance: **updated in source**, fixed-width tests passed.
- Lane-length V33 codec and old RC10 V32 fixture: **PASS**.
- Reference OTA/boot/header unchanged in verified lab image: **PASS**. SHA256 `f0dbf13b1947225ae1866a7f523fe8d8e876b22227e8eb5588703c227dbeabb0`.
- Flashable release packaging blocked: **NO RECOVERY PROOF supplied** for the project's R0 gate. The lab image is not a flashable release.
- Physical FM1 testing and loading of this revised HELP on-device: **NOT DONE**.

The original RC11 production WebInstaller binary SHA256 is `ed2ff602cd4b5ec09240df4b9da794c6414d18f006fe63884455ab0d95e35332`. That binary was not rewritten, repackaged or misrepresented as containing the updated HELP.

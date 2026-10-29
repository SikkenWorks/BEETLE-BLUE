# BEETLE BLUE V1.0 RC10 | Credits-only attribution update

This release changes the on-device CREDITS screen, source annotations and user-facing attribution in the README, source credit documents, Project Manager, WebInstaller and All-in-One Manager. It preserves RC9's audio, battery calculations, project save and Live Mode behavior.

- **Original Felucca battery thresholds:** `firmware/src/ui_draw.c`, `batt_level()` (531/561/591), copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments, GPL-3.0-only. Verified directly against supplied `Felucca-main.zip`.
- **BLUE additions:** four-bar pixel UI, smoothing, invalid-read handling and foreground updates, attributed to SikkenWorks. No charging-current or exact battery-percent claim.
- **Attribution:** Joppe Sikken / SikkenWorks for BEETLE and BLUE; ETCH lineage; Felucca FM1 HAL, audio, display, battery ADC and OTA upstream; JieLi as platform/toolchain vendor. Technical references reSID (Dag Lem) and Nes_Snd_Emu (Shay Green / James Athey) retained as references in source credits, *not* falsely claimed to run in BLUE.
- **Copyright:** all existing GPL/MIT/Apache/OFL files and upstream notices retained. Documentation lists additional Felucca upstream project notices without implying every third-party component ships with BLUE.
- **Native writable build:** PASS, application 536,176 bytes, headroom 45,388 bytes.
- **Native read-only build and forbidden-write-symbol audit:** PASS.
- **WebInstaller/Manager embedded firmware integrity:** PASS; exact SHA-256 and byte equality.
- **JavaScript syntax:** PASS across current WebInstaller, Project Manager and Manager.
- **Source comparison against RC9:** core audio, storage and kit implementations unchanged.
- **Native on-device credits renderer:** PASS, 26 1920x1920 nearest-neighbor source-driven captures.
- **Firmware container verification:** original protected first 16 KiB unchanged; OTA loader unchanged; decrypted application matches native build; package product FM-1_2124.
- **RC10 SHA-256:** `d74d2f1f799a7ce2b75cec18f3e5a395fd45418b25317ca44ca0479922cadd03`.
- **Physical FM1 test:** not performed. OTA recovery, true battery charge measurement and pop-free audio remain unverified on hardware.

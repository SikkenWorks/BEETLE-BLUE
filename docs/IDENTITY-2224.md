# BEETLE BLUE V1.3 — Device identity

BEETLE BLUE V1.3 keeps the running USB MIDI identity and the `.fwsc` package identity **`FM-1_2224`** from V1.2. This update is a version change, **not** another identity change. The 2026-11-03 Hub availability check was made for the earlier identity change; it is not an ongoing reservation.

The OTA loader, protected 16 KiB boot region, stock-V15 whitelist, USB OTA preemption, 15-minute OTA session budget, audio, Storage V2 and `BDR1 V33` project format are unchanged from the V1.2 ID2224 baseline. See `tests/test_v13_ota_integrity.py` and `validation/V1.3-VALIDATION.md` for current offline comparisons. Exact hardware flash/rollback still requires its own verification.

Build with `firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio` and package with `tools/build_blue_v13_package.py reference/packaging-base.fwsc <app.bin> BEETLE-BLUE-V1.3.fwsc --acknowledge-hardware-risk`. The immutable reference has the earlier `FM-1_2124` identity by design; current output must be `FM-1_2224`. Re-embed the resulting firmware in WebInstaller and Manager and update checksums and manifest.

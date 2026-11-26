# Rebuilding BEETLE BLUE V1.4

Use the compatible JieLi Linux toolchain, Python 3 and included GPL sources. Run the commands in the README from the repository root. The packaging reference `reference/v1.3-reference.fwsc` preserves the protected first 16 KiB. The `reference/v1.3.1-reference.fwsc` is a regression fixture, not a release to distribute.

`tools/build_blue_uc25_loader.py` rebuilds the RAM loader. `tools/package_blue_uc25.py` validates both the application and the loader, preserves the boot bytes, exports the current native binaries, and verifies that the current loader's body digest is present in the application staging policy. A different loader requires an explicit staging digest and installer allowlist update before release. `tools/sync_blue_v14_web.py` updates all webtool copies and the embedded package. Run the regression tests, the read-only build and the release-hygiene check before distribution. The optional stock V15 fixture is checked by `tests/test_stock_v15_exit.py /path/to/FM-1_015.fwsc`. The reference V15 SHA is recorded in the test; do not substitute V16.

These checks do not substitute for physical OTA and recovery testing on each flash/bootloader variant.

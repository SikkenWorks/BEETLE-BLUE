# Rebuilding BEETLE BLUE V1.3

Prerequisites: compatible JieLi Linux toolchain in `JIELI_TOOLCHAIN`, Python 3, and the included sources. The packaged reference `reference/packaging-base.fwsc` is historical RC12 and is included only to reproduce the established protected bootloader and OTA bytes. It is **not** the V1.2 firmware to distribute.

```bash
export JIELI_TOOLCHAIN=/path/to/jieli-linux-toolchains-20260730.1
python firmware/fm1-native/build.py --drum-edition --beetle-storage-lab --beetle-v2 --updater --usb-audio
python tools/build_blue_v13_package.py reference/packaging-base.fwsc build/track-fm1-native-beetle-storage-lab-updater-uac/app.bin BEETLE-BLUE-V1.3.fwsc --acknowledge-hardware-risk
python tests/test_v11_release.py
python tools/check_blue_release_hygiene.py
```

The package builder creates a `.fwsc` and a separate V1.3 manifest but does NOT automatically update WebInstaller, Manager, root manifest and checksum list. These are provided pre-generated and validated in this release; if you rebuild a modified binary, regenerate those embedded packages and manifests before redistributing. Do not rename a `.lab` file to `.fwsc`. Formal independent recovery certification remains separate from software package validation.

The source archive preserves previous release history inside `archive/SOURCE-AND-VALIDATION-HISTORY.zip`; its older version names are historical and must not be globally rewritten.

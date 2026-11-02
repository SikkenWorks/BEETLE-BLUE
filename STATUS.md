# BEETLE BLUE V1.2 — Release Status

- **Firmware:** V1.2 complete FM1 `.fwsc` package, built from current source.
- **Identity:** `FM-1_2124` on `AC791N`, unchanged from V1.1.
- **Storage / format:** Storage V2 and `BDR1 V33` unchanged; no wipe or migration for this update.
- **Official return path:** USB-MIDI OTA accepts **the verified M-VAVE stock V15 package** using its pinned flash SHA-256 and original loader metadata, as required by The Firmware Hub's **Back to official V15** workflow. A firmware request now preempts incomplete project or sample transfers, which previously could block it.
- **Long transfers:** global session budget extended to 15 minutes; individual read retries remain bounded and aborts are supported.
- **Stock V16:** not implicitly supported. Different stock versions must be validated separately rather than allowing arbitrary loaders.
- **Hardware:** Developer reports extensive BEETLE BLUE testing on two M-VAVE FM1 devices. No independent log of the *exact V1.2 -> official V15 via Hub* transition is included.
- **Package checks:** Native writable and read-only builds, app memory limit, firmware CRC/identity, bootheader/OTA preservation, and nested webtool integrity passed software tests.
- **Safety:** No bootloader/OTA bytes changed; don't flash without a verified backup, adequate power and working rescue route.
- **Docs:** README is only the current user guide. Version history belongs to CHANGELOG.

**V1.2 firmware SHA-256:** `a70b43f9a3dc1a0ed70fcb04c61ef82ed807294e8156e170217a6a9fcd754a74`
**V1.2 app SHA-256:** `bc7aa07ffd0598a4e259381b7cc6534fd5651e9559002bba7e7d090776bd598d`

# BEETLE BLUE V1.4 — Release Status

- **Firmware:** complete V1.4 `.fwsc`, with matching WebInstaller and Manager.
- **Identity:** `FM-1_2224` / `AC791N`; Storage V2 and `BDR1 V33` unchanged.
- **Flash support:** Puya P25Q80 `0x856014` and UCUN UC25HQ80 `0xB36014` in app and RAM loader; single-evaluation JEDEC detection with success-only caching.
- **OTA:** known staged loader residues from V1.3, V1.3.1 and V1.4 are recognized; existing stock-V15 policy retained, with complete loader-body CRC preflight. Unknown staging contents and unsupported loader layouts are not blindly erased or accepted. V16 is not a V15-equivalent stock target.
- **Boot:** protected first 16 KiB unchanged; rebuilt RAM update loader supplied with source. Existing projects and audio functions retained.
- **Hardware:** BEETLE BLUE has been extensively tested by its developer on two FM1 units. The exact V1.4 physical Puya/UCUN OTA and stock return matrix is still open. See the executed software replay evidence in `validation/V1.4-VALIDATION.md`.
- **Recovery:** if the currently installed V1.3.1 already blocks every OTA attempt, use the independent verified FM1 recovery route after backing up flash. The incoming package cannot fix its outgoing updater before it runs.
- **Documentation:** README is the current user guide, CHANGELOG the release history.

Firmware SHA-256: `59399977c4a2d21771d1940820004c9010fa0c4d19005759e821e2965d453d74`.

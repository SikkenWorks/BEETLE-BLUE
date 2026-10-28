BEETLE BLUE V1.0 RC2 — EXPERIMENTAL STORAGE FIX

- Software-tested flash capacity diagnostics and orphan recovery retry.
- Storage schema BDR1 V31 and V2 flash partition are unchanged.
- A truly oversized project (12 KiB serialized) remains unsupported; do not delete
  other projects to try to fit it. Backup first using existing Project Manager.
- Reproducible on-device failure needs a size/flash diagnosis; no physical FM1 tested.
- DMA DEADLINE/AUDIO STOP, OTA/recovery, USB audio remain unverified.

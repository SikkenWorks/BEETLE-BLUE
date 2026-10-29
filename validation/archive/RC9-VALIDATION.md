# BEETLE BLUE V1.0 RC9 | Firmware and software validation

## Source and scope

Based directly on the user-uploaded `BEETLE-BLUE-RC8-COMPLETE(1).zip`. RC8's verified storage fix, V2 project handling, setlists, audio system, native UI and credit notices are retained. New work is restricted to the battery voltage indication, Live Mode warm audio transitions, an end-of-song tail clear, and User Kit activation after successful store.

## Confirmed changes

1. Battery: the previous raw 550 / 615 / 685 thresholds overestimated the battery level. Replaced by upstream Felucca FM1 531 / 561 / 591 ADC cutoffs, with display levels 0/1/3/4 and 4-second IIR polling; failed/stuck ADC conversions clear the HUD after three attempts. USB enumeration remains a *connection indicator only*, not a charging-status measurement.
2. Live Perform: `blue_live_load_position()` no longer routinely calls `native_pause()` / `native_start()`, which power-cycle PC6 and reset DMA. `native_live_quiesce()` waits for a 256-sample ramp to digital zero plus three DMA blocks for the old audio to drain. The renderer then stays suspended while the existing verified project loader runs. `native_live_resume()` resets voices without switching off PC6 and begins a standard fade-in. The safe-state fallback still uses the old cold restart. Setlist autosave uses the warm route as well.
3. End-of-song: a Live Mode branch that exited mid-buffer without clearing the remaining stereo samples now zero-fills the untouched tail (including any stem capture space).
4. Global User Kit: on successful BBK1 write in Kit Designer the current lane is assigned to the saved kit, and the project is marked dirty so saving it persists the new lane reference. Audio already loads per-pad sound settings from the global BBK1 bank when the lane's source is a User Kit.

## Results

- Native JieLi OTA/storage/V2/USB-audio app build: **PASS**, 536,164 bytes, 45,400 bytes application-space headroom.
- Native read-only build and linked forbidden flash writer symbol audit: **PASS**, rebuilt after all final source changes; linked forbidden-write-symbol audit passed.
- Live Setlist C test (insert/reorder/CRC/end of set): **PASS**.
- Live Mode source contracts / ISR-to-foreground project load: **PASS**.
- Global User Kit BBK1 encode/decode, 12 slots, all 14 pad structures, CRC atomicity: **PASS**.
- FM1 battery voltage band boundaries, filtering and invalid ADC: **PASS**.
- RC8 project SAVE wrapper on Legacy + V2, 12 slots, exact readback and invalid refusal: **PASS**.
- V2 setlist A/B metadata, torn write recovery: **PASS**.
- Firmware payload verified, same protected 16 KiB header and OTA loader as RC8, native app decrypted byte-exact: **PASS**.
- Embedded WebInstaller firmware hash / all-in-one Manager installer source: **PASS**.
- WebInstaller and Manager JavaScript blocks (`node --check`): **PASS**.
- Physical FM1 analog pop/mute test, battery SOC accuracy, wall-charger detection, realtime USB six-channel continuity: **NOT TESTED**.

## Safety

No physical FM1 was connected. Never equate a software fade or a verified OTA file structure with proof of pop-free analog output. The PB1 reading is cell voltage, not a calibrated battery percentage, and with external power attached it may not reflect remaining capacity. Preserve a verified backup and working independent recovery path before experimental firmware installation. Flashing will lose any unsaved project in RAM.

**SHA-256 firmware:** `89bf44eb8f3da622c6423d464e0914e54291b5ab3b4540058fdc4ff2c1d83e83`
**SHA-256 app:** `ed5842f4ab922618296c18c32edfdf55326db65c7d70539fe9cdf9c5d79f6dbe`

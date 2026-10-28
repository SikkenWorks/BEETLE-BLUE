# BEETLE BLUE V1.0.0 RC2 — project save validation

## Reported failure
`PROJECT TOO FULL` while trying to save in slot 2 when only slot 1 is occupied.

## Root cause and boundaries
RC1 collapsed three independent conditions into one message: serialized BDR1 project exceeds the fixed 12,256-byte RAM buffer; packed BPK1 V2 record exceeds 12,272 bytes; or safe V2 page allocation fails (including fragmented/dirty orphan sectors, legitimate global sample data, and A/B recovery reserve). An empty slot alone proves none of these capacities. Without a diagnostic dump of this FM1, the exact triggered condition is unknown.

## Corrections
1. Dedicated reasons distinguish BDR1 data overflow, compressed record overflow, and physical flash shortage. Invalid data reports DATA ERROR, never SIZE ERROR.
2. When physical V2 write rejects capacity before mutation, perform `beetle_v2_collect`, which first reloads and validates both indices and their reference union. Only fully unreferenced, dirty sectors can be reclaimed. Once, if any were reclaimed, retry the identical packed data. No formatting, no changes to another slot, no stealing sample sectors.
3. Preserve BDR1 V31, 12-KiB bank and protected firmware header/OTA loader. Files over the serializer limit are still rejected without mutation.

## Software tests
- Native JieLi writable build with V2 + USB audio passed (app header/audit).
- Native read-only build and forbidden-symbol check passed.
- `python tests/storage_v2/test_native_v2_writer.py`: 12 projects, 13 sample slots, sample transaction, CRC, power cuts, orphan cleanup and repeat GC passes.
- `python tests/storage_v2/test_native_v2_reader.py`: 12 projects, 13 sample roundtrips and recovery passes.
- `python tests/storage_v2/test_project_reserve_1425.py`: project reserve preserved.
- `bash tests/track/run_rc1010_storage_workspace.sh`: complete.
- `bash tests/track/run_rc1011_song_chains.sh`: complete.

## Not proven
Actual FM1 slot-2 failure resolution; physical flash integrity/recovery; 100% DMA/audio reliability; battery charge state; OTA restore; full legacy test suite. **Do not format or overwrite slot 1.**

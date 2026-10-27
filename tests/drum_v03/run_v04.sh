#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
bash tests/drum_v03/run_host_tests.sh
cc -std=c11 -O2 -fsanitize=undefined -fno-omit-frame-pointer -DBEETLE_DRUM_EDITION=1 \
 -Ifirmware/fm1-native tests/drum_v03/test_v04_storage_identity.c \
 firmware/fm1-native/{track_core,track_song,track_mixer,track_chip,track_drum,beetle_drum_rom,track_project_store}.c \
 -o /tmp/beetle-drum-v04-project -lm
UBSAN_OPTIONS=halt_on_error=1 /tmp/beetle-drum-v04-project
cc -std=c11 -O2 -fsanitize=undefined -fno-omit-frame-pointer -DBEETLE_DRUM_EDITION=1 \
 -Ifirmware/fm1-native tests/drum_v03/test_v04_native_stems.c \
 firmware/fm1-native/{track_core,track_song,track_mixer,track_chip,track_drum,beetle_drum_rom,track_project_store}.c \
 -o /tmp/beetle-drum-v04-stems -lm
UBSAN_OPTIONS=halt_on_error=1 /tmp/beetle-drum-v04-stems
python3 tests/storage_v2/test_native_v2_reader.py
python3 tests/storage_v2/test_native_v2_writer.py
python3 tests/storage_v2/test_v16_delete_one_slot.py

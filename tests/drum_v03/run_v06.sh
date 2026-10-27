#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
bash tests/drum_v03/run_v05.sh
C_SRC="firmware/fm1-native/track_core.c firmware/fm1-native/track_song.c firmware/fm1-native/track_mixer.c firmware/fm1-native/track_chip.c firmware/fm1-native/track_drum.c firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/track_project_store.c"
cc -std=c11 -O1 -g -fsanitize=undefined -fno-omit-frame-pointer -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/drum_v03/test_v06_user_samples.c $C_SRC -o /tmp/beetle-drum-v06-user -lm
UBSAN_OPTIONS=halt_on_error=1 /tmp/beetle-drum-v06-user tests/drum_v03/BEETLE-DRUM-V0.5-migration-fixture.bdr tests/drum_v03/BEETLE-DRUM-V0.4-migration-fixture.bdr

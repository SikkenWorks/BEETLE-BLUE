#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
bash tests/drum_v03/run_v04.sh
C_SRC="firmware/fm1-native/track_core.c firmware/fm1-native/track_song.c firmware/fm1-native/track_mixer.c firmware/fm1-native/track_chip.c firmware/fm1-native/track_drum.c firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/track_project_store.c"
cc -std=c11 -O1 -fsanitize=undefined -fno-omit-frame-pointer -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/drum_v03/test_v05_editor.c $C_SRC -o /tmp/beetle-drum-v05-editor -lm
UBSAN_OPTIONS=halt_on_error=1 /tmp/beetle-drum-v05-editor tests/drum_v03/BEETLE-DRUM-V0.4-migration-fixture.bdr

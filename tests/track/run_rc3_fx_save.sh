#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
out="${TMPDIR:-/tmp}/beetle-blue-rc3-fx-save-test-$$"
trap 'rm -f "$out"' EXIT
cc -O1 -g -std=c11 -fsanitize=undefined -DBEETLE_DRUM_EDITION=1 \
 -Ifirmware/fm1-native tests/track/test_rc3_fx_save.c \
 firmware/fm1-native/{track_core,track_song,track_chip,track_drum,beetle_drum_rom,beetle_blue_monolith_rom,track_mixer,track_project_store,track_journal}.c \
 -o "$out"
UBSAN_OPTIONS=halt_on_error=1 "$out"

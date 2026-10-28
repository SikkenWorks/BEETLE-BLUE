#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
OLD_SOURCE="${1:?Pass the RC4 source directory to generate genuine legacy V31 test bytes}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
SRCS='track_core track_song track_chip track_drum beetle_drum_rom beetle_blue_monolith_rom track_mixer track_project_store'
OLD=(); NEW=();for x in $SRCS;do OLD+=("$OLD_SOURCE/firmware/fm1-native/$x.c");NEW+=("firmware/fm1-native/$x.c");done
cc -std=c11 -O1 -DBEETLE_DRUM_EDITION=1 -DMAKE_LEGACY -I"$OLD_SOURCE/firmware/fm1-native" \
 tests/track/test_blue_rc5_v31_migration.c "${OLD[@]}" -o "$TMP/old"
"$TMP/old" "$TMP/legacy-v31.bdr"
cc -std=c11 -O1 -g -fsanitize=undefined -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native \
 tests/track/test_blue_rc5_v31_migration.c "${NEW[@]}" -o "$TMP/new"
UBSAN_OPTIONS=halt_on_error=1 "$TMP/new" "$TMP/legacy-v31.bdr"
cc -std=c11 -O1 -g -fsanitize=undefined -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native \
 tests/track/test_blue_rc5_capacity.c "${NEW[@]}" firmware/fm1-native/beetle_project_pack_v2.c -o "$TMP/cap"
UBSAN_OPTIONS=halt_on_error=1 "$TMP/cap"
cc -std=c11 -O1 -g -fsanitize=undefined -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native \
 tests/track/test_blue_rc5_kit_audio.c firmware/fm1-native/{track_drum,beetle_drum_rom,beetle_blue_monolith_rom,track_mixer}.c -o "$TMP/kit"
UBSAN_OPTIONS=halt_on_error=1 "$TMP/kit"

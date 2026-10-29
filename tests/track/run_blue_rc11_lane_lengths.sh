#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT="$(mktemp -d)";trap 'rm -rf "$OUT"' EXIT
SOURCES=(firmware/fm1-native/{track_core,track_song,track_chip,track_drum,beetle_drum_rom,beetle_blue_monolith_rom,track_mixer,track_project_store}.c)
for name in lane_lengths v32_migration; do
 cc -std=c11 -O1 -g -fsanitize=undefined -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native "tests/track/test_blue_rc11_${name}.c" "${SOURCES[@]}" -lm -o "$OUT/test_${name}"
done
UBSAN_OPTIONS=halt_on_error=1 "$OUT/test_lane_lengths"
UBSAN_OPTIONS=halt_on_error=1 "$OUT/test_v32_migration" tests/track/fixtures/rc10_v32_project.bdrum

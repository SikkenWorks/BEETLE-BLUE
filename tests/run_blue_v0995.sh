#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
SRC=firmware/fm1-native
COMMON=("$SRC/track_drum.c" "$SRC/track_mixer.c" "$SRC/beetle_drum_rom.c" "$SRC/beetle_blue_monolith_rom.c")
gcc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=0 -I "$SRC" tests/test_blue_v0995_choke.c "${COMMON[@]}" -o /tmp/bb995_choke && /tmp/bb995_choke
gcc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=1 -I "$SRC" tests/test_blue_v0995_choke_v2.c "${COMMON[@]}" -o /tmp/bb995_choke_v2 && /tmp/bb995_choke_v2
gcc -std=c11 -O2 -I "$SRC" tests/test_blue_v0995_stream_fast.c "$SRC/beetle_bd4_prefetch.c" -o /tmp/bb995_fast && /tmp/bb995_fast
python tests/test_blue_v0994_attack.py
python tests/test_blue_v0994_flash.py
python tools/verify_blue_v0995_web.py

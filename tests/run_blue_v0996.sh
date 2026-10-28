#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
SRC=firmware/fm1-native
COMMON=("$SRC/track_drum.c" "$SRC/track_mixer.c" "$SRC/beetle_drum_rom.c" "$SRC/beetle_blue_monolith_rom.c")
gcc -std=c11 -O2 -Wall -Wextra -Werror -I "$SRC" tests/test_blue_v0996_live.c -o build/test_blue_v0996_live
build/test_blue_v0996_live
python tests/test_blue_v0996_live_contract.py
python tests/test_blue_v0996_live_flash.py
gcc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=0 -I "$SRC" tests/test_blue_v0995_choke.c "${COMMON[@]}" -o build/blue_v0996_choke
build/blue_v0996_choke
gcc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=1 -I "$SRC" tests/test_blue_v0995_choke_v2.c "${COMMON[@]}" -o build/blue_v0996_choke_v2
build/blue_v0996_choke_v2
gcc -std=c11 -O2 -I "$SRC" tests/test_blue_v0995_stream_fast.c "$SRC/beetle_bd4_prefetch.c" -o build/blue_v0996_stream_fast
build/blue_v0996_stream_fast
python tests/test_blue_v0994_attack.py
python tests/test_blue_v0994_flash.py
python tools/render_blue_live_v0996.py
python tools/verify_blue_v0996_web.py

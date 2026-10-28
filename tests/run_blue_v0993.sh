#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh tests/run_blue_v0992.sh
cc -std=c11 -O2 -Ifirmware/fm1-native tests/test_blue_v0993_lane_routing.c -o build/test_blue_v0993_lane_routing
build/test_blue_v0993_lane_routing
cc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -DBEETLE_NATIVE_V2=0 -Ifirmware/fm1-native \
  tests/test_blue_v0993_distinct_kits.c firmware/fm1-native/track_drum.c \
  firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/beetle_blue_monolith_rom.c \
  firmware/fm1-native/track_mixer.c -o build/test_blue_v0993_distinct_kits
build/test_blue_v0993_distinct_kits

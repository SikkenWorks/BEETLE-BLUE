#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
$CC -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native \
 tests/test_blue_v099_performance.c firmware/fm1-native/track_mixer.c \
 firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/beetle_blue_monolith_rom.c \
 -o /tmp/test_blue_v099_performance
/tmp/test_blue_v099_performance
sh tests/run_blue_v098.sh

#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
INC='-Ifirmware/fm1-native'
DRUM='firmware/fm1-native/track_drum.c firmware/fm1-native/track_mixer.c firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/beetle_blue_monolith_rom.c'
for test in test_blue_v094_rom test_blue_v095_rev test_blue_v096_trim_reverse test_blue_v097_step_fx test_blue_v098_monolith_pitch; do
 $CC -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 $INC "tests/$test.c" $DRUM -o "/tmp/$test" -lm
 "/tmp/$test"
done
$CC -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 $INC tests/test_blue_v098_hat_pair.c \
 firmware/fm1-native/track_core.c firmware/fm1-native/track_song.c \
 firmware/fm1-native/track_chip.c firmware/fm1-native/track_project_store.c \
 $DRUM -o /tmp/test_blue_v098_hat_pair -lm
/tmp/test_blue_v098_hat_pair
$CC -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 $INC tests/test_blue_v095_fx_codec.c firmware/fm1-native/track_core.c -o /tmp/test_blue_v095_fx_codec -lm
/tmp/test_blue_v095_fx_codec
sh tests/run_blue_v091.sh
python3 tests/test_blue_v094_dma_source_contract.py

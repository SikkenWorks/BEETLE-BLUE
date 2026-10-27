#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
SRC='firmware/fm1-native/track_core.c firmware/fm1-native/track_song.c firmware/fm1-native/track_mixer.c firmware/fm1-native/track_drum.c firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/beetle_blue_monolith.c'
cc -O2 -std=c11 -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/test_blue_v093_monolith.c $SRC -o /tmp/blue_v093_monolith -lm
/tmp/blue_v093_monolith
cc -O2 -std=c11 -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/test_blue_v09_double_kick.c $SRC -o /tmp/blue_v093_kick -lm
/tmp/blue_v093_kick
cc -O2 -std=c11 -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/test_blue_v091_lane_kit_project.c $SRC firmware/fm1-native/track_chip.c firmware/fm1-native/track_project_store.c -o /tmp/blue_v093_project -lm
/tmp/blue_v093_project tests/fixtures/blue-v029-lane-kits.bdr

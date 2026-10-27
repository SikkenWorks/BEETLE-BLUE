#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
SRC='firmware/fm1-native/track_core.c firmware/fm1-native/track_song.c firmware/fm1-native/track_mixer.c firmware/fm1-native/track_chip.c firmware/fm1-native/track_drum.c firmware/fm1-native/beetle_drum_rom.c firmware/fm1-native/track_project_store.c'
# The legacy fixture is retained in this repository: offline test needs no previous source.
cc -std=c11 -O2 -DBEETLE_DRUM_EDITION=1 -Ifirmware/fm1-native tests/test_blue_v091_lane_kit_project.c $SRC -o /tmp/blue-v091-test -lm
/tmp/blue-v091-test tests/fixtures/blue-v029-lane-kits.bdr

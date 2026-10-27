#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
cc -std=c11 -Wall -Wextra -Werror -O2 -Ifirmware/fm1-native firmware/fm1-native/beetle_drum6.c tests/drum_v03/test_drum6.c -o /tmp/beetle-drum-v03-test && /tmp/beetle-drum-v03-test
cc -std=c11 -Wall -Wextra -Werror -O2 -Ifirmware/fm1-native firmware/fm1-native/beetle_drum6_kits.c tests/drum_v03/test_drum6_kits.c -o /tmp/beetle-drum-v03-kits && /tmp/beetle-drum-v03-kits

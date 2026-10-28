#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
python tests/test_blue_v0992_punch_lcd.py
sh tests/run_blue_v099.sh

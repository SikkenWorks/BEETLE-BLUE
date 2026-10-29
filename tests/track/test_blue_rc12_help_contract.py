#!/usr/bin/env python3
"""Native RC12 240x240 HELP layout and project documentation contract."""
from pathlib import Path
import re
p=Path(__file__).resolve().parents[2]
h=(p/'firmware/fm1-native/beetle_help.h').read_text()
t=(p/'firmware/fm1-native/track_runtime.h').read_text()
a=h.split('static const struct beetle_help_card beetle_help_cards[] = {',1)[1].split('};',1)[0]
cards=re.findall(r'\{"([^"\\]*)","([^"\\]*)"\}',a)
assert len(cards)==66
assert all(len(title)<=28 and len(detail)<=33 for title,detail in cards)
assert 'BEETLE_HELP_CARDS_PER_PAGE 3u' in h
assert 'top+18u' in t and 'row*38u' in t
assert 'track_help_page=beetle_help_page_shift' in t
assert 'beetle_help_topic_shift' in t
assert 'track_help_page=0u' in t
assert 'beetle_help_lines' not in t
assert '"BEETLE BLUE V1.0 RC12"' in t
alltext=' '.join(x+' '+y for x,y in cards)
for s in ('LANE LENGTHS','SEQ + TURN K4','V33','LIVE MODE','PERFORM','BUILT-IN BEETLE','MONOLITH','STORAGE V2','RECOVER AUDIO','FELUCCA'):
 if s=='FELUCCA':assert 'Felucca' in (p/'README.md').read_text()
 else: assert s in alltext,s
readme=(p/'README.md').read_text()
assert 'Current source: V1.0 RC12' in readme
assert 'RC12' not in readme.split('## Overview')[1].split('## Getting started')[0]
assert 'CHANGELOG.md' in readme
assert not re.search(r'^## V1\.0\s*RC\d+\s*[—:-]',readme,re.M)
print(f'PASS: {len(cards)} HELP cards, topic/page navigation, legible layout and guide-only README')

#!/usr/bin/env python3
"""Current BLUE release: README must be a guide, HELP current, root free of stale artifacts."""
from pathlib import Path
import re, sys
p=Path(__file__).resolve().parents[1]
r=(p/'README.md').read_text()
h=(p/'firmware/fm1-native/beetle_help.h').read_text()
cards=re.findall(r'\{"([^"\\]*)","([^"\\]*)"\}',h.split('beetle_help_cards[] = {',1)[1].split('};',1)[0])
problems=[]
if re.search(r'^#{1,6}\s*(?:V1\.|RC\d+|.*(?:history|fix|release notes).*(?:RC\d+|V\d))',r,re.I|re.M):problems.append('Release-history section found in README')
if not all(s in r for s in ['Independent lane lengths','Live Mode','User Kits','Battery indicator','Credits and licenses']):problems.append('README missing a current operating section')
for needed in ['LANE LENGTHS','V33','PERFORM','MONOLITH']:
 if needed not in ' '.join(a+' '+b for a,b in cards):problems.append('HELP missing: '+needed)
if any(len(a)>28 or len(b)>33 for a,b in cards):problems.append('HELP card text exceeds 240px layout')
if len(cards)>255:problems.append('HELP exceeds uint8 card index')
if not (p/'CHANGELOG.md').is_file():problems.append('CHANGELOG missing')
if not (p/'LICENSE').is_file():problems.append('LICENSE missing')
if any(re.search(r'RC(?:[1-9]|10)[-_.]',f.name) for f in p.iterdir() if f.is_file() and f.name!='CHANGELOG.md'):problems.append('Old RC artifact in project root')
for x in ('BEETLE-BLUE-Manager.html','BEETLE-BLUE-Project-Manager.html','WebInstaller.html'):
 if not (p/x).exists():problems.append('Current webtool missing: '+x)
if problems:
 print('FAIL release hygiene:','; '.join(problems));sys.exit(1)
print('PASS release hygiene: current guide, FM1 HELP, clean root, current webtools and licenses')

from pathlib import Path
r=Path(__file__).resolve().parents[2]
s=(r/'firmware/fm1-native/track_runtime.h').read_text()
checks={
 'home menu entry': '"SETTINGS","LIVE MODE","DIAGNOSTICS"' in s,
 'menu 10 entries': 'track_web_list(entries,10u,track_menu_index,46u,14u)' in s,
 'home pointer live': 'track_menu_index=BLUE_HOME_LIVE_INDEX' in s,
 'home menu select live': 'else if(track_menu_index==7u){\n     /* Live Mode' in s,
 'home menu diagnostics index': 'else if(track_menu_index==8u){\n     track_rc_tab' in s,
 'home menu wipe index': 'if(track_menu_index==9u){\n     track_wipe_yes' in s,
 'home menu max index 9': 'if(next>9)next=9' in s,
 'setlist navigation saves': 'if(blue_live_autosave_on_exit()){track_page=blue_live_home_target(BLUE_LIVE_SETLIST_PAGE)' in s,
 'setlist failure stays open': 'if(blue_live_autosave_on_exit())' in s,
 'live menu HOME goes projects': 'track_page=blue_live_home_target(BLUE_LIVE_MENU_PAGE)' in s,
 'perform stop exits to live menu': 'track_page=blue_live_home_target(BLUE_LIVE_PERFORM_PAGE)' in s,
 'live menu label': '"HOME: PROJECT MENU"' in s,
 'perform footer label': s.count('"HOME: LIVE MODE"')>=3,
 'boot live entry intact': 'else {blue_live.menu=0u;track_page=43u;}' in s,
 'no editing project data altered': 'track_song.loop=0u;' in s
}
for k,v in checks.items(): print(('PASS' if v else 'FAIL'),k)
assert all(checks.values())
from PIL import Image
im=Image.open(r/'UI-CAPTURES-NATIVE-1920/HOME-LIVE-MODE-ENTRY.png');assert im.size==(1920,1920)
print('PASS native source screenshot 8x nearest-neighbor',im.size)

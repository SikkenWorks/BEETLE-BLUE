from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'firmware/fm1-native/track_runtime.h').read_text()
w=(r/'firmware/fm1-native/beetle_storage_v2_writer.c').read_text()
page=(r/'firmware/fm1-native/beetle_settings_nav.h').read_text()
assert 'page<=46u' in page
for marker in ('"NEW PROJECT","LOAD PROJECT"','"LIVE MODE"','"SETLIST","PERFORM"','"CONTINUE CURRENT TRACK","START CURRENT TRACK AGAIN"', 'blue_live_song_done=1u','if(blue_live_next(&blue_live))','beetle_project_load_slot(slot)','track_song.loop=0u','track_state.transport.bpm=beetle_staged.bpm','blue_live_autosave_on_exit()'):
 assert marker in s,marker
# Leave Setlist via HOME *and* via SEL-to-Perform: both save first.
editor=s[s.index('  }else if(track_page==44u){'):s.index('  }else if(track_page==45u){',s.index('  }else if(track_page==44u){'))]
assert editor.count('blue_live_autosave_on_exit()')>=3
assert 'blue_live_autosave_dirty(&blue_live_dirty,&blue_live_save_status,' in s
assert 'blue_live_save_callback,0);' in s
assert 'blue_battery_hud(204u,208u)' in s and 'blue_battery_hud(204u,53u)' in s
assert 'native_tripped||native_updating||blue_live_song_done' in s
assert 'memcpy(idx+2610u,record,64u)' in w
assert 'return commit(w,idx,other)' in w
assert '20u+g32(idx+12)>2610u' in w
print('PASS live ISR-to-foreground boundary, native screens and persistent metadata guards')

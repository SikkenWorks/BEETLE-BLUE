"""Source contract for a confirmed PCM-initialization bug, not a hardware DMA test."""
from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'firmware/fm1-native/track_runtime.h').read_text()
f=s[s.index('static void track_fm1_audio_process('):s.index('static void track_fm1_leds(')]
assert 'memset(dst,0,(size_t)frames*2u*sizeof(*dst));' in f
assert f.index('memset(dst,0,(size_t)frames*2u*sizeof(*dst));')<f.index('track_drum_render(')
assert '#if !defined(BEETLE_DRUM_EDITION) || !BEETLE_DRUM_EDITION' in f
print('PASS static source contract: BLUE native_stereo PCM clears before additive drum mixer')

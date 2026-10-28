/* BEETLE V0.44 · PCM ROM digital drum kit with 6-voice 44.1kHz rendering.
 * No legacy quantized samples, no FP/libm, no malloc, no flash writes.
 * Master output attenuation is applied to the completed stereo mix by
 * audio_target.c; per-lane gain here is always <=100%.
 * GPL-3.0-only.
 */
#include "track_drum.h"
#include "track_mixer.h"
#include "track_core.h"
#include "beetle_stereo_pan.h"
#include "beetle_drum_rom.h"
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
#include "beetle_blue_monolith_rom.h"
#endif
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
#include "beetle_bd4_live.h"
#endif
#include <string.h>
static const char *const names[TRACK_DRUM_COUNT]={"KIK","SNR","HHC","HHO","TM1","TM2","TM3","CRS","RDE","CLP","PC1","PC2","PC3","RIM"};
/* Physical synthesis variations. Snare and hi-hat alternate without repeating. */
static const uint8_t physical[TRACK_DRUM_COUNT]={0u,1u,6u,10u,14u,12u,11u,13u,15u,3u,5u,7u,9u,4u};
const char *track_drum_name(uint8_t id){
 static const char *const user_names[BEETLE_ONESHOT_NOTES]={"U01","U02","U03","U04","U05","U06","U07","U08","U09","U10","U11","U12","U13"};
 if(id==TRACK_NOTE_STOP)return "STP";
 if(id<TRACK_DRUM_COUNT)return names[id];
 if(id>=TRACK_DRUM_NOTE_COUNT)return "---";
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 const char *label=beetle_bd4_live_label((uint8_t)(id-BEETLE_ONESHOT_FIRST));
 if(label)return label;
#endif
 return user_names[id-BEETLE_ONESHOT_FIRST];
}
static uint32_t rnd(uint32_t *state){uint32_t x=*state;x^=x<<13;x^=x>>17;x^=x<<5;*state=x?x:0x9E3779B9u;return *state;}
/* Studio-balanced default digital kit. EQ 50 is neutral; 0..100 is a
 * cut/boost control. All data are project-owned and user-editable. */
static const struct track_drum_timbre beetle_factory[TRACK_DRUM_COUNT]={
 /* ATT DEC SUS TAIL LOW MID HIGH GAIN PITCH */
 {0,66,0,53,72,48,59,100, 0}, /* kick: round sub + defined beater */
 {0,62,0,54,48,64,72,100, 0}, /* snare: shell + rattling wires */
 {0,43,0,32,35,48,73, 88, 0}, /* HHC: very short, bright */
 {0,67,0,72,35,48,72, 87, 0}, /* HHO: smooth extended wash */
 {0,64,0,62,70,57,47, 94,-1}, /* low tom */
 {0,60,0,56,65,58,49, 94, 0}, /* mid tom */
 {0,55,0,49,59,60,51, 93, 1}, /* high tom */
 {0,77,0,79,35,53,65, 88, 0}, /* crash */
 {0,83,0,90,41,57,56, 91, 0}, /* ride bell + wash */
 {0,53,0,52,39,61,69, 99, 0}, /* clap: staggered bursts */
 {0,55,0,45,55,61,45, 95, 0}, /* woodblock */
 {0,70,0,64,40,59,58, 90, 0}, /* cowbell */
 {0,43,0,37,32,57,67, 94, 0}, /* clave */
 {0,41,0,31,40,57,75, 91, 0}  /* rim */
};
void track_drum_component_default(struct track_drum_timbre *p,uint8_t id){
 if(p&&id<TRACK_DRUM_COUNT)*p=beetle_factory[id];
}
void track_drum_timbre_default(struct track_drum_timbre *p){
 if(!p)return;*p=(struct track_drum_timbre){0u,50u,0u,50u,50u,50u,50u,100u,0,0u,0u};
}
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
void track_drum_user_default(struct track_drum_timbre *p){track_drum_timbre_default(p);}
#endif
/* Controls are resolved by NOTE ID. Both local one-shots and Storage V2
 * global streams go through the same velocity/EQ/drive/voice mixer.
 * The DPCM decoder controls its own playback rate, so user PITCH is
 * deliberately not exposed in the editor. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
static struct track_drum_timbre blue_voice_timbre[TRACK_DRUM_POLY];
/* Fixed-size private transient effects. Kept with song/voice RAM rather
 * than the scarce near-memory DSP/USB DMA buffers. No allocation in ISR. */
struct blue_drum_fx_slot {uint8_t type[3],value[3];};
static struct blue_drum_fx_slot blue_voice_fx[TRACK_DRUM_POLY];

#endif
static const struct track_drum_timbre fallback_user={0u,50u,0u,50u,50u,50u,50u,100u,0,0u,0u};
static const struct track_drum_timbre *drum_control(const struct track_drum_engine *d,uint8_t note){
 if(note<TRACK_DRUM_COUNT)return &d->components[note];
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 if(note<TRACK_DRUM_NOTE_COUNT)return &d->user[note-TRACK_DRUM_COUNT];
#endif
 return &fallback_user;
}
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
static int8_t blue_global_drum_semitones;
#endif
void track_drum_init(struct track_drum_engine *d){
 if(!d)return;memset(d,0,sizeof(*d));
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 blue_global_drum_semitones=0;
#endif
 d->rng=0xA17C9E23u;d->sn_last=255u;d->hh_last=255u;d->step_gain=100u;
 for(uint8_t i=0u;i<TRACK_DRUM_COUNT;i++)d->components[i]=beetle_factory[i];
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 for(uint8_t i=0u;i<BEETLE_ONESHOT_NOTES;i++)track_drum_user_default(&d->user[i]);
#endif
}

void track_drum_bind_oneshots(struct track_drum_engine *d,const struct beetle_oneshot_bank *b){if(d)d->oneshots=b;}
static uint8_t choose(struct track_drum_engine *d,uint8_t index){
 if(index==1u){uint8_t id=(uint8_t)(1u+(rnd(&d->rng)&1u));if(id==d->sn_last)id=(uint8_t)(3u-id);d->sn_last=id;return id;}
 if(index==6u){uint8_t id=(rnd(&d->rng)&1u)?8u:6u;if(id==d->hh_last)id=id==6u?8u:6u;d->hh_last=id;return id;}
 return index;
}
/* Identify ALL hat voices by their physical ID, including the MONOLITH
 * procedural closed/open/alternate-hat ROM voices. Formerly only BEETLE PCM
 * hats could choke one another and a MONOLITH open hat rang underneath a
 * subsequent closed hat. The same choke family spans both embedded kits. */
static uint8_t hat(uint8_t id){
 return (uint8_t)(id==6u||id==8u||id==10u
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 ||id==66u||id==67u||id==79u
#endif
 );
}

/* µ-law ROM at 22050 Hz, linearly interpolated at 44100 Hz. The original
 * attack/body/tail is rendered offline by tools/create_v044_kit.py. This
 * replaces the former same-sine-plus-noise engine, without touching flash
 * storage or the persisted drum component structure. */
static const uint32_t tune_q16[73]={8192u,8679u,9195u,9742u,10321u,10935u,11585u,12274u,13004u,13777u,14596u,15464u,16384u,17358u,18390u,19484u,20643u,21870u,23170u,24548u,26008u,27554u,29193u,30929u,32768u,34716u,36781u,38968u,41285u,43740u,46341u,49097u,52016u,55109u,58386u,61858u,65536u,69433u,73562u,77936u,82570u,87480u,92682u,98193u,104032u,110218u,116772u,123715u,131072u,138866u,147123u,155872u,165140u,174960u,185364u,196386u,208064u,220436u,233544u,247431u,262144u,277732u,294247u,311744u,330281u,349920u,370728u,392772u,416128u,440872u,467088u,494862u,524288u};
static uint32_t rate_step(int8_t semi){
 if(semi<-36)semi=-36;if(semi>36)semi=36;
 /* Exact algebraic simplification: ((32768*T + 32768)>>16) ==
  * ((T + 1)>>1). Removes software-emulated 64-bit multiplication from
  * every drum trigger and PITCH retune without changing a single bit. */
 return (tune_q16[(uint8_t)(semi+36)]+1u)>>1u;
}
/* Internal physical drum IDs are deliberately unchanged for sequencer,
 * alternative snare/hats, and legacy test compatibility. The ROM bank holds
 * the 14 user-visible instruments, so map 16 physical IDs to 14 ROM entries. */
static const uint8_t rom_index[16]={0u,1u,14u,9u,13u,10u,2u,11u,15u,12u,3u,4u,5u,7u,6u,8u};
/* The MONOLITH kit is now generated at 44.1kHz from small ROM patch
 * coefficients and a 256-point ROM sinusoid, not encoded samples.
 * No 64-bit divide, streaming reads or decoder loops in the ISR.
 * Original BEETLE µ-law PCM kit is kept unmodified. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
static int32_t blue_rom_wave(struct track_drum_voice *v,uint8_t pad,uint8_t slot){
 const struct blue_rom_patch *p=&blue_rom_patches[pad];
 v->noise_state=rnd(&v->noise_state);
 const int32_t noise=(int16_t)(v->noise_state>>16u);
 const int32_t bright=noise;
 const int32_t tone=blue_rom_sine[v->kit_phase>>24u];
 const int32_t metal=blue_rom_sine[(v->kit_phase*7u)>>24u];
 /* A per-instrument oscillator shape is crucial: sharing nearly identical
  * sine+noise spectra made snare/hats/toms seem like the BEETLE PCM kit.
  * These are cheap arithmetic variations in native ROM synthesis, not copied
  * WAVs or a second PCM bank. Preserve the original pitch/envelopes. */
 int32_t tonal=(tone*3+metal)/4;
 if(pad==1u||pad==14u)tonal=(tone*2+((v->kit_phase&0x80000000u)?-12000:12000))/3;
 else if(pad>=4u&&pad<=6u)tonal=(tone*5+metal)/6;
 else if(pad==7u||pad==8u||pad==15u)tonal=(metal*3+tone)/4;
 else if(pad>=10u&&pad<=12u)tonal=(tone+metal*2)/3;
 else if(pad==13u)tonal=(tone>0?14000:-14000);
 int32_t wave=(tonal*(256-(int32_t)p->noise_mix)+bright*p->noise_mix)>>8u;
 /* Clap: repeated noise bursts; timbral motion without another sample buffer. */
 if(pad==9u){uint32_t pos=v->age;uint32_t phase=(pos/390u)%3u;
  if(phase==1u)wave=(wave*3)>>2u;
  if(phase==2u)wave=(wave*2)>>2u;
 }
 /* Procedural MONOLITH has no stored PCM. Reverse the oscillator direction
  * and rise with a fixed-point envelope. Never divide in the DMA callback.
  * This is explicitly reverse-style synthesis, not literal time-reversed PCM. */
 const uint32_t scale=v->kit_env>>16u;
 /* |wave|<=32768, scale<=256. This signed product fits in int32. */
 wave=(wave*(int32_t)scale)>>8u;
 uint32_t sweep=v->kit_sweep;
 /* MONOLITH's waveform uses kit_phase, not the PCM read cursor. Applying
  * PITCH to phase_step alone previously changed only the note lifetime,
  * not its actual audible oscillator frequency. Multiply Q8 halves here:
  * unsigned 32-bit modular math is exactly appropriate for phase wrapping.
  * This costs one 32-bit multiply and NO 64-bit arithmetic per sample. */
 uint32_t inc=((p->phase_inc+sweep)>>8u)*(v->phase_step>>7u);
 /* MONOLITH has an independent oscillator phase. Route the step-FX tonal
  * modulation to THAT oscillator, never only to the PCM read cursor. */
 if(slot<TRACK_DRUM_POLY && (blue_voice_fx[slot].type[0] ||
    blue_voice_fx[slot].type[1] || blue_voice_fx[slot].type[2])){
  for(uint8_t j=0u;j<3u;j++){
   uint8_t type=blue_voice_fx[slot].type[j];
   uint8_t value=blue_voice_fx[slot].value[j];
   if(type==7u && value){
    uint8_t shift=value>66u?11u:value>33u?12u:13u;
    uint32_t stage=(v->age>>shift)%3u;
    inc=(uint32_t)(((uint64_t)inc*(stage==0u?256u:stage==1u?323u:384u))>>8u);
   }else if((type==5u||type==6u)&&value){
    uint8_t shift=value>66u?5u:value>33u?6u:7u;
    uint32_t phase=(v->age>>shift)&511u;
    int32_t tri=(int32_t)(phase>255u?511u-phase:phase)-128;
    int32_t deviation=(int32_t)((inc>>11u)*value*tri/12800);
    if(deviation<0 && (uint32_t)(-deviation)>=inc)inc=1u;
    else inc=(uint32_t)((int32_t)inc+deviation);
   }
  }
 }
 if(v->reverse)v->kit_phase-=inc;
 else v->kit_phase+=inc;
 if(sweep){uint32_t loss=sweep>>10u;if(!loss)loss=1u;v->kit_sweep-=loss;}
 if(v->reverse){
  uint32_t step=(16777215u-v->kit_env)>>p->env_shift;
  if(!step)step=1u;
  v->kit_env+=step;
 }else if(v->kit_env){uint32_t loss=v->kit_env>>p->env_shift;
  if(!loss)loss=1u;v->kit_env-=loss;}
 return wave;
}
#endif
static uint32_t sample_frames(uint8_t id,const struct beetle_oneshot_bank *bank){
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 if(id>=64u&&id<80u)return blue_rom_patches[id-64u].length_22050;
#endif
 if(id>=16u){uint8_t s=(uint8_t)(id-16u);return bank&&s<BEETLE_ONESHOT_NOTES?bank->length[s]:0u;}
 const uint8_t i=rom_index[id];return beetle_drum_offsets[i+1u]-beetle_drum_offsets[i];
}
static void update_length(struct track_drum_voice *v,const struct track_drum_timbre *p,const struct beetle_oneshot_bank *bank){
 uint32_t step=v->phase_step;if(step==0u)step=1u;
 uint32_t playback=(sample_frames(v->id,bank)*512u)/(step>>7u);
 /* Tail control changes the one-shot length without reading out of ROM. */
 playback=playback*(50u+p->release)/100u;
 if(playback>350000u)playback=350000u;
 v->length=(uint32_t)(playback<64u?64u:playback);
 uint32_t decay=(uint32_t)(150u-p->decay); /* 50 default = gentle 1-fold attenuation */
 uint32_t loss=(decay*65536u)/(100u*(v->length?v->length:1u));
 if(loss<1u)loss=1u;if(loss>65535u)loss=65535u;
 v->env_loss_q16=(uint16_t)loss;
}
int track_drum_trigger(struct track_drum_engine *d,uint8_t index){
 if(!d)return 0;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 const uint8_t mono=(uint8_t)(index>=64u && index<80u);
 if(index>=TRACK_DRUM_NOTE_COUNT&&!mono)return 0;
 const uint8_t component=(uint8_t)(mono?(index==78u?1u:(index==79u?2u:(index-64u))):index);
#else
 if(index>=TRACK_DRUM_NOTE_COUNT)return 0;
 const uint8_t component=index;
 const uint8_t mono=0u;
#endif
 const uint8_t user=(uint8_t)(!mono && index>=TRACK_DRUM_COUNT);
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 const uint8_t global=user && beetle_bd4_live_available((uint8_t)(index-TRACK_DRUM_COUNT));
#else
 const uint8_t global=0u;
#endif
 if(user && !global && (!d->oneshots||!d->oneshots->length[index-BEETLE_ONESHOT_FIRST]))return 0;
 const uint8_t id=mono?index:global?(uint8_t)(32u+index-TRACK_DRUM_COUNT):user?(uint8_t)(16u+index-BEETLE_ONESHOT_FIRST):choose(d,physical[index]);
 /* BLUE V0.99.5: A user sample is MONOPHONIC PER SOURCE by default.
  * An identical U01..U13 (local or pinned V2) retrigger chokes its older
  * voice regardless of group=0, lane, pattern or kit. Other user samples
  * remain independent unless an explicit choke group links them. Existing
  * 48-frame fade avoids a hard PCM discontinuity. Both BEETLE/MONOLITH
  * factory hi-hats are always members of the same choke family. */
 if(hat(id)||user)for(uint8_t i=0;i<TRACK_DRUM_POLY;i++){
  struct track_drum_voice *old=&d->voices[i];
  if(old->active && ((hat(id)&&hat(old->id)) ||
      (user&&old->id==id)) &&
      (!old->release_frames||old->release_frames>48u))old->release_frames=48u;
 }
 unsigned target=TRACK_DRUM_POLY;
 for(unsigned i=0;i<TRACK_DRUM_POLY;i++)if(!d->voices[i].active){target=i;break;}
 if(target==TRACK_DRUM_POLY){uint32_t oldest=0xFFFFFFFFu;
  for(unsigned i=0;i<TRACK_DRUM_POLY;i++){
   if(d->voices[i].release_frames){target=i;break;}
   if(d->voices[i].sequence<oldest){oldest=d->voices[i].sequence;target=i;}
  }
 }
 struct track_drum_voice *v=&d->voices[target];
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 beetle_bd4_live_stop((uint8_t)target);
#endif
 *v=(struct track_drum_voice){0};
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 memset(&blue_voice_fx[target],0,sizeof(blue_voice_fx[target]));
#endif
 v->sequence=++d->trigger_serial;v->id=id;v->component=component;
 v->source_lane=TRACK_DRUM_CHANNEL; /* native DRUM uses six routed lanes */
 /* Choke groups 1–7 are shared across factory and user samples.
  * Group 0 is independent; the original factory open/closed hi-hat
  * choke still works before voice allocation. */
 const struct track_drum_timbre *p=drum_control(d,component);
 if(p->choke_group){
  const uint8_t group=p->choke_group;
  for(uint8_t x=0;x<TRACK_DRUM_POLY;x++){
   struct track_drum_voice *old=&d->voices[x];
   if(old!=v && old->active &&
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
      blue_voice_timbre[x].choke_group==group &&
#else
      drum_control(d,old->component)->choke_group==group &&
#endif
      (!old->release_frames || old->release_frames>48u))old->release_frames=48u;
  }
 }
 if(user&&!global){uint32_t start=0u;for(uint8_t s=0u;s<index-BEETLE_ONESHOT_FIRST;s++)start+=d->oneshots->length[s];
  if(start+d->oneshots->length[index-BEETLE_ONESHOT_FIRST]>BEETLE_ONESHOT_MAX_BYTES){v->active=0u;return 0;}
  v->oneshot_offset=(uint16_t)start;
 }
 /* p resolved above for factory drums, local samples and V2 streams. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 blue_voice_timbre[target]=*p;v->start_percent=0u;v->end_percent=100u;
#endif
 v->gain_q8=(uint16_t)(256u*p->gain/100u);
 v->env_q24=16777215u;v->noise_state=rnd(&d->rng);
 v->pitch_semitones=p->pitch;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 {int32_t pitch=(int32_t)v->pitch_semitones+blue_global_drum_semitones;
  if(pitch<-36)pitch=-36;if(pitch>36)pitch=36;v->pitch_semitones=(int8_t)pitch;}
#endif
 v->phase_step=rate_step(v->pitch_semitones);v->active=1u;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 if(mono){v->kit_phase=0u;v->kit_env=1u<<24u;v->kit_sweep=blue_rom_patches[index-64u].sweep_inc;}
#endif
 if(global){
  v->length=900000u; /* max 20 s @ 44.1 kHz, source determines actual end */
  v->env_loss_q16=1u;
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
  beetle_bd4_live_request((uint8_t)target,(uint8_t)(index-TRACK_DRUM_COUNT),v->sequence);
#endif
 }else update_length(v,p,d->oneshots);
 return 1;
}
int track_drum_trigger_pair(struct track_drum_engine *d,uint8_t first,uint8_t second){
 const uint8_t a=(uint8_t)(first<TRACK_DRUM_NOTE_COUNT
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 ||(first>=64u&&first<80u)
#endif
 ),b=(uint8_t)(second<TRACK_DRUM_NOTE_COUNT
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 ||(second>=64u&&second<80u)
#endif
 );
 if(!a&&!b)return 0;
 if(a&&b){
  /* A single physical hat retrigger per row. BEETLE and MONOLITH must
   * obey identical closed-hat priority when their two columns conflict. */
  if(first==second && (
      (first<TRACK_DRUM_COUNT && hat(physical[first]))
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
      ||hat(first)
#endif
     ))return track_drum_trigger(d,first);
  if(first==2u&&second==3u)return track_drum_trigger(d,first);
  if(first==3u&&second==2u)return track_drum_trigger(d,second);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  if(first==66u&&second==67u)return track_drum_trigger(d,first);
  if(first==67u&&second==66u)return track_drum_trigger(d,second);
#endif
 }
 int n=0;if(a)n+=track_drum_trigger(d,first);if(b)n+=track_drum_trigger(d,second);return n;
}
int track_drum_trigger_pair_percent(struct track_drum_engine *d,uint8_t a,uint8_t b,uint8_t percent){
 if(!d||percent==0u)return 0;if(percent>100u)percent=100u;
 uint32_t serial=d->trigger_serial;int n=track_drum_trigger_pair(d,a,b);
 for(uint8_t i=0;i<TRACK_DRUM_POLY;i++){
  struct track_drum_voice *v=&d->voices[i];
  if(v->active&&(int32_t)(v->sequence-serial)>0)v->gain_q8=(uint16_t)((v->gain_q8*(uint32_t)percent+50u)/100u);
 }return n;
}
int track_drum_trigger_pair_tuned(struct track_drum_engine *d,uint8_t a,uint8_t b,int8_t semitones){
 if(!d)return 0;if(semitones<-12)semitones=-12;if(semitones>12)semitones=12;
 uint32_t before=d->trigger_serial;int n=track_drum_trigger_pair(d,a,b);
 for(uint8_t i=0;i<TRACK_DRUM_POLY;i++){
  struct track_drum_voice *v=&d->voices[i];
  if(v->active&&(int32_t)(v->sequence-before)>0){
   int32_t combined=(int32_t)v->pitch_semitones+semitones;
   if(combined<-36)combined=-36;if(combined>36)combined=36;
   v->pitch_semitones=(int8_t)combined;v->phase_step=rate_step(v->pitch_semitones);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 /* ROM patch tuning is applied via integer oscillator-frequency adjustment at trigger. */
#endif
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
   if(v->id>=32u&&v->id<64u)continue; /* DPCM4 stream owns its decoded frame count. */
#endif
   update_length(v,
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  &blue_voice_timbre[(uint8_t)(v-d->voices)]
#else
  drum_control(d,v->component)
#endif
 ,d->oneshots);
  }
 }return n;
}
void track_drum_retune_voice(struct track_drum_engine *d,struct track_drum_voice *v){
 if(!d||!v||!v->active)return;
 v->phase_step=rate_step(v->pitch_semitones);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 /* ROM patch tuning is applied via integer oscillator-frequency adjustment at trigger. */
#endif
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 if(v->id>=32u&&v->id<64u)return; /* Do not derive ROM length for streamed data. */
#endif
 update_length(v,
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  &blue_voice_timbre[(uint8_t)(v-d->voices)]
#else
  drum_control(d,v->component)
#endif
 ,d->oneshots);
}
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
uint8_t track_drum_trim_local_voice(struct track_drum_engine *d,struct track_drum_voice *v,uint8_t start,uint8_t end){
 if(!d||!v||!v->active||v->id<16u||v->id>=32u||!d->oneshots||
    end>100u||start>=end)return 0u;
 const uint32_t total=sample_frames(v->id,d->oneshots);
 if(!total||total>BEETLE_ONESHOT_MAX_BYTES)return 0u;
 const uint32_t from=total*start/100u,to=total*end/100u;
 if(to<=from||to>total)return 0u;
 v->start_percent=start;v->end_percent=end;
 /* Phase always advances from zero within the trim window. The renderer
  * performs the forward/reverse source mapping, so REV never escapes it. */
 v->phase=0u;
 const uint32_t rate=v->phase_step?v->phase_step:1u;
 const uint32_t max_frames=((to-from)<<16u)/rate;
 if(v->length>(max_frames?max_frames:1u))v->length=max_frames?max_frames:1u;
 return 1u;
}
uint8_t track_drum_reverse_voice(struct track_drum_voice *v){
 if(!v||!v->active || (v->id>=32u&&v->id<64u))return 0u;
 if(!v->reverse){
  v->reverse=1u;
  if(v->id>=64u&&v->id<80u)v->kit_env=0u;
 }
 return 1u;
}
#endif
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
/* One bounded retune per live PITCH change, never an allocation or a flash read.
 * New drumvoices inherit the same offset at trigger time. BD4S streaming
 * currently has fixed-rate source decode; do not claim its pitch changes. */
void track_drum_set_global_pitch(struct track_drum_engine *d,int8_t semitones){
 if(!d)return;if(semitones<-24)semitones=-24;if(semitones>24)semitones=24;
 int32_t delta=(int32_t)semitones-blue_global_drum_semitones;
 if(!delta)return;
 blue_global_drum_semitones=semitones;
 for(uint8_t i=0u;i<TRACK_DRUM_POLY;i++){
  struct track_drum_voice *v=&d->voices[i];
  if(!v->active)continue;
  int32_t pitch=(int32_t)v->pitch_semitones+delta;
  if(pitch<-36)pitch=-36;if(pitch>36)pitch=36;
  v->pitch_semitones=(int8_t)pitch;
  track_drum_retune_voice(d,v);
 }
}
void track_drum_step_fx(struct track_drum_engine *d,struct track_drum_voice *v,
                       uint8_t slot,uint8_t type,uint8_t value,uint16_t bpm){
 if(!d||!v||!v->active||slot>=3u||type>=TRACK_FX_COUNT)return;
 uint8_t i=(uint8_t)(v-d->voices);if(i>=TRACK_DRUM_POLY)return;
 if(value>(type==TRACK_FX_VOLUME?200u:100u))value=type==TRACK_FX_VOLUME?200u:100u;
 blue_voice_fx[i].type[slot]=type;
 blue_voice_fx[i].value[slot]=value;
 if(type==TRACK_FX_VOLUME){
  v->gain_q8=(uint16_t)((uint32_t)v->gain_q8*value/100u);
 }else if(type==TRACK_FX_DRUM_PITCH){
  int32_t pitch=(int32_t)v->pitch_semitones+(int32_t)value-12;
  if(pitch< -36)pitch=-36;if(pitch>36)pitch=36;
  v->pitch_semitones=(int8_t)pitch;
  track_drum_retune_voice(d,v);
 }else if(type==TRACK_FX_REVERSE){
  (void)track_drum_reverse_voice(v);
 }else if(type==8u){
  /* CUT: at 100% use full step, at 0% the shortest click-safe gate. */
  uint32_t step_frames=661500u/(bpm?bpm:120u);
  uint32_t cap=(step_frames*value)/100u;
  if(cap<32u)cap=32u;
  if(v->length>cap)v->length=cap;
 }
}
/* Cheap deterministic LFO, 0..255 triangle, no trigonometric functions. */
static uint8_t blue_fx_triangle(uint32_t age,uint32_t period){
 /* Power-of-two LFO avoids expensive divisions in the DMA callback. */
 const uint8_t shift=(uint8_t)(period<2500u?2u:period<4800u?3u:period<9600u?4u:5u);
 const uint32_t p=(age>>shift)&511u;
 return (uint8_t)(p>255u?511u-p:p);
}
#endif
void track_drum_choke_all(struct track_drum_engine *d){
 if(!d)return;
 for(uint8_t i=0u;i<TRACK_DRUM_POLY;i++)if(d->voices[i].active &&
     (!d->voices[i].release_frames||d->voices[i].release_frames>48u))d->voices[i].release_frames=48u;
}
void track_drum_stop(struct track_drum_engine *d){
 if(d)for(unsigned i=0;i<TRACK_DRUM_POLY;i++)d->voices[i].active=0u;
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
 beetle_bd4_live_choke_all();
#endif
}
static int16_t clamp16(int32_t x){return (int16_t)(x>32767?32767:x<-32768?-32768:x);}
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
/* Post-lane stereo saturation. Previously six individually safe channels
 * could add up to a hard 32767 clamp, causing audible edges on dense
 * patterns. Preserve all values in the linear region, apply a low-cost
 * symmetric soft knee only beyond 24k and cap the output at +/-31500.
 * No state, buffers, division, floating point, or allocations in the ISR. */
static int16_t blue_stereo_soft_limit(int32_t x){
 if(x>24000){x=24000+(x-24000)/8;if(x>31500)x=31500;}
 else if(x< -24000){x=-24000+(x+24000)/8;if(x< -31500)x=-31500;}
 return (int16_t)x;
}
#endif

static int32_t get_pcm(uint8_t id,uint32_t sample,const struct beetle_oneshot_bank *bank,uint16_t start){
 if(id>=16u){
  const uint8_t s=(uint8_t)(id-16u);
  if(!bank||s>=BEETLE_ONESHOT_NOTES||sample>=bank->length[s]||
     (uint32_t)start+sample>=BEETLE_ONESHOT_MAX_BYTES)return 0;
  return beetle_ulaw_decode[beetle_oneshot_get(bank,(uint32_t)start+sample)];
 }
 uint8_t i=rom_index[id];
 uint32_t off=beetle_drum_offsets[i]+sample;
 if(off>=beetle_drum_offsets[i+1u])return 0;
 return beetle_ulaw_decode[beetle_drum_rom[off]];
}
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
/* Read an earlier PCM source position without any delay buffers, interrupts,
 * flash accesses or PCM history allocations. Used for factory/local PCM only.
 * Procedural/stream sources use lightweight tonal alternatives. */
static int32_t blue_fx_tap(const struct track_drum_voice *v,
 const struct beetle_oneshot_bank *bank,uint32_t idx,uint32_t frames,
 uint32_t start,uint32_t delay,const uint8_t *rom){
 if(!rom && (v->id<16u || v->id>=32u))return 0;
 if(idx<delay)return 0;
 uint32_t pos=idx-delay;
 if(pos>=frames)return 0;
 uint32_t at=start+(v->reverse?frames-1u-pos:pos);
 return rom?beetle_ulaw_decode[rom[at]]:get_pcm(v->id,at,bank,v->oneshot_offset);
}
#endif
/* The audio callback is the only renderer on FM1. Keep the fixed per-chunk
 * voice metadata outside its limited interrupt stack, in the established
 * transient RAM pool. The values are overwritten on every render call. */
static const struct track_drum_timbre *drum_chunk_ctrl[TRACK_DRUM_POLY];
static uint32_t drum_chunk_frames[TRACK_DRUM_POLY];
static uint32_t drum_chunk_floor[TRACK_DRUM_POLY];
static uint32_t drum_chunk_attack[TRACK_DRUM_POLY];
static int16_t drum_eq_coeff[TRACK_DRUM_POLY][3];
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
static const uint8_t *drum_chunk_rom[TRACK_DRUM_POLY];
#endif
extern int16_t *beetle_stem_capture;
extern uint32_t beetle_stem_offset;
void track_drum_render(struct track_drum_engine *d,const struct track_mixer *mix,int16_t *dst,uint32_t frames){
 if(!d||!dst)return;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 /* Zero-work idle: do not run 128x6 EQ/pan loops when no drum is playing.
  * BLUE has no synth stems in this capture buffer. */
 uint8_t active=0u;
 for(uint8_t i=0u;i<TRACK_DRUM_POLY;i++)active|=d->voices[i].active;
 if(!active){
  if(beetle_stem_capture)memset(beetle_stem_capture+(beetle_stem_offset*TRACK_CHANNELS),
     0,(size_t)frames*TRACK_CHANNELS*sizeof(int16_t));
  return;
 }
#endif
 const struct track_mixer_channel *m=mix?&mix->ch[TRACK_DRUM_CHANNEL]:0;
 uint8_t any_solo=0u;if(mix)for(uint8_t i=0;i<TRACK_CHANNELS;i++)any_solo|=mix->ch[i].solo;
 const uint32_t gain=
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 100u; /* all six independent lane gain/mute decisions happen on trigger */
#else
 !m?80u:(m->mute||(any_solo&&!m->solo)?0u:m->volume);
#endif
 uint16_t pan_left,pan_right;
 beetle_stereo_pan_gain(m?m->pan:0,&pan_left,&pan_right);
 /* Drum timbres and playback lengths are stable for each audio chunk.
  * Resolve them once, not 6 voices x 128 samples. This also bounds XIP ROM
  * table access on the audio critical path without altering any PCM math. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 uint8_t fx_active[TRACK_DRUM_POLY];
#endif
 const struct track_drum_timbre **voice_ctrl=drum_chunk_ctrl;
 uint32_t *voice_frames=drum_chunk_frames,*voice_floor=drum_chunk_floor,*voice_attack=drum_chunk_attack;
 for(uint8_t i=0;i<TRACK_DRUM_POLY;i++){
  const struct track_drum_voice *v=&d->voices[i];
  /* Do not resolve EQ, ROM offsets or FX metadata for idle voice slots.
   * This matters on the FM1 when only one of six drum voices is sounding. */
  if(!v->active){
   voice_frames[i]=voice_floor[i]=voice_attack[i]=0u;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   fx_active[i]=0u;drum_chunk_rom[i]=0;
#endif
   continue;
  }
  const struct track_drum_timbre *ctrl=
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  &blue_voice_timbre[(uint8_t)(v-d->voices)];
#else
  drum_control(d,v->component);
#endif
  voice_ctrl[i]=ctrl;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  fx_active[i]=(uint8_t)(blue_voice_fx[i].type[0] |
    blue_voice_fx[i].type[1] | blue_voice_fx[i].type[2]);
#endif
  /* Expensive /100 division is evaluated once per block, not 768 times. */
  drum_eq_coeff[i][0]=(int16_t)(((uint32_t)(50u+ctrl->eq_low)*256u+50u)/100u);
  drum_eq_coeff[i][1]=(int16_t)(((uint32_t)(50u+ctrl->eq_mid)*256u+50u)/100u);
  drum_eq_coeff[i][2]=(int16_t)(((uint32_t)(50u+ctrl->eq_high)*256u+50u)/100u);
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  drum_chunk_rom[i]=v->id<16u?beetle_drum_rom+beetle_drum_offsets[rom_index[v->id]]:0;
#endif
  if(v->active){
   /* Pack [trim_start:16|trim_span:16] into the existing metadata word.
    * This keeps FM1 pool RAM unchanged (no extra per-voice state). Streamed
    * BD4S gets its original 0xffffffff sentinel and is never truncated. */
   if(v->id>=32u&&v->id<64u)voice_frames[i]=0xFFFFFFFFu;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   else if(v->id>=16u&&v->id<32u){
    const uint32_t n=sample_frames(v->id,d->oneshots);
    const uint32_t from=n*v->start_percent/100u,to=n*v->end_percent/100u;
    const uint32_t span=to>from?to-from:0u;
    voice_frames[i]=(from<<16u)|span;
   }
#endif
   else voice_frames[i]=sample_frames(v->id,d->oneshots);
   voice_floor[i]=(uint32_t)ctrl->sustain*16777215u/100u;
   voice_attack[i]=(uint32_t)ctrl->attack*25u;
  }else voice_frames[i]=voice_floor[i]=voice_attack[i]=0u;
 }
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 uint16_t lane_pan_l[TRACK_CHANNELS],lane_pan_r[TRACK_CHANNELS];
 uint8_t lane_enabled[TRACK_CHANNELS];
 for(uint8_t c=0u;c<TRACK_CHANNELS;c++){
  const struct track_mixer_channel *mc=mix?&mix->ch[c]:0;
  lane_enabled[c]=(uint8_t)(!mc||(!mc->mute&&(!any_solo||mc->solo)));
  beetle_stereo_pan_gain(mc?mc->pan:0,&lane_pan_l[c],&lane_pan_r[c]);
 }
#endif
 for(uint32_t f=0;f<frames;f++){
  int32_t mono=0;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  int32_t channel_mono[TRACK_CHANNELS]={0};
#endif
  for(uint8_t i=0;i<TRACK_DRUM_POLY;i++){
   struct track_drum_voice *v=&d->voices[i];if(!v->active)continue;
   const struct track_drum_timbre *ctrl=voice_ctrl[i];
   const uint32_t idx=v->phase>>16u;
   const uint32_t pcm_count=
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
    (v->id>=16u&&v->id<32u)?(voice_frames[i]&65535u):
#endif
    voice_frames[i];
   if(idx>=pcm_count||v->age>=v->length){v->active=0u;continue;}
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
   int16_t streamed=0;
   if(v->id>=32u && v->id<64u){
    int state=beetle_bd4_live_next(i,v->sequence,&streamed);
    if(state<0){v->active=0u;continue;}
    /* A global user sample's DMA phase, AGE, amplitude envelope and gate
     * must never advance without its next interpolated PCM source frame.
     * This also protects the first refill AFTER initial prefetch, which
     * previously skipped the rest of NOTE3/step-1 under flash contention. */
    if(state==0){
     /* A pending, now-choked stream must not retain a producer request
      * indefinitely if its PCM window is missing. No audio was emitted. */
     if(v->release_frames){v->active=0u;beetle_bd4_live_stop(i);}
     continue;
    }
   }
#endif
   const uint32_t frac=v->phase&65535u;
   /* Factory-ROM pointers resolved once per chunk. No repeated mapping,
    * offset lookup or two function-level branch ladders per active voice/sample.
    * The second fetch past the final ROM frame must still decode to ZERO. */
   const uint8_t *rom=
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
 drum_chunk_rom[i];
#else
 v->id<16u?beetle_drum_rom+beetle_drum_offsets[rom_index[v->id]]:0;
#endif
   int32_t wave;
#if defined(BEETLE_NATIVE_V2) && BEETLE_NATIVE_V2
   if(v->id>=32u && v->id<64u){
    /* BD4S playback is already fully resampled to 44.1 kHz by its own
     * RAM-only decoder. Skip the unrelated factory-ROM 64-bit interpolation
     * entirely for global user samples (six voices x every audio frame). */
    wave=streamed;
   }else
#endif
   {
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
    if(v->id>=64u && v->id<80u){
      wave=blue_rom_wave(v,(uint8_t)(v->id-64u),i);
    }else
#endif
    {
    /* Literal reverse addressing for immutable BEETLE ROM and project-local
     * sample PCM. Other voices keep their original forward playhead. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
    const uint32_t rev=(uint32_t)(v->reverse&&pcm_count!=0u);
#else
    /* Non-BLUE legacy builds have no reverse voice field. */
    const uint32_t rev=0u;
#endif
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
    const uint32_t source_start=(v->id>=16u&&v->id<32u)?(voice_frames[i]>>16u):0u;
#else
    const uint32_t source_start=0u;
#endif
    const uint32_t at=source_start+(rev?pcm_count-1u-idx:idx);
    const uint32_t next=rev?at-1u:at+1u;
    /* A final interpolated frame must not fetch outside the trimmed region.
     * This is especially important when REV ends at the original attack. */
    const uint8_t has_next=(uint8_t)(idx+1u<pcm_count);
    const int32_t one=rom?beetle_ulaw_decode[rom[at]]:
         get_pcm(v->id,at,d->oneshots,v->oneshot_offset);
    const int32_t two=!has_next?0:(rom?beetle_ulaw_decode[rom[next]]:
         get_pcm(v->id,next,d->oneshots,v->oneshot_offset));
    wave=one+(int32_t)(((int64_t)(two-one)*frac)>>16u);
    }
   }
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   /* DSP step FX in deterministic order. Delay/reverb use bounded ROM taps
    * for PCM without an SRAM circular buffer. Every branch stays ISR-safe. */
   if(fx_active[i])for(uint8_t j=0u;j<3u;j++){
    const uint8_t type=blue_voice_fx[i].type[j],value=blue_voice_fx[i].value[j];
    if(!type||!value)continue;
    if(type==3u||type==4u||type==5u){
     if(v->id<32u){
      const uint32_t time=(type==3u?2205u:type==4u?623u:24u);
      const uint32_t step=(time*(uint32_t)(20u+value))/100u;
      const uint32_t lag=(step*v->phase_step)>>16u;
      int32_t echo=blue_fx_tap(v,d->oneshots,idx,pcm_count,
       (v->id>=16u&&v->id<32u)?(voice_frames[i]>>16u):0u,lag,rom);
      if(type==4u){
       uint32_t lag2=(lag*3u)/2u;
       echo=(echo+blue_fx_tap(v,d->oneshots,idx,pcm_count,
        (v->id>=16u&&v->id<32u)?(voice_frames[i]>>16u):0u,lag2,rom))/2;
      }
      wave+=(echo*(int32_t)value)/(type==3u?165: type==4u?245:320);
     }else if(v->id>=64u){
      /* MONOLITH has no PCM history; amplitude/tonal echo approximation. */
      uint8_t phase=blue_fx_triangle(v->age,(uint32_t)(150u+value*21u));
      wave+=(wave*(int32_t)(phase*value))/(type==4u?90000:145000);
     }
    }
   }
#endif
   /* Real 3-band sample EQ (low/mid/high); all voice state is independent. */
   v->noise_lp+= (wave-v->noise_lp)>>5u; /* approx 220 Hz */
   v->noise_lp2+=(wave-v->noise_lp2)>>2u;/* approx 2 kHz */
   int32_t low=v->noise_lp,mid=v->noise_lp2-v->noise_lp,hi=wave-v->noise_lp2;
   int32_t level=(int32_t)(((low*drum_eq_coeff[i][0])+(mid*drum_eq_coeff[i][1])+(hi*drum_eq_coeff[i][2]))>>8u);
   if(v->age<16u)level=level*(int32_t)(v->age+1u)/16;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   if(v->reverse && v->id<32u){
    /* A reversed one-shot ends on its original attack. Apply a bounded
     * 32-frame exit ramp to prevent an abrupt full-scale PCM discontinuity. */
    uint32_t remaining=pcm_count-idx;
    uint32_t gate_remaining=v->length-v->age;
    if(gate_remaining<remaining)remaining=gate_remaining;
    if(remaining<32u)level=(int32_t)(((int64_t)level*remaining)/32u);
   }
#endif
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   if(fx_active[i])for(uint8_t j=0u;j<3u;j++){
    const uint8_t type=blue_voice_fx[i].type[j],value=blue_voice_fx[i].value[j];
    if(!type)continue;
    if(type==1u){
     /* FILTER: continuously blend the independent per-voice low-pass. */
     level=(level*(int32_t)(100u-value)+v->noise_lp2*(int32_t)value)/100;
    }else if(type==2u){
     int32_t hot=level*(int32_t)(100u+value*2u)/100;
     if(hot>18000)hot=18000+(hot-18000)/4;
     if(hot< -18000)hot=-18000+(hot+18000)/4;
     level=hot>28000?28000:hot< -28000?-28000:hot;
    }else if(type==9u){
     const uint32_t rate=900u+(uint32_t)(100u-value)*30u;
     uint32_t modulation=blue_fx_triangle(v->age,rate);
     level=(int32_t)((level*(int32_t)(255u-value*2u/3u+(modulation*value*2u)/(3u*255u)))/255);
    }
   }
#endif
   if(voice_attack[i] && v->age<voice_attack[i]){uint32_t a=v->age*32768u/voice_attack[i];level=(int32_t)((level*(int32_t)a)>>15u);}
   /* V0.5: bounded optional per-pad drive. Neutral=0 follows bit-for-bit
    * identical PCM math. Fixed-point, no buffers or audio-callback malloc.
    * A piecewise knee keeps post-drive peaks bounded before lane summing. */
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   if(ctrl->drive){
    int32_t colored=(int32_t)((level*(int32_t)(256u+3u*ctrl->drive))>>8u);
    uint8_t shift=ctrl->drive>=70u?2u:ctrl->drive>=35u?1u:0u;
    if(colored>14000)colored=14000+((colored-14000)>>shift);
    else if(colored< -14000)colored=-14000-(((-colored)-14000)>>shift);
    if(colored>30000)colored=30000;
    else if(colored< -30000)colored=-30000;
    level=colored;
   }
#endif
   /* Q15 envelope: after the 30k drive knee this product fits int32.
    * Avoid emulated 64-bit multiply on each active voice/sample. */
   level=(int32_t)((level*(int32_t)(v->env_q24>>9u))>>15u);
   /* After the optional drive knee, normal unboosted EQ is bounded by
    * ROM PCM amplitudes; with documented gain<=100%, the product fits
    * signed 32-bit. A 64-bit multiply in the inner loop is unnecessary. */
   level=(int32_t)((level*(int32_t)v->gain_q8)>>8u);
   if(v->release_frames)level=level*v->release_frames/48;
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   channel_mono[v->source_lane<TRACK_CHANNELS?v->source_lane:TRACK_DRUM_CHANNEL]+=(level*149)>>8u;
#else
   mono+=(level*7)/12; /* V0.46: +4.86 dB native drum gain, independent of mixer fader.
                       * 6-voice mix still keeps downstream master headroom. */
#endif
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
   uint32_t phase_step=v->phase_step;
   if(fx_active[i])for(uint8_t j=0u;j<3u;j++){
    const uint8_t type=blue_voice_fx[i].type[j],value=blue_voice_fx[i].value[j];
    if(!value)continue;
    if(type==6u || type==5u){
     const uint32_t cycle=(uint32_t)(type==5u?850u:1400u)+(100u-value)*25u;
     int32_t triangle=(int32_t)blue_fx_triangle(v->age,cycle)-128;
     int32_t mod=(int32_t)((phase_step>>9u)*value*triangle/12800);
     if(mod<0 && (uint32_t)(-mod)>=phase_step)phase_step=1u;
     else phase_step=(uint32_t)((int32_t)phase_step+mod);
    }else if(type==7u){
     /* Arpeggiator: three tones across the step; no note retrigger. */
     const uint32_t period=(uint32_t)(3000u+(100u-value)*80u);
     uint8_t part=(uint8_t)((v->age/period)%3u);
     phase_step=(uint32_t)((uint64_t)phase_step*(part==0u?256u:part==1u?323u:384u)>>8u);
    }
   }
   v->phase+=phase_step;
#else
   v->phase+=v->phase_step;
#endif
   v->age++;
   /* Error <1/4096 of full-scale vs. old 64-bit envelope multiply. */
   uint32_t loss=((v->env_q24>>12u)*v->env_loss_q16+8u)>>4u;
   if(!loss&&v->env_q24)loss=1u;
   uint32_t next=v->env_q24>loss?v->env_q24-loss:0u;
   v->env_q24=next<voice_floor[i]?voice_floor[i]:next;
   if(v->release_frames){v->release_frames--;if(!v->release_frames)v->active=0u;}
  }
#if defined(BEETLE_DRUM_EDITION) && BEETLE_DRUM_EDITION
  int32_t left=dst[f*2u],right=dst[f*2u+1u];
  for(uint8_t ch=0u;ch<TRACK_CHANNELS;ch++){
   int32_t one=channel_mono[ch];
   if(!lane_enabled[ch])one=0;
   /* Existing velocity/fader gain is applied at trigger; do not amplify twice. */
   if(one>24000)one=24000+(one-24000)/4;
   else if(one< -24000)one=-24000+(one+24000)/4;
   if(one>31500)one=31500;if(one< -31500)one=-31500;
   int32_t absone=one<0?-one:one;
   if((uint32_t)absone>track_lane_pcm_peak[ch])track_lane_pcm_peak[ch]=(uint16_t)absone;
   if(beetle_stem_capture)
    beetle_stem_capture[(beetle_stem_offset+f)*TRACK_CHANNELS+ch]=(int16_t)one;
   left+=(int32_t)((one*(int32_t)lane_pan_l[ch])>>8);
   right+=(int32_t)((one*(int32_t)lane_pan_r[ch])>>8);
  }
  dst[f*2u]=blue_stereo_soft_limit(left);dst[f*2u+1u]=blue_stereo_soft_limit(right);
#else
  mono=(mono*(int32_t)gain*(int32_t)d->step_gain)/10000;
  /* Soft knee only for exceptionally dense drum hits: prevent six-voice
   * clipping without sacrificing the increased single-drum attack. */
  if(mono>24000)mono=24000+(mono-24000)/4;
  else if(mono< -24000)mono=-24000+(mono+24000)/4;
  if(mono>31500)mono=31500;else if(mono< -31500)mono=-31500;
  uint32_t peak=(uint32_t)(mono<0?-(int64_t)mono:mono);
  if(peak>32767u)peak=32767u;
  if(peak>track_lane_pcm_peak[TRACK_DRUM_CHANNEL])track_lane_pcm_peak[TRACK_DRUM_CHANNEL]=(uint16_t)peak;
  if(beetle_stem_capture)
   beetle_stem_capture[(beetle_stem_offset+f)*TRACK_CHANNELS+TRACK_DRUM_CHANNEL]=(int16_t)mono;
  int32_t l=dst[f*2u]+(int32_t)(((int64_t)mono*pan_left)>>8);
  int32_t r=dst[f*2u+1u]+(int32_t)(((int64_t)mono*pan_right)>>8);
  dst[f*2u]=clamp16(l);dst[f*2u+1u]=clamp16(r);
#endif
 }
}

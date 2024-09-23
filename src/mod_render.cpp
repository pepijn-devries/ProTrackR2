#include <cpp11.hpp>
#include "get_mod.h"
#include "pt2-clone/pt2_replayer_light.h"
#include "pt2-clone/pt2_config.h"
#include "pt2-clone/pt2_audio.h"
#include "pt2-clone/pt2_downsample2x.h"
using namespace cpp11;

[[cpp11::register]]
SEXP render_mod_(SEXP mod, doubles render_duration) {
  module_t *my_song = get_mod(mod);
  if (render_duration.size() != 1)
    Rf_error("Arguments should have length 1");
  
  paulaSetup(config.mod2WavOutputFreq * 2, audio.amigaModel);
  song = my_song;
  restartSong();
  clearMixerDownsamplerStates();
  editor.mod2WavOngoing = false; // must be set before calling resetAudio()
  audio.oversamplingFlag = (audio.outputRate < 96000);
  const int32_t paulaMixFrequency = audio.oversamplingFlag ? audio.outputRate*2 : audio.outputRate;
  paulaSetup(paulaMixFrequency, audio.amigaModel);
  generateBpmTable(audio.outputRate, editor.timingMode == TEMPO_MODE_VBLANK);
  clearMixerDownsamplerStates();
  modSetTempo(song->currBPM, true); // update BPM (samples per tick) with the tracker's audio frequency
  
  uint32_t total_samples = round(audio.outputRate * render_duration.at(0) * 2); // 2 for stereo
  uint32_t n_rendered = 0;
  writable::integers result((R_xlen_t)(total_samples));
  uint64_t samplesToMixFrac = 0;
  
  while (n_rendered < total_samples) {
    uint32_t samplesToMix = audio.samplesPerTickInt;
    
    samplesToMixFrac += audio.samplesPerTickFrac;
    if (samplesToMixFrac >= BPM_FRAC_SCALE) {
      samplesToMixFrac &= BPM_FRAC_MASK;
      samplesToMix++;
    }
    
    intMusic();
    int16_t buffer[samplesToMix * 2] = {0};
    outputAudio(buffer, samplesToMix);
    uint32_t i;
    for (i = 0; i < samplesToMix*2; i++) {
      if ((i + n_rendered) >= total_samples) break;
      result.at((int)(i + n_rendered)) = buffer[i];
    }
    n_rendered += samplesToMix*2;
  }  
  return result;
}
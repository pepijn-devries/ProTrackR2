#include <cpp11.hpp>
#include <vector>
#include "get_mod.h"
#include "pt2-clone.h"
using namespace cpp11;

static void calcMod2WavTotalRows(int16_t start_pos);

void reset_speed(list render_options) {
  
  int speed = integers(render_options["speed"]).at(0);
  int tempo = integers(render_options["tempo"]).at(0);
  if (speed < 1 || speed > 0x1F) speed = 6;
  if (tempo < 0x20 || tempo > 0xFF) tempo = 125;
  
  modSetTempo(tempo, true);
  modSetSpeed(speed);
}

double render_prep(SEXP mod, double render_duration, list render_options, int position) {
  module_t *my_song = get_mod(mod);

  config.mod2WavOutputFreq = audio.outputRate = integers(render_options["sample_rate"]).at(0);
  config.stereoSeparation = integers(render_options["stereo_separation"]).at(0);
  config.amigaModel = MODEL_A500;
  if (strings(render_options["amiga_filter"]).at(0) == r_string("A1200"))
    config.amigaModel = MODEL_A1200;
  
  setAmigaFilterModel(config.amigaModel);
  audioSetStereoSeparation(config.stereoSeparation);
  paulaSetup(config.mod2WavOutputFreq * 2, audio.amigaModel);
  
  song = my_song;
  
  restartSong();
  int16_t mpos = (uint16_t)position;
  if (mpos < 0 || mpos >= song->header.songLength)
    stop("'position' is out of range!");
  
  modSetPos(mpos, 0);
  initializeModuleChannels(song);
  modSetPattern(song->header.patternTable[mpos]);
  
  clearMixerDownsamplerStates();
  editor.mod2WavOngoing = false; // must be set before calling resetAudio()
  audio.oversamplingFlag = (config.mod2WavOutputFreq < 96000);
  const int32_t paulaMixFrequency = audio.oversamplingFlag ? config.mod2WavOutputFreq*2 : config.mod2WavOutputFreq;
  paulaSetup(paulaMixFrequency, audio.amigaModel);
  generateBpmTable(config.mod2WavOutputFreq, editor.timingMode == TEMPO_MODE_VBLANK);
  updateReplayerTimingMode();
  clearMixerDownsamplerStates();
  reset_speed(render_options);
  
  double dur = render_duration;
  if (R_IsNA(dur)) {
    calcMod2WavTotalRows(mpos);
    dur = (double)song->songDuration;
  }
  return dur;
}

[[cpp11::register]]
integers render_mod_(SEXP mod, double render_duration, list render_options, int position) {
  double dur = render_prep(mod, render_duration, render_options, position);
  
  uint32_t total_samples = round(audio.outputRate * dur) * 2; // 2 for stereo
  uint32_t n_rendered = 0;
  writable::integers result((R_xlen_t)(total_samples));
  uint64_t samplesToMixFrac = 0;
  
  setLEDFilter(logicals(render_options["led_filter"]).at(0));

  int16_t mpos = (uint16_t)position;
  modSetPos(mpos, 0);
  reset_speed(render_options);

  initializeModuleChannels(song);
  modSetPattern(song->header.patternTable[mpos]);
  
  for (int i = 0; i < (int)song->currSpeed; i++) {
    intMusic(); // skip currSpeed ticks as they are empty.
  }
  
  while (n_rendered < total_samples) {
    uint32_t samplesToMix = audio.samplesPerTickInt;
    
    samplesToMixFrac += audio.samplesPerTickFrac;
    if (samplesToMixFrac >= BPM_FRAC_SCALE) {
      samplesToMixFrac &= BPM_FRAC_MASK;
      samplesToMix++;
    }
    
    intMusic();
    std::vector<int16_t>buff(samplesToMix * 2);
    int16_t * buffer = buff.data();
    
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

[[cpp11::register]]
double mod_duration(SEXP mod, list render_options, int position) {
  return render_prep(mod, NA_REAL, render_options, position);
}

#define CALC__END_OF_SONG                                           \
if (--numLoops < 0)                                                 \
{                                                                   \
  song->songDuration = (float)sample_count/(float)audio.outputRate; \
  calcingRows = false;                                              \
  break;                                                            \
}                                                                   \
else                                                                \
{                                                                   \
  memset(editor.rowVisitTable, 0, 128 * MOD_ROWS);                  \
}

// ONLY used for a visual percentage counter, so accuracy is not very important
static void calcMod2WavTotalRows(int16_t start_pos)
{
  modSetPos(start_pos, 0);
  
  int8_t n_pattpos[PAULA_VOICES], n_loopcount[PAULA_VOICES];
  
  // for pattern loop
  memset(n_pattpos,   0, sizeof (n_pattpos));
  memset(n_loopcount, 0, sizeof (n_loopcount));
  
  song->rowsCounter = song->rowsInTotal  = 0;
  uint64_t samplesToMixFrac = 0;
  uint32_t sample_count = 0;
  uint8_t delayTicks = 0;
  
  uint8_t modRow = 0;
  int16_t modPos = start_pos;
  int8_t modPattern = song->header.patternTable[modPos];
  uint8_t pBreakPosition = 0;
  bool posJumpAssert = false;
  bool pBreakFlag = false;
  
  memset(editor.rowVisitTable, 0, 128 * MOD_ROWS);
  
  int8_t numLoops = editor.mod2WavNumLoops; // make a copy
  
  bool calcingRows = true;
  while (calcingRows)
  {
    editor.rowVisitTable[(modPos * MOD_ROWS) + modRow] = true;
    
    for (int32_t ch = 0; ch < PAULA_VOICES; ch++)
    {
      delayTicks = 0;
      
      note_t *note = &song->patterns[modPattern][(modRow * PAULA_VOICES) + ch];
      if (note->command == 0x0B) // Bxx - Position Jump
      {
        modPos = note->param - 1;
        pBreakPosition = 0;
        posJumpAssert = true;
      }
      else if (note->command == 0x0D) // Dxx - Pattern Break
      {
        pBreakPosition = (((note->param >> 4) * 10) + (note->param & 0x0F));
        if (pBreakPosition > 63)
          pBreakPosition = 0;
        
        posJumpAssert = true;
      }
      else if (note->command == 0x0F)
      {
        if (note->param == 0) {
          CALC__END_OF_SONG
        } else {
          if (editor.timingMode == TEMPO_MODE_VBLANK || (note->param & 0xFF) < 32)
            modSetSpeed(note->param & 0xFF);
          else
            modSetTempo(note->param & 0xFF, false);
        }
      }
      else if (note->command == 0x0E && (note->param >> 4) == 0x06) // E6x - Pattern Loop
      {
        uint8_t pos = note->param & 0x0F;
        if (pos == 0)
        {
          n_pattpos[ch] = modRow;
        }
        else if (n_loopcount[ch] == 0)
        {
          n_loopcount[ch] = pos;

          pBreakPosition = n_pattpos[ch];
          pBreakFlag = true;
          
          for (pos = pBreakPosition; pos <= modRow; pos++)
            editor.rowVisitTable[(modPos * MOD_ROWS) + pos] = false;
        }
        else if (--n_loopcount[ch])
        {
          pBreakPosition = n_pattpos[ch];
          pBreakFlag = true;
          
          for (pos = pBreakPosition; pos <= modRow; pos++)
            editor.rowVisitTable[(modPos * MOD_ROWS) + pos] = false;
        }
      }
      else if (note->command == 0x0E && (note->param >> 4) == 0x0E) // EEx - Delay effect
      {
        delayTicks = note->param & 0x0F;
      }
    }
    
    modRow++;
    song->rowsInTotal++;

    uint32_t samplesToMix = audio.samplesPerTickInt;
    
    samplesToMixFrac += audio.samplesPerTickFrac;
    if (samplesToMixFrac >= BPM_FRAC_SCALE) {
      samplesToMixFrac &= BPM_FRAC_MASK;
      samplesToMix++;
    }
    
    sample_count += samplesToMix * song->currSpeed * (delayTicks + 1);

    if (pBreakFlag)
    {
      modRow = pBreakPosition;
      pBreakPosition = 0;
      pBreakFlag = false;
    }
    
    if (modRow >= MOD_ROWS || posJumpAssert)
    {
      modRow = pBreakPosition;
      pBreakPosition = 0;
      posJumpAssert = false;
      
      modPos = (modPos + 1) & 127;
      if (modPos >= song->header.songLength)
      {
        modPos = 0;
        CALC__END_OF_SONG
      }
      
      modPattern = song->header.patternTable[modPos];
      if (modPattern > MAX_PATTERNS-1)
        modPattern = MAX_PATTERNS-1;
    }
    
    if (calcingRows && editor.rowVisitTable[(modPos * MOD_ROWS) + modRow])
    {
      CALC__END_OF_SONG
    }
  }
}

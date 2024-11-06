// C port of ProTracker 2.3D's replayer (with some modifications, but still accurate)

// for finding memory leaks in debug mode with Visual Studio 
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pt2_audio.h"
#include "pt2_helpers.h"
#include "pt2_tables.h"
#include "pt2_config.h"
#include "pt2_paula.h"

static bool posJumpAssert, pBreakFlag, modRenderDone;
static bool doStopSong; // from F00 (Set Speed)
static int8_t pBreakPosition, modPattern;
static uint8_t pattDelTime, lowMask = 0xFF, pattDelTime2;
static int16_t modPos;
static uint16_t DMACONtemp;
static int32_t modBPM, ciaSetBPM;

static const uint8_t funkTable[16] = // EFx (FunkRepeat/InvertLoop)
  {
  0x00, 0x05, 0x06, 0x07, 0x08, 0x0A, 0x0B, 0x0D,
  0x10, 0x13, 0x16, 0x1A, 0x20, 0x2B, 0x40, 0x80
  };

void initializeModuleChannels(module_t *m)
{
  assert(m != NULL);
  
  memset(m->channels, 0, sizeof (m->channels));
  
  moduleChannel_t *ch = m->channels;
  for (uint8_t i = 0; i < PAULA_VOICES; i++, ch++)
  {
    ch->n_chanindex = i;
    ch->n_dmabit = 1 << i;
  }
}

module_t *createEmptyMod(void)
{
  module_t *m = (module_t *)calloc(1, sizeof (module_t));
  if (m == NULL)
    goto error;
  
  // allocate memory for all sample data blocks (+ 2 extra, for quirk + safety)
  const size_t allocLen = (MOD_SAMPLES + 2) * config.maxSampleLength;
  
  m->sampleData = (int8_t *)calloc(allocLen, 1);
  if (m->sampleData == NULL)
    goto error;
  
  for (int32_t i = 0; i < MAX_PATTERNS; i++)
  {
    m->patterns[i] = (note_t *)calloc(1, MOD_ROWS * sizeof (note_t) * PAULA_VOICES);
    if (m->patterns[i] == NULL)
      goto error;
  }
  
  m->header.songLength = 1;
  
  moduleSample_t *s = m->samples;
  for (int32_t i = 0; i < MOD_SAMPLES; i++, s++)
  {
    s->loopLength = 2;
    
    // sample data offsets (sample data = one huge buffer to rule them all)
    s->offset = config.maxSampleLength * i;
  }
  
  initializeModuleChannels(m);
  
  return m;
  
  error:
    if (m != NULL)
    {
      for (int32_t i = 0; i < MAX_PATTERNS; i++)
      {
        if (m->patterns[i] != NULL)
          free(m->patterns[i]);
      }
      
      if (m->sampleData != NULL)
        free(m->sampleData);
      
      free(m);
    }
    
    return NULL;
}

void turnOffVoices(void)
{

  paulaWriteWord(0xDFF096, 0x000F); // turn off all voice DMAs

  // clear all volumes
  for (int32_t i = 0; i < PAULA_VOICES; i++)
  {
    const uint32_t voiceAddr = 0xDFF0A0 + (i * 16);
    paulaWriteWord(voiceAddr + 8, 0);
    
  }
  
  editor.tuningToneFlag = false;
}

double ciaBpm2Hz(int32_t bpm)
{
  if (bpm == 0)
    return 0.0;
  
  const uint32_t ciaPeriod = 1773447 / bpm; // yes, PT truncates here
  return (double)CIA_PAL_CLK / (ciaPeriod+1); // +1, CIA triggers on underflow
}

void modFree2(module_t *my_song)
{
  if (my_song == NULL)
    return; // not allocated
  
  turnOffVoices();
  
  for (int32_t i = 0; i < MAX_PATTERNS; i++)
  {
    if (my_song->patterns[i] != NULL)
      free(my_song->patterns[i]);
  }
  
  if (my_song->sampleData != NULL)
    free(my_song->sampleData);
  
  free(my_song);

}

void updatePaulaLoops(void) // used after manipulating sample loop points while Paula is live
{

  for (int32_t i = 0; i < PAULA_VOICES; i++)
  {
    const moduleChannel_t *ch = &song->channels[i];
    if (ch->n_samplenum == editor.currSample) // selected sample matches channel's sample?
    {
      const moduleSample_t *s = &song->samples[editor.currSample];
      
      const uint32_t voiceAddr = 0xDFF0A0 + (i * 16);
      
      // set voice data ptr and data length
      paulaWritePtr(voiceAddr + 0, ch->n_start + s->loopStart);
      paulaWriteWord(voiceAddr + 4, (uint16_t)(s->loopLength >> 1));
      
    }
  }
  
}

void doStopIt(bool resetPlayMode)
{

  editor.songPlaying = false;
  
  pattDelTime = pattDelTime2 = 0;
  
  if (resetPlayMode)
  {
    editor.playMode = PLAY_MODE_NORMAL;
    editor.currMode = MODE_IDLE;
    
  }
  
  if (song != NULL)
  {
    moduleChannel_t *ch = song->channels;
    for (int32_t i = 0; i < PAULA_VOICES; i++, ch++)
    {
      ch->n_wavecontrol = 0;
      ch->n_glissfunk = 0;
      ch->n_finetune = 0;
      ch->n_loopcount = 0;
    }
  }
  
  doStopSong = false; // just in case this flag was stuck from command F00 (stop song)
  
}

void modPlay(int16_t patt, int16_t pos, int8_t row)
{

  doStopIt(false);
  turnOffVoices();
  
  if (row != -1)
  {
    if (row >= 0 && row <= 63)
    {
      song->row = row;
      song->currRow = row;
    }
  }
  else
  {
    song->row = 0;
    song->currRow = 0;
  }
  
  if (editor.playMode != PLAY_MODE_PATTERN)
  {
    if (modPos >= song->header.songLength)
      song->currPos = modPos = 0;
    
    if (pos >= 0 && pos < song->header.songLength)
      song->currPos = modPos = pos;
    
    if (pos >= song->header.songLength)
      song->currPos = modPos = 0;
  }
  
  if (patt >= 0 && patt <= MAX_PATTERNS-1)
    song->currPattern = modPattern = (int8_t)patt;
  else
    song->currPattern = modPattern = (int8_t)song->header.patternTable[modPos];
  
  song->tick = song->speed-1;
  ciaSetBPM = -1; // fix possibly stuck "set BPM" flag
  
  modRenderDone = false;
  editor.songPlaying = true;
  editor.didQuantize = false;
  
  // don't reset playback counter in "play/rec pattern" mode
  if (editor.playMode != PLAY_MODE_PATTERN)
  {
    editor.playbackSeconds = 0;
    editor.playbackSecondsFrac = 0;
  }
  
  audio.tickSampleCounter = 0; // zero tick sample counter so that it will instantly initiate a tick
  audio.tickSampleCounterFrac = 0;
  
}

void modSetPos(int16_t pos, int16_t row)
{
  if (row != -1)
  {
    row = CLAMP(row, 0, 63);
    
    song->tick = 0;
    song->row = (int8_t)row;
    song->currRow = (int8_t)row;
  }
  
  if (pos != -1)
  {
    if (pos >= 0)
    {
      song->currPos = modPos = pos;

      if (editor.currMode == MODE_PLAY && editor.playMode == PLAY_MODE_NORMAL)
      {
        modPattern = (int8_t)song->header.patternTable[pos];
        if (modPattern > MAX_PATTERNS-1)
          modPattern = MAX_PATTERNS-1;
        
        song->currPattern = modPattern;
      }
      
    }
  }
}

void modSetTempo(int32_t bpm, bool doLockAudio)
{
  if (bpm < MIN_BPM || bpm > MAX_BPM)
    return;
  
  modBPM = bpm;
  if (!editor.pat2SmpOngoing && !editor.mod2WavOngoing)
    song->currBPM = bpm;

  const int32_t i = bpm - MIN_BPM; // 32..255 -> 0..223
  
  audio.samplesPerTickInt = audio.samplesPerTickIntTab[i];
  audio.samplesPerTickFrac = audio.samplesPerTickFracTab[i];
  
}

void modSetSpeed(int32_t speed)
{
  song->currSpeed = song->speed = speed;
  song->tick = 0;
}

void modSetPattern(uint8_t pattern)
{
  modPattern = pattern;
  song->currPattern = modPattern;
}

void restartSong(void) // for the beginning of MOD2WAV/PAT2SMP
{

  editor.playMode = PLAY_MODE_NORMAL;
  editor.blockMarkFlag = false;
  
  song->row = 0;
  song->currRow = 0;
  song->rowsCounter = 0;
  
  memset(editor.rowVisitTable, 0, 128 * MOD_ROWS); // for MOD2WAV
  
  if (editor.pat2SmpOngoing)
  {
    modSetSpeed(song->currSpeed);
    modSetTempo(song->currBPM, true);
    modPlay(DONT_SET_PATTERN, DONT_SET_ORDER, 0);
  }
  else
  {
    song->currSpeed = 6;
    song->currBPM = 125;
    modSetSpeed(song->currSpeed);
    modSetTempo(song->currBPM, true);
    
    modPlay(DONT_SET_PATTERN, 0, 0);
  }
}

static void nextPosition(void)
{
  if (editor.pat2SmpOngoing)
    modRenderDone = true;
  
  song->row = pBreakPosition;
  pBreakPosition = 0;
  posJumpAssert = false;
  
  if (editor.playMode != PLAY_MODE_PATTERN ||
      (editor.currMode == MODE_RECORD && editor.recordMode != RECORD_PATT))
  {
    if (editor.stepPlayEnabled)
    {
      if (config.keepEditModeAfterStepPlay && editor.stepPlayLastMode == MODE_EDIT)
      {
        doStopIt(false);
        
        editor.playMode = PLAY_MODE_NORMAL;
        editor.currMode = MODE_EDIT;
        // pointerSetModeThreadSafe(POINTER_MODE_EDIT, true);
      }
      else
      {
        doStopIt(true);
      }
      
      editor.stepPlayEnabled = false;
      editor.stepPlayBackwards = false;
      
      if (editor.stepPlayLastMode == MODE_EDIT || editor.stepPlayLastMode == MODE_IDLE)
      {
        song->row &= 63;
        song->currRow = song->row;
      }
      else
      {
        // if we were playing, set replayer row to tracker row (stay in sync)
        song->currRow &= 63;
        song->row = song->currRow;
      }
      return;
    }
    
    modPos = (modPos + 1) & 127;
    if (modPos >= song->header.songLength)
    {
      modPos = 0;
      
      if (config.compoMode) // stop song for music competitions playing
      {
        doStopIt(true);
        turnOffVoices();

        modPos = 0;
        modPattern = (int8_t)song->header.patternTable[modPos];
        song->row = 0;
      }
      
      if (editor.mod2WavOngoing)
        modRenderDone = true;
    }
    
    modPattern = (int8_t)song->header.patternTable[modPos];
    if (modPattern > MAX_PATTERNS-1)
      modPattern = MAX_PATTERNS-1;
  }
}

static void updateFunk(moduleChannel_t *ch)
{
  const int8_t funkSpeed = ch->n_glissfunk >> 4;
  if (funkSpeed == 0)
    return;
  
  ch->n_funkoffset += funkTable[funkSpeed];
  if (ch->n_funkoffset >= 128)
  {
    ch->n_funkoffset = 0;
    
    if (ch->n_loopstart != NULL && ch->n_wavestart != NULL) // ProTracker bugfix
    {
      if (++ch->n_wavestart >= ch->n_loopstart + (ch->n_replen << 1))
        ch->n_wavestart = ch->n_loopstart;
      
      *ch->n_wavestart = -1 - *ch->n_wavestart;
    }
  }
}

static void setGlissControl(moduleChannel_t *ch)
{
  ch->n_glissfunk = (ch->n_glissfunk & 0xF0) | (ch->n_cmd & 0x0F);
}

static void setVibratoControl(moduleChannel_t *ch)
{
  ch->n_wavecontrol = (ch->n_wavecontrol & 0xF0) | (ch->n_cmd & 0x0F);
}

static void setFineTune(moduleChannel_t *ch)
{
  ch->n_finetune = ch->n_cmd & 0xF;
}

static void jumpLoop(moduleChannel_t *ch)
{
  if (song->tick != 0)
    return;
  
  if ((ch->n_cmd & 0xF) == 0)
  {
    ch->n_pattpos = song->row;
  }
  else
  {
    if (ch->n_loopcount == 0)
      ch->n_loopcount = ch->n_cmd & 0xF;
    else if (--ch->n_loopcount == 0)
      return;
    
    pBreakPosition = ch->n_pattpos;
    pBreakFlag = true;
    
    // stuff used for MOD2WAV to determine if the song has reached its end
    if (editor.mod2WavOngoing)
    {
      for (int32_t tempParam = pBreakPosition; tempParam <= song->row; tempParam++)
        editor.rowVisitTable[(modPos * MOD_ROWS) + tempParam] = false;
    }
  }
}

static void setTremoloControl(moduleChannel_t *ch)
{
  ch->n_wavecontrol = ((ch->n_cmd & 0xF) << 4) | (ch->n_wavecontrol & 0xF);
}

/* This is a little used effect, despite being present in original ProTracker.
 ** E8x was sometimes entirely replaced with code used for demo fx syncing in
 ** demo mod players, so it can be turned off by looking at DISABLE_E8X in
 ** protracker.ini if you so desire.
 */
static void karplusStrong(moduleChannel_t *ch)
{
  int8_t a, b;
  
  if (config.disableE8xEffect)
    return;
  
  if (ch->n_loopstart == NULL)
    return; // ProTracker bugfix
  
  int8_t *ptr8 = ch->n_loopstart;
  int16_t end = ((ch->n_replen * 2) & 0xFFFF) - 2;
  do
  {
    a = ptr8[0];
    b = ptr8[1];
    *ptr8++ = (a + b) >> 1;
  }
  while (--end >= 0);
  
  a = ptr8[0];
  b = ch->n_loopstart[0];
  *ptr8 = (a + b) >> 1;
}

static void doRetrg(moduleChannel_t *ch)
{
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  
  // voice DMA off
  paulaWriteWord(0xDFF096, ch->n_dmabit); 
  
  // set voice data ptr, data length and period
  paulaWritePtr(voiceAddr + 0, ch->n_start); // n_start is increased on 9xx
  paulaWriteWord(voiceAddr + 4, ch->n_length);
  paulaWriteWord(voiceAddr + 6, ch->n_period);
  
  // voice DMA on
  paulaWriteWord(0xDFF096, 0x8000 | ch->n_dmabit); 
  
  // set new data ptr and data length (these take effect after the current DMA cycle is done)
  paulaWritePtr(voiceAddr + 0, ch->n_loopstart);
  paulaWriteWord(voiceAddr + 4, ch->n_replen);
  
}

static void retrigNote(moduleChannel_t *ch)
{
  if ((ch->n_cmd & 0xF) > 0)
  {
    if (song->tick == 0 && (ch->n_note & 0xFFF) > 0)
      return;
    
    if (song->tick % (ch->n_cmd & 0xF) == 0)
      doRetrg(ch);
  }
}

static void volumeSlide(moduleChannel_t *ch)
{
  uint8_t param = ch->n_cmd & 0xFF;
  if ((param & 0xF0) == 0)
  {
    ch->n_volume -= param & 0x0F;
    if (ch->n_volume < 0)
      ch->n_volume = 0;
  }
  else
  {
    ch->n_volume += param >> 4;
    if (ch->n_volume > 64)
      ch->n_volume = 64;
  }
}

static void volumeFineUp(moduleChannel_t *ch)
{
  if (song->tick == 0)
  {
    ch->n_volume += ch->n_cmd & 0xF;
    if (ch->n_volume > 64)
      ch->n_volume = 64;
  }
}

static void volumeFineDown(moduleChannel_t *ch)
{
  if (song->tick == 0)
  {
    ch->n_volume -= ch->n_cmd & 0xF;
    if (ch->n_volume < 0)
      ch->n_volume = 0;
  }
}

static void noteCut(moduleChannel_t *ch)
{
  if (song->tick == (ch->n_cmd & 0xF))
    ch->n_volume = 0;
}

static void noteDelay(moduleChannel_t *ch)
{
  if (song->tick == (ch->n_cmd & 0xF) && (ch->n_note & 0xFFF) > 0)
    doRetrg(ch);
}

static void patternDelay(moduleChannel_t *ch)
{
  if (song->tick == 0 && pattDelTime2 == 0)
    pattDelTime = (ch->n_cmd & 0xF) + 1;
}

static void funkIt(moduleChannel_t *ch)
{
  if (song->tick == 0)
  {
    ch->n_glissfunk = ((ch->n_cmd & 0xF) << 4) | (ch->n_glissfunk & 0xF);
    
    if ((ch->n_glissfunk & 0xF0) > 0)
      updateFunk(ch);
  }
}

static void positionJump(moduleChannel_t *ch)
{
  // original PT doesn't do this check, but we have to
  if (editor.playMode != PLAY_MODE_PATTERN || (editor.currMode == MODE_RECORD && editor.recordMode != RECORD_PATT))
    modPos = (ch->n_cmd & 0xFF) - 1; // B00 results in -1, but it safely jumps to order 0
  
  pBreakPosition = 0;
  posJumpAssert = true;
}

static void volumeChange(moduleChannel_t *ch)
{
  ch->n_volume = ch->n_cmd & 0xFF;
  if ((uint8_t)ch->n_volume > 64)
    ch->n_volume = 64;
}

static void patternBreak(moduleChannel_t *ch)
{
  pBreakPosition = (((ch->n_cmd & 0xF0) >> 4) * 10) + (ch->n_cmd & 0x0F);
  if ((uint8_t)pBreakPosition > 63)
    pBreakPosition = 0;
  
  posJumpAssert = true;
}

static void setSpeed(moduleChannel_t *ch)
{
  if ((ch->n_cmd & 0xFF) > 0)
  {
    if (editor.timingMode == TEMPO_MODE_VBLANK || (ch->n_cmd & 0xFF) < 32)
      modSetSpeed(ch->n_cmd & 0xFF);
    else
      ciaSetBPM = ch->n_cmd & 0xFF; // the CIA chip doesn't use its new timer value until the next interrupt, so change it later
  }
  else
  {
    // F00 - stop song
    doStopSong = true;
  }
}

static void arpeggio(moduleChannel_t *ch)
{
  int32_t arpNote;
  
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  
  int32_t arpTick = song->tick % 3; // 0, 1, 2
  if (arpTick == 1)
  {
    arpNote = ch->n_cmd >> 4;
  }
  else if (arpTick == 2)
  {
    arpNote = ch->n_cmd & 0xF;
  }
  else // arpTick 0
  {
    // set voice period
    paulaWriteWord(voiceAddr + 6, ch->n_period);
    
    // setVisualsPeriod(ch->n_chanindex, ch->n_period);
    
    return;
  }
  
  /* 8bitbubsy: If the finetune is -1, this can overflow up to
   ** 15 words outside of the table. The table is padded with
   ** the correct overflow values to allow this to safely happen
   ** and sound correct at the same time.
   */
  const int16_t *periods = &periodTable[ch->n_finetune * 37];
  for (int32_t baseNote = 0; baseNote < 37; baseNote++)
  {
    if (ch->n_period >= periods[baseNote])
    {
      // set voice period
      paulaWriteWord(voiceAddr + 6, periods[baseNote+arpNote]);
      
      // setVisualsPeriod(ch->n_chanindex, periods[baseNote+arpNote]);
      
      break;
    }
  }
}

static void portaUp(moduleChannel_t *ch)
{
  ch->n_period -= (ch->n_cmd & 0xFF) & lowMask;
  lowMask = 0xFF;
  
  if ((ch->n_period & 0xFFF) < 113) // PT BUG: sign removed before comparison, underflow not clamped!
    ch->n_period = (ch->n_period & 0xF000) | 113;
  
  // set voice period
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 6, ch->n_period & 0xFFF);
}

static void portaDown(moduleChannel_t *ch)
{
  ch->n_period += (ch->n_cmd & 0xFF) & lowMask;
  lowMask = 0xFF;
  
  if ((ch->n_period & 0xFFF) > 856)
    ch->n_period = (ch->n_period & 0xF000) | 856;
  
  // set voice period
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 6, ch->n_period & 0xFFF);
}

static void filterOnOff(moduleChannel_t *ch)
{
  if (song->tick == 0) // added this (just pointless to call this during all ticks!)
  {
    const bool filterOn = (ch->n_cmd & 1) ^ 1;
    
    // set "LED" filter
    paulaWriteByte(0xBFE001, filterOn << 1);
    audio.ledFilterEnabled = filterOn;
  }
}

static void finePortaUp(moduleChannel_t *ch)
{
  if (song->tick == 0)
  {
    lowMask = 0xF;
    portaUp(ch);
  }
}

static void finePortaDown(moduleChannel_t *ch)
{
  if (song->tick == 0)
  {
    lowMask = 0xF;
    portaDown(ch);
  }
}

static void setTonePorta(moduleChannel_t *ch)
{
  uint16_t note = ch->n_note & 0xFFF;
  const int16_t *portaPointer = &periodTable[ch->n_finetune * 37];
  
  int32_t i = 0;
  while (true)
  {
    // portaPointer[36] = 0, so i=36 is safe
    if (note >= portaPointer[i])
      break;
    
    if (++i >= 37)
    {
      i = 35;
      break;
    }
  }
  
  if ((ch->n_finetune & 8) && i > 0)
    i--;
  
  ch->n_wantedperiod = portaPointer[i];
  ch->n_toneportdirec = 0;
  
  if (ch->n_period == ch->n_wantedperiod) ch->n_wantedperiod = 0;
  else if (ch->n_period > ch->n_wantedperiod) ch->n_toneportdirec = 1;
}

static void tonePortNoChange(moduleChannel_t *ch)
{
  if (ch->n_wantedperiod <= 0)
    return;
  
  if (ch->n_toneportdirec > 0)
  {
    ch->n_period -= ch->n_toneportspeed;
    if (ch->n_period <= ch->n_wantedperiod)
    {
      ch->n_period = ch->n_wantedperiod;
      ch->n_wantedperiod = 0;
    }
  }
  else
  {
    ch->n_period += ch->n_toneportspeed;
    if (ch->n_period >= ch->n_wantedperiod)
    {
      ch->n_period = ch->n_wantedperiod;
      ch->n_wantedperiod = 0;
    }
  }
  
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  
  if ((ch->n_glissfunk & 0xF) == 0)
  {
    // set voice period
    paulaWriteWord(voiceAddr + 6, ch->n_period);
  }
  else
  {
    const int16_t *portaPointer = &periodTable[ch->n_finetune * 37];
    
    int32_t i = 0;
    while (true)
    {
      // portaPointer[36] = 0, so i=36 is safe
      if (ch->n_period >= portaPointer[i])
        break;
      
      if (++i >= 37)
      {
        i = 35;
        break;
      }
    }
    
    // set voice period
    paulaWriteWord(voiceAddr + 6, portaPointer[i]);
    
  }
}

static void tonePortamento(moduleChannel_t *ch)
{
  if ((ch->n_cmd & 0xFF) > 0)
  {
    ch->n_toneportspeed = ch->n_cmd & 0xFF;
    ch->n_cmd &= 0xFF00;
  }
  
  tonePortNoChange(ch);
}

static void vibrato2(moduleChannel_t *ch)
{
  uint16_t vibratoData;
  
  const uint8_t vibratoPos = (ch->n_vibratopos >> 2) & 0x1F;
  const uint8_t vibratoType = ch->n_wavecontrol & 3;
  
  if (vibratoType == 0) // sine
  {
    vibratoData = vibratoTable[vibratoPos];
  }
  else if (vibratoType == 1) // ramp
  {
    if (ch->n_vibratopos < 128)
      vibratoData = vibratoPos << 3;
    else
      vibratoData = 255 - (vibratoPos << 3);
  }
  else // square
  {
    vibratoData = 255;
  }
  
  vibratoData = (vibratoData * (ch->n_vibratocmd & 0xF)) >> 7;
  
  if (ch->n_vibratopos < 128)
    vibratoData = ch->n_period + vibratoData;
  else
    vibratoData = ch->n_period - vibratoData;
  
  // set voice period
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 6, vibratoData);
  
  ch->n_vibratopos += (ch->n_vibratocmd >> 2) & 0x3C;
}

static void vibrato(moduleChannel_t *ch)
{
  if ((ch->n_cmd & 0x0F) > 0)
    ch->n_vibratocmd = (ch->n_vibratocmd & 0xF0) | (ch->n_cmd & 0x0F);
  
  if ((ch->n_cmd & 0xF0) > 0)
    ch->n_vibratocmd = (ch->n_cmd & 0xF0) | (ch->n_vibratocmd & 0x0F);
  
  vibrato2(ch);
}

static void tonePlusVolSlide(moduleChannel_t *ch)
{
  tonePortNoChange(ch);
  volumeSlide(ch);
}

static void vibratoPlusVolSlide(moduleChannel_t *ch)
{
  vibrato2(ch);
  volumeSlide(ch);
}

static void tremolo(moduleChannel_t *ch)
{
  int16_t tremoloData;
  
  if ((ch->n_cmd & 0x0F) > 0)
    ch->n_tremolocmd = (ch->n_tremolocmd & 0xF0) | (ch->n_cmd & 0x0F);
  
  if ((ch->n_cmd & 0xF0) > 0)
    ch->n_tremolocmd = (ch->n_cmd & 0xF0) | (ch->n_tremolocmd & 0x0F);
  
  const uint8_t tremoloPos = (ch->n_tremolopos >> 2) & 0x1F;
  const uint8_t tremoloType = (ch->n_wavecontrol >> 4) & 3;
  
  if (tremoloType == 0) // sine
  {
    tremoloData = vibratoTable[tremoloPos];
  }
  else if (tremoloType == 1) // ramp
  {
    if (ch->n_vibratopos < 128) // PT bug, should've been ch->n_tremolopos
      tremoloData = tremoloPos << 3;
    else
      tremoloData = 255 - (tremoloPos << 3);
  }
  else // square
  {
    tremoloData = 255;
  }
  
  tremoloData = ((uint16_t)tremoloData * (ch->n_tremolocmd & 0xF)) >> 6;
  
  if (ch->n_tremolopos < 128)
  {
    tremoloData = ch->n_volume + tremoloData;
    if (tremoloData > 64)
      tremoloData = 64;
  }
  else
  {
    tremoloData = ch->n_volume - tremoloData;
    if (tremoloData < 0)
      tremoloData = 0;
  }
  
  // set voice volume
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 8, tremoloData);
  
  // setVisualsVolume(ch->n_chanindex, tremoloData);
  
  ch->n_tremolopos += (ch->n_tremolocmd >> 2) & 0x3C;
}

static void sampleOffset(moduleChannel_t *ch)
{
  if ((ch->n_cmd & 0xFF) > 0)
    ch->n_sampleoffset = ch->n_cmd & 0xFF;
  
  uint16_t newOffset = ch->n_sampleoffset << 7;
  
  // this signed test is the reason for the 9xx "sample >64kB = silence" bug
  if ((int16_t)newOffset < (int16_t)ch->n_length)
  {
    ch->n_length -= newOffset;
    ch->n_start += newOffset << 1;
  }
  else
  {
    ch->n_length = 1;
  }
}

static void E_Commands(moduleChannel_t *ch)
{
  const uint8_t ecmd = (ch->n_cmd & 0x00F0) >> 4;
  switch (ecmd)
  {
  case 0x0: filterOnOff(ch);       return;
  case 0x1: finePortaUp(ch);       return;
  case 0x2: finePortaDown(ch);     return;
  case 0x3: setGlissControl(ch);   return;
  case 0x4: setVibratoControl(ch); return;
  case 0x5: setFineTune(ch);       return;
  case 0x6: jumpLoop(ch);          return;
  case 0x7: setTremoloControl(ch); return;
  case 0x8: karplusStrong(ch);     return;
  case 0xE: patternDelay(ch);      return;
  default: break;
  }
  
  if (editor.muted[ch->n_chanindex])
    return;
  
  switch (ecmd)
  {
  case 0x9: retrigNote(ch);     return;
  case 0xA: volumeFineUp(ch);   return;
  case 0xB: volumeFineDown(ch); return;
  case 0xC: noteCut(ch);        return;
  case 0xD: noteDelay(ch);      return;
  case 0xF: funkIt(ch);         return;
  default: break;
  }
}

static void checkMoreEffects(moduleChannel_t *ch)
{
  const uint8_t cmd = (ch->n_cmd & 0x0F00) >> 8;
  switch (cmd)
  {
  case 0x9: sampleOffset(ch); return; // note the returns here, not breaks!
  case 0xB: positionJump(ch); return;
  case 0xD: patternBreak(ch); return;
  case 0xE: E_Commands(ch);   return;
  case 0xF: setSpeed(ch);     return;
  default: break;
  }
  
  if (editor.muted[ch->n_chanindex])
    return;
  
  if (cmd == 0xC)
  {
    volumeChange(ch);
    return;
  }
  
  // set voice period
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 6, ch->n_period);
}

static void chkefx2(moduleChannel_t *ch)
{
  updateFunk(ch);
  
  if ((ch->n_cmd & 0xFFF) == 0)
    return;
  
  const uint8_t cmd = (ch->n_cmd & 0x0F00) >> 8;
  switch (cmd)
  {
  case 0x0: arpeggio(ch);            return; // note the returns here, not breaks!
  case 0x1: portaUp(ch);             return;
  case 0x2: portaDown(ch);           return;
  case 0x3: tonePortamento(ch);      return;
  case 0x4: vibrato(ch);             return;
  case 0x5: tonePlusVolSlide(ch);    return;
  case 0x6: vibratoPlusVolSlide(ch); return;
  case 0xE: E_Commands(ch);          return;
  default: break;
  }
  
  // set voice period
  const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
  paulaWriteWord(voiceAddr + 6, ch->n_period);
  
  if (cmd == 0x7)
    tremolo(ch);
  else if (cmd == 0xA)
    volumeSlide(ch);
}

static void checkEffects(moduleChannel_t *ch)
{
  if (editor.muted[ch->n_chanindex])
    return;
  
  chkefx2(ch);
  
  /* This is not very clear in the original PT replayer code,
   ** but the tremolo effect skips chkefx2()'s return address
   ** in the stack so that it jumps to checkEffects()'s return
   ** address instead of ending up here. In other words, volume
   ** is not updated here after tremolo (it's done inside the
   ** tremolo routine itself).
   */
  const uint8_t cmd = (ch->n_cmd & 0x0F00) >> 8;
  if (cmd != 0x7)
  {
    // set voice volume
    const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
    paulaWriteWord(voiceAddr + 8, ch->n_volume);
    
    // setVisualsVolume(ch->n_chanindex, ch->n_volume);
  }
}

static void setDMA(void)
{
  if (editor.muted[0]) DMACONtemp &= ~1;
  if (editor.muted[1]) DMACONtemp &= ~2;
  if (editor.muted[2]) DMACONtemp &= ~4;
  if (editor.muted[3]) DMACONtemp &= ~8;
  
  // start DMAs for selected voices
  paulaWriteWord(0xDFF096, 0x8000 | DMACONtemp);
  
  // setVisualsDMACON(0x8000 | DMACONtemp);
  
  moduleChannel_t *ch = song->channels;
  for (int32_t i = 0; i < PAULA_VOICES; i++, ch++)
  {
    // set new voice data ptr and length (these take effect after the current DMA cycle is done)
    const uint32_t voiceAddr = 0xDFF0A0 + (i * 16);
    paulaWritePtr(voiceAddr + 0, ch->n_loopstart);
    paulaWriteWord(voiceAddr + 4, ch->n_replen);
    
  }
}

static bool renderEndCheck(void) // for MOD2WAV/PAT2SMP
{
  if (!editor.mod2WavOngoing && !editor.pat2SmpOngoing)
    return true; // we're not doing MOD2WAV/PAT2SMP
  
  bool noPatternDelay = (pattDelTime2 == 0);
  if (noPatternDelay)
  {
    if (editor.pat2SmpOngoing)
    {
      if (modRenderDone)
        return false; // we're done rendering
    }
    
    if (editor.mod2WavOngoing && song->tick == song->speed-1)
    {
      bool rowVisited = editor.rowVisitTable[(modPos * MOD_ROWS) + song->row];
      if (rowVisited || modRenderDone)
      {
        modRenderDone = false;
        return false; // we're done rendering
      }
    }
  }
  
  return true;
}

static void setPeriod(moduleChannel_t *ch)
{
  int32_t i;
  
  uint16_t note = ch->n_note & 0xFFF;
  for (i = 0; i < 37; i++)
  {
    // periodTable[36] = 0, so i=36 is safe
    if (note >= periodTable[i])
      break;
  }
  
  // yes it's safe if i=37 because of zero-padding
  ch->n_period = periodTable[(ch->n_finetune * 37) + i];
  
  if ((ch->n_cmd & 0xFF0) != 0xED0) // no note delay
  {
    // voice DMA off (turned on in setDMA() later)
    paulaWriteWord(0xDFF096, ch->n_dmabit);
    
    if ((ch->n_wavecontrol & 0x04) == 0) ch->n_vibratopos = 0;
    if ((ch->n_wavecontrol & 0x40) == 0) ch->n_tremolopos = 0;
    
    const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
    
    // set voice data length and data ptr
    paulaWriteWord(voiceAddr + 4, ch->n_length);
    paulaWritePtr(voiceAddr + 0, ch->n_start);
    
    if (ch->n_start == NULL)
    {
      ch->n_loopstart = NULL;
      paulaWriteWord(voiceAddr + 4, 1); // length
      ch->n_replen = 1;
    }
    
    // set voice period
    paulaWriteWord(voiceAddr + 6, ch->n_period);
    
    DMACONtemp |= ch->n_dmabit;
  }
  
  checkMoreEffects(ch);
}

static void playVoice(moduleChannel_t *ch)
{
  if (ch->n_note == 0 && ch->n_cmd == 0) // test period, command and command parameter
  {
    // set voice period
    const uint32_t voiceAddr = 0xDFF0A0 + (ch->n_chanindex * 16);
    paulaWriteWord(voiceAddr + 6, ch->n_period);
    
  }
  
  note_t note = song->patterns[modPattern][(song->row * PAULA_VOICES) + ch->n_chanindex];
  
  ch->n_note = note.period;
  ch->n_cmd = (note.command << 8) | note.param;
  
  if (note.sample >= 1 && note.sample <= 31) // SAFETY BUG FIX: don't handle sample-numbers >31
  {
    ch->n_samplenum = note.sample - 1;
    moduleSample_t *s = &song->samples[ch->n_samplenum];
    
    ch->n_start = &song->sampleData[s->offset];
    ch->n_finetune = s->fineTune & 0xF;
    ch->n_volume = s->volume;
    ch->n_length = (uint16_t)(s->length >> 1);
    ch->n_replen = (uint16_t)(s->loopLength >> 1);
    
    const uint16_t repeat = (uint16_t)(s->loopStart >> 1);
    if (repeat > 0)
    {
      ch->n_loopstart = ch->n_start + (repeat << 1);
      ch->n_wavestart = ch->n_loopstart;
      ch->n_length = repeat + ch->n_replen;
    }
    else
    {
      ch->n_loopstart = ch->n_start;
      ch->n_wavestart = ch->n_start;
    }
    
    // non-PT2 requirement (set safe sample space for uninitialized voices - f.ex. "the ultimate beeper.mod")
    if (ch->n_length == 0)
      ch->n_loopstart = ch->n_wavestart = paulaGetNullSamplePtr();
  }
  
  if ((ch->n_note & 0xFFF) > 0)
  {
    if ((ch->n_cmd & 0xFF0) == 0xE50) // set finetune
    {
      setFineTune(ch);
      setPeriod(ch);
    }
    else
    {
      uint8_t cmd = (ch->n_cmd & 0x0F00) >> 8;
      if (cmd == 3 || cmd == 5)
      {
        setTonePorta(ch);
        checkMoreEffects(ch);
      }
      else if (cmd == 9)
      {
        checkMoreEffects(ch);
        setPeriod(ch);
      }
      else
      {
        setPeriod(ch);
      }
    }
  }
  else
  {
    checkMoreEffects(ch);
  }
}

static void setCurrRowToVisited(void) // for MOD2WAV
{
  if (editor.mod2WavOngoing)
    editor.rowVisitTable[(modPos * MOD_ROWS) + song->row] = true;
}

static void increasePlaybackTimer(void)
{
  // (the timer is not counting in "play pattern" mode)
  if (editor.playMode != PLAY_MODE_PATTERN && modBPM >= MIN_BPM && modBPM <= MAX_BPM)
  {
    if (editor.timingMode == TEMPO_MODE_CIA)
      editor.playbackSecondsFrac += musicTimeTab52[modBPM-MIN_BPM];
    else
      editor.playbackSecondsFrac += musicTimeTab52[(MAX_BPM-MIN_BPM)+1]; // vblank tempo mode
    
    if (editor.playbackSecondsFrac >= 1ULL << 52)
    {
      editor.playbackSecondsFrac &= (1ULL << 52)-1;
      editor.playbackSeconds++;
    }
  }
}

bool intMusic(void) // replayer ticker
{
  if (song->currRow % 64 == 0)
  // quirk: CIA BPM changes are delayed by one tick in PT, so handle previous tick's BPM change now
  if (ciaSetBPM != -1)
  {
    const int32_t newBPM = ciaSetBPM;
    modSetTempo(newBPM, false);
    ciaSetBPM = -1;
  }
  
  increasePlaybackTimer();
  
  if (!editor.stepPlayEnabled)
    song->tick++;
  
  bool readNewNote = false;
  if ((uint32_t)song->tick >= (uint32_t)song->speed)
  {
    song->tick = 0;
    readNewNote = true;
  }
  if (readNewNote || editor.stepPlayEnabled) // tick 0
  {
    if (pattDelTime2 == 0) // no pattern delay, time to read note data
    {
      DMACONtemp = 0; // reset Paula DMA trigger states
      
      setCurrRowToVisited(); // for MOD2WAV

      // read note data and trigger voices
      moduleChannel_t *ch = song->channels;
      for (int32_t i = 0; i < PAULA_VOICES; i++, ch++)
      {
        playVoice(ch);

        // set voice volume
        const uint32_t voiceAddr = 0xDFF0A0 + (i * 16);
        paulaWriteWord(voiceAddr + 8, ch->n_volume);
        
      }
      
      setDMA();
    }
    else // pattern delay is on-going
    {
      moduleChannel_t *ch = song->channels;
      for (int32_t i = 0; i < PAULA_VOICES; i++, ch++)
        checkEffects(ch);
    }
    
    // increase row
    if (!editor.stepPlayBackwards)
    {
      song->row++;
      song->rowsCounter++; // for MOD2WAV's progress bar
    }
    
    if (pattDelTime > 0)
    {
      pattDelTime2 = pattDelTime;
      pattDelTime = 0;
    }
    
    // undo row increase if pattern delay is on-going
    if (pattDelTime2 > 0)
    {
      pattDelTime2--;
      if (pattDelTime2 > 0)
      {
        song->row--;
        song->rowsCounter--; // for MOD2WAV's progress bar
      }
    }
    
    if (pBreakFlag)
    {
      song->row = pBreakPosition;
      pBreakPosition = 0;
      pBreakFlag = false;
    }
    
    // step-play handling
    if (editor.stepPlayEnabled)
    {
      if (config.keepEditModeAfterStepPlay && editor.stepPlayLastMode == MODE_EDIT)
      {
        doStopIt(false);
        
        editor.playMode = PLAY_MODE_NORMAL;
        editor.currMode = MODE_EDIT;
        // pointerSetModeThreadSafe(POINTER_MODE_EDIT, true);
      }
      else
      {
        doStopIt(true);
      }
      
      if (editor.stepPlayLastMode == MODE_EDIT || editor.stepPlayLastMode == MODE_IDLE)
      {
        song->row &= 63;
        song->currRow = song->row;
      }
      else
      {
        // if we were playing, set replayer row to tracker row (stay in sync)
        song->currRow &= 63;
        song->row = song->currRow;
      }
      
      editor.stepPlayEnabled = false;
      editor.stepPlayBackwards = false;
      
      return true;
    }
    
    if (song->row >= MOD_ROWS || posJumpAssert)
      nextPosition();
    
  }
  else // tick > 0 (handle effects)
  {
    moduleChannel_t *ch = song->channels;
    for (int32_t i = 0; i < PAULA_VOICES; i++, ch++)
      checkEffects(ch);
    
    if (posJumpAssert)
      nextPosition();
  }
  
  // command F00 = stop song, do it here (so that the scopes are updated properly)
  if (doStopSong)
  {
    doStopSong = false;
    
    editor.songPlaying = false;
    
    editor.playMode = PLAY_MODE_NORMAL;
    editor.currMode = MODE_IDLE;
  }
  
  return renderEndCheck(); // MOD2WAV/PAT2SMP listens to the return value (true = not done yet)
}

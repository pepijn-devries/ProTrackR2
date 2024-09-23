#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include "pt2_helpers.h"
#include "pt2_sampler.h"
#include "pt2_config.h"

bool checkSaveTarget(uint8_t * target, uint8_t * origin, uint32_t increment, uint32_t max_size) {
  int data_pos = target - origin;
  return (data_pos + increment) <= max_size;
}

uint32_t modSave2(module_t * my_song, uint8_t *target, uint32_t target_size)
{
  uint8_t * start = &(*target);
  
  if (!checkSaveTarget(target, start, 20, target_size)) return -1;
  
  memcpy(target, &my_song->header.name, 20);
  target += 20;

  for (int32_t i = 0; i < MOD_SAMPLES; i++)
  {
    moduleSample_t *s = &my_song->samples[i];
    
    if (!checkSaveTarget(target, start, 22, target_size)) return -1;
    memcpy(target, &s->text, 22);
    target += 22;

    uint16_t length = SWAP16(s->length >> 1);
    if (!checkSaveTarget(target, start, sizeof (int16_t), target_size)) return -1;
    memcpy(target, &length, sizeof (int16_t));
    target += sizeof (int16_t);

    if (!checkSaveTarget(target, start, 2, target_size)) return -1;
    target[0] = s->fineTune & 0xF;
    target[1] = ((uint8_t)s->volume > 64) ? 64 : s->volume;
    target += 2;

    int32_t loopStart = s->loopStart;
    int32_t loopLength = s->loopLength;
    
    if (loopLength < 2)
      loopLength = 2;
    
    if (loopStart+loopLength <= 2 || loopStart+loopLength > s->length)
    {
      loopStart = 0;
      loopLength = 2;
    }
    
    uint16_t loopStart16 = SWAP16(loopStart >> 1);
    uint16_t loopLength16 = SWAP16(loopLength >> 1);
    
    if (!checkSaveTarget(target, start, sizeof (int16_t), target_size)) return -1;
    memcpy(target, &loopStart16, sizeof (int16_t));
    target += sizeof (int16_t);
    if (!checkSaveTarget(target, start, sizeof (int16_t), target_size)) return -1;
    memcpy(target, &loopLength16, sizeof (int16_t));
    target += sizeof (int16_t);
  }
  
  if (!checkSaveTarget(target, start, 2, target_size)) return -1;
  target[0] = (uint8_t)my_song->header.songLength;
  target[1] = 0x7F;
  target += 2;

  for (int32_t i = 0; i < 128; i++) {
    if (!checkSaveTarget(target, start, 1, target_size)) return -1;
    target[0] = (uint8_t)my_song->header.patternTable[i];
    target++;
    // fputc((uint8_t)my_song->header.patternTable[i], f);
  }
  
  int32_t numPatterns = 0;
  for (int32_t i = 0; i < 128; i++)
  {
    if (my_song->header.patternTable[i] > numPatterns)
      numPatterns = my_song->header.patternTable[i];
  }
  
  numPatterns++;
  if (numPatterns > MAX_PATTERNS)
    numPatterns = MAX_PATTERNS;
  
  if (!checkSaveTarget(target, start, 4, target_size)) return -1;
  memcpy(target, (numPatterns <= 64) ? "M.K." : "M!K!", 4);
  target += 4;

  for (int32_t i = 0; i < numPatterns; i++)
  {
    note_t *note = my_song->patterns[i];
    for (int32_t j = 0; j < MOD_ROWS * PAULA_VOICES; j++, note++)
    {
      if (!checkSaveTarget(target, start, 4, target_size)) return -1;
      target[0] = (note->sample & 0xF0) | (note->period >> 8);
      target[1] = note->period & 0xFF;
      target[2] = ((note->sample << 4) & 0xF0) | (note->command & 0x0F);
      target[3] = note->param;
      target += 4;
    }
  }
  
  for (int32_t i = 0; i < MOD_SAMPLES; i++)
  {
    moduleSample_t *s = &my_song->samples[i];
    const int8_t *smpPtr8 = &my_song->sampleData[s->offset];
    
    // clear first two bytes of non-looping samples (prevents stuck beep)
    if (s->length >= 2 && s->loopStart+s->loopLength == 2)
    {
      if (!checkSaveTarget(target, start, 2, target_size)) return -1;
      target[0] = 0;
      target[1] = 0;
      target += 2;
      if (!checkSaveTarget(target, start, (s->length-2), target_size)) return -1;
      memcpy(target, &smpPtr8[2], s->length-2);
      target += (s->length-2);
    }
    else
    {
      if (!checkSaveTarget(target, start, s->length, target_size)) return -1;
      memcpy(target, smpPtr8, s->length);
      target += s->length;
    }
  }
  
  return target - start;
}

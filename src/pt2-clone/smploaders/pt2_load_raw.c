#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../pt2_header.h"
#include "../pt2_config.h"
#include "../pt2_structs.h"
#include "../pt2_replayer_light.h"

bool loadRAWSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr) {
  uint32_t sampleLength = filesize;
  if (sampleLength > (uint32_t)config.maxSampleLength)
    sampleLength = config.maxSampleLength;
  
  memcpy(smpDataPtr, input, sampleLength);
  
  if (sampleLength & 1)
  {
    if (++sampleLength > (uint32_t)config.maxSampleLength)
      sampleLength = config.maxSampleLength;
  }
  
  s->volume = 64;
  s->fineTune = 0;
  s->length = sampleLength;
  s->loopStart = 0;
  s->loopLength = 2;
  
  return true;
}

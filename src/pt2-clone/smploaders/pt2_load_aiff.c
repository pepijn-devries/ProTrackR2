#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../pt2_header.h"
#include "../pt2_config.h"
#include "../pt2_structs.h"
#include "../pt2_helpers.h"
#include "../pt2_replayer_light.h"
#include "../pt2_downsample2x.h"
#include "../pt2_audio.h"

bool loadAIFFSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr) {

  uint8_t sampleRateBytes[10];
  uint16_t bitDepth = 0, numChannels = 0;
  uint32_t offset = 0;
  
  // zero out chunk pointers and lengths
  uint32_t commPtr = 0; uint32_t commLen = 0;
  uint32_t ssndPtr = 0; uint32_t ssndLen = 0;
  
  bool unsigned8bit = false;
  
  uint32_t pos = 12;
  while (pos < filesize-12)
  {
    uint32_t blockName, blockSize;
    
    blockName = ((uint32_t *)(input + pos))[0];
    blockSize = ((uint32_t *)(input + pos + 4))[0];
    pos += 8;

    blockName = SWAP32(blockName);
    blockSize = SWAP32(blockSize);
    
    switch (blockName)
    {
    case 0x434F4D4D: // "COMM"
    {
      commPtr = pos;
      commLen = blockSize;
    }
      break;
      
    case 0x53534E44: // "SSND"
    {
      ssndPtr = pos;
      ssndLen = blockSize;
    }
      break;
      
    default: break;
    }
    
    pos += blockSize + (blockSize & 1);
  }
  
  if (commPtr == 0 || commLen < 18 || ssndPtr == 0)
  {
    return false;
  }
  
  // kludge for some really strange AIFFs
  if (ssndLen == 0)
    ssndLen = filesize - ssndPtr;
  
  if (ssndPtr+ssndLen > filesize)
    ssndLen = filesize - ssndPtr;
  
  numChannels = SWAP16(((uint16_t *)(input + commPtr))[0]);
  bitDepth    = SWAP16(((uint16_t *)(input + commPtr + 6))[0]);
  memcpy(sampleRateBytes, input + commPtr + 8, 10);

  if (numChannels != 1 && numChannels != 2) // sample type
  {
    return false;
  }
  
  if (bitDepth != 8 && bitDepth != 16 && bitDepth != 24 && bitDepth != 32)
  {
    return false;
  }

  // read compression type (if present)
  if (commLen > 29)
  {
    if (memcmp(input + commPtr + 25, "NONE", 4))
    {
      return false;
    }
  }

  offset = ((uint32_t *)(input + ssndPtr))[0];  
  if (offset > 0)
  {
    return false;
  }
  
  uint32_t ssndPtr2 = ssndPtr + 8; // excluding offset and blocksize

  ssndLen -= 8; // don't include offset and blockSize datas
  
  int32_t sampleLength = ssndLen;
  if (sampleLength == 0)
  {
    return false;
  }

  if (bitDepth == 8) // 8-BIT INTEGER SAMPLE
  {
    if (sampleLength > config.maxSampleLength*4)
      sampleLength = config.maxSampleLength*4;
    
    int8_t *audioDataS8 = (int8_t *)malloc(sampleLength * sizeof (int8_t));
    if (audioDataS8 == NULL)
      return false;
    
    // read sample data
    memcpy(audioDataS8, input + ssndPtr2, sampleLength);

    if (unsigned8bit)
    {
      for (int32_t i = 0; i < sampleLength; i++)
        audioDataS8[i] ^= 0x80;
    }
    
    // convert from stereo to mono (if needed)
    if (numChannels == 2)
    {
      sampleLength >>= 1;
      for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
        audioDataS8[i] = (audioDataS8[(i * 2) + 0] + audioDataS8[(i * 2) + 1]) >> 1;;
    }
    
    if (sampleLength > config.maxSampleLength)
      sampleLength = config.maxSampleLength;
    
    memcpy(smpDataPtr, audioDataS8, sampleLength);

    free(audioDataS8);
  }
  else if (bitDepth == 16) // 16-BIT INTEGER SAMPLE
  {
    sampleLength >>= 1;
    if (sampleLength > config.maxSampleLength*4)
      sampleLength = config.maxSampleLength*4;
    
    int16_t *audioDataS16 = (int16_t *)malloc(sampleLength * sizeof (int16_t));
    if (audioDataS16 == NULL)
      return false;
    
    // read sample data
    memcpy(audioDataS16, input + ssndPtr2, 2*sampleLength);

    // fix endianness
    for (int32_t i = 0; i < sampleLength; i++)
      audioDataS16[i] = SWAP16(audioDataS16[i]);
    
    // convert from stereo to mono (if needed)
    if (numChannels == 2)
    {
      sampleLength >>= 1;
      for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
        audioDataS16[i] = (audioDataS16[(i << 1) + 0] + audioDataS16[(i << 1) + 1]) >> 1;
    }
    
    if (sampleLength > config.maxSampleLength)
      sampleLength = config.maxSampleLength;
    
    double dAmp = 1.0;
    const double dPeak = get16BitPeak(audioDataS16, sampleLength);
    if (dPeak > 0.0)
      dAmp = INT8_MAX / dPeak;

    for (int32_t i = 0; i < sampleLength; i++)
    {
      int32_t smp32 = (int32_t)round(audioDataS16[i] * dAmp);
      assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
      smpDataPtr[i] = (int8_t)smp32;
    }
    
    free(audioDataS16);
  }
  else if (bitDepth == 24) // 24-BIT INTEGER SAMPLE
  {
    sampleLength /= 3;
    if (sampleLength > config.maxSampleLength*4)
      sampleLength = config.maxSampleLength*4;
    
    int32_t *audioDataS32 = (int32_t *)malloc(sampleLength * sizeof (int32_t));
    if (audioDataS32 == NULL)
      return false;
    
    // read sample data
    uint8_t *audioDataU8 = (uint8_t *)audioDataS32;
    for (int32_t i = 0; i < sampleLength; i++)
    {
      audioDataU8[0] = 0;
      memcpy(audioDataU8 + 1, input + ssndPtr2 + 3*i, 3);
      audioDataU8 += sizeof (int32_t);
    }

    // convert to 32-bit
    for (int32_t i = 0; i < sampleLength; i++)
    {
      audioDataS32[i] = (audioDataU8[0] << 24) | (audioDataU8[1] << 16) | (audioDataU8[2] << 8);
      audioDataU8 += 3;
    }
    
    // convert from stereo to mono (if needed)
    if (numChannels == 2)
    {
      sampleLength >>= 1;
      for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
      {
        int64_t smp = ((int64_t)audioDataS32[(i << 1) + 0] + audioDataS32[(i << 1) + 1]) >> 1;
        audioDataS32[i] = (int32_t)smp;
      }
    }
    
    if (sampleLength > config.maxSampleLength)
      sampleLength = config.maxSampleLength;
    
    double dAmp = 1.0;
    const double dPeak = get32BitPeak(audioDataS32, sampleLength);
    if (dPeak > 0.0)
      dAmp = INT8_MAX / dPeak;

    for (int32_t i = 0; i < sampleLength; i++)
    {
      int32_t smp32 = (int32_t)round(audioDataS32[i] * dAmp);
      smpDataPtr[i] = (int8_t)smp32;
    }
    
    free(audioDataS32);
  }
  else if (bitDepth == 32) // 32-BIT INTEGER SAMPLE
  {
    sampleLength >>= 2;
    if (sampleLength > config.maxSampleLength*4)
      sampleLength = config.maxSampleLength*4;
    
    int32_t *audioDataS32 = (int32_t *)malloc(sampleLength * sizeof (int32_t));
    if (audioDataS32 == NULL)
      return false;
    
    // read sample data
    memcpy(audioDataS32, input + ssndPtr2, 4*sampleLength);

    // fix endianness
    for (int32_t i = 0; i < sampleLength; i++)
      audioDataS32[i] = SWAP32(audioDataS32[i]);
    
    // convert from stereo to mono (if needed)
    if (numChannels == 2)
    {
      sampleLength >>= 1;
      for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
      {
        int64_t smp = ((int64_t)audioDataS32[(i << 1) + 0] + audioDataS32[(i << 1) + 1]) >> 1;
        audioDataS32[i] = (int32_t)smp;
      }
    }
    
    if (sampleLength > config.maxSampleLength)
      sampleLength = config.maxSampleLength;
    
    double dAmp = 1.0;
    const double dPeak = get32BitPeak(audioDataS32, sampleLength);
    if (dPeak > 0.0)
      dAmp = INT8_MAX / dPeak;

    for (int32_t i = 0; i < sampleLength; i++)
    {
      int32_t smp32 = (int32_t)round(audioDataS32[i] * dAmp);
      smpDataPtr[i] = (int8_t)smp32;
    }
    
    free(audioDataS32);
  }
  
  if (sampleLength & 1)
  {
    if (++sampleLength > config.maxSampleLength)
      sampleLength = config.maxSampleLength;
  }
  
  s->length = sampleLength;
  s->fineTune = 0;
  s->volume = 64;
  s->loopStart = 0;
  s->loopLength = 2;
  
  return true;
}

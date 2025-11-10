#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "../pt2_header.h"
#include "../pt2_config.h"
#include "../pt2_structs.h"
#include "../pt2_helpers.h"
#include "../pt2_replayer_light.h"
#include "../pt2_downsample2x.h"
#include "../pt2_audio.h"

enum
{
	WAV_FORMAT_PCM = 0x0001,
	WAV_FORMAT_IEEE_FLOAT = 0x0003
};

bool loadWAVSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr) {
  uint16_t audioFormat, numChannels, bitsPerSample;
  uint32_t sampleRate;
  
  // zero out chunk pointers and lengths
  uint32_t fmtPtr  = 0; uint32_t fmtLen = 0;
  uint32_t dataPtr = 0; uint32_t dataLen = 0;
  uint32_t inamPtr = 0;  int32_t inamLen = 0;
  uint32_t xtraPtr = 0; uint32_t xtraLen = 0;
  uint32_t smplPtr = 0; uint32_t smplLen = 0;

  uint32_t pos = 12;
  if (pos > filesize) return false;
  uint32_t bytesRead = 0;
  while (bytesRead < (uint32_t)filesize-12)
  {
    uint32_t chunkID, chunkSize;
	if ((pos + 8) > filesize) return false;
    chunkID = ((uint32_t *)(input + pos))[0];
    chunkSize = ((uint32_t *)(input + pos + 4))[0];
    pos += 8;
    uint32_t endOfChunk = (pos + chunkSize) + (chunkSize & 1);
    switch (chunkID)
    {
    case 0x20746D66: // "fmt "
    {
      fmtPtr = pos;
      fmtLen = chunkSize;
    }
      break;
      
    case 0x61746164: // "data"
    {
      dataPtr = pos;
      dataLen = chunkSize;
    }
      break;
      
    case 0x5453494C: // "LIST"
    {
      if (chunkSize >= 4)
    {
	  if ((pos + 4) > filesize) return false;
      chunkID = ((uint32_t *)(input + pos))[0];
      pos += 4;
      if (chunkID == 0x4F464E49) // "INFO"
      {
        bytesRead = 0;
        while (bytesRead < chunkSize)
        {
		  if ((pos + 8) > filesize) return false;
          chunkID = ((uint32_t *)(input + pos))[0];
          chunkSize = ((uint32_t *)(input + pos + 4))[0];
          pos += 8;

          switch (chunkID)
          {
          case 0x4D414E49: // "INAM"
          {
            inamPtr = pos;
            inamLen = chunkSize;
          }
            break;
            
          default: break;
          }
          
          bytesRead += chunkSize + (chunkSize & 1);
        }
      }
    }
    }
      break;
      
    case 0x61727478: // "xtra"
    {
      xtraPtr = pos;
      xtraLen = chunkSize;
    }
      break;
      
    case 0x6C706D73: // "smpl"
    {
      smplPtr = pos;
      smplLen = chunkSize;
    }
      break;
      
    default: break;
    }
    
    bytesRead += 8 + chunkSize + (chunkSize & 1);
    pos = endOfChunk;
  }

  // we need at least "fmt " and "data" - check if we found them sanely
  if (fmtPtr == 0 || fmtLen < 16 || dataPtr == 0 || dataLen == 0)
    return false;
  
  // ---- READ "fmt " CHUNK ----
  if ((fmtPtr + 18) > filesize) return false;
  audioFormat   = ((uint16_t *)(input + fmtPtr))[0];
  numChannels   = ((uint16_t *)(input + fmtPtr + 2))[0];
  sampleRate    = ((uint32_t *)(input + fmtPtr + 4))[0];
  bitsPerSample = ((uint16_t *)(input + fmtPtr + 14))[0];
  int32_t sampleLength = dataLen;
  // ---------------------------

  if (sampleRate == 0 || sampleLength == 0 || sampleLength >= (int32_t)filesize*(bitsPerSample/8))
    return false;
  
  if (audioFormat != WAV_FORMAT_PCM && audioFormat != WAV_FORMAT_IEEE_FLOAT)
    return false;
  
  if (numChannels == 0 || numChannels > 2)
    return false;
  
  if (audioFormat == WAV_FORMAT_IEEE_FLOAT && bitsPerSample != 32 && bitsPerSample != 64)
    return false;
  
  if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32 && bitsPerSample != 64)
    return false;

	if (bitsPerSample == 8) // 8-BIT INTEGER SAMPLE
	{
	  if (sampleLength > config.maxSampleLength*4)
	    sampleLength = config.maxSampleLength*4;
	  
	  uint8_t *audioDataU8 = (uint8_t *)malloc(sampleLength * sizeof (uint8_t));
	  if (audioDataU8 == NULL)
	    return false;
	  memcpy(audioDataU8, input + dataPtr, sampleLength);
	  
	  // convert from stereo to mono (if needed)
	  if (numChannels == 2)
	  {
	    sampleLength >>= 1;
	    for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
	    {
	      int32_t smp32 = (audioDataU8[(i << 1) + 0] - 128) + (audioDataU8[(i << 1) + 1] - 128);
	      smp32 = 128 + (smp32 >> 1);
	      audioDataU8[i] = (uint8_t)smp32;
	    }
	  }
	  
	  if (sampleLength > config.maxSampleLength)
	    sampleLength = config.maxSampleLength;

	  for (int32_t i = 0; i < sampleLength; i++)
	    smpDataPtr[i] = audioDataU8[i] - 128;
	  
	  free(audioDataU8);
	}
	else if (bitsPerSample == 16) // 16-BIT INTEGER SAMPLE
	{
	  sampleLength >>= 1;
	  if (sampleLength > config.maxSampleLength*4)
	    sampleLength = config.maxSampleLength*4;
	  
	  int16_t *audioDataS16 = (int16_t *)malloc(sampleLength * sizeof (int16_t));
	  if (audioDataS16 == NULL)
	    return false;
	  
	  memcpy(audioDataS16, input + dataPtr, 2*sampleLength);
	  
	  // convert from stereo to mono (if needed)
	  if (numChannels == 2)
	  {
	    sampleLength >>= 1;
	    for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
	      audioDataS16[i] = (audioDataS16[(i << 1) + 0] + audioDataS16[(i << 1) + 1]) >> 1;;
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
	else if (bitsPerSample == 24) // 24-BIT INTEGER SAMPLE
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
	    memcpy(audioDataU8 + 1, input + dataPtr + 3*i, 3);
	    audioDataU8 += sizeof (int32_t);
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
	    assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
	    smpDataPtr[i] = (int8_t)smp32;
	  }
	  
	  free(audioDataS32);
	}
	else if (audioFormat == WAV_FORMAT_PCM && bitsPerSample == 32) // 32-BIT INTEGER SAMPLE
	{
	  sampleLength >>= 2;
	  if (sampleLength > config.maxSampleLength*4)
	    sampleLength = config.maxSampleLength*4;
	  
	  int32_t *audioDataS32 = (int32_t *)malloc(sampleLength * sizeof (int32_t));
	  if (audioDataS32 == NULL)
	    return false;
	  
	  // read sample data
	  memcpy(audioDataS32, input + dataPtr, 4*sampleLength);

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
	    assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
	    smpDataPtr[i] = (int8_t)smp32;
	  }
	  
	  free(audioDataS32);
	}
	else if (audioFormat == WAV_FORMAT_IEEE_FLOAT && bitsPerSample == 32) // 32-BIT FLOATING POINT SAMPLE
	{
	  sampleLength >>= 2;
	  if (sampleLength > config.maxSampleLength*4)
	    sampleLength = config.maxSampleLength*4;
	  
	  uint32_t *audioDataU32 = (uint32_t *)malloc(sampleLength * sizeof (uint32_t));
	  if (audioDataU32 == NULL)
	    return false;
	  
	  // read sample data
	  memcpy(audioDataU32, input + dataPtr, 4*sampleLength);
	  
	  float *fAudioDataFloat = (float *)audioDataU32;
	  
	  // convert from stereo to mono (if needed)
	  if (numChannels == 2)
	  {
	    sampleLength >>= 1;
	    for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
	      fAudioDataFloat[i] = (fAudioDataFloat[(i * 2) + 0] + fAudioDataFloat[(i * 2) + 1]) * 0.5f;
	  }
	  
	  if (sampleLength > config.maxSampleLength)
	    sampleLength = config.maxSampleLength;
	  
	  float fAmp = 1.0f;
	  const float fPeak = getFloatPeak(fAudioDataFloat, sampleLength);
	  if (fPeak > 0.0f)
	    fAmp = INT8_MAX / fPeak;
	  
	  for (int32_t i = 0; i < sampleLength; i++)
	  {
	    int32_t smp32 = (int32_t)roundf(fAudioDataFloat[i] * fAmp);
	    assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
	    smpDataPtr[i] = (int8_t)smp32;
	  }
	  
	  free(audioDataU32);
	}
	else if (audioFormat == WAV_FORMAT_IEEE_FLOAT && bitsPerSample == 64) // 64-BIT FLOATING POINT SAMPLE
	{
	  sampleLength >>= 3;
	  if (sampleLength > config.maxSampleLength*4)
	    sampleLength = config.maxSampleLength*4;
	  
	  uint32_t *audioDataU32 = (uint32_t *)malloc(sampleLength * (sizeof (uint32_t) * 2));
	  if (audioDataU32 == NULL)
	    return false;
	  
	  // read sample data
	  memcpy(audioDataU32, input + dataPtr, 8*sampleLength);
	  
	  double *dAudioDataDouble = (double *)audioDataU32;
	  
	  // convert from stereo to mono (if needed)
	  if (numChannels == 2)
	  {
	    sampleLength >>= 1;
	    for (int32_t i = 0; i < sampleLength-1; i++) // add right channel to left channel
	      dAudioDataDouble[i] = (dAudioDataDouble[(i * 2) + 0] + dAudioDataDouble[(i * 2) + 1]) * 0.5;
	  }
	  
	  if (sampleLength > config.maxSampleLength)
	    sampleLength = config.maxSampleLength;
	  
	  double dAmp = 1.0;
	  const double dPeak = getDoublePeak(dAudioDataDouble, sampleLength);
	  if (dPeak > 0.0)
	    dAmp = INT8_MAX / dPeak;
	  
	  for (int32_t i = 0; i < sampleLength; i++)
	  {
	    int32_t smp32 = (int32_t)round(dAudioDataDouble[i] * dAmp);
	    assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
	    smpDataPtr[i] = (int8_t)smp32;
	  }
	  
	  free(audioDataU32);
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
	
	// ---- READ "smpl" chunk ----
	if (smplPtr != 0 && smplLen > 52)
	{
	  int32_t loopStart, loopEnd;
	  uint32_t loopFlags;
	  
	  loopFlags = ((uint32_t *)(input + 28))[0];
	  loopStart = ((uint32_t *)(input + 40))[0];
	  loopEnd   = ((uint32_t *)(input + 44))[0];
	  loopEnd++;
	  
	  if (loopFlags) // loop enabled?
	  {
	    int32_t loopLength = loopEnd - loopStart;
	    
	    loopStart &= ~1;
	    loopLength &= ~1;
	    
	    if (loopLength < 2 || loopStart+loopLength >= s->length)
	    {
	      loopStart = 0;
	      loopLength = 2;
	    }
	    
	    s->loopStart = loopStart;
	    s->loopLength = loopLength;
	  }
	}
	// ---------------------------

		// ---- READ "xtra" chunk ----
		if (xtraPtr != 0 && xtraLen >= 8)
		{
			uint16_t tempVol;

			tempVol = ((uint16_t *)(input + xtraPtr + 6))[0];
			if (tempVol > 256)
				tempVol = 256;

			tempVol >>= 2; // 0..256 -> 0..64

			s->volume = (int8_t)tempVol;
		}
		// ---------------------------

		// ---- READ "INAM" chunk ----
		if (inamPtr != 0 && inamLen > 0)
		{
		  if (inamLen > 21) inamLen = 21;
		  memcpy(s->text, input + inamPtr, inamLen);
		}
	
  return true;
}

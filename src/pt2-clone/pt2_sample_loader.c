// for finding memory leaks in debug mode with Visual Studio 
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pt2_sampler.h"
#include "pt2_audio.h"
#include "pt2_helpers.h"
#include "pt2_config.h"
#include "pt2_downsample2x.h"
#include "pt2_replayer_light.h"

enum
{
  SAMPLETYPE_RAW,
  SAMPLETYPE_IFF,
  SAMPLETYPE_AIFF,
  SAMPLETYPE_WAV,
  SAMPLETYPE_FLAC
};

bool loadRAWSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr);
bool loadIFFSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr);
bool loadAIFFSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr);
bool loadWAVSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr);

bool loadSample2(uint8_t * data, uint32_t filesize, moduleSample_t * samp, int8_t * sampData)
{
  // defaults to RAW if no format was identified
  uint8_t sampleType = SAMPLETYPE_RAW;
  memset(samp, 0, sizeof(moduleSample_t));
  
  // first, check heades before we eventually load as RAW
  if (filesize > 16)
  {
    uint32_t ID = ((uint32_t *)data)[0];
    
    if (ID == 0x43614C66) // "fLaC" (XXX: weak detection)
    {
      sampleType = SAMPLETYPE_FLAC;
    }
    else if (ID == 0x46464952) // "RIFF" (WAV)
    {
      ID = ((uint32_t *)(data + 8))[0];
      if (ID == 0x45564157) // "WAVE"
        sampleType = SAMPLETYPE_WAV;
    }
    else if (ID == 0x4D524F46) // "FORM" (IFF/AIFF)
    {
      
      ID = ((uint32_t *)(data + 8))[0];
      
      // check if it's an Amiga IFF sample
      if (ID == 0x58565338 || ID == 0x56533631) // "8SVX" (normal) and "16SV" (FT2 sample)
      {
        sampleType = SAMPLETYPE_IFF;
      }
      
      // check if it's an AIFF sample
      else if (ID == 0x46464941) // "AIFF"
      {
        sampleType = SAMPLETYPE_AIFF;
      }
      
      else if (ID == 0x43464941) // "AIFC" (compressed AIFF)
      {
        return false;
      }
    }
  }
  
  memset(sampData, 0, config.maxSampleLength);
  
  bool result = false;
  switch (sampleType)
  {
  case SAMPLETYPE_RAW:
    result = loadRAWSample2(data, filesize, samp, sampData);
    break;
    
  case SAMPLETYPE_IFF:
    result = loadIFFSample2(data, filesize, samp, sampData);
    break;
    
  case SAMPLETYPE_AIFF:
    result = loadAIFFSample2(data, filesize, samp, sampData);
    break;
    
  case SAMPLETYPE_WAV:
    result = loadWAVSample2(data, filesize, samp, sampData);
    break;
    
  default: break;
  }
  
  if (result == true)
  {
  	// sample load was successful

  	if (samp->length > config.maxSampleLength)
  		samp->length = config.maxSampleLength;

  	if (samp->loopStart + samp->loopLength > samp->length)
  	{
  		samp->loopStart = 0;
  		samp->loopLength = 2;
  	}

  	fixSampleBeep2(samp, sampData);

  }
  
  return result;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "../pt2_header.h"
#include "../pt2_config.h"
#include "../pt2_structs.h"
#include "../pt2_helpers.h"
#include "../pt2_replayer_light.h"
#include "../pt2_downsample2x.h"
#include "../pt2_audio.h"

uint32_t loadIFFSample2(uint8_t *input, uint32_t filesize, moduleSample_t * s, int8_t * smpDataPtr) {
  int32_t loopStart, loopLength;
  uint32_t pos = 0;
  
  // zero out chunk pointers and lengths
  uint32_t vhdrPtr = 0; uint32_t vhdrLen = 0;
  uint32_t bodyPtr = 0; uint32_t bodyLen = 0;
  uint32_t namePtr = 0;  int32_t nameLen = 0;

  pos += 8;  
  bool is16Bit = !strncmp((char *)(input + pos), "16SV", 4);
  pos += 4;
	int32_t sampleLength = 0;
	uint32_t sampleVolume = 65536; // max volume

	while (pos < filesize - 12)
	{
	  uint32_t blockName, blockSize;
	  
	  blockName = ((uint32_t *)(input + pos))[0];
	  pos += 4;
	  blockSize = ((uint32_t *)(input + pos))[0];
	  pos += 4;

	  blockName = SWAP32(blockName);
	  blockSize = SWAP32(blockSize);

    switch (blockName)
	  {
	  case 0x56484452: // VHDR
	  {
	    vhdrPtr = pos;
	    vhdrLen = blockSize;
	  }
	    break;
	    
	  case 0x4E414D45: // NAME
	  {
	    namePtr = pos;
	    nameLen = blockSize;
	    if (nameLen > 21) nameLen = 21;
	    memcpy(s->text, input + pos, nameLen);

	  }
	    break;
	    
	  case 0x424F4459: // BODY
	  {
	    bodyPtr = pos;
	    bodyLen = blockSize;
	  }
	    break;
	    
	  default: break;
	  }
    int size_aligned = blockSize + (blockSize & 1);
    if (pos + size_aligned > filesize)
      return false;
	  pos += (blockSize + (blockSize & 1));
	}
	if (vhdrPtr == 0 || vhdrLen < 20 || bodyPtr == 0)
	{
	  return false;
	}
	
	// kludge for some really strange IFFs
	if (bodyLen == 0)
	  bodyLen = filesize - bodyPtr;
	
	if (bodyPtr+bodyLen > filesize)
	  bodyLen = filesize - bodyPtr;
	
	loopStart  = SWAP32(((uint32_t *)(input + vhdrPtr))[0]);
	loopLength = SWAP32(((uint32_t *)(input + vhdrPtr + 4))[0]);
	if ((input + vhdrPtr + 15)[0] != 0) // sample type
	{
	  return false;
	}
	sampleVolume = SWAP32(((uint32_t *)(input + vhdrPtr + 16))[0]);
	if (sampleVolume > 65536)
	  sampleVolume = 65536;
	
	sampleVolume = (sampleVolume + 512) / 1024; // rounded
	if (sampleVolume > 64)
	  sampleVolume = 64;
	
	sampleLength = bodyLen;

	if (sampleLength == 0)
	{
	  return false;
	}
	
	int32_t maxSampleLength = config.maxSampleLength;
	if (is16Bit)
	  maxSampleLength *= 2;

	int8_t *sampleData = (int8_t *)malloc(sampleLength);
	if (sampleData == NULL)
	  return false;
	
	if (is16Bit)
	{
	  sampleLength >>= 1;
	  loopStart >>= 1;
	  loopLength >>= 1;
	}

	if (is16Bit) // FT2-specific 16SV format (little-endian samples)
	{
	  memcpy(sampleData, input + bodyPtr, sampleLength << 1);
	  int16_t *ptr16 = (int16_t *)sampleData;
	  
	  if (sampleLength > config.maxSampleLength)
	    sampleLength = config.maxSampleLength;
	  
	  double dAmp = 1.0;
	  
    const double dPeak = get16BitPeak(ptr16, sampleLength);
    if (dPeak > 0.0)
      dAmp = INT8_MAX / dPeak;

	  for (int32_t i = 0; i < sampleLength; i++)
	  {
	    int32_t smp32 = (int32_t)round(ptr16[i] * dAmp);
	    assert(smp32 >= -128 && smp32 <= 127); // shouldn't happen according to dAmp (but just in case)
	    smpDataPtr[i] = (int8_t)smp32;
	  }
	}
	else
	{
	  memcpy(sampleData, input + bodyPtr, sampleLength);

	  if (sampleLength > config.maxSampleLength)
	    sampleLength = config.maxSampleLength;
	  
	  memcpy(smpDataPtr, sampleData, sampleLength);
	}
	
	if (nameLen > 0) {
	  memset(s->text, 0, 22);
	  memcpy(s->text, input + namePtr, nameLen);
	}
	s->volume = sampleVolume;
	s->length = sampleLength;
	s->loopStart = loopStart;
	s->loopLength = loopLength;
	
	free(sampleData);
  return true;
}

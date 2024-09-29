// for finding memory leaks in debug mode with Visual Studio 
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pt2_sampler.h"
#include "pt2_replayer_light.h"
#include "pt2_audio.h"
#include "pt2_helpers.h"
#include "pt2_sample_loader.h"
#include "pt2_config.h"
#include "modloaders/pt2_load_mod15.h"
#include "modloaders/pt2_load_mod31.h"
#include "modloaders/pt2_xpk_unpack.h"
#include "modloaders/pt2_pp_unpack.h"

module_t * song; /*Edit PdV*/

static void fixZeroesInString(char *str, uint32_t maxLength); // converts zeroes to spaces in a string, up until the last zero found

module_t *modLoad2(uint8_t *data, uint32_t *data_size)
{
  uint32_t powerPackerID;
  
  uint8_t *modBuffer = NULL;
  module_t *newMod = NULL;
  
  // check if mod is a powerpacker mod
  memcpy(&powerPackerID, data, 4); // Note: does not account for endianess

  if (powerPackerID == 0x30325850) // "PX20" // Encrypted
  {
    goto loadError2;
  }
  else if (powerPackerID == 0x30325050) // "PP20"
  {
    modBuffer = unpackPP2(data, data_size);
    if (modBuffer == NULL)
      goto loadError2; // error msg is set in unpackPPModule()
  }
  else
  {
    if (detectXPK2(data, *data_size))
    {
      modBuffer = unpackXPK2(data, data_size);
      if (modBuffer == NULL)
        goto loadError2;

    }
    else
    {
      modBuffer = (uint8_t *)malloc(*data_size);
      if (modBuffer == NULL)
      {
        goto loadError2;
      }
      
      memcpy(modBuffer, data, *data_size);
    }
  }
  // read module is in now in modBuffer, time to load...
  
  bool isMod31 = detectMod31(modBuffer, *data_size);
  if (isMod31)
    newMod = loadMod31(modBuffer, *data_size);
  else
    newMod = loadMod15(modBuffer, *data_size);
  free(modBuffer);
  modBuffer = NULL;
  
  if (newMod == NULL)
    goto loadError2;

  // module is loaded, do some sanitation...
  
  newMod->header.name[20] = '\0';
  
  // convert illegal song name characters to space
  for (int32_t i = 0; i < 20; i++)
  {
    char tmpChar = newMod->header.name[i];
    if ((tmpChar < ' ' || tmpChar > '~') && tmpChar != '\0')
      tmpChar = ' ';
    
    newMod->header.name[i] = tmpChar;
  }

  fixZeroesInString(newMod->header.name, 20);
  
  moduleSample_t *s = newMod->samples;
  for (int32_t i = 0; i < MOD_SAMPLES; i++, s++)
  {
    s->text[22] = '\0';
    
    // convert illegal sample name characters to space
    for (int32_t j = 0; j < 22; j++)
    {
      char tmpChar = s->text[j];
      if ((tmpChar < ' ' || tmpChar > '~') && tmpChar != '\0')
        tmpChar = ' ';
      
      s->text[j] = tmpChar;
    }
    
    fixZeroesInString(s->text, 22);
    
    if (s->length > config.maxSampleLength)
      s->length = config.maxSampleLength;
    
    if ((uint8_t)s->volume > 64)
      s->volume = 64;
    
    if (s->loopLength < 2)
      s->loopLength = 2;
    
    // we don't support samples bigger than 65534 (or 128kB) bytes, disable uncompatible loops
    if (s->loopStart > config.maxSampleLength || s->loopStart+s->loopLength > config.maxSampleLength)
    {
      s->loopStart = 0;
      s->loopLength = 2;
    }
    
    // some modules are broken like this, adjust sample length if possible (this is ok if we have room)
    if (s->length > 0 && s->loopLength > 2 && s->loopStart+s->loopLength > s->length)
    {
      int32_t loopOverflowVal = (s->loopStart + s->loopLength) - s->length;
      if (s->length+loopOverflowVal <= config.maxSampleLength)
      {
        s->length += loopOverflowVal; // this is safe, we're allocating 65534 bytes per sample slot
      }
      else
      {
        s->loopStart = 0;
        s->loopLength = 2;
      }
    }
    
    // clear first two bytes of non-looping samples to prevent beep after sample has been played
    if (s->length >= 2 && s->loopStart+s->loopLength <= 2)
    {
      newMod->sampleData[s->offset+0] = 0;
      newMod->sampleData[s->offset+1] = 0;
    }
  }
  
  initializeModuleChannels(newMod);
  return newMod;
  
  loadError2:
    if (modBuffer != NULL)
      free(modBuffer);
    
    return NULL;
}

static void fixZeroesInString(char *str, uint32_t maxLength)
{
	int32_t i;

	for (i = maxLength-1; i >= 0; i--)
	{
		if (str[i] != '\0')
			break;
	}

	// convert zeroes to spaces
	if (i > 0)
	{
		for (int32_t j = 0; j < i; j++)
		{
			if (str[j] == '\0')
				str[j] = ' ';
		}
	}
}

void setupLoadedMod(void)
{
	moduleSample_t *s = song->samples;
	for (int32_t i = 0; i < MOD_SAMPLES; i++, s++)
		fillSampleRedoBuffer((uint8_t)i);

	modSetPos(0, 0);
	modSetPattern(0); // set pattern to 00 instead of first order's pattern


	// calculate MOD size
	// ui.updateSongSize = true;

	editor.muted[0] = false;
	editor.muted[1] = false;
	editor.muted[2] = false;
	editor.muted[3] = false;

	editor.editMoveAdd = 1;
	editor.currSample = 0;

	editor.playbackSeconds = 0;
	editor.playbackSecondsFrac = 0;

	editor.modLoaded = true;
	editor.blockMarkFlag = false;
	editor.sampleZero = false;
	editor.hiLowInstr = 0;

	// disable LED filter after module load (real PT doesn't do this)
	setLEDFilter(false);


	editor.timingMode = TEMPO_MODE_CIA;
	updateReplayerTimingMode();

	modSetSpeed(6);
	modSetTempo(song->header.initialTempo, false); // 125 for normal MODs, custom value for certain STK/UST MODs

	editor.samplePos = 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
// #include "pt2_mouse.h"
// #include "pt2_textout.h"
#include "pt2_helpers.h"
#include "pt2_sampler.h"
#include "pt2_config.h"
// #include "pt2_askbox.h"

bool modSave(char *fileName)
{
	FILE *f = fopen(fileName, "wb");
	if (f == NULL)
	{
		// displayErrorMsg("FILE I/O ERROR !");
		return false;
	}

	fwrite(song->header.name, 1, 20, f);

	for (int32_t i = 0; i < MOD_SAMPLES; i++)
	{
		moduleSample_t *s = &song->samples[i];

		fwrite(s->text, 1, 22, f);

		uint16_t length = SWAP16(s->length >> 1);
		fwrite(&length, sizeof (int16_t), 1, f);

		fputc(s->fineTune & 0xF, f);
		fputc(((uint8_t)s->volume > 64) ? 64 : s->volume, f);

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

		fwrite(&loopStart16, sizeof (int16_t), 1, f);
		fwrite(&loopLength16, sizeof (int16_t), 1, f);
	}

	fputc((uint8_t)song->header.songLength, f);
	fputc(0x7F, f); // ProTracker puts 0x7F at this place (restartPos/tempo in other trackers)

	for (int32_t i = 0; i < 128; i++)
		fputc((uint8_t)song->header.patternTable[i], f);

	int32_t numPatterns = 0;
	for (int32_t i = 0; i < 128; i++)
	{
		if (song->header.patternTable[i] > numPatterns)
			numPatterns = song->header.patternTable[i];
	}

	numPatterns++;
	if (numPatterns > MAX_PATTERNS)
		numPatterns = MAX_PATTERNS;

	fwrite((numPatterns <= 64) ? "M.K." : "M!K!", 1, 4, f);

	for (int32_t i = 0; i < numPatterns; i++)
	{
		note_t *note = song->patterns[i];
		for (int32_t j = 0; j < MOD_ROWS * PAULA_VOICES; j++, note++)
		{
			fputc((note->sample & 0xF0) | (note->period >> 8), f);
			fputc(note->period & 0xFF, f);
			fputc(((note->sample << 4) & 0xF0) | (note->command & 0x0F), f);
			fputc(note->param, f);
		}
	}

	for (int32_t i = 0; i < MOD_SAMPLES; i++)
	{
		moduleSample_t *s = &song->samples[i];
		const int8_t *smpPtr8 = &song->sampleData[s->offset];

		// clear first two bytes of non-looping samples (prevents stuck beep)
		if (s->length >= 2 && s->loopStart+s->loopLength == 2)
		{
			fputc(0, f);
			fputc(0, f);

			fwrite(&smpPtr8[2], 1, s->length-2, f);
		}
		else
		{
			fwrite(smpPtr8, 1, s->length, f);
		}
	}

	fclose(f);

	// displayMsg("MODULE SAVED !");
	// setMsgPointer();
  //
	// diskop.cached = false;
	// if (ui.diskOpScreenShown)
	// 	ui.updateDiskOpFileList = true;
	// 
	// updateWindowTitle(MOD_NOT_MODIFIED);
	return true;
}

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
  // fwrite(my_song->header.name, 1, 20, f);
  
  for (int32_t i = 0; i < MOD_SAMPLES; i++)
  {
    moduleSample_t *s = &my_song->samples[i];
    
    if (!checkSaveTarget(target, start, 22, target_size)) return -1;
    memcpy(target, &s->text, 22);
    target += 22;
    // fwrite(s->text, 1, 22, f);
    
    uint16_t length = SWAP16(s->length >> 1);
    if (!checkSaveTarget(target, start, sizeof (int16_t), target_size)) return -1;
    memcpy(target, &length, sizeof (int16_t));
    target += sizeof (int16_t);
    // fwrite(&length, sizeof (int16_t), 1, f);
    
    if (!checkSaveTarget(target, start, 2, target_size)) return -1;
    target[0] = s->fineTune & 0xF;
    target[1] = ((uint8_t)s->volume > 64) ? 64 : s->volume;
    target += 2;
    // fputc(s->fineTune & 0xF, f);
    // fputc(((uint8_t)s->volume > 64) ? 64 : s->volume, f);
    
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
    // fwrite(&loopStart16, sizeof (int16_t), 1, f);
    // fwrite(&loopLength16, sizeof (int16_t), 1, f);
  }
  
  if (!checkSaveTarget(target, start, 2, target_size)) return -1;
  target[0] = (uint8_t)my_song->header.songLength;
  target[1] = 0x7F;
  target += 2;
  // fputc((uint8_t)my_song->header.songLength, f);
  // fputc(0x7F, f); // ProTracker puts 0x7F at this place (restartPos/tempo in other trackers)
  
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
  // fwrite((numPatterns <= 64) ? "M.K." : "M!K!", 1, 4, f);
  
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
      // fputc((note->sample & 0xF0) | (note->period >> 8), f);
      // fputc(note->period & 0xFF, f);
      // fputc(((note->sample << 4) & 0xF0) | (note->command & 0x0F), f);
      // fputc(note->param, f);
    }
  }
  
  for (int32_t i = 0; i < MOD_SAMPLES; i++)
  {
    moduleSample_t *s = &my_song->samples[i];
    const int8_t *smpPtr8 = &my_song->sampleData[s->offset];
    
    // clear first two bytes of non-looping samples (prevents stuck beep)
    if (s->length >= 2 && s->loopStart+s->loopLength == 2)
    {
      // fputc(0, f);
      // fputc(0, f);
      if (!checkSaveTarget(target, start, 2, target_size)) return -1;
      target[0] = 0;
      target[1] = 0;
      target += 2;
      if (!checkSaveTarget(target, start, (s->length-2), target_size)) return -1;
      memcpy(target, &smpPtr8[2], s->length-2);
      target += (s->length-2);
      // fwrite(&smpPtr8[2], 1, s->length-2, f);
    }
    else
    {
      if (!checkSaveTarget(target, start, s->length, target_size)) return -1;
      memcpy(target, smpPtr8, s->length);
      target += s->length;
      // fwrite(smpPtr8, 1, s->length, f);
    }
  }
  
  // fclose(f);
  //
  // displayMsg("MODULE SAVED !");
  // setMsgPointer();
  //
  // diskop.cached = false;
  // if (ui.diskOpScreenShown)
  // 	ui.updateDiskOpFileList = true;
  // 
  // updateWindowTitle(MOD_NOT_MODIFIED);
  return target - start;
}

// 
// bool saveModule(bool checkIfFileExist, bool giveNewFreeFilename)
// {
// 	char fileName[128], tmpBuffer[64];
// 	struct stat statBuffer;
// 
// 	memset(tmpBuffer, 0, sizeof (tmpBuffer));
// 	memset(fileName, 0, sizeof (fileName));
// 
// 	if (config.modDot)
// 	{
// 		// extension.filename
// 		if (song->header.name[0] == '\0')
// 		{
// 			strcat(fileName, "mod.untitled");
// 		}
// 		else
// 		{
// 			strcat(fileName, "mod.");
// 			for (int32_t i = 4; i < 20+4; i++)
// 			{
// 				fileName[i] = (char)tolower(song->header.name[i-4]);
// 				if (fileName[i] == '\0') break;
// 				sanitizeFilenameChar(&fileName[i]);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		// filename.extension
// 		if (song->header.name[0] == '\0')
// 		{
// 			strcat(fileName, "untitled.mod");
// 		}
// 		else
// 		{
// 			for (int32_t i = 0; i < 20; i++)
// 			{
// 				fileName[i] = (char)tolower(song->header.name[i]);
// 				if (fileName[i] == '\0') break;
// 				sanitizeFilenameChar(&fileName[i]);
// 			}
// 			strcat(fileName, ".mod");
// 		}
// 	}
// 
// 	if (giveNewFreeFilename && stat(fileName, &statBuffer) == 0)
// 	{
// 		for (int32_t i = 1; i <= 999; i++)
// 		{
// 			if (config.modDot)
// 			{
// 				// extension.filename
// 				if (song->header.name[0] == '\0')
// 				{
// 					sprintf(fileName, "mod.untitled-%d", i);
// 				}
// 				else
// 				{
// 					for (int32_t j = 0; j < 20; j++)
// 					{
// 						tmpBuffer[j] = (char)tolower(song->header.name[j]);
// 						if (tmpBuffer[j] == '\0') break;
// 						sanitizeFilenameChar(&tmpBuffer[j]);
// 					}
// 					sprintf(fileName, "mod.%s-%d", tmpBuffer, i);
// 				}
// 			}
// 			else
// 			{
// 				// filename.extension
// 				if (song->header.name[0] == '\0')
// 				{
// 					sprintf(fileName, "untitled-%d.mod", i);
// 				}
// 				else
// 				{
// 					for (int32_t j = 0; j < 20; j++)
// 					{
// 						tmpBuffer[j] = (char)tolower(song->header.name[j]);
// 						if (tmpBuffer[j] == '\0') break;
// 						sanitizeFilenameChar(&tmpBuffer[j]);
// 					}
// 					sprintf(fileName, "%s-%d.mod", tmpBuffer, i);
// 				}
// 			}
// 
// 			if (stat(fileName, &statBuffer) != 0)
// 				break; // this filename can be used
// 		}
// 	}
// 
// 	if (checkIfFileExist && stat(fileName, &statBuffer) == 0)
// 	{
// 		if (!askBox(ASKBOX_YES_NO, "OVERWRITE FILE ?"))
// 			return false;
// 	}
// 
// 	return modSave(fileName);
// }

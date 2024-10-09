#pragma once

#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include "pt2_header.h"
#include "pt2_hpc.h"
#include "pt2_paula.h"

#ifdef __cplusplus
extern "C" {
#endif

// for .WAV sample loading/saving
typedef struct wavHeader_t
{
	uint32_t chunkID, chunkSize, format, subchunk1ID, subchunk1Size;
	uint16_t audioFormat, numChannels;
	uint32_t sampleRate, byteRate;
	uint16_t blockAlign, bitsPerSample;
	uint32_t subchunk2ID, subchunk2Size;
} wavHeader_t;

typedef struct sampleLoop_t
{
	uint32_t dwIdentifier, dwType, dwStart;
	uint32_t dwEnd, dwFraction, dwPlayCount;
} sampleLoop_t;

typedef struct samplerChunk_t
{
	uint32_t chunkID, chunkSize, dwManufacturer, dwProduct;
	uint32_t dwSamplePeriod, dwMIDIUnityNote, wMIDIPitchFraction;
	uint32_t dwSMPTEFormat, dwSMPTEOffset, cSampleLoops, cbSamplerData;
	sampleLoop_t loop;
} samplerChunk_t;

typedef struct mptExtraChunk_t
{
	uint32_t chunkID, chunkSize, flags;
	uint16_t defaultPan, defaultVolume, globalVolume, reserved;
	uint8_t vibratoType, vibratoSweep, vibratoDepth, vibratoRate;
} mptExtraChunk_t;
// -----------------------------------------

typedef struct note_t
{
	uint8_t param, sample, command;
	uint16_t period;
} note_t;

typedef struct moduleHeader_t
{
	char name[20 + 1];
	uint16_t patternTable[128], songLength;
	uint16_t initialTempo; // used for STK/UST modules after module is loaded
} moduleHeader_t;

typedef struct moduleSample_t
{
	char text[22 + 1];
	int8_t volume;
	uint8_t fineTune;
	int32_t offset, length, loopStart, loopLength;
} moduleSample_t;

typedef struct moduleChannel_t
{
	int8_t *n_start, *n_wavestart, *n_loopstart, n_volume, n_dmabit;
	int8_t n_toneportdirec, n_pattpos, n_loopcount;
	uint8_t n_wavecontrol, n_glissfunk, n_sampleoffset, n_toneportspeed;
	uint8_t n_vibratocmd, n_tremolocmd, n_finetune, n_funkoffset, n_samplenum;
	uint8_t n_vibratopos, n_tremolopos;
	int16_t n_period, n_note, n_wantedperiod;
	uint16_t n_cmd, n_length, n_replen;
	uint32_t n_scopedelta, n_chanindex;

} moduleChannel_t;

typedef struct module_t
{
	bool loaded, modified;
	int8_t *sampleData;

	volatile int32_t tick, speed;

	int8_t row; // used for different things, so must not be internal to replayer

	moduleHeader_t header;
	moduleSample_t samples[MOD_SAMPLES];
	moduleChannel_t channels[PAULA_VOICES];
	note_t *patterns[MAX_PATTERNS];

	// for pattern viewer
	int8_t currRow;
	int32_t currSpeed, currBPM;
	uint16_t currPos, currPattern;

	// for MOD2WAV progress bar
	uint32_t rowsCounter, rowsInTotal;
	float songDuration; // duration in seconds
} module_t;

typedef struct editor_t
{
	volatile int8_t mod2WavNumLoops, mod2WavFadeOutSeconds;
	volatile bool songPlaying, programRunning, mod2WavOngoing, pat2SmpOngoing, mainLoopOngoing, abortMod2Wav, mod2WavFadeOut;

	char *entryNameTmp, *currPath, *dropTempFileName;
	UNICHAR *fileNameTmpU, *currPathU, *modulesPathU, *samplesPathU;

	bool multiFlag, metroFlag, keypadToggle8CFlag, normalizeFiltersFlag;
	bool sampleAllFlag, halfClipFlag, newOldFlag, pat2SmpHQ, mixFlag;
	bool modLoaded, autoInsFlag, repeatKeyFlag, sampleZero, tuningToneFlag;
	bool stepPlayEnabled, stepPlayBackwards, blockBufferFlag, blockMarkFlag, didQuantize;
	bool swapChannelFlag, configFound, chordLengthMin, rowVisitTable[128 * MOD_ROWS];
	bool muted[PAULA_VOICES];

	int8_t smpRedoFinetunes[MOD_SAMPLES], smpRedoVolumes[MOD_SAMPLES], multiModeNext[4], trackPattFlag;
	int8_t *smpRedoBuffer[MOD_SAMPLES], currSample, recordMode, sampleFrom, sampleTo, autoInsSlot;
	int8_t hiLowInstr, note1, note2, note3, note4, oldNote1, oldNote2, oldNote3, oldNote4, stepPlayLastMode;
	uint8_t playMode, currMode, tuningChan, tuningVol, errorMsgCounter, buffFromPos, buffToPos;
	uint8_t blockFromPos, blockToPos, timingMode, f6Pos, f7Pos, f8Pos, f9Pos, f10Pos, keyOctave, pNoteFlag;
	uint8_t tuningNote, resampleNote, initialTempo, initialSpeed, editMoveAdd;

	int16_t modulateSpeed;
	uint16_t metroSpeed, metroChannel, sampleVol;
	uint16_t effectMacros[10], currPlayNote, vol1, vol2, lpCutOff, hpCutOff;
	int32_t smpRedoLoopStarts[MOD_SAMPLES], smpRedoLoopLengths[MOD_SAMPLES], smpRedoLengths[MOD_SAMPLES];
	int32_t oldTempo, modulatePos, modulateOffset, markStartOfs, markEndOfs, samplePos, chordLength;
	uint32_t playbackSeconds;
	uint64_t playbackSecondsFrac;

	uint32_t framesPassed;

	note_t trackBuffer[MOD_ROWS], cmdsBuffer[MOD_ROWS], blockBuffer[MOD_ROWS];
	note_t patternBuffer[MOD_ROWS * PAULA_VOICES], undoBuffer[MOD_ROWS * PAULA_VOICES];

#ifdef __APPLE__
	bool macCmdQIssued;
#endif
} editor_t;

typedef struct diskop_t
{
	volatile bool cached, isFilling, forceStopReading;
	bool modPackFlg;
	int8_t mode, smpSaveType;
	int32_t numEntries, scrollOffset;
} diskop_t;
extern editor_t editor;
extern diskop_t diskop;
extern module_t *song; // pt2_main.c

#ifdef __cplusplus
}
#endif

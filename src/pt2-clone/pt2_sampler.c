// for finding memory leaks in debug mode with Visual Studio 
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "pt2_helpers.h"
#include "pt2_audio.h"
#include "pt2_tables.h"
#include "pt2_sampler.h"
#include "pt2_config.h"
#include "pt2_rcfilters.h"
#include "pt2_replayer_light.h"

sampler_t sampler; // globalized

void killSample(void)
{
	if (editor.sampleZero)
		return;

	turnOffVoices();
	moduleSample_t *s = &song->samples[editor.currSample];

	s->fineTune = 0;
	s->volume = 0;
	s->length = 0;
	s->loopStart = 0;
	s->loopLength = 2;

	memset(s->text, 0, sizeof (s->text));
	memset(&song->sampleData[(editor.currSample * config.maxSampleLength)], 0, config.maxSampleLength);

	editor.samplePos = 0;
}

void upSample(void)
{
	if (editor.sampleZero)
		return;

	moduleSample_t *s = &song->samples[editor.currSample];

	int32_t newLength = (s->length >> 1) & config.maxSampleLength;
	if (newLength < 2)
		return;

	turnOffVoices();

	// upsample
	int8_t *ptr8 = &song->sampleData[s->offset];
	for (int32_t i = 0; i < newLength; i++)
		ptr8[i] = ptr8[i << 1];

	// clear junk after shrunk sample
	if (newLength < config.maxSampleLength)
		memset(&ptr8[newLength], 0, config.maxSampleLength - newLength);

	s->length = newLength;
	s->loopStart = (s->loopStart >> 1) & ~1;
	s->loopLength = (s->loopLength >> 1) & ~1;

	if (s->loopLength < 2)
	{
		s->loopStart = 0;
		s->loopLength = 2;
	}

	fixSampleBeep(s);
}

void downSample(void)
{
	if (editor.sampleZero)
		return;

	moduleSample_t *s = &song->samples[editor.currSample];

	int32_t newLength = s->length << 1;
	if (newLength > config.maxSampleLength)
		newLength = config.maxSampleLength;

	turnOffVoices();

	// downsample
	int8_t *ptr8 = &song->sampleData[s->offset];
	int8_t *ptr8_2 = ptr8 - 1;
	for (int32_t i = s->length-1; i > 0; i--)
	{
		ptr8[i<<1] = ptr8[i];
		ptr8_2[i<<1] = ptr8_2[i];
	}

	s->length = newLength;

	if (s->loopLength > 2)
	{
		int32_t loopStart = s->loopStart << 1;
		int32_t loopLength = s->loopLength << 1;

		if (loopStart+loopLength > s->length)
		{
			loopStart = 0;
			loopLength = 2;
		}

		s->loopStart = loopStart;
		s->loopLength = loopLength;
	}

	fixSampleBeep(s);
}

void fixSampleBeep2(moduleSample_t *s, int8_t * sampleData)
{
	if (s->length >= 2 && s->loopStart+s->loopLength <= 2)
	{
		sampleData[s->offset+0] = 0;
		sampleData[s->offset+1] = 0;
	}
}

void fixSampleBeep(moduleSample_t *s)
{
  fixSampleBeep2(s, song->sampleData);
}

void updateSamplePos(void)
{
	moduleSample_t *s;

	assert(editor.currSample >= 0 && editor.currSample <= 30);
	if (editor.currSample >= 0 && editor.currSample <= 30)
	{
		s = &song->samples[editor.currSample];
		if (editor.samplePos > s->length)
			editor.samplePos = s->length;

	}
}

void fillSampleFilterUndoBuffer(void)
{
	moduleSample_t *s;

	assert(editor.currSample >= 0 && editor.currSample <= 30);
	if (editor.currSample >= 0 && editor.currSample <= 30)
	{
		s = &song->samples[editor.currSample];
		memcpy(sampler.sampleUndoCopy, &song->sampleData[s->offset], s->length);
	}
}

// static int8_t getScaledSample(int32_t index)
// {
// 	if (sampler.samLength <= 0 || index < 0 || index > sampler.samLength)
// 		return 0;
// 
// 	const int8_t *ptr8 = sampler.samStart;
// 	if (ptr8 == NULL)
// 		return 0;
// 
// 	return ptr8[index] >> 2;
// }

void highPassSample(int32_t cutOff)
{
	onePoleFilter_t filterHi;

	assert(editor.currSample >= 0 && editor.currSample <= 30);

	if (editor.sampleZero)
		return;

	if (cutOff == 0)
		return;

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length == 0)
		return;

	int32_t from = 0;
	int32_t to = s->length;

	if (editor.markStartOfs != -1)
	{
		from = editor.markStartOfs;
		to = editor.markEndOfs;

		if (to > s->length)
			to = s->length;

		if (from == to || from >= s->length || to < from)
		{
			from = 0;
			to = s->length;
		}
	}

	double *dSampleData = (double *)malloc(s->length * sizeof (double));
	if (dSampleData == NULL)
		return;

	fillSampleFilterUndoBuffer();

	// setup filter coefficients

	double dBaseFreq = FILTERS_BASE_FREQ;

	double dCutOff = (double)cutOff;
	if (dCutOff >= dBaseFreq/2.0)
	{
		dCutOff = dBaseFreq/2.0;
		editor.hpCutOff = (uint16_t)dCutOff;
	}

	setupOnePoleFilter(dBaseFreq, dCutOff, &filterHi);

	clearOnePoleFilterState(&filterHi);
	if (to <= s->length)
	{
		const int8_t *smpPtr = &song->sampleData[s->offset];
		for (int32_t i = from; i < to; i++)
		{
			double dSmp = smpPtr[i];
			onePoleHPFilter(&filterHi, dSmp, &dSampleData[i]);
		}
	}

	double dAmp = 1.0;
	if (editor.normalizeFiltersFlag)
	{
		const double dPeak = getDoublePeak(dSampleData, s->length);
		if (dPeak > 0.0)
			dAmp = INT8_MAX / dPeak;
	}

	int8_t *smpPtr = &song->sampleData[s->offset];
	for (int32_t i = from; i < to; i++)
	{
		int16_t smp16 = (int16_t)round(dSampleData[i] * dAmp);
		CLAMP8(smp16);
		smpPtr[i] = (int8_t)smp16;
	}

	free(dSampleData);

	fixSampleBeep(s);
}

void lowPassSample(int32_t cutOff)
{
	onePoleFilter_t filterLo;

	if (editor.sampleZero)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);

	if (cutOff == 0)
		return;

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length == 0)
		return;

	int32_t from = 0;
	int32_t to = s->length;

	if (editor.markStartOfs != -1)
	{
		from = editor.markStartOfs;
		to = editor.markEndOfs;

		if (to > s->length)
			to = s->length;

		if (from == to || from >= s->length || to < from)
		{
			from = 0;
			to = s->length;
		}
	}

	double *dSampleData = (double *)malloc(s->length * sizeof (double));
	if (dSampleData == NULL)
		return;

	fillSampleFilterUndoBuffer();

	// setup filter coefficients

	double dBaseFreq = FILTERS_BASE_FREQ;

	double dCutOff = (double)cutOff;
	if (dCutOff >= dBaseFreq/2.0)
	{
		dCutOff = dBaseFreq/2.0;
		editor.lpCutOff = (uint16_t)dCutOff;
	}

	setupOnePoleFilter(dBaseFreq, dCutOff, &filterLo);

	// copy over sample data to double buffer
	for (int32_t i = 0; i < s->length; i++)
		dSampleData[i] = song->sampleData[s->offset+i];

	clearOnePoleFilterState(&filterLo);
	if (to <= s->length)
	{
		const int8_t *smpPtr = &song->sampleData[s->offset];
		for (int32_t i = from; i < to; i++)
		{ 
			double dSmp = smpPtr[i];
			onePoleLPFilter(&filterLo, dSmp, &dSampleData[i]);
		}
	}

	double dAmp = 1.0;

	if (editor.normalizeFiltersFlag)
	{
		const double dPeak = getDoublePeak(dSampleData, s->length);
		if (dPeak > 0.0)
			dAmp = INT8_MAX / dPeak;
	}

	int8_t *smpPtr = &song->sampleData[s->offset];
	for (int32_t i = from; i < to; i++)
	{
		int16_t smp16 = (int16_t)round(dSampleData[i] * dAmp);
		CLAMP8(smp16);
		smpPtr[i] = (int8_t)smp16;
	}

	free(dSampleData);

	fixSampleBeep(s);
}

void redoSampleData(int8_t sample)
{
	if (editor.sampleZero)
		return;

	assert(sample >= 0 && sample <= 30);
	moduleSample_t *s = &song->samples[sample];

	turnOffVoices();

	if (editor.smpRedoBuffer[sample] != NULL && editor.smpRedoLengths[sample] > 0)
	{
		memcpy(&song->sampleData[s->offset], editor.smpRedoBuffer[sample], editor.smpRedoLengths[sample]);

		if (editor.smpRedoLengths[sample] < config.maxSampleLength)
			memset(&song->sampleData[s->offset + editor.smpRedoLengths[sample]], 0, config.maxSampleLength - editor.smpRedoLengths[sample]);
	}
	else
	{
		memset(&song->sampleData[s->offset], 0, config.maxSampleLength);
	}

	s->fineTune = editor.smpRedoFinetunes[sample];
	s->volume = editor.smpRedoVolumes[sample];
	s->length = editor.smpRedoLengths[sample];
	s->loopStart = editor.smpRedoLoopStarts[sample];
	s->loopLength = (editor.smpRedoLoopLengths[sample] < 2) ? 2 : editor.smpRedoLoopLengths[sample];

	editor.samplePos = 0;
}

void fillSampleRedoBuffer(int8_t sample)
{
	assert(sample >= 0 && sample <= 30);
	moduleSample_t *s = &song->samples[sample];

	if (editor.smpRedoBuffer[sample] != NULL)
	{
		free(editor.smpRedoBuffer[sample]);
		editor.smpRedoBuffer[sample] = NULL;
	}

	editor.smpRedoFinetunes[sample] = s->fineTune;
	editor.smpRedoVolumes[sample] = s->volume;
	editor.smpRedoLengths[sample] = s->length;
	editor.smpRedoLoopStarts[sample] = s->loopStart;
	editor.smpRedoLoopLengths[sample] = s->loopLength;

	if (s->length > 0)
	{
		editor.smpRedoBuffer[sample] = (int8_t *)malloc(s->length);
		if (editor.smpRedoBuffer[sample] != NULL)
			memcpy(editor.smpRedoBuffer[sample], &song->sampleData[s->offset], s->length);
	}
}

bool allocSamplerVars(void) // must be called after config is loaded
{
	sampler.copyBuf = (int8_t *)malloc(config.maxSampleLength);
	sampler.blankSample = (int8_t *)calloc(config.maxSampleLength, 1);
	sampler.sampleUndoCopy = (int8_t *)calloc(config.maxSampleLength, 1);

	if (sampler.copyBuf == NULL || sampler.blankSample == NULL || sampler.sampleUndoCopy == NULL)
		return false;

	return true;
}

void deAllocSamplerVars(void)
{
	if (sampler.copyBuf != NULL)
	{
		free(sampler.copyBuf);
		sampler.copyBuf = NULL;
	}

	if (sampler.blankSample != NULL)
	{
		free(sampler.blankSample);
		sampler.blankSample = NULL;
	}
	
	if (sampler.sampleUndoCopy != NULL)
	{
		free(sampler.sampleUndoCopy);
		sampler.sampleUndoCopy = NULL;
	}

	for (int32_t i = 0; i < MOD_SAMPLES; i++)
	{
		if (editor.smpRedoBuffer[i] != NULL)
		{
			free(editor.smpRedoBuffer[i]);
			editor.smpRedoBuffer[i] = NULL;
		}
	}
}

void samplerRemoveDcOffset(void)
{
	if (editor.sampleZero)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length == 0)
		return;

	int8_t *smpDat = &song->sampleData[s->offset];

	int32_t from = 0;
	int32_t to = s->length;

	if (editor.markStartOfs != -1)
	{
		from = editor.markStartOfs;
		to = editor.markEndOfs;

		if (to > s->length)
			to = s->length;

		if (from == to || from >= s->length || to < from)
		{
			from = 0;
			to = s->length;
		}
	}

	if (to <= 0)
		return;

	// calculate offset value
	int32_t offset = 0;
	for (int32_t i = from; i < to; i++)
		offset += smpDat[i];
	offset /= to;

	// remove DC offset
	for (int32_t i = from; i < to; i++)
	{
		int32_t smp32 = smpDat[i] - offset;
		CLAMP8(smp32);
		smpDat[i] = (int8_t)smp32;
	}

	fixSampleBeep(s);
}

#define INTRP_LINEAR_TAPS 2
#define INTRP8_LINEAR(s1, s2, f) /* output: -127..128 */ \
	s2 -= s1; \
	s2 *= (int32_t)(f >> 16); \
	s1 <<= 8; \
	s2 >>= 16-8; \
	s1 += s2; \
	s1 >>= 8; \

void samplerResample(void)
{
	int32_t samples[INTRP_LINEAR_TAPS];

	if (editor.sampleZero)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);
	assert(editor.tuningNote <= 35 && editor.resampleNote <= 35);

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length == 0)
		return;

	// setup resampling variables
	int32_t readPos = 0;
	int32_t writePos = 0;
	int8_t *writeData = &song->sampleData[s->offset];
	int16_t refPeriod = periodTable[editor.tuningNote];
	int16_t newPeriod = periodTable[(37 * (s->fineTune & 0xF)) + editor.resampleNote];
	int32_t readLength = s->length;
	int32_t writeLength = (readLength * newPeriod) / refPeriod;

	if (readLength == writeLength)
		return; // no resampling needed

	// allocate memory for our sample duplicate
	int8_t *readData = (int8_t *)malloc(s->length);
	if (readData == NULL)
		return;

	if (writeLength <= 0)
	{
		free(readData);
		return;
	}

	uint64_t delta64 = ((uint64_t)readLength << 32) / writeLength;
	assert(delta64 != 0);

	writeLength = writeLength & ~1;
	if (writeLength > config.maxSampleLength)
		writeLength = config.maxSampleLength;

	memcpy(readData, writeData, readLength);

	// resample

	turnOffVoices();

	uint64_t frac64 = 0;
	while (writePos < writeLength)
	{
		// collect samples for interpolation
		for (int32_t i = 0; i < INTRP_LINEAR_TAPS; i++)
		{
			int32_t pos = readPos + i;
			if (pos >= readLength)
				samples[i] = 0;
			else
				samples[i] = readData[pos];
		}

		INTRP8_LINEAR(samples[0], samples[1], frac64);
		writeData[writePos++] = (int8_t)samples[0];

		frac64 += delta64;
		readPos += frac64 >> 32;
		frac64 &= UINT32_MAX;
	}
	free(readData);

	// wipe non-used data in new sample
	if (writeLength < config.maxSampleLength)
		memset(&writeData[writePos], 0, config.maxSampleLength - writeLength);

	// update sample attributes
	s->length = writeLength;
	s->fineTune = 0;

	// scale loop points (and deactivate if overflowing)
	if ((s->loopStart + s->loopLength) > 2)
	{
		int32_t loopStart = (int32_t)(((uint64_t)s->loopStart << 32) / delta64) & ~1;
		int32_t loopLength = (int32_t)(((uint64_t)s->loopLength << 32) / delta64) & ~1;

		if (loopStart+loopLength > s->length)
		{
			s->loopStart = 0;
			s->loopLength = 2;
		}
		else
		{
			s->loopStart = loopStart;
			s->loopLength = loopLength;
		}
	}

	fixSampleBeep(s);
}

// reads two hex chars from pointer and converts them to one byte
// static uint8_t hexToInteger2(char *ptr)
// {
// 	char hi = ptr[0];
// 	char lo = ptr[1];
// 
// 	if (hi >= '0' && hi <= '9')
// 		hi -= '0';
// 	else if (hi >= 'A' && hi <= 'F')
// 		hi -= 'A'-10;
// 	else if (hi >= 'a' && hi <= 'f')
// 		hi -= 'a'-10;
// 	else
// 		hi = 0;
// 
// 	if (lo >= '0' && lo <= '9')
// 		lo -= '0';
// 	else if (lo >= 'A' && lo <= 'F')
// 		lo -= 'A'-10;
// 	else if (lo >= 'a' && lo <= 'f')
// 		lo -= 'a'-10;
// 	else
// 		lo = 0;
// 
// 	return (hi << 4) | lo;
// }

// this is actually treble increase
void boostSample(int32_t sample, bool ignoreMark)
{
	assert(sample >= 0 && sample <= 30);

	moduleSample_t *s = &song->samples[sample];
	if (s->length == 0)
		return; // don't display warning/show warning pointer, it is done elsewhere

	int8_t *smpDat = &song->sampleData[s->offset];

	int32_t from = 0;
	int32_t to = s->length;

	if (!ignoreMark)
	{
		if (editor.markStartOfs != -1)
		{
			from = editor.markStartOfs;
			to = editor.markEndOfs;

			if (to > s->length)
				to = s->length;

			if (from == to || from >= s->length || to < from)
			{
				from = 0;
				to = s->length;
			}
		}
	}

	int8_t prevSmp = 0;
	for (int32_t i = from; i < to; i++)
	{
		int16_t tmp16 = smpDat[i] - prevSmp;

		int16_t smp16;
		if (tmp16 < 0)
			smp16 = smpDat[i] - (-tmp16 >> 2);
		else
			smp16 = smpDat[i] + ( tmp16 >> 2);

		prevSmp = smpDat[i];

		CLAMP8(smp16);
		smpDat[i] = (int8_t)smp16;
	}

	fixSampleBeep(s);

	// don't redraw sample here, it is done elsewhere
}

// this is actually treble decrease
void filterSample(int32_t sample, bool ignoreMark)
{
	assert(sample >= 0 && sample <= 30);

	moduleSample_t *s = &song->samples[sample];
	if (s->length == 0)
		return; // don't display warning/show warning pointer, it is done elsewhere

	int8_t *smpDat = &song->sampleData[s->offset];

	int32_t from = 1;
	int32_t to = s->length;

	if (!ignoreMark)
	{
		if (editor.markStartOfs != -1)
		{
			from = editor.markStartOfs;
			to = editor.markEndOfs;

			if (to > s->length)
				to = s->length;

			if (from == to || from >= s->length || to < from)
			{
				from = 0;
				to = s->length;
			}
		}
	}

	if (to < 1)
		return;
	to--;

	for (int32_t i = from; i < to; i++)
		smpDat[i] = (smpDat[i+0] + smpDat[i+1]) >> 1;

	fixSampleBeep(s);
}

void samplerSamCopy(void)
{
	if (editor.sampleZero)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);
	moduleSample_t *s = &song->samples[editor.currSample];

	if (s->length == 0)
		return;

	if (editor.markStartOfs == -1)
		return;

	if (editor.markEndOfs-editor.markStartOfs <= 0)
		return;

	sampler.copyBufSize = editor.markEndOfs - editor.markStartOfs;

	if ((int32_t)(editor.markStartOfs + sampler.copyBufSize) > config.maxSampleLength)
		return;

	memcpy(sampler.copyBuf, &song->sampleData[s->offset+editor.markStartOfs], sampler.copyBufSize);
}

void samplerSamPaste(void)
{
	if (editor.sampleZero)
		return;

	if (sampler.copyBuf == NULL || sampler.copyBufSize == 0)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length > 0 && editor.markStartOfs == -1)
		return;

	int32_t markStart = editor.markStartOfs;
	if (s->length == 0)
		markStart = 0;

	if (s->length+sampler.copyBufSize > config.maxSampleLength)
		return;

	int8_t *tmpBuf = (int8_t *)malloc(config.maxSampleLength);
	if (tmpBuf == NULL)
		return;

	uint32_t readPos = 0;
	turnOffVoices();

	// copy start part
	if (markStart > 0)
	{
		memcpy(&tmpBuf[readPos], &song->sampleData[s->offset], markStart);
		readPos += markStart;
	}

	// copy actual buffer
	memcpy(&tmpBuf[readPos], sampler.copyBuf, sampler.copyBufSize);

	// copy end part
	if (markStart >= 0)
	{
		readPos += sampler.copyBufSize;

		if (s->length-markStart > 0)
			memcpy(&tmpBuf[readPos], &song->sampleData[s->offset+markStart], s->length - markStart);
	}

	int32_t newLength = (s->length + sampler.copyBufSize) & ~1;
	if (newLength > config.maxSampleLength)
		newLength = config.maxSampleLength;

	sampler.samLength = s->length = newLength;

	if (s->loopLength > 2) // loop enabled?
	{
		if (markStart > s->loopStart)
		{
			if (markStart < s->loopStart+s->loopLength)
			{
				// we pasted data inside the loop, increase loop length

				if (s->loopLength+sampler.copyBufSize > config.maxSampleLength)
				{
					s->loopStart = 0;
					s->loopLength = 2;
				}
				else
				{
					s->loopLength = (s->loopLength + sampler.copyBufSize) & config.maxSampleLength;
					if (s->loopStart+s->loopLength > s->length)
					{
						s->loopStart = 0;
						s->loopLength = 2;
					}
				}
			}

			// we pasted data after the loop, don't modify loop points
		}
		else
		{
			// we pasted data before the loop, adjust loop start point
			if (s->loopStart+sampler.copyBufSize > config.maxSampleLength)
			{
				s->loopStart = 0;
				s->loopLength = 2;
			}
			else
			{
				s->loopStart = (s->loopStart + sampler.copyBufSize) & config.maxSampleLength;
				if (s->loopStart+s->loopLength > s->length)
				{
					s->loopStart = 0;
					s->loopLength = 2;
				}
			}
		}
	}

	memcpy(&song->sampleData[s->offset], tmpBuf, s->length);

	// clear data after sample's length (if present)
	if (s->length < config.maxSampleLength)
		memset(&song->sampleData[s->offset+s->length], 0, config.maxSampleLength - s->length);

	free(tmpBuf);

	editor.markStartOfs = -1;

	fixSampleBeep(s);
}

// static void playCurrSample(uint8_t chn, int32_t startOffset, int32_t endOffset, bool playWaveformFlag)
// {
// 	assert(editor.currSample >= 0 && editor.currSample <= 30);
// 	assert(chn < PAULA_VOICES);
// 	assert(editor.currPlayNote <= 35);
// 
// 	moduleSample_t *s = &song->samples[editor.currSample];
// 	moduleChannel_t *ch = &song->channels[chn];
// 
// 	ch->n_samplenum = editor.currSample;
// 	ch->n_volume = s->volume;
// 	ch->n_period = periodTable[(37 * (s->fineTune & 0xF)) + editor.currPlayNote];
// 	
// 	if (playWaveformFlag)
// 	{
// 		ch->n_start = &song->sampleData[s->offset];
// 		ch->n_length = (uint16_t)((s->loopStart > 0) ? (s->loopStart + s->loopLength) >> 1 : s->length >> 1);
// 		ch->n_loopstart = &song->sampleData[s->offset + s->loopStart];
// 		ch->n_replen = (uint16_t)(s->loopLength >> 1);
// 	}
// 	else
// 	{
// 		ch->n_start = &song->sampleData[s->offset + startOffset];
// 		ch->n_length = (uint16_t)((uint32_t)(endOffset - startOffset) >> 1);
// 		ch->n_loopstart = &song->sampleData[s->offset];
// 		ch->n_replen = 1;
// 	}
// 
// 	if (ch->n_length == 0)
// 		ch->n_length = 1;
// 
// 	const uint32_t voiceAddr = 0xDFF0A0 + (chn * 16);
// 
// 	paulaWriteWord(voiceAddr + 8, ch->n_volume);
// 	paulaWriteWord(voiceAddr + 6, ch->n_period);
// 	paulaWritePtr(voiceAddr + 0, ch->n_start);
// 	paulaWriteWord(voiceAddr + 4, ch->n_length);
// 
// 	if (!editor.muted[chn])
// 		paulaWriteWord(0xDFF096, 0x8000 | ch->n_dmabit); // voice DMA on
// 	else
// 		paulaWriteWord(0xDFF096, ch->n_dmabit); // voice DMA off
// 
// 	// these take effect after the current DMA cycle is done
// 	if (playWaveformFlag)
// 	{
// 		paulaWritePtr(voiceAddr + 0, ch->n_loopstart);
// 		paulaWriteWord(voiceAddr + 4, ch->n_replen);
// 	}
// 	else
// 	{
// 		paulaWritePtr(voiceAddr + 0, NULL); // data
// 		paulaWriteWord(voiceAddr + 4, 1); // length
// 	}
// 
// }

void samplerLoopToggle(void)
{
	if (editor.sampleZero)
		return;

	assert(editor.currSample >= 0 && editor.currSample <= 30);

	moduleSample_t *s = &song->samples[editor.currSample];
	if (s->length < 2)
		return;

	turnOffVoices();

	if (s->loopStart+s->loopLength > 2)
	{
		// disable loop

		sampler.tmpLoopStart = s->loopStart;
		sampler.tmpLoopLength = s->loopLength;

		s->loopStart = 0;
		s->loopLength = 2;
	}
	else
	{
		// enable loop

		if (sampler.tmpLoopStart == 0 && sampler.tmpLoopLength == 0)
		{
			s->loopStart = 0;
			s->loopLength = s->length;
		}
		else
		{
			s->loopStart = sampler.tmpLoopStart;
			s->loopLength = sampler.tmpLoopLength;

			if (s->loopStart+s->loopLength > s->length)
			{
				s->loopStart = 0;
				s->loopLength = s->length;
			}
		}
	}
	updatePaulaLoops();
}

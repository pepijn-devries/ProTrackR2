#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pt2_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sampler_t
{
	const int8_t *samStart;
	int8_t *blankSample, *copyBuf, *sampleUndoCopy;
	int16_t loopStartPos, loopEndPos;
	uint16_t dragStart, dragEnd;
	int32_t samPointWidth, samOffset, samLength, saveMouseX, lastSamPos;
	int32_t lastMouseX, lastMouseY, tmpLoopStart, tmpLoopLength, copyBufSize;
} sampler_t;

extern sampler_t sampler; // pt2_sampler.c

// void sampleLine(int32_t line_x1, int32_t line_x2, int32_t line_y1, int32_t line_y2);

void killSample(void);
void downSample(void);
void upSample(void);
// void createSampleMarkTable(void);
int32_t smpPos2Scr(int32_t pos);
int32_t scr2SmpPos(int32_t x);
void fixSampleBeep(moduleSample_t *s);
void highPassSample(int32_t cutOff);
void lowPassSample(int32_t cutOff);
void samplerRemoveDcOffset(void);
void samplerResample(void);
void boostSample(int32_t sample, bool ignoreMark);
void filterSample(int32_t sample, bool ignoreMark);
void samplerSamPaste(void);
void samplerSamCopy(void);
void samplerLoopToggle(void);
void redoSampleData(int8_t sample);
void fillSampleRedoBuffer(int8_t sample);
void updateSamplePos(void);
void fillSampleFilterUndoBuffer(void);
void renderSampleData(void);
bool allocSamplerVars(void);
void deAllocSamplerVars(void);

#ifdef __cplusplus
}
#endif

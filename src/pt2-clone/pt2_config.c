// for finding memory leaks in debug mode with Visual Studio 
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h>
#include <limits.h>
#endif
#include "pt2_helpers.h"
#include "pt2_config.h"
#include "pt2_tables.h"
#include "pt2_structs.h" // editor
#include "pt2_sampler.h"

config_t config; // globalized

void loadConfig(void)
{
	// set default config values first
	config.noDownsampleOnSmpLoad = false;
	config.disableE8xEffect = false;
	config.fullScreenStretch = false;
	config.pattDots = false;
	config.waveformCenterLine = true;
	config.amigaModel = MODEL_A1200;
	config.soundFrequency = 48000;
	config.rememberPlayMode = false;
	config.stereoSeparation = 20;
	config.autoFitVideoScale = true;
	config.videoScaleFactor = 0; // will be set later if autoFitVideoScale is set
	config.realVuMeters = false;
	config.modDot = false;
	config.accidental = 0; // sharp
	config.quantizeValue = 1;
	config.transDel = false;
	config.blankZeroFlag = false;
	config.compoMode = false;
	config.soundBufferSize = 1024;
	config.autoCloseDiskOp = true;
	config.vsyncOff = false;
	config.hwMouse = true;
	config.startInFullscreen = false;
	config.pixelFilter = PIXELFILTER_NEAREST;
	config.integerScaling = true;
	config.audioInputFrequency = 44100;
	config.mod2WavOutputFreq = 44100;
	config.keepEditModeAfterStepPlay = false;
	config.maxSampleLength = 65534;
	config.restrictedPattEditClick = false;

}

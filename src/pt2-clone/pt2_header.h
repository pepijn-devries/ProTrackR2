#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h> // MAX_PATH
#else
#include <limits.h> // PT2_PATH_MAX
#endif
#include <stdint.h>
#include "pt2_unicode.h"

#define PROG_VER_STR "1.70"

#ifdef _WIN32
#define DIR_DELIMITER '\\'
#define PT2_PATH_MAX MAX_PATH
#else
#define DIR_DELIMITER '/'
#define PT2_PATH_MAX PATH_MAX
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif

#define MIN_AUDIO_FREQUENCY 44100
#define MAX_AUDIO_FREQUENCY 192000

// main crystal oscillator for PAL Amiga systems
#define AMIGA_PAL_XTAL_HZ 28375160
#define AMIGA_PAL_CCK_HZ (AMIGA_PAL_XTAL_HZ/8.0)
#define CIA_PAL_CLK (AMIGA_PAL_CCK_HZ / 5.0)

// nominal framerate in normal PAL videomodes (~49.92Hz)
#define AMIGA_PAL_VBLANK_HZ (AMIGA_PAL_CCK_HZ / (double)(313*227))

/* "60Hz" ranges everywhere from 59..61Hz depending on the monitor, so with
** no vsync we will get stuttering because the rate is not perfect...
*/
#define VBLANK_HZ 60

#define MOD_ROWS 64
#define MOD_SAMPLES 31
#define MAX_PATTERNS 100


#ifdef __cplusplus
extern "C" {
#endif

enum
{
	FLAG_NOTE = 1,
	FLAG_SAMPLE = 2,
	FLAG_NEWSAMPLE = 4,

	TEMPFLAG_START = 1,
	TEMPFLAG_DELAY = 2,

	NO_CARRY = 0,
	DO_CARRY = 1,

	INCREMENT_SLOW = 0,
	INCREMENT_FAST = 1,

	NO_SAMPLE_CUT = 0,
	SAMPLE_CUT = 1,

	EDIT_SPECIAL = 0,
	EDIT_NORMAL = 1,

	EDIT_TEXT_NO_UPDATE = 0,
	EDIT_TEXT_UPDATE = 1,

	TRANSPOSE_ALL = 1,

	MOUSE_BUTTON_NOT_HELD = 0,
	MOUSE_BUTTON_HELD = 1,

	DONT_SET_ORDER = -1,
	DONT_SET_PATTERN = -1,
	DONT_SET_ROW = -1,

	REMOVE_SAMPLE_MARKING = 0,
	KEEP_SAMPLE_MARKING  = 1,

	MOD_NOT_MODIFIED = 0,
	MOD_IS_MODIFIED = 1,

	DONT_CHECK_IF_FILE_EXIST = 0,
	CHECK_IF_FILE_EXIST = 1,

	DONT_GIVE_NEW_FILENAME = 0,
	GIVE_NEW_FILENAME = 1,

	DONT_DOWNSAMPLE = 0,
	DO_DOWNSAMPLE = 1,

	MODE_IDLE = 0,
	MODE_EDIT = 1,
	MODE_PLAY = 2,
	MODE_RECORD = 3,

	RECORD_PATT = 0,
	RECORD_SONG = 1,

	PLAY_MODE_NORMAL = 0,
	PLAY_MODE_PATTERN = 1,

	OCTAVE_HIGH = 0,
	OCTAVE_LOW = 1,

	DISKOP_MODE_MOD = 0,
	DISKOP_MODE_SMP = 1,

	DISKOP_SMP_WAV = 0,
	DISKOP_SMP_IFF = 1,
	DISKOP_SMP_RAW = 2,

	TEMPO_MODE_CIA = 0,
	TEMPO_MODE_VBLANK = 1,

};

#ifdef __cplusplus
}
#endif
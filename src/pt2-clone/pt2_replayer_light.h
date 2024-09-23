#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "pt2_structs.h"

#define MIN_BPM 32
#define MAX_BPM 255

void initializeModuleChannels(module_t *m);
module_t *createEmptyMod(void);
void turnOffVoices(void);
double ciaBpm2Hz(int32_t bpm);
void modFree2(module_t *my_song);
void updatePaulaLoops(void);
void modPlay(int16_t patt, int16_t pos, int8_t row);
void modStop(void);
void modSetPos(int16_t pos, int16_t row);
void modSetTempo(int32_t bpm, bool doLockAudio);
void modSetSpeed(int32_t speed);
void modSetPattern(uint8_t pattern);
void restartSong(void);
bool intMusic(void);

#ifdef __cplusplus
}
#endif
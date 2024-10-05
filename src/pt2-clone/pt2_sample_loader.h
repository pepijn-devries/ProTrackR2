#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pt2_structs.h"
#include "pt2_unicode.h"

#ifdef __cplusplus
extern "C" {
#endif

//bool loadSample(UNICHAR *fileName, char *entryName);
bool loadSample2(uint8_t * data, uint32_t filesize, moduleSample_t * samp, int8_t * sampData);

#ifdef __cplusplus
}
#endif

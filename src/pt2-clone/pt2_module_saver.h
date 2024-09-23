#pragma once

#include <stdbool.h>
#include "pt2_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

// bool saveModule(bool checkIfFileExist, bool giveNewFreeFilename);

bool modSave(char *fileName); // used by saveModule() and crash handler
uint32_t modSave2(module_t * my_song, uint8_t *target, uint32_t target_size);

#ifdef __cplusplus
}
#endif

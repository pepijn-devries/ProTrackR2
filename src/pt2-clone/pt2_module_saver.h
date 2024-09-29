#pragma once

#include <stdbool.h>
#include "pt2_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void cellCompacter(note_t * source, uint8_t * destination, uint32_t number_of_notes);
uint32_t modSave2(module_t * my_song, uint8_t *target, uint32_t target_size);

#ifdef __cplusplus
}
#endif

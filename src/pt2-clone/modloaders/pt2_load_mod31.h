#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../pt2_structs.h"

bool detectMod31(uint8_t *buffer, uint32_t filesize);
module_t *loadMod31(uint8_t *buffer, uint32_t filesize);

#ifdef __cplusplus
}
#endif

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../pt2_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

module_t *loadMod15(uint8_t *buffer, uint32_t filesize);

#ifdef __cplusplus
}
#endif

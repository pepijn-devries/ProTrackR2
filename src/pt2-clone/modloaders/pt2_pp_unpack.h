#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *unpackPP(FILE *f, uint32_t *filesize);
uint8_t *unpackPP2(uint8_t *data, uint32_t *data_size);

#ifdef __cplusplus
}
#endif

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool detectXPK(FILE *f);
bool unpackXPK(FILE *f, uint32_t *filesize, uint8_t **out);

bool detectXPK2(uint8_t *data, uint32_t data_size);
bool unpackXPK2(uint8_t *data, uint32_t *data_size, uint8_t **out);

#ifdef __cplusplus
}
#endif

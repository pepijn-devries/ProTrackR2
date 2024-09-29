#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool detectXPK2(uint8_t *data, uint32_t data_size);
uint8_t * unpackXPK2(uint8_t *data, uint32_t *data_size);

#ifdef __cplusplus
}
#endif

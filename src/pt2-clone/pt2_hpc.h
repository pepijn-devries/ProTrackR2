#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint64_t freq64;
	double dFreqMulMicro, dFreqMulMs;
} hpcFreq_t;

typedef struct
{
	uint64_t durationInt, durationFrac;
	uint64_t endTimeInt, endTimeFrac;
	uint64_t frameCounter, resetFrame;
} hpc_t;

extern hpcFreq_t hpcFreq;

void hpc_Init(void);
void hpc_SetDurationInHz(hpc_t *hpc, double dHz); // dHz = max 4095.999inf Hz (0.24ms)
void hpc_SetDurationInMs(hpc_t *hpc, double dMs); // dMs = minimum 0.2442002442 ms
void hpc_ResetCounters(hpc_t *hpc);
void hpc_Wait(hpc_t *hpc);

#ifdef __cplusplus
}
#endif

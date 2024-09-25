#include <stdint.h>
#include "pt2_tables.h"
#include "pt2_structs.h"
#include "pt2_config.h"

int32_t periodToNote(int32_t period) // 0 = no note, 1 = illegal note, 2..37 = note
{
	if (period == 0)
		return 0;

	int32_t beg = 0;
	int32_t end = 36 - 1;

	// do binary search
	while (beg <= end)
	{
		const int32_t mid = (beg + end) >> 1;

		int32_t tableVal = periodTable[mid];
		if (period == tableVal)
			return 2+mid;

		if (period < tableVal)
			beg = mid+1;
		else
			end = mid-1;
	}

	return 1; // illegal note
}

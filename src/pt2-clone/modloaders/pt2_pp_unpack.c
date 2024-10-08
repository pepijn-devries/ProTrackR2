/* PowerPacker unpack code taken from Heikki Orsila's amigadepack. Seems to have no license,
** so I'll assume it fits into BSD 3-Clause. If not, feel free to contact me at my email
** address found at the bottom of 16-bits.org.
**
** Edited by me (8bitbubsy).
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define READ_BITS(nbits, var) \
	bitCnt = (nbits); \
	while (bitsLeft < bitCnt) \
	{ \
		if (bufSrc < src) \
			return false; \
		bitBuffer |= ((*--bufSrc) << bitsLeft); \
		bitsLeft += 8; \
	} \
	(var) = 0; \
	bitsLeft -= bitCnt; \
	while (bitCnt--) \
	{ \
		(var) = ((var) << 1) | (bitBuffer & 1); \
		bitBuffer >>= 1; \
	}

static bool decrunch(uint8_t *src, uint8_t *dst, uint8_t *offsetLens, uint32_t srcLen, uint32_t dstLen, uint8_t skipBits)
{
	uint8_t bitCnt;
	uint32_t x, left, offset;

	if (src == NULL || dst == NULL || offsetLens == NULL)
		return false;

	uint8_t bitsLeft = 0;
	uint32_t bitBuffer = 0;
	uint32_t written = 0;
	uint8_t *bufSrc = src + srcLen;
	uint8_t *out = dst + dstLen;
	uint8_t *dstEnd = out;

	READ_BITS(skipBits, x);
	while (written < dstLen)
	{
		READ_BITS(1, x);
		if (x == 0)
		{
			left = 1;

			do
			{
				READ_BITS(2, x);
				left += x;
			}
			while (x == 3);

			while (left--)
			{
				READ_BITS(8, x);
				if (out <= dst)
					return false;

				*--out = (uint8_t)x;
				written++;
			}

			if (written == dstLen)
				break;
		}

		READ_BITS(2, x);
		uint32_t offBits = offsetLens[x];
		left = x + 2;

		if (x == 3)
		{
			READ_BITS(1, x);
			if (x == 0) offBits = 7;

			READ_BITS((uint8_t)offBits, offset);
			do
			{
				READ_BITS(3, x);
				left += x;
			}
			while (x == 7);
		}
		else
		{
			READ_BITS((uint8_t)offBits, offset);
		}

		if (out+offset >= dstEnd)
			return false;

		while (left--)
		{
			x = out[offset];
			if (out <= dst)
				return false;

			*--out = (uint8_t)x;
			written++;
		}
	}

	return true;
}

uint8_t *unpackPP2(uint8_t *data, uint32_t *data_size)
{
	uint8_t *outBuffer, ppCrunchData[4], *ppBuffer;
	uint32_t ppPackLen, ppUnpackLen;

	ppPackLen = *data_size;
	if ((ppPackLen & 3) != 0 || ppPackLen <= 12)
		return NULL;

	ppBuffer = (uint8_t *)malloc(ppPackLen);
	if (ppBuffer == NULL)
		return NULL;

	memcpy(ppCrunchData, data + ppPackLen - 4, 4);

	ppUnpackLen = (ppCrunchData[0] << 16) | (ppCrunchData[1] << 8) | ppCrunchData[2];

	outBuffer = (uint8_t *)malloc(ppUnpackLen);
	if (outBuffer == NULL)
	{
		free(ppBuffer);
		return NULL;
	}

	memcpy(ppBuffer, data, ppPackLen);

	if (!decrunch(ppBuffer+8, outBuffer, ppBuffer+4, ppPackLen-12, ppUnpackLen, ppCrunchData[3]))
	{
		free(ppBuffer);
		return NULL;
	}

	free(ppBuffer);
	*data_size = ppUnpackLen;

	return outBuffer;
}

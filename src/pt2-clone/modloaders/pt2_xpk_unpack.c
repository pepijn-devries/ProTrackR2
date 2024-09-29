/* Taken from the OpenMPT project and converted to C.
** OpenMPT shares the same license as the PT2 clone (BSD 3-clause),
** so the licensing is compatible.
**
** This is probably not 100% safe code, but it works.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../pt2_helpers.h"

typedef struct XPKFILEHEADER
{
	char XPKF[4];
	uint32_t SrcLen;
	char SQSH[4];
	uint32_t DstLen;
	char Name[16];
	uint32_t Reserved;
} XPKFILEHEADER;

typedef struct XPK_BufferBounds
{
	const uint8_t *pSrcBeg;
	size_t SrcSize;
} XPK_BufferBounds;

static const uint8_t xpk_table[56] =
{
	2,3,4,5,6,7,8,0,3,2,4,5,6,7,8,0,4,3,5,
	2,6,7,8,0,5,4,6,2,3,7,8,0,6,5,7,2,3,4,
	8,0,7,6,8,2,3,4,5,0,8,7,6,2,3,4,5,0
};

static inline uint8_t SrcRead(size_t index, XPK_BufferBounds *bufs)
{
	assert(index < bufs->SrcSize);
	if (index >= bufs->SrcSize)
		return 0; // this is actually not how to do it, ugh...

	return bufs->pSrcBeg[index];
}

static inline int32_t bfextu(size_t p, int32_t bo, int32_t bc, XPK_BufferBounds *bufs)
{
	p += (uint32_t)bo >> 3;

	uint32_t r = SrcRead(p, bufs); p++;
	r <<= 8;
	r |= SrcRead(p, bufs); p++;
	r <<= 8;
	r |= SrcRead(p, bufs);
	r <<= bo & 7;
	r &= 0x00FFFFFF;
	r >>= 24 - bc;

	return r;
}

static inline int32_t bfexts(size_t p, int32_t bo, int32_t bc, XPK_BufferBounds *bufs)
{
	p += (uint32_t)bo >> 3;

	uint32_t r = SrcRead(p, bufs); p++;
	r <<= 8;
	r |= SrcRead(p, bufs); p++;
	r <<= 8;
	r |= SrcRead(p, bufs);
	r <<= (bo & 7) + 8;
	r = (int32_t)r >> (32-bc);

	return r;
}

static inline uint8_t XPK_ReadTable(int32_t index)
{
	if (index < 0 || index >= (int32_t)sizeof (xpk_table))
		return 0; // this is actually not how to do it, ugh...

	return xpk_table[index];
}

static bool XPK_DoUnpack(const uint8_t *src_, uint32_t srcLen, int32_t len, uint8_t **out)
{
	if (len <= 0)
		return false;

	int32_t d0, d1, d2, d3, d4, d5, d6, a2, a5, cup1;
	const uint32_t unpackedLen = MIN((uint32_t)len, MIN(srcLen, UINT32_MAX / 20) * 20);

	uint8_t *unpackedData = (uint8_t *)malloc(unpackedLen);
	if (unpackedData == NULL)
		return false;

	uint8_t *PtrEnd = unpackedData + unpackedLen;
	uint8_t *PtrOut = unpackedData;

	XPK_BufferBounds bufs;
	bufs.pSrcBeg = src_;
	bufs.SrcSize = srcLen;

	size_t src = 0;
	size_t c = src;
	while (len > 0)
	{
		int32_t type = SrcRead(c+0, &bufs);
		int32_t cp = (SrcRead(c+4, &bufs)<<8) | (SrcRead(c+5, &bufs)); // packed
		cup1 = (SrcRead(c+6, &bufs)<<8) | (SrcRead(c+7, &bufs)); // unpacked

		c += 8;
		src = c + 2;

		if (type == 0) // RAW chunk
		{
			if (cp < 0 || cp > len)
			{
				free(unpackedData);
				return false;
			}

			for (int32_t i = 0; i < cp; i++)
				*PtrOut++ = SrcRead(c + i, &bufs);

			c += cp;
			len -= cp;
			continue;
		}

		if (type != 1)
			break;

		if (cup1 > len)
			cup1 = len;

		len -= cup1;
		cp = (cp + 3) & 0xFFFC;
		c += cp;

		d0 = d1 = d2 = a2 = 0;
		d3 = SrcRead(src, &bufs); src++;
		*PtrOut++ = (uint8_t)d3;
		cup1--;

		while (cup1 > 0)
		{
			if (d1 >= 8) goto l6dc;
			if (bfextu(src, d0, 1, &bufs)) goto l75a;
			d0++;
			d5 = 0;
			d6 = 8;
			goto l734;

l6dc:
			if (bfextu(src, d0, 1, &bufs)) goto l726;
			d0++;
			if (!bfextu(src, d0, 1, &bufs)) goto l75a;
			d0++;
			if (bfextu(src, d0, 1, &bufs)) goto l6f6;
			d6 = 2;
			goto l708;

l6f6:
			d0++;
			if (!bfextu(src, d0, 1, &bufs)) goto l706;
			d6 = bfextu(src, d0, 3, &bufs);
			d0 += 3;
			goto l70a;

l706:
			d6 = 3;
l708:
			d0++;
l70a:
			d6 = XPK_ReadTable((a2*8) + d6 - 17);
			if (d6 != 8) goto l730;
l718:
			if (d2 >= 20)
			{
				d5 = 1;
				goto l732;
			}
			d5 = 0;
			goto l734;

l726:
			d0++;
			d6 = 8;
			if (d6 == a2) goto l718;
			d6 = a2;
l730:
			d5 = 4;
l732:
			d2 += 8;
l734:
			while (d5 >= 0 && cup1 > 0)
			{
				d4 = bfexts(src, d0, d6, &bufs);
				d0 += d6;
				d3 -= d4;
				*PtrOut++ = (uint8_t)d3;
				cup1--;
				d5--;
			}

			if (d1 != 31)
				d1++;

			a2 = d6;
l74c:
			d6 = d2;
			d6 >>= 3;
			d2 -= d6;
		}
	}

	if (PtrOut > PtrEnd)
	{
		free(unpackedData);
		return false;
	}

	*out = unpackedData;
	return true;

l75a:
	d0++;
	if (bfextu(src, d0, 1, &bufs)) goto l766;
	d4 = 2;
	goto l79e;

l766:
	d0++;
	if (bfextu(src, d0, 1, &bufs)) goto l772;
	d4 = 4;
	goto l79e;

l772:
	d0++;
	if (bfextu(src, d0, 1, &bufs)) goto l77e;
	d4 = 6;
	goto l79e;

l77e:
	d0++;
	if (bfextu(src, d0, 1, &bufs)) goto l792;
	d0++;
	d6 = bfextu(src, d0, 3, &bufs);
	d0 += 3;
	d6 += 8;
	goto l7a8;

l792:
	d0++;
	d6 = bfextu(src, d0, 5, &bufs);
	d0 += 5;
	d4 = 16;
	goto l7a6;

l79e:
	d0++;
	d6 = bfextu(src, d0, 1, &bufs);
	d0++;
l7a6:
	d6 += d4;
l7a8:
	if (bfextu(src, d0, 1, &bufs))
	{
		d5 = 12;
		a5 = -0x100;
	}
	else
	{
		d0++;
		if (bfextu(src, d0, 1, &bufs))
		{
			d5 = 14;
			a5 = -0x1100;
		}
		else
		{
			d5 = 8;
			a5 = 0;
		}
	}

	d0++;
	d4 = bfextu(src, d0, d5, &bufs);
	d0 += d5;
	d6 -= 3;
	if (d6 >= 0)
	{
		if (d6 > 0)
			d1--;

		d1--;
		if (d1 < 0)
			d1 = 0;
	}
	d6 += 2;

	size_t phist = (size_t)(PtrOut-unpackedData) + a5 - d4 - 1;
	if (phist >= (size_t)(PtrOut-unpackedData))
	{
		free(unpackedData);
		return false;
	}

	while (d6 >= 0 && cup1 > 0)
	{
		d3 = unpackedData[phist];
		phist++;
		*PtrOut++ = (uint8_t)d3;
		cup1--;
		d6--;
	}

	goto l74c;
}

static bool ValidateHeader(XPKFILEHEADER *header)
{
	if (memcmp(header->XPKF, "XPKF", 4) != 0)
		return false;

	if (memcmp(header->SQSH, "SQSH", 4) != 0)
		return false;

	if (header->SrcLen == 0 || header->DstLen == 0)
		return false;

	if (header->SrcLen < sizeof (XPKFILEHEADER)-8)
		return false;

	return true;
}

void ReadHeader2(uint8_t *data, XPKFILEHEADER *header)
{
  memcpy(header, data, sizeof(XPKFILEHEADER));

  header->SrcLen = SWAP32(header->SrcLen);
  header->DstLen = SWAP32(header->DstLen);
  
  return;
}

bool detectXPK2(uint8_t *data, uint32_t data_size)
{
  XPKFILEHEADER header;
  ReadHeader2(data, &header);
  return ValidateHeader(&header);
}

uint8_t * unpackXPK2(uint8_t *data, uint32_t *out_size)
{
  XPKFILEHEADER header;
  uint32_t inputfilesize = *out_size - 8;
  
  ReadHeader2(data, &header);

  if (!ValidateHeader(&header))
    return NULL;

  data += sizeof (XPKFILEHEADER);

  if (header.SrcLen != inputfilesize)
    return NULL;
  
  uint8_t *unpackedData = NULL;
  bool result = XPK_DoUnpack(data, header.SrcLen - (sizeof (XPKFILEHEADER) - 8), header.DstLen, &unpackedData);

  if (result == true)
  {
    *out_size = header.DstLen;
    return unpackedData;
  }
  return NULL;
}

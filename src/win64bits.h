
#ifdef _MSC_VER
static int BSF (UINT64 x)
{
  int r;
  _BitScanForward64 (&r, x);
  return r;
}

static int BSR (UINT64 x)
{
  int r;
  _BitScanReverse64 (&r, x);
  return r;
}
#else
static int
BSF (UINT64 x)
{
  _asm
    {
      bsf rax, x
    }
}
static int
BSR (UINT64 x)
{
  _asm
    {
      bsr rax, x
    }
}
#endif

#ifdef HAS_POPCNT
#ifdef INTEL_COMPILER
#include "nmmintrin.h"
static INLINE long long POPCNT (unsigned long long x)
	{
  	return _mm_popcnt_u64(x);
	}
//__forceinline int POPCNT(unsigned __int64 x) {
//	return (int) _mm_popcnt_u64(x);
//}
#else
static int POPCNT (UINT64 x)
{ 
  _asm
    {
      popcnt rax, x
    }
}
#endif
#else /* no POPCNT */
static INLINE int
POPCNT (uint64 w)
{
  w = w - ((w >> 1) & 0x5555555555555555ULL);
  w = (w & 0x3333333333333333ULL) + ((w >> 2) & 0x3333333333333333ULL);
  w = (w + (w >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
  return (w * 0x0101010101010101ull) >> 56;
}
#endif /* HAS_POPCNT */

#define BitClear(b, B) B &= (B - 1)
#define BitSet(b, B) B |= ((uint64) (1)) << (b)


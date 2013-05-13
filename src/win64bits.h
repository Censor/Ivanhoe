
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
static int
POPCNT (UINT64 x)
{
  /* return __popcnt64 (x); */
  _asm
    {
      popcnt rax, x
    }
}
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


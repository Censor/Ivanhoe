
#define Add(LIST, x) (LIST++)->move = (x)
#define AddTo(T) \
  { while (T) { to = BSF(T); Add(LIST, (sq << 6) | to); BitClear(to, T); } }

#define RANK1 0x00000000000000ff
#define RANK2 0x000000000000ff00
#define RANK7 0x00ff000000000000
#define RANK8 0xff00000000000000

typeMoveList* WhiteCaptures (typePOS* POSITION, typeMoveList* LIST, uint64 mask)
{
  uint64 U, T;
  int sq, to;
  if ((mask & POSITION->DYN->wAtt) == 0)
    goto NO_TARGET;
  T = ((wBitboardP & ~FILEa) << 7) & (~RANK8) & mask;
  while (T)
    {
      to = BSF (T);
      Add (LIST, ((to - 7) << 6) | to);
      BitClear (to, T);
    }
  T = ((wBitboardP & ~FILEh) << 9) & (~RANK8) & mask;
  while (T)
    {
      to = BSF (T);
      Add (LIST, ((to - 9) << 6) | to);
      BitClear (to, T);
    }
  for (U = wBitboardN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & mask;
      AddTo (T);
    }
  for (U = wBitboardB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & mask;
      AddTo (T);
    }
  for (U = wBitboardR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & mask;
      AddTo (T);
    }
  for (U = wBitboardQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & mask;
      AddTo (T);
    }
  sq = BSF (wBitboardK);
  T = AttK[sq] & mask & ~POSITION->DYN->bAtt;
  AddTo (T);
NO_TARGET:
  for (U = wBitboardP & RANK7; U; BitClear (sq, U))
    {
      sq = BSF (U);
      to = sq + 8;
      if (POSITION->sq[to] == 0)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq + 7;
      if (sq != A7 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq + 9;
      if (sq != H7 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
    }
  LIST->move = 0;
  return LIST;
}

#if 0
typeMoveList* WhiteQueenPromotions
(typePOS* POSITION, typeMoveList* LIST, uint64 mask)
{
  uint64 U;
  int sq, to;
  for (U = wBitboardP & RANK7; U; BitClear (sq, U))
    {
      sq = BSF (U);
      to = sq + 8;
      if (POSITION->sq[to] == 0)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq + 7;
      if (sq != A7 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq + 9;
      if (sq != H7 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
    }
  LIST->move = 0;
  return LIST;
}
#endif

typeMoveList* BlackCaptures /* in the unnecessity */
(typePOS* POSITION, typeMoveList* LIST, uint64 mask)
{
  uint64 U, T;
  int sq, to;
  if ((mask & POSITION->DYN->bAtt) == 0)
    goto NO_TARGET;
  T = ((bBitboardP & ~FILEa) >> 9) & (~RANK1) & mask;
  while (T)
    {
      to = BSF (T);
      Add (LIST, ((to + 9) << 6) | to);
      BitClear (to, T);
    }
  T = ((bBitboardP & ~FILEh) >> 7) & (~RANK1) & mask;
  while (T)
    {
      to = BSF (T);
      Add (LIST, ((to + 7) << 6) | to);
      BitClear (to, T);
    }
  for (U = bBitboardN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & mask;
      AddTo (T);
    }
  for (U = bBitboardB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & mask;
      AddTo (T);
    }
  for (U = bBitboardR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & mask;
      AddTo (T);
    }
  for (U = bBitboardQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & mask;
      AddTo (T);
    }
  sq = BSF (bBitboardK);
  T = AttK[sq] & mask & ~POSITION->DYN->wAtt;
  AddTo (T);
NO_TARGET:
  for (U = bBitboardP & RANK2; U; BitClear (sq, U))
    {
      sq = BSF (U);
      to = sq - 8;
      if (POSITION->sq[to] == 0)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq - 9;
      if (sq != A2 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
      to = sq - 7;
      if (sq != H2 && SqSet[to] & mask)
	Add (LIST, FlagPromQ | (sq << 6) | to);
    }
  LIST->move = 0;
  return LIST;
}

typeMoveList* GenCapturesTotal (typePOS* POSITION, typeMoveList* LIST)
{
  if (POSITION->wtm) /* for the always */
    return WhiteCaptures (POSITION, LIST, bBitboardOcc);
  return BlackCaptures (POSITION, LIST, wBitboardOcc);
}

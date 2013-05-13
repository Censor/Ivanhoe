
#ifdef UTILITIES
#define Ranks78 0xffff000000000000
#define Ranks678 0xffffff0000000000
#define Ranks12 0x000000000000ffff
#define Ranks123 0x00000000000ffffff

#include "RobboLito.h"
#include "evaluation.v"

#define EE printf
#define EXPLAIN TRUE
#define OpEg(x) \
  ((sint16) ((((x) & 0xffff) < 0) + ((x) >> 16))), (sint16) ((x) & 0xffff)

extern uint8 YakovWTM[24576];
extern uint8 YakovBTM[24576];

static void AdjustPositionalGainB (typePOS* POSITION, int move)
{
  int v, p, m;
  if (POSITION->DYN->cp)
    return;
  p = POSITION->sq[TO (move)];
  m = move & 07777;
  v = POSITION->DYN->PositionalValue - ((POSITION->DYN - 1)->PositionalValue);
  if (MAX_POS_GAIN (p ,m) <= v)
    MAX_POS_GAIN (p, m) = v;
  else
    MAX_POS_GAIN (p, m)--;
}

static void AdjustPositionalGainW (typePOS* POSITION, int move)
{
  int v, p, m;
  if (POSITION->DYN->cp)
    return;
  p = POSITION->sq[TO (move)];
  m = move & 07777;
  v = ((POSITION->DYN - 1)->PositionalValue) - POSITION->DYN->PositionalValue;
  if (MAX_POS_GAIN (p ,m) <= v)
    MAX_POS_GAIN (p, m) = v;
  else
    MAX_POS_GAIN (p, m)--;
}


#ifdef HAS_PREFETCH
#define PREFETCH_PAWN_HASH __builtin_prefetch (&PAWN_POINTER, 1, 2);
#else
#define PREFETCH_PAWN_HASH
#endif

static int MaterialValue (typePOS* POSITION)
{
  int i;
  int Value = 975 * (POPCNT (wBitboardQ) - POPCNT (bBitboardQ));
  Value += 500 * (POPCNT (wBitboardR) - POPCNT (bBitboardR));
  Value += 325 * (POPCNT (wBitboardB) - POPCNT (bBitboardB));
  Value += 325 * (POPCNT (wBitboardN) - POPCNT (bBitboardN));
  Value += 100 * (POPCNT (wBitboardP) - POPCNT (bBitboardP));
  if (wBitboardBL && wBitboardBD)
    Value += 50;
  if (bBitboardBL && bBitboardBD)
    Value -= 50;
  EE ("Material Value: ");
  for (i = 0; i < POPCNT (wBitboardQ); i++)
    EE ("Q");
  for (i = 0; i < POPCNT (wBitboardR); i++)
    EE ("R");
  for (i = 0; i < POPCNT (wBitboardB); i++)
    EE ("B");
  for (i = 0; i < POPCNT (wBitboardN); i++)
    EE ("N");
  for (i = 0; i < POPCNT (wBitboardP); i++)
    EE ("P");
  for (i = 0; i < POPCNT (bBitboardQ); i++)
    EE ("q");
  for (i = 0; i < POPCNT (bBitboardR); i++)
    EE ("r");
  for (i = 0; i < POPCNT (bBitboardB); i++)
    EE ("b");
  for (i = 0; i < POPCNT (bBitboardN); i++)
    EE ("n");
  for (i = 0; i < POPCNT (bBitboardP); i++)
    EE ("p");
  EE ("[special] %d\n", Value);
  return Value;
}

static void KingPawnWhite (typePOS* POSITION, int matval, uint8 TOKEN,
			   typePawnEval* PAWN_INFO)
{
  int Value, WhiteLeader, BlackLeader, sq, tr;
  uint8 C;
  uint64 A, T;
  if (PAWN_INFO->PAWN_HASH != POSITION->DYN->PAWN_HASH)
    PawnEval (POSITION, PAWN_INFO);
  POSITION->DYN->wXray = POSITION->DYN->bXray = 0;
  Value = ((POSITION->DYN->STATIC) + (PAWN_INFO->SCORE));
  Value = (sint16) (Value & 0xffff);
  WhiteLeader = 0;
  C = PAWN_INFO->wPassedFiles;
  while (C)
    {
      sq = BSR (FileArray[BSF (C)] & wBitboardP);
      tr = RANK (sq);
      BitClear (0, C);
      if ((ShepherdWK[sq] & wBitboardK) == 0)
	{
	  if (wBitboardOcc & OpenFileW[sq]
	      || (bBitboardK & QuadrantBKwtm[sq]) == 0)
	    continue;
	  if (WhiteLeader <= tr)
	    WhiteLeader = tr;
	}
      else if (WhiteLeader <= tr)
	WhiteLeader = tr;
    }
  BlackLeader = 0;
  C = PAWN_INFO->bPassedFiles;
  while (C)
    {
      sq = BSF (FileArray[BSF (C)] & bBitboardP);
      tr = R8 - RANK (sq);
      BitClear (0, C);
      if ((ShepherdBK[sq] & bBitboardK) == 0)
	{
	  if (bBitboardOcc & OpenFileB[sq]
	      || (wBitboardK & QuadrantWKwtm[sq]) == 0)
	    continue;
	  if (BlackLeader <= tr)
	    BlackLeader = tr;
	}
      else if (BlackLeader <= tr)
	BlackLeader = tr;
    }
  POSITION->DYN->Value = (TOKEN * (Value + matval)) / 128;
  POSITION->DYN->bKcheck = POSITION->DYN->wKcheck = 0;
  if (WhiteLeader > BlackLeader
      && (bBitboardP & InFrontB[R8 - WhiteLeader + 1]) == 0)
    POSITION->DYN->Value += 150 + 50 * WhiteLeader;
  if (BlackLeader > WhiteLeader + 1
      && (wBitboardP & InFrontW[BlackLeader - 2]) == 0)
    POSITION->DYN->Value -= 150 + 50 * BlackLeader;

  A = (wBitboardP & (~FILEa)) << 7;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck |= (T >> 7);
  POSITION->DYN->wAtt = A;
  A = (wBitboardP & (~FILEh)) << 9;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck |= (T >> 9);
  POSITION->DYN->wAtt |= A | AttK[POSITION->wKsq];
  A = (bBitboardP & (~FILEh)) >> 7;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 7);
  POSITION->DYN->bAtt = A;
  A = (bBitboardP & (~FILEa)) >> 9;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 9);
  POSITION->DYN->bAtt |= A | AttK[POSITION->bKsq];
  if (bBitboardK & AttK[POSITION->wKsq])
    {
      POSITION->DYN->bKcheck |= SqSet[POSITION->wKsq];
      POSITION->DYN->wKcheck |= SqSet[POSITION->bKsq];
    }
  if (POSITION->DYN->Value > 0 && !wBitboardP)
    POSITION->DYN->Value = 0;
  if (POSITION->DYN->Value < 0 && !bBitboardP)
    POSITION->DYN->Value = 0;
  if (POSITION->DYN->Value > 0)
    {
      if ((wBitboardP & ~FILEh) == 0
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
	POSITION->DYN->Value = 0;
      if ((wBitboardP & ~FILEa) == 0
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
	POSITION->DYN->Value = 0;
      if ((POSITION->DYN->flags & 28) == 28)
	{
	  sq = BSF (wBitboardP);
	  tr = RANK (sq);
	  Value = YakovWTM[384 * POSITION->wKsq + 6 * POSITION->bKsq + tr - 1]
	    & (1 << FILE (sq));
	  if (!Value)
	    POSITION->DYN->Value = 0;
	  else
	    POSITION->DYN->Value =
	      ((sint16) (POSITION->DYN->STATIC & 0xffff)) + 75 * tr + 250;
	}
    }
  if (POSITION->DYN->Value < 0)
    {
      if ((bBitboardP & ~FILEh) == 0
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[H1])
	POSITION->DYN->Value = 0;
      if ((bBitboardP & ~FILEa) == 0
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[A1])
	POSITION->DYN->Value = 0;
      if ((POSITION->DYN->flags & 28) == 28)
	{
	  sq = H8 - BSR (bBitboardP);
	  tr = RANK (sq);
	  Value = YakovBTM[384 * (H8 - POSITION->bKsq) +
			   6 * (H8 - POSITION->wKsq) + tr - 1]
	    & (1 << FILE (sq));
	  if (!Value)
	    POSITION->DYN->Value = 0;
	  else
	    POSITION->DYN->Value =
	      ((sint16) (POSITION->DYN->STATIC & 0xffff)) - 75 * tr - 250;
	}
    }
}

static void KingPawnBlack (typePOS* POSITION, int matval, uint8 TOKEN,
			   typePawnEval* PAWN_INFO)
{
  int Value, WhiteLeader, BlackLeader, sq, tr;
  uint8 C;
  uint64 A, T;
  if (PAWN_INFO->PAWN_HASH != POSITION->DYN->PAWN_HASH)
    PawnEval (POSITION, PAWN_INFO);
  POSITION->DYN->wXray = POSITION->DYN->bXray = 0;
  Value = ((POSITION->DYN->STATIC) + (PAWN_INFO->SCORE));
  Value = (sint16) (Value & 0xffff);
  WhiteLeader = 0;
  C = PAWN_INFO->wPassedFiles;
  while (C)
    {
      sq = BSR (FileArray[BSF (C)] & wBitboardP);
      tr = RANK (sq);
      BitClear (0, C);
      if ((ShepherdWK[sq] & wBitboardK) == 0)
	{
	  if (wBitboardOcc & OpenFileW[sq]
	      || (bBitboardK & QuadrantBKbtm[sq]) == 0)
	    continue;
	  if (WhiteLeader <= tr)
	    WhiteLeader = tr;
	}
      else if (WhiteLeader <= tr)
	WhiteLeader = tr;
    }
  BlackLeader = 0;
  C = PAWN_INFO->bPassedFiles;
  while (C)
    {
      sq = BSF (FileArray[BSF (C)] & bBitboardP);
      tr = R8 - RANK (sq);
      BitClear (0, C);
      if ((ShepherdBK[sq] & bBitboardK) == 0)
	{
	  if (bBitboardOcc & OpenFileB[sq]
	      || (wBitboardK & QuadrantWKbtm[sq]) == 0)
	    continue;
	  if (BlackLeader <= tr)
	    BlackLeader = tr;
	}
      else if (BlackLeader <= tr)
	BlackLeader = tr;
    }
  POSITION->DYN->Value = -(TOKEN * (Value + matval)) / 128;
  POSITION->DYN->bKcheck = POSITION->DYN->wKcheck = 0;
  if (WhiteLeader > BlackLeader + 1
      && (bBitboardP & InFrontB[R8 - WhiteLeader + 2]) == 0)
    POSITION->DYN->Value -= 150 + 50 * WhiteLeader;
  if (BlackLeader > WhiteLeader
      && (wBitboardP & InFrontW[BlackLeader - 1]) == 0)
    POSITION->DYN->Value += 150 + 50 * BlackLeader;

  A = (wBitboardP & (~FILEa)) << 7;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck |= (T >> 7);
  POSITION->DYN->wAtt = A;
  A = (wBitboardP & (~FILEh)) << 9;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck |= (T >> 9);
  POSITION->DYN->wAtt |= A | AttK[POSITION->wKsq];
  A = (bBitboardP & (~FILEh)) >> 7;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 7);
  POSITION->DYN->bAtt = A;
  A = (bBitboardP & (~FILEa)) >> 9;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 9);
  POSITION->DYN->bAtt |= A | AttK[POSITION->bKsq];
  if (bBitboardK & AttK[POSITION->wKsq])
    {
      POSITION->DYN->bKcheck |= SqSet[POSITION->wKsq];
      POSITION->DYN->wKcheck |= SqSet[POSITION->bKsq];
    }
  if (POSITION->DYN->Value < 0 && !wBitboardP)
    POSITION->DYN->Value = 0;
  if (POSITION->DYN->Value > 0 && !bBitboardP)
    POSITION->DYN->Value = 0;
  if (POSITION->DYN->Value < 0)
    {
      if ((wBitboardP & ~FILEh) == 0
	  && (AttK[POSITION->bKsq] | bBitboardK) &
	  SqSet[H8])
	POSITION->DYN->Value = 0;
      if ((wBitboardP & ~FILEa) == 0
	  && (AttK[POSITION->bKsq] | bBitboardK) &
	  SqSet[A8])
	POSITION->DYN->Value = 0;
      if ((POSITION->DYN->flags & 28) == 28)
	{
	  sq = BSF (wBitboardP);
	  tr = RANK (sq);
	  Value = YakovBTM[384 * POSITION->wKsq + 6 * POSITION->bKsq + tr - 1]
	           & (1 << FILE (sq));
	  if (!Value)
	    POSITION->DYN->Value = 0;
	  else
	    POSITION->DYN->Value =
	      -((sint16) (POSITION->DYN->STATIC & 0xffff)) - 75 * tr - 250;
	}
    }
  if (POSITION->DYN->Value > 0)
    {
      if ((bBitboardP & ~FILEh) == 0
	  && (AttK[POSITION->wKsq] | wBitboardK) & SqSet[H1])
	POSITION->DYN->Value = 0;
      if ((bBitboardP & ~FILEa) == 0
	  && (AttK[POSITION->wKsq] | wBitboardK) & SqSet[A1])
	POSITION->DYN->Value = 0;
      if ((POSITION->DYN->flags & 28) == 28)
	{
	  sq = H8 - BSR (bBitboardP);
	  tr = RANK (sq);
	  Value = YakovWTM[384 * (H8 - POSITION->bKsq) +
			   6 * (H8 - POSITION->wKsq) + tr - 1]
	    & (1 << FILE (sq));
	  if (!Value)
	    POSITION->DYN->Value = 0;
	  else
	    POSITION->DYN->Value =
	      -((sint16) (POSITION->DYN->STATIC & 0xffff)) + 75 * tr + 250;
	}
    }
}

void EvalExplanation (typePOS* POSITION, int min, int max, int move)
{
  typePawnEval* PAWN_POINTER;
  int index, matval, Value;
  int b, tr, antiphase, phase;
  int to, cp, wKs, bKs;
  uint64 U, wKatt, bKatt, A, AttB, AttR;
  sint32 wKhit, bKhit;
  uint64 wGoodMinor, bGoodMinor, wSafeMob, bSafeMob, wOKxray, bOKxray;
  uint64 T, bPatt, wPatt;
  int open, end;
  uint8 bGoodAtt, wGoodAtt;
  uint8 TOKEN;
  int v, positional;
  typePawnEval PAWN_INFO[1];
  int ch;

  PAWN_POINTER = PawnHash + (POSITION->DYN->PAWN_HASH & (PawnHashSize - 1));
  PREFETCH_PAWN_HASH;
  index = (POSITION->DYN->material >> 8) & 0x7ffff;
  TOKEN = MATERIAL[index].token;
  POSITION->DYN->flags = MATERIAL[index].flags;
  POSITION->DYN->exact = FALSE;
  if (!(POSITION->DYN->material & 0x80000000))
    {
      MaterialValueExplain (index);
      matval = MATERIAL[index].Value;
    }
  else
    {
      if (POPCNT (wBitboardQ) > 1 || POPCNT (bBitboardQ) > 1
	  || POPCNT (wBitboardR) > 2 || POPCNT (bBitboardR) > 2
	  || POPCNT (wBitboardBL) > 1 || POPCNT (bBitboardBL) > 1
	  || POPCNT (wBitboardBD) > 1 || POPCNT (bBitboardBD) > 1
	  || POPCNT (wBitboardN) > 2 || POPCNT (bBitboardN) > 2)
	{
	  TOKEN = 0x80;
	  matval = MaterialValue (POSITION);
	  POSITION->DYN->flags = 0;
	  if (wBitboardQ | wBitboardR | wBitboardB | wBitboardN)
	    POSITION->DYN->flags |= 2;
	  if (bBitboardQ | bBitboardR | bBitboardB | bBitboardN)
	    POSITION->DYN->flags |= 1;
	}
      else
	{
	  MaterialValueExplain (index);
	  matval = MATERIAL[index].Value;
	  POSITION->DYN->material &= 0x7fffffff;
	}
    }

  if (((POSITION->DYN->HASH ^ EvalHash[POSITION->DYN->HASH & EVAL_HASH_MASK]) &
       0xffffffffffff0000) == 0 && !EXPLAIN)
    {
      Value =
	(int) ((sint16) (EvalHash[POSITION->DYN->HASH & EVAL_HASH_MASK] & 0xffff));
      POSITION->DYN->lazy = 0;
      Mobility (POSITION);
      POSITION->DYN->PositionalValue =
	((POSITION->wtm) ? Value : -Value) - matval;
      POSITION->DYN->Value = Value;
      if (move && !(POSITION->DYN - 1)->lazy)
	{
	  POSITION->wtm ?
	    AdjustPositionalGainW (POSITION, move) :
	    AdjustPositionalGainB (POSITION, move);
	}
      if (Value > 15000 || Value < -15000)
	    POSITION->DYN->exact = TRUE;
      return;
    }

#ifdef CON_ROBBO_BUILD
  if (ROBBO_LOAD && !EXPLAIN)
  {
    int c = POPCNT (POSITION->OccupiedBW);
    boolean b;
    if (c <= 5)
      {
	int va;
	Mobility (POSITION);
	if (TRIPLE_VALUE (POSITION, &va, &b, FALSE, FALSE, FALSE))
	  {
	    int molt = 21000 - (128 * c) - 64 * HEIGHT (POSITION);
	    if (va == 0)
	      POSITION->DYN->Value = 0;
	    else
	      {
		POSITION->DYN->Value = (va > 0) ? molt : -molt;
		POSITION->DYN->Value += (POSITION->wtm ? matval : -matval);
	      }
	    POSITION->DYN->lazy = 1;
	    POSITION->DYN->PositionalValue = 0;
	    EvalHash[POSITION->DYN->HASH & EVAL_HASH_MASK] =
	      (POSITION->DYN->HASH & 0xffffffffffff0000) |
	      (POSITION->DYN->Value & 0xffff);
	    POSITION->DYN->exact = TRUE;
	    POSITION->tbhits++;
	    return;
	  }
      }
  }
#endif

  memcpy (PAWN_INFO, PAWN_POINTER, sizeof (typePawnEval));
#if 1 /* HYATT hashing */
  PAWN_INFO->PAWN_HASH ^= (((uint64*) (PAWN_INFO)) + 0x1)[0];
  PAWN_INFO->PAWN_HASH ^= (((uint64*) (PAWN_INFO)) + 0x2)[0];
  PAWN_INFO->PAWN_HASH ^= (((uint64*) (PAWN_INFO)) + 0x3)[0];
#endif

  if ((POSITION->DYN->material & 0xff) == 0)
    {
      POSITION->wtm ?
	KingPawnWhite (POSITION, matval, TOKEN, PAWN_INFO) :
	KingPawnBlack (POSITION, matval, TOKEN, PAWN_INFO);
      POSITION->DYN->lazy = 1;
      POSITION->DYN->PositionalValue = 0;
      EvalHash[POSITION->DYN->HASH & EVAL_HASH_MASK] =
	(POSITION->DYN->HASH & 0xffffffffffff0000) |
	(POSITION->DYN->Value & 0xffff);
      return;
    }

#define WHITE_MINOR_ONLY (8 << 2)
#define BLACK_MINOR_ONLY (16 << 2)

  if (EXPLAIN)
    PawnEvalExplanation (POSITION, PAWN_INFO);
  else if ((POSITION->DYN->flags & WHITE_MINOR_ONLY
	    || POSITION->DYN->flags & BLACK_MINOR_ONLY)
	   && PAWN_INFO->PAWN_HASH != POSITION->DYN->PAWN_HASH)
    PawnEval (POSITION, PAWN_INFO);

  if ((POSITION->DYN->flags & WHITE_MINOR_ONLY
       && PAWN_INFO->wPfile_count <= 1) ||
      (POSITION->DYN->flags & BLACK_MINOR_ONLY
       && PAWN_INFO->bPfile_count <= 1) ||
      (POSITION->DYN->flags & 128));
  else if (!EXPLAIN)
    {
      if (POSITION->wtm)
	{
	  positional = (POSITION->DYN - 1)->PositionalValue;
	  cp = POSITION->DYN->cp;
	  to = TO (move);
	  Value = PST (POSITION->sq[to], to) - PST (POSITION->sq[to], FROM (move));
	  if (cp)
	    Value -= PST (cp, to);
	  phase = MIN (POSITION->DYN->material & 0xff, 32);
	  end = (sint16) (Value & 0xffff);
	  open = (end < 0) + (sint16) ((Value >> 16) & 0xffff);
	  antiphase = 32 - phase;
	  Value = (end * antiphase + open * phase) / 32;
	  positional += Value;
	  v = positional + matval;
	  if (v < -max - 16 * (int) (POSITION->DYN - 1)->lazy
	      || v > -min + 16 * (int) (POSITION->DYN - 1)->lazy)
	    {
	      POSITION->DYN->lazy = (POSITION->DYN - 1)->lazy + 1;
	      POSITION->DYN->Value = v;
	      POSITION->DYN->PositionalValue = positional;
	      Mobility (POSITION);
	      return;
	    }
	}
      else
	{
	  positional = (POSITION->DYN - 1)->PositionalValue;
	  cp = POSITION->DYN->cp;
	  to = TO (move);
	  Value = PST (POSITION->sq[to], to) - PST (POSITION->sq[to], FROM (move));
	  if (cp)
	    Value -= PST (cp, to);
	  phase = MIN (POSITION->DYN->material & 0xff, 32);
	  end = (sint16) (Value & 0xffff);
	  open = (end < 0) + (sint16) ((Value >> 16) & 0xffff);
	  antiphase = 32 - phase;
	  Value = (end * antiphase + open * phase) / 32;
	  positional += Value;
	  v = positional + matval;
	  if (v < min - 16 * (int) (POSITION->DYN - 1)->lazy
	      || v > max + 16 * (int) (POSITION->DYN - 1)->lazy)
	    {
	      POSITION->DYN->lazy = (POSITION->DYN - 1)->lazy + 1;
	      POSITION->DYN->Value = -v;
	      POSITION->DYN->PositionalValue = positional;
	      Mobility (POSITION);
	      return;
	    }
	}
    }

  wKs = POSITION->wKsq;
  bKs = POSITION->bKsq;
  wKatt = AttK[wKs];
  bKatt = AttK[bKs];
  bGoodAtt = wGoodAtt = 0;
  if (PAWN_INFO->PAWN_HASH != POSITION->DYN->PAWN_HASH)
    PawnEval (POSITION, PAWN_INFO);
  Value = (POSITION->DYN->STATIC) + (PAWN_INFO->SCORE);

  POSITION->DYN->wXray = 0;

  A = (wBitboardP & (~FILEa)) << 7;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck = (T >> 7);
  wPatt = A;
  A = (wBitboardP & (~FILEh)) << 9;
  T = A & bBitboardK;
  POSITION->DYN->bKcheck |= (T >> 9);
  wPatt |= A;
  POSITION->DYN->wAtt = wPatt;
  A = (bBitboardP & (~FILEh)) >> 7;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck = (T << 7);
  bPatt = A;
  A = (bBitboardP & (~FILEa)) >> 9;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 9);
  bPatt |= A;
  POSITION->DYN->bAtt = bPatt;

  bOKxray = (~bBitboardP) & ~wPatt;
  wOKxray = (~wBitboardP) & ~bPatt;
  wGoodMinor = (wBitboardN | wBitboardB) & wPatt;
  bGoodMinor = (bBitboardN | bBitboardB) & bPatt;

  /****************************************************************/

  EE ("STATIC %d:%d\n", OpEg (POSITION->DYN->STATIC));
  if (wPatt & bKatt)
    bKhit = HitP;
  else
    bKhit = 0;
  U = (POSITION->OccupiedBW >> 8) & wBitboardP;
  if (U)
    EE ("wP blocked ");
  while (U)
    {
      b = BSF (U);
      Value -= PawnAntiMobility;
      EE ("%c%c -%d:-%d ",'a' + FILE(b), '1' + RANK(b), OpEg (PawnAntiMobility));
      BitClear (b, U);
    }
  if ((POSITION->OccupiedBW >> 8) & wBitboardP)
    EE ("\n");
  wSafeMob = ~(bPatt | wBitboardOcc);

  U = wBitboardQ;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      AttB = AttB (b);
      AttR = AttR (b);
      EE ("wQ%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
	  (AttB | AttR ) & bKatt ? '*' : ' ', OpEg (PST (wEnumQ, b)));
      if (bBitboardK & DIAG[b])
	{
	  T = AttB (bKs) & AttB;
	  if (T)
	    {
	      Value += wQxrayD[POSITION->sq[BSF (T)]];
	      EE (" xrayD %d:%d ", OpEg (wQxrayD[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      else if (bBitboardK & ORTHO[b])
	{
	  T = AttR (bKs) & AttR;
	  if (T)
	    {
	      Value += wQxrayO[POSITION->sq[BSF (T)]];
	      EE (" xrayO %d:%d ", OpEg (wQxrayO[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      A = AttB | AttR;
      T = A & wSafeMob;
      POSITION->DYN->wAtt |= A;
      if (A & bKatt)
	bKhit += HitQ;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	{
	  Value += QguardK;
	  EE ("guardK %d:%d ", OpEg (QguardK));
	}
      Value += MobQ (T);
      EE ("mob %d:%d ", OpEg (MobQ (T)));
      if (A & (~bPatt) & bBitboardOcc)
	{
	  Value += Qatt;
	  EE ("att %d:%d ", OpEg (Qatt));
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattQ;
	  EE ("Patt -%d:-%d ", OpEg (PattQ));
	  bGoodAtt += 1;
	}
      if (RANK (b) == R7)
	{
	  if ((bBitboardP | bBitboardK) & Ranks78)
	    {
	      Value += Queen7th;
	      EE ("7th %d:%d ", OpEg (Queen7th));
	      if (wBitboardR & RANK7 & AttR && bBitboardK & RANK8)
		{
		  Value += DoubQueen7th;
		  EE ("doub %d:%d ", OpEg (DoubQueen7th));
		}
	    }
	}
      EE ("\n");
    }

  U = wBitboardR;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttR (b);
      EE ("wR%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
	  A & bKatt ? '*' : ' ', OpEg (PST (wEnumR, b)));
      POSITION->DYN->wAtt |= A;
      if (bBitboardK & ORTHO[b])
	{
	  T = AttR (bKs) & A;
	  if (T)
	    {
	      Value += wRxray[POSITION->sq[BSF (T)]];
	      EE ("xray %d:%d ", OpEg (wRxray[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      if (A & bKatt)
	bKhit += HitR;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	{
	  Value += RguardK;
	  EE ("guardK %d:%d ", OpEg (RguardK));
	}
      Value += MobR (A & wOKxray);
      EE ("mob %d:%d ", OpEg (MobR (A & wOKxray)));

      if (A & (~bPatt) & bBitboardP)
	{
	  Value += RattP;
	  EE ("attP %d:%d ", OpEg (RattP));
	}
      if (A &
	  ((bBitboardN | bBitboardB) & ~bPatt))
	{
	  Value += RattBN;
	  EE ("attBN %d:%d ", OpEg (RattBN));
	}
      if (A & bBitboardQ)
	{
	  Value += RattQ;
	  EE ("attQ %d:%d ", OpEg (RattQ));
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattR;
	  EE ("Patt -%d:-%d ", OpEg (PattR));
	  bGoodAtt += 1;
	}

      if ((wBitboardP & OpenFileW[b]) == 0)
	{
	  Value += RookHalfOpen;
	  if ((bBitboardP & OpenFileW[b]) == 0)
	    {
	      T = bGoodMinor & OpenFileW[b];
	      if (!T)
		{
		  Value += RookOpenFile;
		  EE ("open %d:%d ", OpEg (RookOpenFile + RookHalfOpen));
		}
	      else
		{
		  int t = BSF (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontB[RANK (t)] & wBitboardP) == 0)
		    {
		      Value += RookOpenFixedMinor;
		      EE ("openfixedBN %d:%d ", OpEg (RookOpenFixedMinor));
		    }
		  else
		    {
		      Value += RookOpenMinor;
		      EE ("openBN %d:%d ", OpEg (RookOpenMinor));
		    }
		}
	    }
	  else
	    {
	      T = OpenFileW[b] & bBitboardP;
	      EE ("hopen %d:%d ", OpEg (RookHalfOpen));
	      if (T)
		{
		  int t = BSF (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontW[RANK (t)] & bBitboardP) == 0)
		    {
		      Value += RookHalfOpenPawn;
		      EE ("pawn %d:%d ", OpEg (RookHalfOpenPawn));
		    }
		}
	    }
	  if (bBitboardK & OpenFileW[b])
	    {
	      Value += RookHalfOpenKing;
	      EE ("king %d:%d ", OpEg (RookHalfOpenKing));
	    }
	}

      if (SqSet[b] & wOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontW[RANK (b)] & bBitboardP) == 0)
	{
	  if (wBitboardP & AttPw[b])
	    {
	      Value += OutpostRook;
	      if (A & (bKatt | (bBitboardOcc & ~bPatt)) & RankArray[RANK (b)])
		{
		  Value += OutpostRookGuarded;
		  EE ("postG %d:%d ", OpEg (OutpostRook + OutpostRookGuarded));
		}
	      else
		EE ("post %d:%d ", OpEg (OutpostRook));
	    }
	}

      if (RANK (b) == R8)
	{
	  if (bBitboardK & RANK8)
	    {
	      Value += RookKing8th;
	      EE ("8th %d:%d ", OpEg (RookKing8th));
	    }
	}
      if (RANK (b) == R7)
	{
	  if ((bBitboardP | bBitboardK) & Ranks78)
	    {
	      Value += Rook7thKingPawn;
	      EE ("7th %d:%d ", OpEg (Rook7thKingPawn));
	      if (bBitboardK & RANK8 && (wBitboardQ | wBitboardR) & RANK7 & A)
		{
		  Value += DoubRook7thKingPawn;
		  EE ("doub %d:%d ", OpEg (DoubRook7thKingPawn));
		}
	    }
	}
      if (RANK (b) == R6
	  && (bBitboardP | bBitboardK) & Ranks678)
	{
	  Value += Rook6thKingPawn;
	  EE ("6th %d:%d ", OpEg (Rook6thKingPawn));
	}
      EE ("\n");
    }

  wSafeMob |= bBitboardOcc ^ bBitboardP;

  U = wBitboardB;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttB (b);
      EE ("wB%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
	  A & bKatt ? '*' : ' ', OpEg (PST (wEnumBL, b)));
      POSITION->DYN->wAtt |= A;
      if (bBitboardK & DIAG[b])
	{
	  T = AttB (bKs) & A;
	  if (T)
	    {
	      Value += wBxray[POSITION->sq[BSF (T)]];
              EE ("xray %d:%d ", OpEg (wBxray[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      if (A & bKatt) /* or bBitboardK ? */
	bKhit += HitB;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	{
	  Value += BguardK;
          EE ("guardK %d:%d ", OpEg (BguardK));
	}
      Value += MobB (A & wSafeMob, InFrontW[RANK (b)]);
      EE ("mob %d:%d ", OpEg ( MobB (A & wSafeMob, InFrontW[RANK (b)])));

      if (A & (~bPatt) & bBitboardP)
	{
	  Value += bAttP;
          EE ("attP %d:%d ", OpEg (bAttP));
	}
      if (A & (~bPatt) & bBitboardN)
	{
	  Value += bAttN;
          EE ("bAttN %d:%d ", OpEg (bAttN));
	}
      if (A & (bBitboardR | bBitboardQ))
	{
	  Value += bAttRQ;
          EE ("attRQ %d:%d ", OpEg (bAttRQ));
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattB;
          EE ("Patt -%d:-%d ", OpEg (PattB));
	  bGoodAtt += 1;
	}

      if (SqSet[b] & LIGHT)
	{
	  int x1;
	  x1 = (PAWN_INFO->wPlight + PAWN_INFO->bPlight / 2) * SCORE (1, 1);
	  Value -= x1;
	  EE ("bad -%d:-%d ", OpEg (x1));
	  x1 = POPCNT (bBitboardP & LIGHT & InFrontB[RANK (b)] & ~bPatt)
	    * SCORE (0, 2);
	  Value += x1;
	  if (x1) EE (" attP %d:%d ", OpEg (x1));
	}
      else
	{
	  int x1;
	  x1 = (PAWN_INFO->wPdark + PAWN_INFO->bPdark / 2) * SCORE (1, 1);
	  Value -= x1;
	  EE ("bad -%d:-%d ", OpEg (x1));	  
	  x1 = POPCNT (bBitboardP & DARK & InFrontB[RANK (b)] & ~bPatt)
	    * SCORE (0, 2);
	  Value += x1;
	  if (x1) EE (" attP %d:%d ", OpEg (x1));
	}

      if (SqSet[b] & wOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontW[RANK (b)] & bBitboardP) == 0)
	{
	  if (wBitboardP & AttPw[b])
	    {
	      Value += OutpostBishop;
	      if (A & (bKatt | (bBitboardOcc & ~bPatt)))
		{
		  Value += OutpostBishopGuarded;
		  EE ("postG %d:%d ", OpEg (OutpostBishop + OutpostBishopGuarded));
		}
	      else
		EE ("post %d:%d ", OpEg (OutpostBishop));
	    }
	}
      if (POSITION->sq[BishopTrapSq[b]] == bEnumP)
	{
	  Value -= BishopTrapValue;
	  if (POSITION->sq[GoodBishopTrapSq[b]] == bEnumP)
	    {
	      Value -= BishopTrapGuardValue;
	      EE ("trapG %d:%d ", OpEg (BishopTrapValue + BishopTrapGuardValue));
	    }
	  else
	    EE ("trap %d:%d ", OpEg (BishopTrapValue));
	}
      EE ("\n");
    }

  U = wBitboardN;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttN[b];
      EE ("wN%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
	  A & (bKatt | bBitboardK) ? '*' : ' ', OpEg (PST (wEnumN, b)));
      POSITION->DYN->wAtt |= A;
      if (A & (bKatt | bBitboardK))
	bKhit += HitN;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & (wKatt | wBitboardK))
	{
	  Value += NguardK;
          EE ("guardK %d:%d ", OpEg (NguardK));
	}
      Value += MobN (A & wSafeMob, InFrontW[RANK (b)]);
      EE ("mob %d:%d ", OpEg (MobN (A & wSafeMob, InFrontW[RANK (b)])));

      if (A & (~bPatt) & bBitboardP)
	{
	  Value += NattP;
          EE ("attP %d:%d ", OpEg (NattP));
	}
      if (A & (~bPatt) & bBitboardB)
	{
	  Value += NattB;
          EE ("attB %d:%d ", OpEg (NattB));
	}
      if (A & (bBitboardR | bBitboardQ))
	{
	  Value += NattRQ;
          EE ("attRQ %d:%d ", OpEg (NattRQ));
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattN;
          EE ("Patt -%d:-%d ", OpEg (PattN));
	  bGoodAtt += 1;
	}

      if (SqSet[b] & wOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontW[RANK (b)] & bBitboardP) == 0)
	{
	  Value += OutpostKnight;
	  if (wBitboardP & AttPw[b])
	    {
	      Value += OutpostKnightPawn;
	      if (A & (bKatt | (bBitboardOcc & ~bPatt)))
		{
		  Value += OutpostKnightAttacks;
		  EE ("postPG %d:%d ", OpEg (OutpostKnight + OutpostKnightPawn
					     + OutpostKnightAttacks));
		  if (RANK (b) == R5)
		    {
		      Value += OutpostKnight5th;
		      EE ("5th %d:%d ", OpEg (OutpostKnight5th));
		    }
		  if (FILE (b) == FD || FILE (b) == FE)
		    {
		      Value += OutpostKnightONde;
		      EE ("5th %d:%d ", OpEg (OutpostKnightONde));
		    }
		}
	      else
		EE ("postP %d:%d ", OpEg (OutpostKnight + OutpostKnightPawn));
	    }
	  else
	    EE ("post %d:%d ", OpEg (OutpostKnight));
	}
      EE ("\n");
    }

  /*****/ /*****/  /*****/ /*****/  /*****/ /*****/  /*****/ /*****/

  if (bPatt & wKatt)
    wKhit = HitP;
  else
    wKhit = 0;
  POSITION->DYN->bXray = 0;
  U = (POSITION->OccupiedBW << 8) & bBitboardP;
  if (U)
    EE ("bP blocked ");
  while (U)
    {
      b = BSF (U);
      Value += PawnAntiMobility;
      EE ("%c%c %d:%d ",'a' + FILE(b), '1' + RANK(b), OpEg (PawnAntiMobility));
      BitClear (b, U);
    }
  if ((POSITION->OccupiedBW << 8) & bBitboardP)
    EE ("\n");

  bSafeMob = ~(wPatt | bBitboardOcc);

  U = bBitboardQ;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      AttB = AttB (b);
      AttR = AttR (b);
      EE ("bQ%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
          (AttB | AttR ) & wKatt ? '*' : ' ', OpEg (PST (bEnumQ, b)));
      if (wBitboardK & DIAG[b])
	{
	  T = AttB (wKs) & AttB;
	  if (T)
	    {
	      Value -= bQxrayD[POSITION->sq[BSF (T)]];
              EE (" xrayD -%d:-%d ", OpEg (bQxrayD[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      else if (wBitboardK & ORTHO[b])
	{
	  T = AttR (wKs) & AttR;
	  if (T)
	    {
	      Value -= bQxrayO[POSITION->sq[BSF (T)]];
              EE (" xrayO -%d:-%d ", OpEg (bQxrayO[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      A = AttB | AttR;
      T = A & bSafeMob;
      POSITION->DYN->bAtt |= A;
      if (A & wKatt)
	wKhit += HitQ;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	{
	  Value -= QguardK;
          EE ("guardK -%d:-%d ", OpEg (QguardK));
	}
      Value -= MobQ (T);
      EE ("mob -%d:-%d ", OpEg (MobQ (T)));
      if (A & (~wPatt) & wBitboardOcc)
	{
	  Value -= Qatt;
          EE ("att -%d:-%d ", OpEg (Qatt));
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattQ;
          EE ("Patt %d:%d ", OpEg (PattQ));
	  wGoodAtt += 1;
	}

      if (RANK (b) == R2)
	{
	  if ((wBitboardP | wBitboardK) & Ranks12)
	    {
	      Value -= Queen7th;
              EE ("7th -%d:-%d ", OpEg (Queen7th));
	      if (bBitboardR & RANK2 & AttR && wBitboardK & RANK1)
		{
		  Value -= DoubQueen7th;
                  EE ("doub -%d:-%d ", OpEg (DoubQueen7th));
		}
	    }
	}
      EE ("\n");
    }

  U = bBitboardR;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttR (b);
      EE ("bR%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
          A & wKatt ? '*' : ' ', OpEg (PST (bEnumR, b)));
      POSITION->DYN->bAtt |= A;
      if (wBitboardK & ORTHO[b])
	{
	  T = A & AttR (wKs);
	  if (T)
	    {
	      Value -= bRxray[POSITION->sq[BSF (T)]];
              EE ("xray -%d:-%d ", OpEg (bRxray[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      if (A & wKatt)
	wKhit += HitR;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	{
	  Value -= RguardK;
          EE ("guardK -%d:-%d ", OpEg (RguardK));
	}
      Value -= MobR (A & bOKxray);
      EE ("mob -%d:-%d ", OpEg (MobR (A & bOKxray)));

      if (A & (~wPatt) & wBitboardP)
	{
	  Value -= RattP;
          EE ("attP -%d:-%d ", OpEg (RattP));
	}
      if (A & (wBitboardN | wBitboardB) & ~wPatt)
	{
	  Value -= RattBN;
          EE ("attBN -%d:-%d ", OpEg (RattBN));
	}
      if (A & wBitboardQ)
	{
	  Value -= RattQ;
          EE ("attQ -%d:-%d ", OpEg (RattQ));
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattR;
          EE ("Patt %d:%d ", OpEg (PattR));
	  wGoodAtt += 1;
	}

      if ((bBitboardP & OpenFileB[b]) == 0)
	{
	  Value -= RookHalfOpen;
	  if ((wBitboardP & OpenFileB[b]) == 0)
	    {
	      T = wGoodMinor & OpenFileB[b];
	      if (!T)
		{
		  Value -= RookOpenFile;
                  EE ("open -%d:-%d ", OpEg (RookOpenFile + RookHalfOpen));
		}
	      else
		{
		  int t = BSR (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontW[RANK (t)] & bBitboardP) == 0)
		    {
		      Value -= RookOpenFixedMinor;
                      EE ("openfixedBN -%d:-%d ", OpEg (RookOpenFixedMinor));
		    }
		  else
		    {
		      Value -= RookOpenMinor;
                      EE ("openBN -%d:-%d ", OpEg (RookOpenMinor));
		    }
		}
	    }
	  else
	    {
	      T = OpenFileB[b] & wBitboardP;
              EE ("hopen -%d:-%d ", OpEg (RookHalfOpen));
	      if (T)
		{
		  int t = BSR (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontB[RANK (t)] & wBitboardP) == 0)
		    {
		      Value -= RookHalfOpenPawn;
                      EE ("pawn -%d:-%d ", OpEg (RookHalfOpenPawn));
		    }
		}
	    }
	  if (wBitboardK & OpenFileB[b])
	    {
	      Value -= RookHalfOpenKing;
              EE ("king -%d:-%d ", OpEg (RookHalfOpenKing));
	    }
	}

      if (SqSet[b] & bOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontB[RANK (b)] & wBitboardP) == 0)
	{
	  if (bBitboardP & AttPb[b])
	    {
	      Value -= OutpostRook;
	      if (A & (wKatt | (wBitboardOcc & ~wPatt)) & RankArray[RANK (b)])
		{
		  Value -= OutpostRookGuarded;
		  EE ("postG -%d:-%d ", OpEg (OutpostRook + OutpostRookGuarded));
		}
	      else
		EE ("post -%d:-%d ", OpEg (OutpostRook));
	    }
	}

      if (RANK (b) == R1)
	{
	  if (wBitboardK & RANK1)
	    {
	      Value -= RookKing8th;
              EE ("8th -%d:-%d ", OpEg (RookKing8th));
	    }
	}
      if (RANK (b) == R2)
	{
	  if ((wBitboardP | wBitboardK) & Ranks12)
	    {
	      Value -= Rook7thKingPawn;
              EE ("7th -%d:-%d ", OpEg (Rook7thKingPawn));
	      if (wBitboardK & RANK1 && (bBitboardQ | bBitboardR) & RANK2 & A)
		{
		  Value -= DoubRook7thKingPawn;
                  EE ("doub -%d:-%d ", OpEg (DoubRook7thKingPawn));
		}
	    }
	}
      if (RANK (b) == R3 && (wBitboardP | wBitboardK) & Ranks123)
	{
	  Value -= Rook6thKingPawn;
          EE ("6th -%d:-%d ", OpEg (Rook6thKingPawn));
	}
      EE ("\n");
    }

  bSafeMob |= wBitboardOcc ^ wBitboardP;

  U = bBitboardB;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttB (b);
      EE ("bB%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
          A & wKatt ? '*' : ' ', OpEg (PST (bEnumBL, b)));
      POSITION->DYN->bAtt |= A;
      if (wBitboardK & DIAG[b])
	{
	  T = A & AttB (wKs);
	  if (T)
	    {
	      Value -= bBxray[POSITION->sq[BSF (T)]];
              EE ("xray -%d:-%d ", OpEg (bBxray[POSITION->sq[BSF (T)]]));
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      if (A & wKatt) /* or wBitboardK ? */
	wKhit += HitB;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	{
	  Value -= BguardK;
          EE ("guardK -%d:-%d ", OpEg (BguardK));
	}
      Value -= MobB (A & bSafeMob, InFrontB[RANK (b)]);
      EE ("mob -%d:-%d ", OpEg ( MobB (A & bSafeMob, InFrontB[RANK (b)])));

      if (A & (~wPatt) & wBitboardP)
	{
	  Value -= bAttP;
          EE ("attP -%d:-%d ", OpEg (bAttP));
	}
      if (A & (~wPatt) & wBitboardN)
	{
	  Value -= bAttN;
          EE ("bAttN -%d:-%d ", OpEg (bAttN));
	}
      if (A & (wBitboardR | wBitboardQ))
	{
	  Value -= bAttRQ;
          EE ("attRQ -%d:-%d ", OpEg (bAttRQ));
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattB;
          EE ("Patt %d:%d ", OpEg (PattB));
	  wGoodAtt += 1;
	}

      if (SqSet[b] & LIGHT)
	{
	  int x1;
	  x1 = (PAWN_INFO->bPlight + PAWN_INFO->wPlight / 2) * SCORE (1, 1);
	  Value += x1;
	  EE ("bad %d:%d ", OpEg (x1));
	  x1 = POPCNT (wBitboardP & LIGHT & InFrontW[RANK (b)] & ~wPatt)
	    * SCORE (0, 2);
	  Value -= x1;
          if (x1) EE (" attP -%d:-%d ", OpEg (x1));
	}
      else
	{
	  int x1;
	  x1 = (PAWN_INFO->bPdark + PAWN_INFO->wPdark / 2) * SCORE (1, 1);
	  Value += x1;
          EE ("bad %d:%d ", OpEg (x1));
	  x1 = POPCNT (wBitboardP & DARK & InFrontW[RANK (b)] & ~wPatt)
	    * SCORE (0, 2);
	  Value -= x1;
          if (x1) EE (" attP -%d:-%d ", OpEg (x1));
	}

      if (SqSet[b] & bOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontB[RANK (b)] & wBitboardP) == 0)
	{
	  if (bBitboardP & AttPb[b])
	    {
	      Value -= OutpostBishop;
	      if (A & (wKatt | (wBitboardOcc & ~wPatt)))
		{
		  Value -= OutpostBishopGuarded;
                  EE ("postG -%d:-%d ", OpEg (OutpostBishop + OutpostBishopGuarded));
		}
	      else
                EE ("post -%d:-%d ", OpEg (OutpostBishop));
	    }
	}

      if (POSITION->sq[BishopTrapSq[b]] == wEnumP)
	{
	  Value += BishopTrapValue;
	  if (POSITION->sq[GoodBishopTrapSq[b]] == wEnumP)
	    {
	      Value += BishopTrapGuardValue;
              EE ("trapG -%d:-%d ", OpEg (BishopTrapValue + BishopTrapGuardValue));
	    }
	  else
            EE ("trap -%d:-%d ", OpEg (BishopTrapValue));
	}
      EE ("\n");
    }

  U = bBitboardN;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttN[b];
      EE ("bN%c%c%c%d:%d ", 'a' + FILE (b), '1' + RANK (b),
          A & (wKatt | wBitboardK) ? '*' : ' ', OpEg (PST (bEnumN, b)));

      POSITION->DYN->bAtt |= A;
      if (A & (wKatt | wBitboardK))
	wKhit += HitN;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & (bKatt | bBitboardK))
	{
	  Value -= NguardK;
          EE ("guardK -%d:-%d ", OpEg (NguardK));
	}
      Value -= MobN (A & bSafeMob, InFrontB[RANK (b)]);
      EE ("mob -%d:-%d ", OpEg (MobN (A & bSafeMob, InFrontB[RANK (b)])));

      if (A & (~wPatt) & wBitboardP)
	{
	  Value -= NattP;
          EE ("attP -%d:-%d ", OpEg (NattP));
	}
      if (A & (~wPatt) & wBitboardB)
	{
	  Value -= NattB;
          EE ("attB -%d:-%d ", OpEg (NattB));
	}
      if (A & (wBitboardR | wBitboardQ))
	{
	  Value -= NattRQ;
          EE ("attRQ -%d:-%d ", OpEg (NattRQ));
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattN;
          EE ("Patt %d:%d ", OpEg (PattN));
	  wGoodAtt += 1;
	}

      if (SqSet[b] & bOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontB[RANK (b)] & wBitboardP) == 0)
	{
	  Value -= OutpostKnight;
	  if (bBitboardP & AttPb[b])
	    {
	      Value -= OutpostKnightPawn;
	      if (A & (wKatt | (wBitboardOcc & ~wPatt)))
		{
		  Value -= OutpostKnightAttacks;
		  EE ("postPG -%d:-%d ", OpEg (OutpostKnight + OutpostKnightPawn
                                             + OutpostKnightAttacks));
		  if (RANK (b) == R4)
		    {
		      Value -= OutpostKnight5th;
                      EE ("5th -%d:-%d ", OpEg (OutpostKnight5th));
		    }
		  if (FILE (b) == FD || FILE (b) == FE)
		    {
		      Value -= OutpostKnightONde;
                      EE ("5th -%d:-%d ", OpEg (OutpostKnightONde));
		    }
		}
              else
                EE ("postP -%d:-%d ", OpEg (OutpostKnight + OutpostKnightPawn));
	    }
	  else
            EE ("post -%d:-%d ", OpEg (OutpostKnight));
	}
      EE ("\n");
    }

  /*****/ /*****/  /*****/ /*****/  /*****/ /*****/  /*****/ /*****/

  POSITION->DYN->wAtt |= wKatt;
  POSITION->DYN->bAtt |= bKatt;
  if (bKatt & wBitboardK)
    {
      POSITION->DYN->wKcheck |= SqSet[POSITION->bKsq];
      POSITION->DYN->bKcheck |= SqSet[POSITION->wKsq];
    }

  /********************/  /********************/ /********************/

  if ((~POSITION->DYN->bAtt) & wKatt & bBitboardP)
    {
      Value += KingAttUnguardedPawn;
      EE ("wK attacks unguarded pawn %d:%d\n", OpEg (KingAttUnguardedPawn));
    }

  T = RookTrapped[wKs] & wBitboardR;
  if (T)
    {
      int t = BSF (T);
      T = OpenFileW[t] & wBitboardP;
      if (T)
	{
	  t = BSF (T);
	  t >>= 3;
	  Value -= SCORE (10 * (6 - t), 0);
	  EE ("wR trapped -%d:-%d\n", OpEg (SCORE (10 * (6 - t), 0)));
	}
    }

  if (wKatt & bKatt)
    wKhit += HitK;
  ch = (((KingSafetyMult[wKhit >> 16] *
	  (wKhit & 0xffff)) / KingSafetyDivider) << 16) + PAWN_INFO->wKdanger;
  EE ("wKing %d|%d %d", wKhit >> 16, wKhit & 0xffff,
      ((KingSafetyMult[wKhit >> 16] * (wKhit & 0xffff)) / KingSafetyDivider));
  EE (" + shelter %d = %d ", PAWN_INFO->wKdanger >> 16, ch >> 16);
  if (!bBitboardQ)
    {
      ch >>= 16;
      ch *= POPCNT (bBitboardR | bBitboardN | bBitboardB);
      ch >>= 3;
      EE ("* (%d/8) -> -%d:0", POPCNT (bBitboardR | bBitboardN | bBitboardB), ch);
      ch <<= 16;
    }
  EE ("\n");
  Value -= ch;

  /********************/  /********************/ /********************/

  if ((~POSITION->DYN->wAtt) & bKatt & wBitboardP)
    {
      Value -= KingAttUnguardedPawn;
      EE ("bK attacks unguarded pawn -%d:-%d\n", OpEg (KingAttUnguardedPawn));
    }

  T = RookTrapped[bKs] & bBitboardR;
  if (T)
    {
      int t = BSR (T);
      T = OpenFileB[t] & bBitboardP;
      if (T)
	{
	  t = BSR (T);
	  t >>= 3;
	  Value += SCORE (10 * (t - 1), 0);
          EE ("bR trapped %d:%d\n", OpEg (SCORE (10 * (t - 1), 0)));
	}
    }

  if (wKatt & bKatt)
    bKhit += HitK;
  ch = (((KingSafetyMult[bKhit >> 16] *
	  (bKhit & 0xffff)) / KingSafetyDivider) << 16) + PAWN_INFO->bKdanger;
  EE ("bKing %d|%d %d", bKhit >> 16, bKhit & 0xffff,
      ((KingSafetyMult[bKhit >> 16] * (bKhit & 0xffff)) / KingSafetyDivider));
  EE (" + shelter %d = %d ", PAWN_INFO->bKdanger >> 16, ch >> 16);
  if (!wBitboardQ)
    {
      ch >>= 16;
      ch *= POPCNT (wBitboardR | wBitboardN | wBitboardB);
      ch >>= 3;
      EE ("* (%d/8) -> %d:0", POPCNT (wBitboardR | wBitboardN | wBitboardB), ch);
      ch <<= 16;
    }
  EE ("\n");
  Value += ch;

  /********************/  /********************/ /********************/

  if (wGoodAtt >= 2)
    {
      Value += MultipleAtt;
      EE ("wGoodAtt %d:%d\n", OpEg (MultipleAtt));
    }
  if (bGoodAtt >= 2)
    {
      Value -= MultipleAtt;
      EE ("bGoodAtt -%d:-%d\n", OpEg (MultipleAtt));
    }

  /********************/  /********************/ /********************/

#define QueenEnd ( (POSITION->DYN->flags & 28) == 4)
#define RookEnd ( (POSITION->DYN->flags & 28) == 8)

  if ((wBitboardR | wBitboardQ) & CrampFile[FILE (bKs)])
    {
      Value += SCORE (0, 5);
      if ((CrampFile[FILE (bKs)] & (wBitboardP | bBitboardP)) == 0)
	{
	  Value += SCORE (5, 15);
	  EE ("bKcramp %d:%d\n", OpEg (SCORE (5, 20)));
	}      
      else
	EE ("bKcramp %d:%d\n", OpEg (SCORE (0, 5)));
    }
  if ((bBitboardR | bBitboardQ) & CrampFile[FILE (wKs)])
    {
      Value -= SCORE (0, 5);
      if ((CrampFile[FILE (wKs)] & (bBitboardP | wBitboardP)) == 0)
	{
	  Value -= SCORE (5, 15);
	  EE ("wKcramp -%d:-%d\n", OpEg (SCORE (5, 20)));
	}
      else
	EE ("wKcramp -%d:-%d\n", OpEg (SCORE (0, 5)));
    }

  /********************/  /********************/ /********************/

  U = PAWN_INFO->wPassedFiles;
  while (U)
    {
      b = BSR (FileArray[BSF (U)] & wBitboardP);
      BitClear (0, U);
      tr = RANK (b);
      if (tr <= R3)
	continue;
      EE ("wPassed %c%c ", 'a' + FILE (b), '1' + RANK (b));
      if (RookEnd)
	{
	  if (wBitboardR & OpenFileW[b])
	    {
	      if (tr == R7)
		{
		  Value -= Rook7thEnd;
		  EE ("rook7th -%d:-%d ", OpEg (Rook7thEnd));
		}
	      else if (tr == R6)
		{
		  Value -= Rook6thEnd;
		  EE ("rook6th -%d:-%d ", OpEg (Rook6thEnd));
		}
	    }
	  if (OpenFileW[b] & wBitboardK && CrampFile[FILE (wKs)] & bBitboardR)
	    {
	      Value -= SCORE (0, 1 << (tr - R2));
	      EE ("rookcramp -%d:-%d ", OpEg (SCORE (0, 1 << (tr - R2))));
	    }
	}
      if (POSITION->sq[b + 8] == 0)
	{
	  Value += PassedPawnCanMove[tr];
	  EE ("canmove %d:%d ", OpEg (PassedPawnCanMove[tr]));
	}
      if ((OpenFileW[b] & wBitboardOcc) == 0)
	{
	  Value += PassedPawnMeClear[tr];
	  EE ("meclear %d:%d ", OpEg (PassedPawnMeClear[tr]));
	}
      if ((OpenFileW[b] & bBitboardOcc) == 0)
	{
	  Value += PassedPawnOppClear[tr];
	  EE ("oppclear %d:%d ", OpEg (PassedPawnOppClear[tr]));
	}
      if ((OpenFileW[b] & (~POSITION->DYN->wAtt) & POSITION->DYN->bAtt) == 0)
	{
	  Value += PassedPawnIsFree[tr];
	  EE ("isfree %d:%d ", OpEg (PassedPawnIsFree[tr]));
	}
      if (QueenEnd)
	{
	  if (tr == R7 && wBitboardQ & OpenFileW[b])
	    {
	      Value -= Queen7thEnd;
	      EE ("queen7th -%d:-%d ", OpEg (Queen7thEnd));
	    }
	  Value += RankQueenEnd[tr];
	  EE ("Qend %d:%d ", OpEg (RankQueenEnd[tr]));
	}
      EE ("\n");
    }

  U = PAWN_INFO->bPassedFiles;
  while (U)
    {
      b = BSF (FileArray[BSF (U)] & bBitboardP);
      BitClear (0, U);
      tr = RANK (b);
      if (tr >= R6)
	continue;
      EE ("bPassed %c%c ", 'a' + FILE (b), '1' + RANK (b));
      if (RookEnd)
	{
	  if (bBitboardR & OpenFileB[b])
	    {
	      if (tr == R2)
		{
		  Value += Rook7thEnd;
                  EE ("rook7th %d:%d ", OpEg (Rook7thEnd));
		}
	      else if (tr == R3)
		{
		  Value += Rook6thEnd;
                  EE ("rook6th %d:%d ", OpEg (Rook6thEnd));
		}
	    }
	  if (OpenFileB[b] & bBitboardK && CrampFile[FILE (bKs)] & wBitboardR)
	    {
	      Value += SCORE (0, 1 << (R7 - tr));
              EE ("rookcramp %d:%d ", OpEg (SCORE (0, 1 << (R7 - tr))));
	    }
	}
      if (POSITION->sq[b - 8] == 0)
	{
	  Value -= PassedPawnCanMove[7 - tr];
          EE ("canmove -%d:-%d ", OpEg (PassedPawnCanMove[7 - tr]));
	}
      if ((OpenFileB[b] & bBitboardOcc) == 0)
	{
	  Value -= PassedPawnMeClear[7 - tr];
          EE ("meclear -%d:-%d ", OpEg (PassedPawnMeClear[7 - tr]));
	}
      if ((OpenFileB[b] & wBitboardOcc) == 0)
	{
	  Value -= PassedPawnOppClear[7 - tr];
          EE ("oppclear -%d:-%d ", OpEg (PassedPawnOppClear[7 - tr]));
	}
      if ((OpenFileB[b] & POSITION->DYN->
	   wAtt & ~POSITION->DYN->bAtt) == 0)
	{
	  Value -= PassedPawnIsFree[7 - tr];
          EE ("isfree -%d:-%d ", OpEg (PassedPawnIsFree[7 - tr]));
	}
      if (QueenEnd)
	{
	  if (tr == R2 && bBitboardQ & OpenFileB[b])
	    {
	      Value += Queen7thEnd;
              EE ("queen7th %d:%d ", OpEg (Queen7thEnd));
	    }
	  Value -= RankQueenEnd[7 - tr];
          EE ("Qend -%d:-%d ", OpEg (RankQueenEnd[7 - tr]));
	}
      EE ("\n");
    }

  /*******/  /*******/  /*******/  /*******/  /*******/  /*******/  /*******/

  phase = MIN (POSITION->DYN->material & 0xff, 32);
  end = (sint16) (Value & 0xffff);
  open = (end < 0) + (sint16) ((Value >> 16) & 0xffff);
  antiphase = 32 - phase;
  Value = end * antiphase + open * phase;
  EE ("Interpolate: open %d end %d phase %d -> Value %d\n", open, end, phase, Value / 32);
  Value = Value / 32 + matval;
  Value = (Value * TOKEN) / 128;
  EE ("material %d token %d/128 -> Value %d\n", matval, TOKEN, Value);
  if (Value > 0)
    {
      Value -= (PAWN_INFO->wDrawWeight * MIN (Value, 100)) / 64;
      EE ("DrawWeight %d/64 -> Value %d\n", PAWN_INFO->wDrawWeight, Value);
    }
  else
    {
      Value += (PAWN_INFO->bDrawWeight * MIN (-Value, 100)) / 64;
      EE ("DrawWeight %d/64 -> Value %d\n", PAWN_INFO->bDrawWeight, Value);
    }

#define BISHOP_KNIGHT_MATE (POSITION->DYN->flags & 128)
#define MAX_DIST(i,j) ( MAX (FileDistance (i, j), RankDistance (i, j) ) )
#define MIN_DIST(i,j) ( MIN ( FileDistance (i, j), RankDistance (i, j) ) )

  if (BISHOP_KNIGHT_MATE)
    {
      if (Value > 0)
	{
	  if (wBitboardBL)
	    Value -= 20 * MIN (MAX_DIST (A8, POSITION->bKsq),
			       MAX_DIST (H1, POSITION->bKsq)) +
	             10 * MIN (MIN_DIST (A8, POSITION->bKsq),
			       MIN_DIST (H1, POSITION->bKsq));
	  else
	    Value -= 20 * MIN (MAX_DIST (A1, POSITION->bKsq),
			       MAX_DIST (H8, POSITION->bKsq)) +
	             10 * MIN (MIN_DIST (A1, POSITION->bKsq),
			       MIN_DIST (H8, POSITION->bKsq));
	}
      else
	{
	  if (bBitboardBL)
	    Value += 20 * MIN (MAX_DIST (A8, POSITION->wKsq),
			       MAX_DIST (H1, POSITION->wKsq)) +
	             10 * MIN (MIN_DIST (A8, POSITION->wKsq),
			       MIN_DIST (H1, POSITION->wKsq));
	  else
	    Value += 20 * MIN (MAX_DIST (A1, POSITION->wKsq),
			       MAX_DIST (H8, POSITION->wKsq)) +
	             10 * MIN (MIN_DIST (A1, POSITION->wKsq),
			       MIN_DIST (H8, POSITION->wKsq));
	}
      EE ("BNmate -> %d\n", Value);
    }

  if (POSITION->DYN->reversible > 50)
    {
      Value *= (114 - POSITION->DYN->reversible);
      Value /= 64;
      EE ("50move rule (%d/64) -> %d\n", (114 - POSITION->DYN->reversible), Value);
    }

#define WHITE_MINOR_ONLY (POSITION->DYN->flags & 32)
#define WHITE_HAS_PIECE (POSITION->DYN->flags & 2)
#define BLACK_MINOR_ONLY (POSITION->DYN->flags & 64)
#define BLACK_HAS_PIECE (POSITION->DYN->flags & 1)

  if (Value > 0)
    {
      if (!POSITION->wtm && !BLACK_HAS_PIECE
	  && (bBitboardK ^ bBitboardP) == bBitboardOcc
	  && !((bBitboardP >> 8) & ~POSITION->OccupiedBW)
	  && !(AttK[POSITION->bKsq] & ~POSITION->DYN->wAtt)
	  && !POSITION->DYN->bKcheck)
	Value = 0;
      if (WHITE_MINOR_ONLY)
	{
	  if (wBitboardN)
	    {
	      if (wBitboardP == SqSet[A7]
		  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
		Value = 0;
	      if (wBitboardP == SqSet[H7]
		  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
		Value = 0;
	    }
	  else if (wBitboardBL && !(wBitboardP & NOTh)
		   && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
	    {
	      if (wBitboardP & SqSet[H5] &&
		  bBitboardP == (SqSet[G7] | SqSet[H6]));
	      else
		Value = 0;
	    }
	  else if (wBitboardBD && !(wBitboardP & NOTa)
		   && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
	    {
	      if (wBitboardP & SqSet[A5] &&
		  bBitboardP == (SqSet[B7] | SqSet[A6]));
	      else
		Value = 0;
	    }
	  if (!wBitboardP)
	    Value = 0;
	}
      if (Value == 0)
	EE ("Value -> 0 due to no winning chances\n");
    }
  else if (Value < 0) /* HACK */
    {
      if (POSITION->wtm && !WHITE_HAS_PIECE
	  && (wBitboardK ^ wBitboardP) == wBitboardOcc
	  && !((wBitboardP << 8) & ~POSITION->OccupiedBW)
	  && !(AttK[POSITION->wKsq] & ~POSITION->DYN->bAtt)
	  && !POSITION->DYN->wKcheck)
	Value = 0;
      if (BLACK_MINOR_ONLY)
	{
	  if (bBitboardN)
	    {
	      if (bBitboardP == SqSet[A2]
		  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[A1])
		Value = 0;
	      if (bBitboardP == SqSet[H2]
		  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[H1])
		Value = 0;
	    }
	  else if (bBitboardBD && !(bBitboardP & NOTh)
		   && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[H1])
	    {
	      if (bBitboardP & SqSet[H4]
		  && wBitboardP == (SqSet[G2] | SqSet[H3]));
	      else
		Value = 0;
	    }
	  else if (bBitboardBL && !(bBitboardP & NOTa)
		   && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[A1])
	    {
	      if (bBitboardP & SqSet[A4]
		  && wBitboardP == (SqSet[B2] | SqSet[A3]));
	      else
		Value = 0;
	    }
	  if (!bBitboardP)
	    Value = 0;
	}
      if (Value == 0)
	EE ("Value -> 0 due to no winning chances\n");
    }

  POSITION->DYN->Value = POSITION->wtm ? Value : -Value;
  POSITION->DYN->PositionalValue = Value - matval;
  POSITION->DYN->lazy = 0;
  EvalHash[POSITION->DYN->HASH & EVAL_HASH_MASK] =
    (POSITION->DYN->HASH & 0xffffffffffff0000) | (POSITION->DYN->Value & 0xffff);
  if (move && !(POSITION->DYN - 1)->lazy)
    POSITION->wtm ?
      AdjustPositionalGainW (POSITION, move) : AdjustPositionalGainB (POSITION, move);
  EE ("Final Evaluation %d\n", Value);
}
#endif /* UTILITIES */



#define Ranks78 0xffff000000000000
#define Ranks678 0xffffff0000000000
#define Ranks12 0x000000000000ffff
#define Ranks123 0x00000000000ffffff

#include "RobboLito.h"
#include "evaluation.v"

#ifndef MINIMAL
typedef struct { uint64 RandKey; uint8 pad[56]; } RAND; /* cache */
static RAND Rand[MAX_CPUS]; /* init ? */
static uint32 Random32 (int cpu)
{
  Rand[cpu].RandKey =
    Rand[cpu].RandKey * 0x7913cc52088a6cfULL + 0x99f2e6bb0313ca0dULL;
  return ((Rand[cpu].RandKey >> 18) & 0xffffffff);
}

void InitRandom32 (uint64 x)
{
  int cpu;
  for (cpu = 0; cpu < MAX_CPUS; cpu++)
    {
      x = x * 0xb18ec564ff729005ULL + 0x86ee25701b5e244fULL;
      Rand[cpu].RandKey = x;
    }
}
#endif

/* SLAB?? */
uint8 YakovWTM[24576] = {
#include "YakovChart.bi"
};

uint8 YakovBTM[24576] = {
#include "YakovChart.ne"
};

static void AdjustPositionalGain (typePOS* POSITION, int move)
{
  int v, p, m;
  if (POSITION->DYN->cp)
    return;
  p = POSITION->sq[TO (move)];
  m = move & 07777;
  v = ((POSITION->DYN - 1)->PositionalValue) - POSITION->DYN->PositionalValue;
  v = POSITION->wtm ? v : -v;
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
  int Value = 975 * (POPCNT (wBitboardQ) - POPCNT (bBitboardQ));
  Value += 500 * (POPCNT (wBitboardR) - POPCNT (bBitboardR));
  Value += 325 * (POPCNT (wBitboardB) - POPCNT (bBitboardB));
  Value += 325 * (POPCNT (wBitboardN) - POPCNT (bBitboardN));
  Value += 100 * (POPCNT (wBitboardP) - POPCNT (bBitboardP));
  if (wBitboardBL && wBitboardBD)
    Value += 50;
  if (bBitboardBL && bBitboardBD)
    Value -= 50;
  Value *= UCI_MATERIAL_WEIGHTING;
  Value >>= 10;
  if (Value > 2000)
    Value -= (Value - 2000) / 2;
  if (Value < -2000)
    Value -= (Value + 2000) / 2;
  return Value;
}

static void KingPawnWhite
(typePOS* POSITION, int matval, uint8 TOKEN, typePawnEval* PAWN_INFO)
{
  int Value, WhiteLeader, BlackLeader, sq, tr;
  uint8 C;
  uint64 A, T, wPatt, bPatt;
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
  POSITION->DYN->wAtt |= A;
  wPatt = POSITION->DYN->wAtt;
  POSITION->DYN->wAtt |= AttK[POSITION->wKsq];
  A = (bBitboardP & (~FILEh)) >> 7;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 7);
  POSITION->DYN->bAtt = A;
  A = (bBitboardP & (~FILEa)) >> 9;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 9);
  POSITION->DYN->bAtt |= A;
  bPatt = POSITION->DYN->bAtt;
  POSITION->DYN->bAtt |= AttK[POSITION->bKsq];
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
      if (!(wBitboardP & ~FILEh)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
	POSITION->DYN->Value = 0;
      if (!(wBitboardP & ~FILEa)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[B6] == wEnumP && POSITION->sq[B7] == bEnumP
	  && !(wBitboardP & ~FILEb)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[G6] == wEnumP && POSITION->sq[G7] == bEnumP
	  && !(wBitboardP & ~FILEg)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
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
      if (POSITION->sq[B3] == bEnumP && POSITION->sq[B2] == wEnumP
	  && !(bBitboardP & ~FILEb)
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[A1])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[G3] == bEnumP && POSITION->sq[G2] == wEnumP
	  && !(bBitboardP & ~FILEg)
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[H1])
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
  if (POSITION->DYN->Value < 0 && !((wBitboardP << 8) & ~POSITION->OccupiedBW)
      && !(wPatt & bBitboardOcc) && !POSITION->DYN->ep
      && !(AttK[POSITION->wKsq] & ~POSITION->DYN->bAtt)
      && !POSITION->DYN->wKcheck)
    POSITION->DYN->Value = 0;
}

static void KingPawnBlack
(typePOS* POSITION, int matval, uint8 TOKEN, typePawnEval* PAWN_INFO)
{
  int Value, WhiteLeader, BlackLeader, sq, tr;
  uint8 C;
  uint64 A, T, wPatt, bPatt;
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
  POSITION->DYN->wAtt |= A;
  wPatt = POSITION->DYN->wAtt;
  POSITION->DYN->wAtt |= AttK[POSITION->wKsq];
  A = (bBitboardP & (~FILEh)) >> 7;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 7);
  POSITION->DYN->bAtt = A;
  A = (bBitboardP & (~FILEa)) >> 9;
  T = A & wBitboardK;
  POSITION->DYN->wKcheck |= (T << 9);
  POSITION->DYN->bAtt |= A;
  bPatt = POSITION->DYN->bAtt;
  POSITION->DYN->bAtt |= AttK[POSITION->bKsq];
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
	  && (AttK[POSITION->bKsq] | bBitboardK) & SqSet[H8])
	POSITION->DYN->Value = 0;
      if ((wBitboardP & ~FILEa) == 0
	  && (AttK[POSITION->bKsq] | bBitboardK) & SqSet[A8])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[B6] == wEnumP && POSITION->sq[B7] == bEnumP
	  && !(wBitboardP & ~FILEb)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[A8])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[G6] == wEnumP && POSITION->sq[G7] == bEnumP
	  && !(wBitboardP & ~FILEg)
	  && (bBitboardK | AttK[POSITION->bKsq]) & SqSet[H8])
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
      if (POSITION->sq[B3] == bEnumP && POSITION->sq[B2] == wEnumP
	  && !(bBitboardP & ~FILEb)
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[A1])
	POSITION->DYN->Value = 0;
      if (POSITION->sq[G3] == bEnumP && POSITION->sq[G2] == wEnumP
	  && !(bBitboardP & ~FILEg)
	  && (wBitboardK | AttK[POSITION->wKsq]) & SqSet[H1])
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
  if (POSITION->DYN->Value < 0 && !((bBitboardP >> 8) & ~POSITION->OccupiedBW)
      && !(bPatt & wBitboardOcc) && !POSITION->DYN->ep
      && !(AttK[POSITION->bKsq] & ~POSITION->DYN->wAtt)
      && !POSITION->DYN->bKcheck)
    POSITION->DYN->Value = 0;
}

#define EVAL_TWEAK 0x12345678
#define GET_EVAL_HASH(Z) EvalHash[(Z ^ EVAL_TWEAK) & EVAL_HASH_MASK]
void Eval (typePOS* POSITION, int min, int max, int move, int depth)
{
  typePawnEval* PAWN_POINTER;
  int index, matval, Value, MobValue = 0;
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
  boolean STORE_EVAL_HASH = TRUE;
  ZUGZWANG_DETECT_DECLARE;

  PAWN_POINTER = PawnHash + (POSITION->DYN->PAWN_HASH & (PawnHashSize - 1));
  PREFETCH_PAWN_HASH;
  index = (POSITION->DYN->material >> 8) & 0x7ffff;
  TOKEN = MATERIAL[index].token;
  POSITION->DYN->flags = MATERIAL[index].flags;
  POSITION->DYN->exact = FALSE;
  if (!(POSITION->DYN->material & 0x80000000))
    matval = MATERIAL[index].Value;
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
	  if (!(wBitboardOcc ^ (wBitboardBL | wBitboardK | wBitboardP)) ||
	      !(wBitboardOcc ^ (wBitboardBD | wBitboardK | wBitboardP)))
	    POSITION->DYN->flags |= 32;
	  if (!(bBitboardOcc ^ (bBitboardBL | bBitboardK | bBitboardP)) ||
	      !(bBitboardOcc ^ (bBitboardBD | bBitboardK | bBitboardP)))
	    POSITION->DYN->flags |= 64;
	}
      else
	{
	  matval = MATERIAL[index].Value;
	  POSITION->DYN->material &= 0x7fffffff;
	}
    }

  if (((POSITION->DYN->HASH ^ GET_EVAL_HASH (POSITION->DYN->HASH)) &
       0xffffffffffff0000) == 0)
    {
      Value = (int) ((sint16) (GET_EVAL_HASH (POSITION->DYN->HASH) & 0xffff));
      POSITION->DYN->lazy = 0;
      Mobility (POSITION);
      POSITION->DYN->PositionalValue = ((POSITION->wtm) ? Value : -Value) - matval;
      POSITION->DYN->Value = Value;
      if (move && !(POSITION->DYN - 1)->lazy)
	AdjustPositionalGain (POSITION, move);
      if (Value == 0)
	POSITION->DYN->exact = 3;
      if (ABS (Value) > 13000)
	POSITION->DYN->exact = 1; /* NewBuy, no total */
      if (ABS (Value) > 19000)
	POSITION->DYN->exact = 3;
      return;
    }

#ifdef CON_ROBBO_BUILD
  if (ROBBO_LOAD && depth >= TRIPLE_WEAK_PROBE_DEPTH
      && ROBBO_TRIPLE_CONDITION (POSITION))
    {
      int va, v;
      boolean total;
      Mobility (POSITION);
      if ((v = TRIPLE_VALUE
	   (POSITION, &va, &total, !SEARCH_ROBBO_BASES, /* demand TOTAL */
	    depth < TRIPLE_DEFINITE_PROBE_DEPTH &&
	          HEIGHT (POSITION) > TRIPLE_DEFINITE_PROBE_HEIGHT, FALSE)) == TRUE)
	{
	  int c = POPCNT (POSITION->OccupiedBW);
	  int molt = (total ? 25000 : 19000) - (128 * c) - 16 * HEIGHT (POSITION);
	  if (va == 0)
	    POSITION->DYN->Value = 0;
	  else
	    {
	      POSITION->DYN->Value = (va > 0) ? molt : -molt;
	      POSITION->DYN->Value += (POSITION->wtm ? matval : -matval);
	    }
	  POSITION->DYN->lazy = 1;
	  POSITION->DYN->PositionalValue = 0;
	  GET_EVAL_HASH (POSITION->DYN->HASH) =
	    (POSITION->DYN->HASH & 0xffffffffffff0000) |
	    (POSITION->DYN->Value & 0xffff);
	  POSITION->DYN->exact = (!va || total) ? 3 : TRUE; /* HACK */
	  return;
	}
      if (v == FALSE) /* mere */
	STORE_EVAL_HASH = FALSE;
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
      if (POSITION->DYN->Value == 0)
	POSITION->DYN->Value = 1;
      GET_EVAL_HASH (POSITION->DYN->HASH) =
	(POSITION->DYN->HASH & 0xffffffffffff0000) |
	(POSITION->DYN->Value & 0xffff);
      return;
    }

#define WHITE_MINOR_ONLY (POSITION->DYN->flags & 32)
#define BLACK_MINOR_ONLY (POSITION->DYN->flags & 64)

  if ((WHITE_MINOR_ONLY || BLACK_MINOR_ONLY)
      && PAWN_INFO->PAWN_HASH != POSITION->DYN->PAWN_HASH)
    PawnEval (POSITION, PAWN_INFO);

  if ((WHITE_MINOR_ONLY && PAWN_INFO->wPfile_count <= 1)
      || (BLACK_MINOR_ONLY && PAWN_INFO->bPfile_count <= 1)
      || (POSITION->DYN->flags & 128));
  else
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
  Value = (PAWN_INFO->SCORE);
  end = (sint16) (POSITION->DYN->STATIC & 0xffff);
  open = (end < 0) + (sint16) ((POSITION->DYN->STATIC >> 16) & 0xffff);
  open *= UCI_STATIC_WEIGHTING;
  open >>= 10;
  end *= UCI_STATIC_WEIGHTING;
  end >>= 10;
  Value += SCORE (open, end);

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

  if (wPatt & bKatt)
    bKhit = HitP;
  else
    bKhit = 0;
  U = (POSITION->OccupiedBW >> 8) & wBitboardP;
  while (U)
    {
      b = BSF (U);
      MobValue -= PawnAntiMobility;
      BitClear (b, U);
    }
  wSafeMob = ~(bPatt | wBitboardOcc);

  U = wBitboardQ;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      AttB = AttB (b);
      AttR = AttR (b);
      if (bBitboardK & DIAG[b])
	{
	  T = AttB (bKs) & AttB;
	  if (T)
	    {
	      Value += wQxrayD[POSITION->sq[BSF (T)]];
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
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      A = AttB | AttR;
      T = A & wSafeMob;
      ZUGZWANG_DETECT_FACT_WHITE;
      POSITION->DYN->wAtt |= A;
      if (A & bKatt)
	bKhit += HitQ;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	Value += QguardK;
      MobValue += MobQ (T);
      if (A & (~bPatt) & bBitboardOcc)
	Value += Qatt;
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattQ;
	  bGoodAtt += 1;
	}
      if (RANK (b) == R7)
	{
	  if ((bBitboardP | bBitboardK) & Ranks78)
	    {
	      Value += Queen7th;
	      if (wBitboardR & RANK7 & AttR && bBitboardK & RANK8)
		Value += DoubQueen7th;
	    }
	}
    }

  U = wBitboardR;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttR (b);
      ZUGZWANG_DETECT_FACT_WHITE;
      POSITION->DYN->wAtt |= A;
      if (bBitboardK & ORTHO[b])
	{
	  T = AttR (bKs) & A;
	  if (T)
	    {
	      Value += wRxray[POSITION->sq[BSF (T)]];
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      if (A & bKatt)
	bKhit += HitR;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	Value += RguardK;
      MobValue += MobR (A & wOKxray);

      if (A & (~bPatt) & bBitboardP)
	Value += RattP;
      if (A & ((bBitboardN | bBitboardB) & ~bPatt))
	Value += RattBN;
      if (A & bBitboardQ)
	{
	  Value += RattQ;
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattR;
	  bGoodAtt += 1;
	}

      if ((wBitboardP & OpenFileW[b]) == 0)
	{
	  Value += RookHalfOpen;
	  if ((bBitboardP & OpenFileW[b]) == 0)
	    {
	      T = bGoodMinor & OpenFileW[b];
	      if (!T)
		Value += RookOpenFile;
	      else
		{
		  int t = BSF (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontB[RANK (t)] & wBitboardP) == 0)
		    Value += RookOpenFixedMinor;
		  else
		    Value += RookOpenMinor;
		}
	    }
	  else
	    {
	      T = OpenFileW[b] & bBitboardP;
	      if (T)
		{
		  int t = BSF (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontW[RANK (t)] & bBitboardP) == 0)
		    Value += RookHalfOpenPawn;
		}
	    }
	  if (bBitboardK & OpenFileW[b])
	    Value += RookHalfOpenKing;
	}

      if (SqSet[b] & wOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontW[RANK (b)] & bBitboardP) == 0)
	{
	  if (wBitboardP & AttPw[b])
	    {
	      Value += OutpostRook;
	      if (A & (bKatt | (bBitboardOcc & ~bPatt)) & RankArray[RANK (b)])
		Value += OutpostRookGuarded;
	    }
	}

      if (RANK (b) == R8)
	{
	  if (bBitboardK & RANK8)
	    Value += RookKing8th;
	}
      if (RANK (b) == R7)
	{
	  if ((bBitboardP | bBitboardK) & Ranks78)
	    {
	      Value += Rook7thKingPawn;
	      if (bBitboardK & RANK8 && (wBitboardQ | wBitboardR) & RANK7 & A)
		Value += DoubRook7thKingPawn;
	    }
	}
      if (RANK (b) == R6 && (bBitboardP | bBitboardK) & Ranks678)
	Value += Rook6thKingPawn;
    }

  wSafeMob |= bBitboardOcc ^ bBitboardP;

  U = wBitboardB;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttB (b);
      ZUGZWANG_DETECT_FACT_WHITE;
      POSITION->DYN->wAtt |= A;
      if (bBitboardK & DIAG[b])
	{
	  T = AttB (bKs) & A;
	  if (T)
	    {
	      Value += wBxray[POSITION->sq[BSF (T)]];
	      POSITION->DYN->wXray |= T;
	      POSITION->XRAYw[BSF (T)] = b;
	    }
	}
      if (A & bKatt)
	bKhit += HitB;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & wKatt)
	Value += BguardK;
      MobValue += MobB (A & wSafeMob, InFrontW[RANK (b)]);

      if (A & (~bPatt) & bBitboardP)
	Value += bAttP;
      if (A & (~bPatt) & bBitboardN)
	Value += bAttN;
      if (A & (bBitboardR | bBitboardQ))
	{
	  Value += bAttRQ;
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattB;
	  bGoodAtt += 1;
	}

      if (SqSet[b] & LIGHT)
	{
	  Value -= (PAWN_INFO->wPlight + PAWN_INFO->bPlight / 2) * SCORE (1, 1);
	  Value +=
	    POPCNT (bBitboardP & LIGHT & InFrontB[RANK (b)] & ~bPatt) *
	    SCORE (0, 2);
	}
      else
	{
	  Value -= (PAWN_INFO->wPdark + PAWN_INFO->bPdark / 2) * SCORE (1, 1);
	  Value += POPCNT (bBitboardP & DARK & InFrontB[RANK (b)] & ~bPatt) *
	           SCORE (0, 2);
	}

      if (SqSet[b] & wOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontW[RANK (b)] & bBitboardP) == 0)
	{
	  if (wBitboardP & AttPw[b])
	    {
	      Value += OutpostBishop;
	      if (A & (bKatt | (bBitboardOcc & ~bPatt)))
		Value += OutpostBishopGuarded;
	    }
	}
      if (POSITION->sq[BishopTrapSq[b]] == bEnumP)
	{
	  Value -= BishopTrapValue;
	  if (POSITION->sq[GoodBishopTrapSq[b]] == bEnumP)
	    Value -= BishopTrapGuardValue;
	}
    }

  U = wBitboardN;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttN[b];
      ZUGZWANG_DETECT_FACT_WHITE;
      POSITION->DYN->wAtt |= A;
      if (A & (bKatt | bBitboardK))
	bKhit += HitN;
      if (A & bBitboardK)
	POSITION->DYN->bKcheck |= SqSet[b];
      if (A & (wKatt | wBitboardK))
	Value += NguardK;
      MobValue += MobN (A & wSafeMob, InFrontW[RANK (b)]);
      if (RANK (b) <= R4 && POSITION->sq[b + 24] == bEnumP)
	Value -= SCORE ( R5 - RANK (b), R5 - RANK (b));
      if (A & (~bPatt) & bBitboardP)
	Value += NattP;
      if (A & (~bPatt) & bBitboardB)
	Value += NattB;
      if (A & (bBitboardR | bBitboardQ))
	{
	  Value += NattRQ;
	  wGoodAtt += 1;
	}
      if (bBitboardP & AttPb[b])
	{
	  Value -= PattN;
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
		  if (RANK (b) == R5)
		    Value += OutpostKnight5th;
		  if (FILE (b) == FD || FILE (b) == FE)
		    Value += OutpostKnightONde;
		}
	    }
	}
    }

  if (bPatt & wKatt)
    wKhit = HitP;
  else
    wKhit = 0;
  U = (POSITION->OccupiedBW << 8) & bBitboardP;
  POSITION->DYN->bXray = 0;
  while (U)
    {
      b = BSF (U);
      MobValue += PawnAntiMobility;
      BitClear (b, U);
    }
  bSafeMob = ~(wPatt | bBitboardOcc);

  U = bBitboardQ;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      AttB = AttB (b);
      AttR = AttR (b);
      if (wBitboardK & DIAG[b])
	{
	  T = AttB (wKs) & AttB;
	  if (T)
	    {
	      Value -= bQxrayD[POSITION->sq[BSF (T)]];
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
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      A = AttB | AttR;
      ZUGZWANG_DETECT_FACT_BLACK;
      T = A & bSafeMob;
      POSITION->DYN->bAtt |= A;
      if (A & wKatt)
	wKhit += HitQ;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	Value -= QguardK;
      MobValue -= MobQ (T);

      if (A & (~wPatt) & wBitboardOcc)
	Value -= Qatt;
      if (wBitboardP & AttPw[b])
	{
	  Value += PattQ;
	  wGoodAtt += 1;
	}

      if (RANK (b) == R2)
	{
	  if ((wBitboardP | wBitboardK) & Ranks12)
	    {
	      Value -= Queen7th;
	      if (bBitboardR & RANK2 & AttR && wBitboardK & RANK1)
		Value -= DoubQueen7th;
	    }
	}
    }

  U = bBitboardR;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttR (b);
      ZUGZWANG_DETECT_FACT_BLACK;
      POSITION->DYN->bAtt |= A;
      if (wBitboardK & ORTHO[b])
	{
	  T = A & AttR (wKs);
	  if (T)
	    {
	      Value -= bRxray[POSITION->sq[BSF (T)]];
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      if (A & wKatt)
	wKhit += HitR;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	Value -= RguardK;
      MobValue -= MobR (A & bOKxray);

      if (A & (~wPatt) & wBitboardP)
	Value -= RattP;
      if (A & (wBitboardN | wBitboardB) & ~wPatt)
	Value -= RattBN;
      if (A & wBitboardQ)
	{
	  Value -= RattQ;
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattR;
	  wGoodAtt += 1;
	}

      if ((bBitboardP & OpenFileB[b]) == 0)
	{
	  Value -= RookHalfOpen;
	  if ((wBitboardP & OpenFileB[b]) == 0)
	    {
	      T = wGoodMinor & OpenFileB[b];
	      if (!T)
		Value -= RookOpenFile;
	      else
		{
		  int t = BSR (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontW[RANK (t)] & bBitboardP) == 0)
		    Value -= RookOpenFixedMinor;
		  else
		    Value -= RookOpenMinor;
		}
	    }
	  else
	    {
	      T = OpenFileB[b] & wBitboardP;
	      if (T)
		{
		  int t = BSR (T);
		  if ((IsolatedFiles[FILE (t)] &
		       InFrontB[RANK (t)] & wBitboardP) == 0)
		    Value -= RookHalfOpenPawn;
		}
	    }
	  if (wBitboardK & OpenFileB[b])
	    Value -= RookHalfOpenKing;
	}

      if (SqSet[b] & bOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontB[RANK (b)] & wBitboardP) == 0)
	{
	  if (bBitboardP & AttPb[b])
	    {
	      Value -= OutpostRook;
	      if (A & (wKatt | (wBitboardOcc & ~wPatt)) & RankArray[RANK (b)])
		Value -= OutpostRookGuarded;
	    }
	}

      if (RANK (b) == R1)
	{
	  if (wBitboardK & RANK1)
	    Value -= RookKing8th;
	}
      if (RANK (b) == R2)
	{
	  if ((wBitboardP | wBitboardK) & Ranks12)
	    {
	      Value -= Rook7thKingPawn;
	      if (wBitboardK & RANK1 && (bBitboardQ | bBitboardR) & RANK2 & A)
		Value -= DoubRook7thKingPawn;
	    }
	}
      if (RANK (b) == R3 && (wBitboardP | wBitboardK) & Ranks123)
	Value -= Rook6thKingPawn;
    }

  bSafeMob |= wBitboardOcc ^ wBitboardP;

  U = bBitboardB;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttB (b);
      ZUGZWANG_DETECT_FACT_BLACK;
      POSITION->DYN->bAtt |= A;
      if (wBitboardK & DIAG[b])
	{
	  T = A & AttB (wKs);
	  if (T)
	    {
	      Value -= bBxray[POSITION->sq[BSF (T)]];
	      POSITION->DYN->bXray |= T;
	      POSITION->XRAYb[BSF (T)] = b;
	    }
	}
      if (A & wKatt)
	wKhit += HitB;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & bKatt)
	Value -= BguardK;
      MobValue -= MobB (A & bSafeMob, InFrontB[RANK (b)]);

      if (A & (~wPatt) & wBitboardP)
	Value -= bAttP;
      if (A & (~wPatt) & wBitboardN)
	Value -= bAttN;
      if (A & (wBitboardR | wBitboardQ))
	{
	  Value -= bAttRQ;
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattB;
	  wGoodAtt += 1;
	}

      if (SqSet[b] & LIGHT)
	{
	  Value += (PAWN_INFO->bPlight + PAWN_INFO->wPlight / 2) * SCORE (1, 1);
	  Value -= POPCNT (wBitboardP & LIGHT & InFrontW[RANK (b)] & ~wPatt) *
	           SCORE (0, 2);
	}
      else
	{
	  Value += (PAWN_INFO->bPdark + PAWN_INFO->wPdark / 2) * SCORE (1, 1);
	  Value -= POPCNT (wBitboardP & DARK & InFrontW[RANK (b)] & ~wPatt) *
	           SCORE (0, 2);
	}

      if (SqSet[b] & bOutpost
	  && (IsolatedFiles[FILE (b)] & InFrontB[RANK (b)] & wBitboardP) == 0)
	{
	  if (bBitboardP & AttPb[b])
	    {
	      Value -= OutpostBishop;
	      if (A & (wKatt | (wBitboardOcc & ~wPatt)))
		Value -= OutpostBishopGuarded;
	    }
	}

      if (POSITION->sq[BishopTrapSq[b]] == wEnumP)
	{
	  Value += BishopTrapValue;
	  if (POSITION->sq[GoodBishopTrapSq[b]] == wEnumP)
	    {
	      Value += BishopTrapGuardValue;
	    }
	}
    }

  U = bBitboardN;
  while (U)
    {
      b = BSF (U);
      BitClear (b, U);
      A = AttN[b];
      ZUGZWANG_DETECT_FACT_BLACK;
      POSITION->DYN->bAtt |= A;
      if (A & (wKatt | wBitboardK))
	wKhit += HitN;
      if (A & wBitboardK)
	POSITION->DYN->wKcheck |= SqSet[b];
      if (A & (bKatt | bBitboardK))
	Value -= NguardK;
      MobValue -= MobN (A & bSafeMob, InFrontB[RANK (b)]);
      if (RANK (b) >= R5 && POSITION->sq[b - 24] == wEnumP)
	Value += SCORE ( RANK (b) - R4, RANK (b) - R4);
      if (A & (~wPatt) & wBitboardP)
	Value -= NattP;
      if (A & (~wPatt) & wBitboardB)
	Value -= NattB;
      if (A & (wBitboardR | wBitboardQ))
	{
	  Value -= NattRQ;
	  bGoodAtt += 1;
	}
      if (wBitboardP & AttPw[b])
	{
	  Value += PattN;
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
		  if (RANK (b) == R4)
		    Value -= OutpostKnight5th;
		  if (FILE (b) == FD || FILE (b) == FE)
		    Value -= OutpostKnightONde;
		}
	    }
	}
    }

#if 1
  end = (sint16) (MobValue & 0xffff);
  open = (end < 0) + (sint16) ((MobValue >> 16) & 0xffff);
  open *= UCI_MOBILITY_WEIGHTING;
  open >>= 10;
  end *= UCI_MOBILITY_WEIGHTING;
  end >>= 10;
  MobValue = SCORE (open, end);  
#endif
  Value += MobValue;
  POSITION->DYN->wAtt |= wKatt;
  POSITION->DYN->bAtt |= bKatt;
  if (bKatt & wBitboardK)
    {
      POSITION->DYN->wKcheck |= SqSet[POSITION->bKsq];
      POSITION->DYN->bKcheck |= SqSet[POSITION->wKsq];
    }

  if ((~POSITION->DYN->bAtt) & wKatt & bBitboardP)
    Value += KingAttUnguardedPawn;

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
	}
    }

  phase = MIN (POSITION->DYN->material & 0xff, 32);
  if (wKatt & bKatt)
    wKhit += HitK;
  ch = (((KingSafetyMult[wKhit >> 16] * (wKhit & 0xffff)) / KingSafetyDivider)
	<< 16) + PAWN_INFO->wKdanger;
#if 1
  ch >>= 16;
  ch *= UCI_KING_SAFETY_WEIGHTING;
  ch >>= 10;  
  ch <<= 16;
#endif
  if (!bBitboardQ)
    {
      ch >>= 16;
      ch *= POPCNT (bBitboardR | bBitboardN | bBitboardB);
      ch >>= 3;
      ch <<= 16;
    }
  POSITION->DYN->wKdanger = (uint8) (((uint32) (ch * phase)) >> 23); /* /4 */
  Value -= ch;

  if ((~POSITION->DYN->wAtt) & bKatt & wBitboardP)
    {
      Value -= KingAttUnguardedPawn;
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
	}
    }

  if (wKatt & bKatt)
    bKhit += HitK;
  ch = (((KingSafetyMult[bKhit >> 16] * (bKhit & 0xffff)) / KingSafetyDivider)
	<< 16) + PAWN_INFO->bKdanger;
#if 1
  ch >>= 16;
  ch *= UCI_KING_SAFETY_WEIGHTING;
  ch >>= 10;
  ch <<= 16;
#endif
  if (!wBitboardQ)
    {
      ch >>= 16;
      ch *= POPCNT (wBitboardR | wBitboardN | wBitboardB);
      ch >>= 3;
      ch <<= 16;
    }
  POSITION->DYN->bKdanger = (uint8) (((uint32) (ch * phase)) >> 23); /* /4 */
  Value += ch;

  if (wGoodAtt >= 2)
    Value += MultipleAtt;
  if (bGoodAtt >= 2)
    Value -= MultipleAtt;

#define QueenEnd ( (POSITION->DYN->flags & 28) == 4)
#define RookEnd ( (POSITION->DYN->flags & 28) == 8)

  if ((wBitboardR | wBitboardQ) & CrampFile[FILE (bKs)])
    {
      Value += SCORE (0, 5);
      if ((CrampFile[FILE (bKs)] & (wBitboardP | bBitboardP)) == 0)
	Value += SCORE (5, 15);
    }
  if ((bBitboardR | bBitboardQ) & CrampFile[FILE (wKs)])
    {
      Value -= SCORE (0, 5);
      if ((CrampFile[FILE (wKs)] & (bBitboardP | wBitboardP)) == 0)
	Value -= SCORE (5, 15);
    }

  U = PAWN_INFO->wPassedFiles;
  while (U)
    {
      b = BSR (FileArray[BSF (U)] & wBitboardP);
      BitClear (0, U);
      tr = RANK (b);
      if (tr <= R3)
	continue;
      if (RookEnd)
	{
	  if (wBitboardR & OpenFileW[b])
	    {
	      if (tr == R7)
		Value -= Rook7thEnd;
	      else if (tr == R6)
		Value -= Rook6thEnd;
	    }
	  if (OpenFileW[b] & wBitboardK && CrampFile[FILE (wKs)] & bBitboardR)
	    Value -= SCORE (0, 1 << (tr - R2));
	}
      if (POSITION->sq[b + 8] == 0)
	Value += PassedPawnCanMove[tr];
      if ((OpenFileW[b] & wBitboardOcc) == 0)
	Value += PassedPawnMeClear[tr];
      if ((OpenFileW[b] & bBitboardOcc) == 0)
	Value += PassedPawnOppClear[tr];
      if ((OpenFileW[b] & (~POSITION->DYN->wAtt) & POSITION->DYN->bAtt) == 0)
	Value += PassedPawnIsFree[tr];
      if (QueenEnd)
	{
	  if (tr == R7 && wBitboardQ & OpenFileW[b])
	    Value -= Queen7thEnd;
	  Value += RankQueenEnd[tr];
	}
    }

  U = PAWN_INFO->bPassedFiles;
  while (U)
    {
      b = BSF (FileArray[BSF (U)] & bBitboardP);
      BitClear (0, U);
      tr = RANK (b);
      if (tr >= R6)
	continue;
      if (RookEnd)
	{
	  if (bBitboardR & OpenFileB[b])
	    {
	      if (tr == R2)
		Value += Rook7thEnd;
	      else if (tr == R3)
		Value += Rook6thEnd;
	    }
	  if (OpenFileB[b] & bBitboardK && CrampFile[FILE (bKs)] & wBitboardR)
	    Value += SCORE (0, 1 << (R7 - tr));
	}
      if (POSITION->sq[b - 8] == 0)
	Value -= PassedPawnCanMove[7 - tr];
      if ((OpenFileB[b] & bBitboardOcc) == 0)
	Value -= PassedPawnMeClear[7 - tr];
      if ((OpenFileB[b] & wBitboardOcc) == 0)
	Value -= PassedPawnOppClear[7 - tr];
      if ((OpenFileB[b] & POSITION->DYN->wAtt & ~POSITION->DYN->bAtt) == 0)
	Value -= PassedPawnIsFree[7 - tr];
      if (QueenEnd)
	{
	  if (tr == R2 && bBitboardQ & OpenFileB[b])
	    Value += Queen7thEnd;
	  Value -= RankQueenEnd[7 - tr];
	}
    }

  end = (sint16) (Value & 0xffff);
  open = (end < 0) + (sint16) ((Value >> 16) & 0xffff);
  antiphase = 32 - phase;
  Value = end * antiphase + open * phase;
  Value = Value / 32 + matval;
  Value = (Value * TOKEN) / 128;

  if (Value > 0)
    Value -= (PAWN_INFO->wDrawWeight * MIN (Value, 100)) / 64;
  else
    Value += (PAWN_INFO->bDrawWeight * MIN (-Value, 100)) / 64;

#define BISHOP_KNIGHT_MATE (POSITION->DYN->flags & 128)
#define MAX_DIST(i,j) ( MAX ( FileDistance (i, j), RankDistance (i, j) ) )
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
    }

  if (POSITION->DYN->reversible > 50)
    {
      Value *= (114 - POSITION->DYN->reversible);
      Value /= 64;
    }

#define WHITE_MINOR_ONLY (POSITION->DYN->flags & 32)
#define BLACK_MINOR_ONLY (POSITION->DYN->flags & 64)
#define WHITE_HAS_PIECE (wBitboardOcc ^ (wBitboardK | wBitboardP))
#define BLACK_HAS_PIECE (bBitboardOcc ^ (bBitboardK | bBitboardP))
#define BitBoard3(x, y, z) ((1ULL << (x)) | (1ULL << (y)) | (1ULL << (z)))
#define BitBoard4(w, x, y, z) ((1ULL << (w)) | (1ULL << (x)) | (1ULL << (y)) | (1ULL << (z)))
#define A7A8B8 BitBoard3 (A7, A8, B8)
#define F8G8H8 BitBoard3 (F8, G8, H8)
#define E8F8G8H8 BitBoard4 (E8, F8, G8, H8)
#define H7G8H8 BitBoard3 (H7, G8, H8)
#define A8B8C8D8 BitBoard4 (A8, B8, C8, D8)
#define A8B8C8 BitBoard3 (A8, B8, C8)
#define A1B1A2 BitBoard3 (A1, B1, A1)
#define F1G1H1 BitBoard3 (F1, G1, H1)
#define E1F1G1H1 BitBoard4 (E1, F1, G1, H1)
#define G1H1H2 BitBoard3 (G1, H1, H2)
#define A1B1C1D1 BitBoard4 (A1, B1, C1, D1)
#define A1B1C1 BitBoard3 (A1, B1, C1)

  if (Value > 0)
    {
      if (!POSITION->wtm && !BLACK_HAS_PIECE
	  && (bBitboardK ^ bBitboardP) == bBitboardOcc
	  && !((bBitboardP >> 8) & ~POSITION->OccupiedBW)
	  && !(bPatt & wBitboardOcc) && !POSITION->DYN->ep
	  && !(AttK[POSITION->bKsq] & ~POSITION->DYN->wAtt)
	  && !POSITION->DYN->bKcheck)
	Value = 0; /* patt */
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
	  if (wBitboardBL)
	    {
	      if (POSITION->sq[B6] == wEnumP && POSITION->sq[B7] == bEnumP
		  && !(wBitboardP & ~SqSet[B6]) && !(bBitboardP & ~FILEb)
		  && !((bBitboardP >> 8) & ~POSITION->OccupiedBW)
		  && !(wBitboardBL & (wBitboardBL - 1)) /* one bit */
		  && (bBitboardK & B8C8D8) && !(wBitboardK & A7A8B8))
		Value = 0;
	      if (POSITION->sq[G6] == wEnumP && POSITION->sq[G7] == bEnumP
		  && !((wBitboardP | bBitboardP) & ~FILEg)
		  && (bBitboardK & E8F8G8H8) && !(wBitboardK & H7G8H8))
		Value = 0;
	    }
	  if (wBitboardBD)
	    {
	      if (POSITION->sq[B6] == wEnumP && POSITION->sq[B7] == bEnumP
		  && !((wBitboardP | bBitboardP) & ~FILEb)
		  && (bBitboardK & A8B8C8D8) && !(wBitboardK & A7A8B8))
		Value = 0;
	      if (POSITION->sq[G6] == wEnumP && POSITION->sq[G7] == bEnumP
		  && !(wBitboardP & ~SqSet[G6]) && !(bBitboardP & ~FILEg)
		  && !((bBitboardP >> 8) & ~POSITION->OccupiedBW)
		  && !(bBitboardBL & (bBitboardBL - 1)) /* one bit */
		  && (bBitboardK & F8G8H8) && !(wBitboardK & H7G8H8))
		Value = 0;
	    }
	  if (!wBitboardP) /* valid ? */
	    Value = 0;
	}
    }

  else
    {
      if (POSITION->wtm && !WHITE_HAS_PIECE
	  && (wBitboardK ^ wBitboardP) == wBitboardOcc
	  && !((wBitboardP << 8) & ~POSITION->OccupiedBW)
	  && !(wPatt & bBitboardOcc) && !POSITION->DYN->ep
	  && !(AttK[POSITION->wKsq] & ~POSITION->DYN->bAtt)
	  && !POSITION->DYN->wKcheck)
	Value = 0; /* patt */
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
	  if (bBitboardBL)
	    {
	      if (POSITION->sq[B3] == bEnumP && POSITION->sq[B2] == wEnumP
		  && !((bBitboardP | wBitboardP) & ~FILEb)
		  && (wBitboardK & A1B1C1D1) && !(bBitboardK & A1B1A2))
		Value = 0;
	      if (POSITION->sq[G3] == bEnumP && POSITION->sq[G2] == wEnumP
		  && !(bBitboardP & ~SqSet[G3]) && !(wBitboardP & ~FILEg)
		  && !((wBitboardP << 8) & ~POSITION->OccupiedBW)
		  && !(bBitboardBL & (bBitboardBL - 1)) /* one bit */
		  && (wBitboardK & F1G1H1) && !(bBitboardK & G1H1H2))
		Value = 0;
	    }
	  if (bBitboardBD)
	    {
	      if (POSITION->sq[B3] == bEnumP && POSITION->sq[B2] == wEnumP
		  && !(bBitboardP & ~SqSet[B3]) && !(wBitboardP & ~FILEb)
		  && !((wBitboardP << 8) & ~POSITION->OccupiedBW)
		  && !(bBitboardBD & (bBitboardBD - 1)) /* one bit */
		  && (wBitboardK & A1B1C1) && !(bBitboardK & A1B1A2))
		Value = 0;
	      if (POSITION->sq[G3] == bEnumP && POSITION->sq[G2] == wEnumP
		  && !((bBitboardP | wBitboardP) & ~FILEg)
		  && (wBitboardK & E1F1G1H1) && !(bBitboardK & G1H1H2))
		Value = 0;
	    }
	  if (!bBitboardP) /* valid ? */
	    Value = 0;
	}
    }

  if (Value > 0 && PAWN_INFO->wPfile_count <= 1)
    Value -= Value / 4;
  if (Value > 0 && !wBitboardP && Value > matval)
    Value -= (Value - matval) / 2;
  if (Value < 0 && PAWN_INFO->bPfile_count <= 1)
    Value -= Value / 4;
  if (Value < 0 && !bBitboardP && Value < matval)
    Value -= (Value - matval) / 2;

#ifndef MINIMAL
  if (RANDOM_COUNT) /* omit Random32 */
    {
      uint32 r;
      int n, adj = 0;
      int mask = (1 << RANDOM_BITS) - 1;
      r = Random32 (POSITION->cpu);
      for (n = 0; n < RANDOM_COUNT; n++)
	{
	  adj = r & mask;
	  r >>= RANDOM_BITS;
	  Value += (r & 1) ? adj : -adj;
	  r >>= 1;
	}
    }
#endif

  ZUGZWANG_DETECT_COMPLETE;
  if (Value == 0)
    Value = 1; /* HACK */
  POSITION->DYN->Value = POSITION->wtm ? Value : -Value;
  POSITION->DYN->PositionalValue = Value - matval;
  POSITION->DYN->lazy = 0;
  if (STORE_EVAL_HASH)
    GET_EVAL_HASH (POSITION->DYN->HASH) =
      (POSITION->DYN->HASH & 0xffffffffffff0000) | (POSITION->DYN->Value & 0xffff);
  if (move && !(POSITION->DYN - 1)->lazy)
    AdjustPositionalGain (POSITION, move);
}

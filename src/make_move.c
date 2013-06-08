#include "RobboLito.h"
#include "make_unmake.h"
#include "material_value.h"

#define REV_CASTLE(POS) (POS)->DYN->reversible = 0

typedef enum { ooK = 1, ooQ = 2, ook = 4, ooq = 8 } CastlingTable;
typedef enum
{ KQkq = ooK | ooQ | ook | ooq,
  Qkq = ooQ | ook | ooq, Kkq = ooK | ook | ooq, kq = ook | ooq,
  KQk = ooK | ooQ | ook, KQ = ooK | ooQ, KQq = ooK | ooQ | ooq
} KQkqTable;

static uint8 CastleTable[64] =
  {
    Kkq, KQkq, KQkq, KQkq, kq, KQkq, KQkq, Qkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq, KQkq,
    KQk, KQkq, KQkq, KQkq, KQ, KQkq, KQkq, KQq
  };

void ReFuel960Castle ()
{
  int sq;
  for (sq = A1; sq <= H8; sq++)
    CastleTable[sq] = KQkq;
  if (!UCI_OPTION_CHESS_960)
    {
      CastleTable[A1] = Kkq;
      CastleTable[E1] = kq;
      CastleTable[H1] = Qkq;
      CastleTable[A8] = KQk;
      CastleTable[E8] = KQ;
      CastleTable[H8] = KQq;
    }
  else
    {
      if (CHESS_960_KING_FILE != 0xff)
	{
	  CastleTable[CHESS_960_KING_FILE] = kq;
	  CastleTable[CHESS_960_KING_FILE + 070] = KQ;
	}
      if (CHESS_960_QR_FILE != 0xff)
	{
	  CastleTable[CHESS_960_QR_FILE] = Kkq;
	  CastleTable[CHESS_960_QR_FILE + 070] = KQk;
	}
      if (CHESS_960_KR_FILE != 0xff)
	{
	  CastleTable[CHESS_960_KR_FILE] = Qkq;
	  CastleTable[CHESS_960_KR_FILE + 070] = KQq;
	}
    }
}

const static uint64 WhiteEP[8] =
  {
    Bitboard2 (B4, B4), Bitboard2 (A4, C4),
    Bitboard2 (B4, D4), Bitboard2 (C4, E4),
    Bitboard2 (D4, F4), Bitboard2 (E4, G4),
    Bitboard2 (F4, H4), Bitboard2 (G4, G4)
  };
const static uint64 BlackEP[8] =
  {
    Bitboard2 (B5, B5), Bitboard2 (A5, C5),
    Bitboard2 (B5, D5), Bitboard2 (C5, E5),
    Bitboard2 (D5, F5), Bitboard2 (E5, G5),
    Bitboard2 (F5, H5), Bitboard2 (G5, G5)
  };

static INLINE void MakeWhiteOO (typePOS* POSITION, int to)
{
  if (to == G1)
    {
      wBitboardOcc ^= F1H1;
      wBitboardR ^= F1H1;
      POSITION->OccupiedBW ^= F1H1;
#ifndef MAGIC_BITBOARDS
      POSITION->OccupiedL90 ^= F1H1Left90;
      POSITION->OccupiedL45 ^= F1H1Left45;
      POSITION->OccupiedR45 ^= F1H1Right45;
#endif
      POSITION->DYN->STATIC += PST (wEnumR, F1) - PST (wEnumR, H1);
      POSITION->DYN->HASH ^= Zobrist (wEnumR, F1) ^ Zobrist (wEnumR, H1);
      POSITION->sq[H1] = 0;
      POSITION->sq[F1] = wEnumR;
    }
  else if (to == C1)
    {
      wBitboardOcc ^= A1D1;
      wBitboardR ^= A1D1;
      POSITION->OccupiedBW ^= A1D1;
#ifndef MAGIC_BITBOARDS
      POSITION->OccupiedL90 ^= A1D1Left90;
      POSITION->OccupiedL45 ^= A1D1Left45;
      POSITION->OccupiedR45 ^= A1D1Right45;
#endif
      POSITION->DYN->STATIC += PST (wEnumR, D1) - PST (wEnumR, A1);
      POSITION->DYN->HASH ^= Zobrist (wEnumR, A1) ^ Zobrist (wEnumR, D1);
      POSITION->sq[A1] = 0;
      POSITION->sq[D1] = wEnumR;
    }
}

static INLINE void MakeBlackOO (typePOS* POSITION, int to)
{
  if (to == G8)
    {
      bBitboardOcc ^= F8H8;
      bBitboardR ^= F8H8;
      POSITION->OccupiedBW ^= F8H8;
#ifndef MAGIC_BITBOARDS
      POSITION->OccupiedL90 ^= F8H8Left90;
      POSITION->OccupiedL45 ^= F8H8Left45;
      POSITION->OccupiedR45 ^= F8H8Right45;
#endif
      POSITION->DYN->STATIC += PST (bEnumR, F8) - PST (bEnumR, H8);
      POSITION->DYN->HASH ^= Zobrist (bEnumR, F8) ^ Zobrist (bEnumR, H8);
      POSITION->sq[H8] = 0;
      POSITION->sq[F8] = bEnumR;
    }
  else if (to == C8)
    {
      bBitboardOcc ^= A8D8;
      bBitboardR ^= A8D8;
      POSITION->OccupiedBW ^= A8D8;
#ifndef MAGIC_BITBOARDS
      POSITION->OccupiedL90 ^= A8D8Left90;
      POSITION->OccupiedL45 ^= A8D8Left45;
      POSITION->OccupiedR45 ^= A8D8Right45;
#endif
      POSITION->DYN->STATIC += PST (bEnumR, D8) - PST (bEnumR, A8);
      POSITION->DYN->HASH ^= Zobrist (bEnumR, A8) ^ Zobrist (bEnumR, D8);
      POSITION->sq[A8] = 0;
      POSITION->sq[D8] = bEnumR;
    }
}

#ifdef CHESS_960
#ifndef MAGIC_BITBOARDS
#error
#endif
static INLINE void Castle960White (typePOS* POSITION, int move, int to, int fr)
{
  int fl;
  POSITION->DYN++;
  REV_HASH (POSITION);
  POSITION->DYN->reversible++;
  REV_CASTLE (POSITION);
  POSITION->DYN->move = move;
  fl = POSITION->DYN->oo & 0xc;
  POSITION->DYN->HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->HASH ^= ZobristWTM;
  POSITION->DYN->PAWN_HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->oo = fl;
  if (POSITION->DYN->ep)
    {
      POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
      POSITION->DYN->ep = 0;
    }
  POSITION->sq[to] = 0;
  POSITION->sq[fr] = 0;
  wBitboardK ^= SqSet[fr];
  wBitboardR ^= SqSet[to];
  wBitboardOcc ^= SqSet[to] | SqSet[fr];
  POSITION->wtm ^= 1;
  POSITION->height++;
  POSITION->DYN->HASH ^= Zobrist (wEnumK, fr) ^ Zobrist (wEnumR, to);
  POSITION->DYN->PAWN_HASH ^= Zobrist (wEnumK, fr);
  POSITION->DYN->STATIC -= PST (wEnumK, fr) + PST (wEnumR, to);
  if (to > fr)
    {
      POSITION->sq[F1] = wEnumR;
      POSITION->sq[G1] = wEnumK;
      wBitboardOcc |= SqSet[F1] | SqSet[G1];
      wBitboardK |= SqSet[G1];
      wBitboardR |= SqSet[F1];
      POSITION->DYN->HASH ^= Zobrist (wEnumK, G1) ^ Zobrist (wEnumR, F1);
      POSITION->DYN->PAWN_HASH ^= Zobrist (wEnumK, G1);
      POSITION->wKsq = G1;
      POSITION->DYN->STATIC += PST (wEnumK, G1) + PST (wEnumR, F1);
    }
  if (to < fr)
    {
      POSITION->sq[D1] = wEnumR;
      POSITION->sq[C1] = wEnumK;
      wBitboardOcc |= SqSet[D1] | SqSet[C1];
      wBitboardK |= SqSet[C1];
      wBitboardR |= SqSet[D1];
      POSITION->DYN->HASH ^= Zobrist (wEnumK, C1) ^ Zobrist (wEnumR, D1);
      POSITION->DYN->PAWN_HASH ^= Zobrist (wEnumK, C1);
      POSITION->wKsq = C1;
      POSITION->DYN->STATIC += PST (wEnumK, C1) + PST (wEnumR, D1);
    }
  POSITION->OccupiedBW = wBitboardOcc | bBitboardOcc; /* HACK */
  POSITION->STACK[++(POSITION->StackHeight)] = POSITION->DYN->HASH;
  REV_HASH (POSITION);
  VALIDATE (POSITION, 0, move);
}

static INLINE void Castle960Black (typePOS* POSITION, int move, int to, int fr)
{
  int fl;
  POSITION->DYN++;
  REV_HASH (POSITION);
  POSITION->DYN->reversible++;
  REV_CASTLE (POSITION);
  POSITION->DYN->move = move;
  fl = POSITION->DYN->oo & 0x3;
  POSITION->DYN->HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->HASH ^= ZobristWTM;
  POSITION->DYN->PAWN_HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->oo = fl;
  if (POSITION->DYN->ep)
    {
      POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
      POSITION->DYN->ep = 0;
    }
  POSITION->sq[to] = 0;
  POSITION->sq[fr] = 0;
  bBitboardK ^= SqSet[fr];
  bBitboardR ^= SqSet[to];
  bBitboardOcc ^= SqSet[to] | SqSet[fr];
  POSITION->wtm ^= 1;
  POSITION->height++;
  POSITION->DYN->HASH ^= Zobrist (bEnumK, fr) ^ Zobrist (bEnumR, to);
  POSITION->DYN->PAWN_HASH ^= Zobrist (bEnumK, fr);
  POSITION->DYN->STATIC -= PST (bEnumK, fr) + PST (bEnumR, to);
  if (to > fr)
    {
      POSITION->sq[F8] = bEnumR;
      POSITION->sq[G8] = bEnumK;
      bBitboardOcc |= SqSet[F8] | SqSet[G8];
      bBitboardK |= SqSet[G8];
      bBitboardR |= SqSet[F8];
      POSITION->DYN->HASH ^= Zobrist (bEnumK, G8) ^ Zobrist (bEnumR, F8);
      POSITION->DYN->PAWN_HASH ^= Zobrist (bEnumK, G8);
      POSITION->bKsq = G8;
      POSITION->DYN->STATIC += PST (bEnumK, G8) + PST (bEnumR, F8);
    }
  if (to < fr)
    {
      POSITION->sq[D8] = bEnumR;
      POSITION->sq[C8] = bEnumK;
      bBitboardOcc |= SqSet[D8] | SqSet[C8];
      bBitboardK |= SqSet[C8];
      bBitboardR |= SqSet[D8];
      POSITION->DYN->HASH ^= Zobrist (bEnumK, C8) ^ Zobrist (bEnumR, D8);
      POSITION->DYN->PAWN_HASH ^= Zobrist (bEnumK, C8);
      POSITION->bKsq = C8;
      POSITION->DYN->STATIC += PST (bEnumK, C8) + PST (bEnumR, D8);
    }
  POSITION->OccupiedBW = wBitboardOcc | bBitboardOcc; /* HACK */
  POSITION->STACK[++(POSITION->StackHeight)] = POSITION->DYN->HASH;
  REV_HASH (POSITION);
  VALIDATE (POSITION, 0, move);
}
#endif

const static uint8 PromW[8] =
  { 0, 0, 0, 0,  wEnumN, wEnumBL, wEnumR, wEnumQ };

void MakeWhite (typePOS* POSITION, uint32 move)
{
  int fr, to, pi, fl, cp, z;
  uint64 mask;
  TRACE (TRACE_MAKE_UNMAKE,
	 printf ("Mw %d %s\n", HEIGHT (POSITION), Notate(move, STRING1[POSITION->cpu])));
  POSITION->nodes++;
  if (!TITANIC_MODE) /* new */
    {
      NODE_CHECK++;
      if ((NODE_CHECK & 4095) == 0)
	CheckDone (POSITION, 0);
    }
  memcpy (POSITION->DYN + 1, POSITION->DYN, 32);
  fr = FROM (move);
  to = TO (move);
#ifdef CHESS_960
  if (UCI_OPTION_CHESS_960 && MoveIsOO (move))
    {
      Castle960White (POSITION, move, to, fr);
      return;
    }
#endif
  pi = POSITION->sq[fr];
  POSITION->DYN++;
  REV_HASH (POSITION);
  POSITION->DYN->reversible++;
  POSITION->DYN->move = move;
  fl = CastleTable[fr] & CastleTable[to] & POSITION->DYN->oo;
  POSITION->DYN->HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->PAWN_HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->oo = fl;
  if (POSITION->DYN->ep)
    {
      POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
      POSITION->DYN->ep = 0;
    }
  POSITION->sq[fr] = 0;
  mask = SqClear[fr];
  wBitboardOcc &= mask;
  POSITION->bitboard[pi] &= mask;
  ClearOccupied (mask, fr);
  POSITION->DYN->STATIC += PST (pi, to) - PST (pi, fr);
  mask = Zobrist (pi, fr) ^ Zobrist (pi, to);
  cp = POSITION->sq[to];
  POSITION->DYN->cp = cp;
  POSITION->DYN->HASH ^= mask;
  if (pi == wEnumP)
    POSITION->DYN->PAWN_HASH ^= mask;
  POSITION->wtm ^= 1;
  POSITION->height++;
  POSITION->DYN->HASH ^= ZobristWTM;
  if (pi == wEnumK)
    {
      POSITION->DYN->PAWN_HASH ^= mask;
      POSITION->wKsq = to;
    }
  if (cp)
    {
      mask = SqClear[to];
      bBitboardOcc &= mask;
      POSITION->bitboard[cp] &= mask;
      POSITION->DYN->material -= MATERIAL_VALUE[cp];
      POSITION->DYN->STATIC -= PST (cp, to);
      if (cp == bEnumP)
	POSITION->DYN->PAWN_HASH ^= Zobrist (cp, to);
      POSITION->DYN->HASH ^= Zobrist (cp, to);
      POSITION->DYN->reversible = 0;
    }
  else
    {
      mask = SqSet[to];
      SetOccupied (mask, to);
      if (MoveIsOO (move))
	{
	  REV_CASTLE (POSITION);
	  MakeWhiteOO (POSITION, to);
	}
    }
  POSITION->sq[to] = pi;
  wBitboardOcc |= SqSet[to];
  POSITION->bitboard[pi] |= SqSet[to];
  if (pi == wEnumP)
    {
      POSITION->DYN->reversible = 0;
      if (MoveIsEP (move))
	{
	  z = to ^ 8;
	  mask = SqClear[z];
	  bBitboardOcc &= mask;
	  bBitboardP &= mask;
	  ClearOccupied (mask, z);
	  POSITION->DYN->material -= MATERIAL_VALUE[bEnumP];
	  POSITION->DYN->STATIC -= PST (bEnumP, z);
	  POSITION->DYN->HASH ^= Zobrist (bEnumP, z);
	  POSITION->DYN->PAWN_HASH ^= Zobrist (bEnumP, z);
	  POSITION->sq[z] = 0;
	}
      else if (MoveIsProm (move))
	{
	  pi = PromW[(move & FLAG_MASK) >> 12];
	  if (pi == wEnumBL && SqSet[to] & DARK)
	    pi = wEnumBD;
	  POSITION->sq[to] = pi;
	  if (POSITION->bitboard[pi])
	    POSITION->DYN->material |= 0x80000000;
	  wBitboardP &= SqClear[to];
	  POSITION->bitboard[pi] |= SqSet[to];
	  POSITION->DYN->material += MATERIAL_VALUE[pi] - MATERIAL_VALUE[wEnumP];
	  POSITION->DYN->STATIC += PST (pi, to) - PST (wEnumP, to);
	  POSITION->DYN->HASH ^= Zobrist (pi, to) ^ Zobrist (wEnumP, to);
	  POSITION->DYN->PAWN_HASH ^= Zobrist (wEnumP, to);
	}
      else if ((to ^ fr) == 16)
	{
	  if (WhiteEP[to & 7] & bBitboardP)
	    {
	      z = (fr + to) >> 1;
	      POSITION->DYN->ep = z;
	      POSITION->DYN->HASH ^= ZobristEP[z & 7];
	    }
	}
    }
  POSITION->STACK[++(POSITION->StackHeight)] = POSITION->DYN->HASH;
  REV_HASH (POSITION);
  VALIDATE (POSITION, 0, move);
}

const static uint8 PromB[8] =
  { 0, 0, 0, 0, bEnumN, bEnumBL, bEnumR, bEnumQ };

void MakeBlack (typePOS* POSITION, uint32 move)
{
  int fr, to, pi, fl, cp, z;
  uint64 mask;
  TRACE (TRACE_MAKE_UNMAKE,
	 printf ("Mb %d %s\n", HEIGHT (POSITION), Notate(move, STRING1[POSITION->cpu])));
  POSITION->nodes++;
  memcpy (POSITION->DYN + 1, POSITION->DYN, 32);
  fr = FROM (move);
  to = TO (move);
#ifdef CHESS_960
  if (UCI_OPTION_CHESS_960 && MoveIsOO (move))
    {
      Castle960Black (POSITION, move, to, fr);
      return;
    }
#endif
  pi = POSITION->sq[fr];
  POSITION->DYN++;
  REV_HASH (POSITION);
  POSITION->DYN->reversible++;
  POSITION->DYN->move = move;
  fl = CastleTable[fr] & CastleTable[to] & POSITION->DYN->oo;
  POSITION->DYN->HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->PAWN_HASH ^= ZobristCastling[POSITION->DYN->oo ^ fl];
  POSITION->DYN->oo = fl;
  if (POSITION->DYN->ep)
    {
      POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
      POSITION->DYN->ep = 0;
    }
  POSITION->sq[fr] = 0;
  mask = SqClear[fr];
  bBitboardOcc &= mask;
  POSITION->bitboard[pi] &= mask;
  ClearOccupied (mask, fr);
  POSITION->DYN->STATIC += PST (pi, to) - PST (pi, fr);
  mask = Zobrist (pi, fr) ^ Zobrist (pi, to);
  cp = POSITION->sq[to];
  POSITION->DYN->cp = cp;
  POSITION->DYN->HASH ^= mask;
  if (pi == bEnumP)
    POSITION->DYN->PAWN_HASH ^= mask;
  POSITION->wtm ^= 1;
  POSITION->height++;
  POSITION->DYN->HASH ^= ZobristWTM;
  if (pi == bEnumK)
    {
      POSITION->DYN->PAWN_HASH ^= mask;
      POSITION->bKsq = to;
    }
  if (cp)
    {
      mask = SqClear[to];
      wBitboardOcc &= mask;
      POSITION->bitboard[cp] &= mask;
      POSITION->DYN->material -= MATERIAL_VALUE[cp];
      POSITION->DYN->STATIC -= PST (cp, to);
      if (cp == wEnumP)
	POSITION->DYN->PAWN_HASH ^= Zobrist (cp, to);
      POSITION->DYN->HASH ^= Zobrist (cp, to);
      POSITION->DYN->reversible = 0;
    }
  else
    {
      mask = SqSet[to];
      SetOccupied (mask, to);
      if (MoveIsOO (move))
	{
	  REV_CASTLE (POSITION);
	  MakeBlackOO (POSITION, to);
	}
    }
  POSITION->sq[to] = pi;
  bBitboardOcc |= SqSet[to];
  POSITION->bitboard[pi] |= SqSet[to];
  if (pi == bEnumP)
    {
      POSITION->DYN->reversible = 0;
      if (MoveIsEP (move))
	{
	  z = to ^ 8;
	  mask = SqClear[z];
	  wBitboardOcc &= mask;
	  wBitboardP &= mask;
	  ClearOccupied (mask, z);
	  POSITION->DYN->material -= MATERIAL_VALUE[wEnumP];
	  POSITION->DYN->STATIC -= PST (wEnumP, z);
	  POSITION->DYN->HASH ^= Zobrist (wEnumP, z);
	  POSITION->DYN->PAWN_HASH ^= Zobrist (wEnumP, z);
	  POSITION->sq[z] = 0;
	}
      else if (MoveIsProm (move))
	{
	  pi = PromB[(move & FLAG_MASK) >> 12];
	  if (pi == bEnumBL && SqSet[to] & DARK)
	    pi = bEnumBD;
	  POSITION->sq[to] = pi;
	  if (POSITION->bitboard[pi])
	    POSITION->DYN->material |= 0x80000000;
	  POSITION->bitboard[bEnumP] &= SqClear[to];
	  POSITION->bitboard[pi] |= SqSet[to];
	  POSITION->DYN->material += MATERIAL_VALUE[pi] - MATERIAL_VALUE[bEnumP];
	  POSITION->DYN->STATIC += PST (pi, to) - PST (bEnumP, to);
	  POSITION->DYN->HASH ^= Zobrist (pi, to) ^ Zobrist (bEnumP, to);
	  POSITION->DYN->PAWN_HASH ^= Zobrist (bEnumP, to);
	}
      else if ((to ^ fr) == 16)
	{
	  if (BlackEP[to & 7] & wBitboardP)
	    {
	      z = (fr + to) >> 1;
	      POSITION->DYN->ep = z;
	      POSITION->DYN->HASH ^= ZobristEP[z & 7];
	    }
	}
    }
  POSITION->STACK[++(POSITION->StackHeight)] = POSITION->DYN->HASH;
  REV_HASH (POSITION);
  VALIDATE (POSITION, 0, move);
}

void Make (typePOS* POSITION, uint32 move)
{
  if (POSITION->wtm)
    {
      if (NODE_CHECK & 4095)
	NODE_CHECK--;
      POSITION->nodes--;
      MakeWhite (POSITION, move);
    }
  else
    {
      POSITION->nodes--;
      MakeBlack (POSITION, move);
    }
}

#ifndef BUILD_mossa_gen
#define BUILD_mossa_gen

#include "RobboLito.h"

#define Add(L, x, y) { (L++)->move = (x) | (y);}
#define AddTo(T, w) \
  { while (T) \
      { to = BSF (T); c = POSITION->sq[to]; \
	Add (LIST, (sq << 6) | to, w);	BitClear (to, T); } }

#include "init_gen.i"

#define PtoQ ( 0xd8 << 24 )
#define PtoN ( 0xc2 << 24 )

#define FLAG_CHECK 0x8000

#define MoveAdd(L, x, pi, to, check) \
  { (L++)->move = (x) | ( (SqSet[to] & (check) ) ? FLAG_CHECK : 0) \
                      | ( HISTORY_PI_TO (POSITION, (pi), (to)) << 16); }

#define MovesTo(T, pi, check) \
  { while (T) \
      { to = BSF (T); MoveAdd (LIST, (sq << 6) | to, pi, to, check); \
	BitClear (to, T); } }

#define UnderPromWhite() \
  { if ( (AttN[to] & bBitboardK) == 0) \
      MoveAdd ( LIST, FlagPromN | (sq << 6) | to, wEnumP, to, 0); \
    MoveAdd (LIST, FlagPromR | (sq << 6) | to, wEnumP, to, 0); \
    MoveAdd (LIST, FlagPromB | (sq << 6) | to, wEnumP, to, 0); }

#define UnderPromBlack() \
  { if ( (AttN[to] & wBitboardK) == 0) \
      MoveAdd (LIST, FlagPromN | (sq << 6) | to, bEnumP, to, 0); \
    MoveAdd (LIST, FlagPromR | (sq << 6) | to, bEnumP, to, 0); \
    MoveAdd (LIST, FlagPromB | (sq << 6) | to, bEnumP, to, 0); }

#define OK(x) \
  ( ((x & 0x7fff) != s1) && ((x & 0x7fff) != s2) && ((x & 0x7fff) != s3) )

void SortOrdinary
(typeMoveList* m1, typeMoveList* m2, uint32 s1, uint32 s2, uint32 s3)
{
  typeMoveList *p, *q;
  int move;
  if (m1 == m2)
    return;
  for (p = m2 - 1; p >= m1; p--)
    {
      if (OK (p->move))
	break;
      p->move = 0;
    }
  while (p > m1)
    {
      p--;
      move = p->move;
      if (OK (move))
	{
	  for (q = p + 1; q < m2; q++)
	    {
	      if (move < q->move)
		(q - 1)->move = q->move;
	      else
		break;
	    }
	  q--;
	  q->move = move;
	}
      else
	{
	  m2--;
	  for (q = p; q < m2; q++)
	    q->move = (q + 1)->move;
	  m2->move = 0;
	}
    }
}

#define AddGain(L, x, pi, to) \
  { int v = ( (int) MAX_POS_GAIN ((pi), (x) & 07777)); \
    if (v >= av) (L++)->move = (x) | (v << 16); }
#define AddGainTo(T, pi) \
  { while (T) \
      { to = BSF(T); \
	AddGain (LIST, (sq << 6) | to, pi, to);	BitClear(to, T); } }

#define Sort \
  for (p = LIST - 1; p >= sm; p--) \
    { move = p->move; \
      for (q = p + 1; q < LIST; q++) \
	{ \
	  if ( move < q->move ) (q - 1)->move = q->move; \
	  else break; \
	} \
      q--; \
      q->move = move; }

typeMoveList* EvasionMoves (typePOS* POSITION, typeMoveList* list, uint64 mask)
{
  if (POSITION->wtm)
    return WhiteEvasions (POSITION, list, mask);
  return BlackEvasions (POSITION, list, mask);
}

typeMoveList* OrdinaryMoves (typePOS* POSITION, typeMoveList* list)
{
  if (POSITION->wtm)
    return WhiteOrdinary (POSITION, list);
  return BlackOrdinary (POSITION, list);
}

typeMoveList* CaptureMoves (typePOS* POSITION, typeMoveList* list, uint64 mask)
{
  if (POSITION->wtm)
    return WhiteCaptures (POSITION, list, mask & bBitboardOcc);
  return BlackCaptures (POSITION, list, mask & wBitboardOcc);
}

#include "move_gen.c"
#include "white.h"
#else
#include "black.h"
#endif

typeMoveList* MyEvasion (typePOS* POSITION, typeMoveList* LIST, uint64 c2)
{
  uint64 U, T, att, mask;
  int sq, to, fr, c, king, pi;
  king = MyKingSq;
  att = MyKingCheck;
  sq = BSF (att);
  pi = POSITION->sq[sq];
  mask = (~OppAttacked) & (((pi == EnumOppP) ? AttK[king] : 0) |
			   Evade (king, sq)) & (~MyOccupied) & c2;
  BitClear (sq, att);
  if (att)
    {
      sq = BSF (att);
      pi = POSITION->sq[sq];
      mask = mask & (PIECE_IS_OPP_PAWN (pi) | Evade (king, sq));
      sq = king;
      AddTo (mask, CAPTURE_VALUE[EnumMyK][c]);
      LIST->move = 0;
      return LIST;
    }
  c2 &= InterPose (king, sq);
  sq = king;
  AddTo (mask, CAPTURE_VALUE[EnumMyK][c]);
  if (!c2)
    {
      LIST->move = 0;
      return LIST;
    }

  if (CAPTURE_RIGHT & (c2 & OppOccupied))
    {
      to = BSF (c2 & OppOccupied);
      c = POSITION->sq[to];
      if (EIGHTH_RANK (to))
	{
	  Add (LIST, FlagPromQ | FROM_LEFT (to) | to,
	       (0x20 << 24) + CAPTURE_VALUE[EnumMyP][c]);
	  Add (LIST, FlagPromN | FROM_LEFT (to) | to, 0);
	  Add (LIST, FlagPromR | FROM_LEFT (to) | to, 0);
	  Add (LIST, FlagPromB | FROM_LEFT (to) | to, 0);
	}
      else
	Add (LIST, FROM_LEFT (to) | to, CAPTURE_VALUE[EnumMyP][c]);
    }

  if (CAPTURE_LEFT & (c2 & OppOccupied))
    {
      to = BSF (c2 & OppOccupied);
      c = POSITION->sq[to];
      if (EIGHTH_RANK (to))
	{
	  Add (LIST, FlagPromQ | FROM_RIGHT (to) | to,
	       (0x20 << 24) + CAPTURE_VALUE[EnumMyP][c]);
	  Add (LIST, FlagPromN | FROM_RIGHT (to) | to, 0);
	  Add (LIST, FlagPromR | FROM_RIGHT (to) | to, 0);
	  Add (LIST, FlagPromB | FROM_RIGHT (to) | to, 0);
	}
      else
	Add (LIST, FROM_RIGHT (to) | to, CAPTURE_VALUE[EnumMyP][c]);
    }
  to = POSITION->DYN->ep;
  if (to)
    {
      if (CAPTURE_RIGHT & SqSet[to] && SqSet[BACKWARD (to)] & c2)
	Add (LIST, FlagEP | FROM_LEFT (to) | to,
	     CAPTURE_VALUE[EnumMyP][EnumOppP]);
      if (CAPTURE_LEFT & SqSet[to] && SqSet[BACKWARD (to)] & c2)
	Add (LIST, FlagEP | FROM_RIGHT (to) | to,
	     CAPTURE_VALUE[EnumMyP][EnumOppP]);
    }
  T = BitboardMyP & BACK_SHIFT ((c2 & OppOccupied) ^ c2);
  while (T)
    {
      fr = BSF (T);
      BitClear (fr, T);
      if (SEVENTH_RANK (fr))
	{
	  Add (LIST, FlagPromQ | (fr << 6) | FORWARD (fr),
	       CAPTURE_VALUE[EnumMyP][0]);
	  Add (LIST, FlagPromN | (fr << 6) | FORWARD (fr), 0);
	  Add (LIST, FlagPromR | (fr << 6) | FORWARD (fr), 0);
	  Add (LIST, FlagPromB | (fr << 6) | FORWARD (fr), 0);
	}
      else
	Add (LIST, (fr << 6) | FORWARD (fr), CAPTURE_VALUE[EnumMyP][0]);
    }

  T = BitboardMyP & BACK_SHIFT2 ((c2 & OppOccupied) ^ c2) &
      SECOND_RANK & BACK_SHIFT (~POSITION->OccupiedBW);
  while (T)
    {
      fr = BSF (T);
      BitClear (fr, T);
      Add (LIST, (fr << 6) | FORWARD2 (fr), CAPTURE_VALUE[EnumMyP][0]);
    }
  for (U = BitboardMyN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & c2;
      AddTo (T, CAPTURE_VALUE[EnumMyN][c]);
    }
  for (U = BitboardMyB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & c2;
      AddTo (T, CAPTURE_VALUE[EnumMyBL][c]);
    }
  for (U = BitboardMyR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & c2;
      AddTo (T, CAPTURE_VALUE[EnumMyR][c]);
    }
  for (U = BitboardMyQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & c2;
      AddTo (T, CAPTURE_VALUE[EnumMyQ][c]);
    }
  LIST->move = 0;
  return LIST;
}

typeMoveList* MyPositionalGain (typePOS* POSITION, typeMoveList * LIST, int av)
{
  uint64 empty = ~POSITION->OccupiedBW, U, T;
  int to, sq;
  typeMoveList *sm, *p, *q;
  int move;
  /* Buys not OO in Gain ? */
  sm = LIST;
  for (U = FORWARD_SHIFT (BitboardMyP & SECOND_SIXTH_RANKS) & empty; U;
       BitClear (sq, U))
    {
      to = BSF (U);
      if (ON_THIRD_RANK (to) && POSITION->sq[FORWARD (to)] == 0)
	AddGain (LIST, (BACKWARD (to) << 6) | FORWARD (to),
		 EnumMyP, FORWARD (to));
      AddGain (LIST, (BACKWARD (to) << 6) | to, EnumMyP, to);
    }
  for (U = BitboardMyN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & empty;
      AddGainTo (T, EnumMyN);
    }
  for (U = BitboardMyBL; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & empty;
      AddGainTo (T, EnumMyBL);
    }
  for (U = BitboardMyBD; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & empty;
      AddGainTo (T, EnumMyBD);
    }
  for (U = BitboardMyR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & empty;
      AddGainTo (T, EnumMyR);
    }
  for (U = BitboardMyQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & empty;
      AddGainTo (T, EnumMyQ);
    }
  sq = MyKingSq;
  T = AttK[sq] & empty & (~OppAttacked);
  AddGainTo (T, EnumMyK);
  LIST->move = 0;
  Sort;
  return LIST;
}

typeMoveList* MyCapture (typePOS* POSITION, typeMoveList* LIST, uint64 mask)
{
  uint64 U, T, AttR, AttB;
  int sq, to, c;
  to = POSITION->DYN->ep;
  if (to)
    {
      if (CAPTURE_LEFT & SqSet[to])
	Add (LIST, FlagEP | FROM_RIGHT (to) | to, CaptureEP);
      if (CAPTURE_RIGHT & SqSet[to])
	Add (LIST, FlagEP | FROM_LEFT (to) | to, CaptureEP);
    }
  if ((mask & MyAttacked) == 0)
    goto NO_TARGET;

  T = CAPTURE_LEFT & (~BITBOARD_EIGHTH_RANK) & mask;
  while (T)
    {
      to = BSF (T);
      c = POSITION->sq[to];
      Add (LIST, FROM_RIGHT (to) | to, CAPTURE_VALUE[EnumMyP][c]);
      BitClear (to, T);
    }
  T = CAPTURE_RIGHT & (~BITBOARD_EIGHTH_RANK) & mask;
  while (T)
    {
      to = BSF (T);
      c = POSITION->sq[to];
      Add (LIST, FROM_LEFT (to) | to, CAPTURE_VALUE[EnumMyP][c]);
      BitClear (to, T);
    }

  for (U = BitboardMyN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & mask;
      AddTo (T, CAPTURE_VALUE[EnumMyN][c]);
    }
  for (U = BitboardMyB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      AttB = AttB (sq);
      T = AttB & mask;
      AddTo (T, CAPTURE_VALUE[EnumMyBL][c]);
    }
  for (U = BitboardMyR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      AttR = AttR (sq);
      T = AttR & mask;
      AddTo (T, CAPTURE_VALUE[EnumMyR][c]);
    }
  for (U = BitboardMyQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      AttR = AttR (sq);
      AttB = AttB (sq);
      T = (AttB | AttR) & mask;
      AddTo (T, CAPTURE_VALUE[EnumMyQ][c]);
    }
  sq = BSF (BitboardMyK);
  T = AttK[sq] & mask & (~OppAttacked);
  AddTo (T, CAPTURE_VALUE[EnumMyK][c]);
NO_TARGET:
  for (U = BitboardMyP & BITBOARD_SEVENTH_RANK; U; BitClear (sq, U))
    {
      sq = BSF (U);
      to = FORWARD (sq);
      if (POSITION->sq[to] == 0)
	{
	  Add (LIST, FlagPromQ | (sq << 6) | to, PtoQ);
	  if (AttN[to] & BitboardOppK)
	    Add (LIST, FlagPromN | (sq << 6) | to, PtoN);
	}
      to = FORWARD_LEFT (sq);
      if (sq != WHITE_A7 && SqSet[to] & mask)
	{
	  c = POSITION->sq[to];
	  Add (LIST, FlagPromQ | (sq << 6) | to, PromQueenCap);
	  if (AttN[to] & BitboardOppK)
	    Add (LIST, FlagPromN | (sq << 6) | to, PromKnightCap);
	}
      to = FORWARD_RIGHT (sq);
      if (sq != WHITE_H7 && SqSet[to] & mask)
	{
	  c = POSITION->sq[to];
	  Add (LIST, FlagPromQ | (sq << 6) | to, PromQueenCap);
	  if (AttN[to] & BitboardOppK)
	    Add (LIST, FlagPromN | (sq << 6) | to, PromKnightCap);
	}
    }
  LIST->move = 0;
  return LIST;
}

typeMoveList* MyOrdinary (typePOS* POSITION, typeMoveList* LIST)
{
  uint64 empty = ~POSITION->OccupiedBW, U, T, ROOK, BISHOP, PAWN;
  int to, sq, opks = OppKingSq;

#ifdef CHESS_960
  if (UCI_OPTION_CHESS_960) /* Attend Chess960 */
    {
      if (CastleOO)
	{
	  uint64 T = 0;
	  int f;
	  for (f = CHESS_960_KING_FILE; f <= FG; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];
	  if (T & OppAttacked)
	    goto NO_OO;
	  T = 0;
	  for (f = CHESS_960_KR_FILE + 1; f <= FF; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  for (f = CHESS_960_KR_FILE - 1; f >= FF; f--)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  for (f = CHESS_960_KING_FILE + 1; f <= FG; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  T &= SqClear[CHESS_960_KING_FILE + 8 * NUMBER_RANK1];
	  T &= SqClear[CHESS_960_KR_FILE + 8 * NUMBER_RANK1];
	  if (T & POSITION->OccupiedBW)
	    goto NO_OO;
	  MoveAdd (LIST, FlagOO | (CHESS_960_KR_FILE + 8 * NUMBER_RANK1) |
		   ((CHESS_960_KING_FILE + 8 * NUMBER_RANK1) << 6),
		   EnumMyK, CHESS_960_KR_FILE + 8 * NUMBER_RANK1, 0);
	NO_OO: ;
	}
      if (CastleOOO)
	{
	  uint64 T = 0;
	  int f;
	  for (f = CHESS_960_KING_FILE; f <= FC; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];
	  for (f = CHESS_960_KING_FILE; f >= FC; f--)
	    T |= SqSet[f + 8 * NUMBER_RANK1];
	  if (T & OppAttacked)
	    goto NO_OOO;
	  T = 0;
	  for (f = CHESS_960_QR_FILE + 1; f <= FD; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  for (f = CHESS_960_QR_FILE - 1; f >= FD; f--)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  for (f = CHESS_960_KING_FILE + 1; f <= FC; f++)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  for (f = CHESS_960_KING_FILE - 1; f >= FC; f--)
	    T |= SqSet[f + 8 * NUMBER_RANK1];	  
	  T &= SqClear[CHESS_960_KING_FILE + 8 * NUMBER_RANK1];
	  T &= SqClear[CHESS_960_QR_FILE + 8 * NUMBER_RANK1];
	  if (T & POSITION->OccupiedBW)
	    goto NO_OOO;
	  MoveAdd (LIST, FlagOO | (CHESS_960_QR_FILE + 8 * NUMBER_RANK1) |
		   ((CHESS_960_KING_FILE + 8 * NUMBER_RANK1) << 6),
		   EnumMyK, CHESS_960_QR_FILE + 8 * NUMBER_RANK1, 0);
	NO_OOO: ;
	}
    }
  else
#endif
    {
      if (CastleOO && ((POSITION->OccupiedBW | OppAttacked) & WHITE_F1G1) == 0)
	MoveAdd (LIST, FlagOO | (WHITE_E1 << 6) | WHITE_G1, EnumMyK, WHITE_G1, 0);
      if (CastleOOO && (POSITION->OccupiedBW & WHITE_B1C1D1) == 0
	  && (OppAttacked & WHITE_C1D1) == 0)
	MoveAdd (LIST, FlagOO | (WHITE_E1 << 6) | WHITE_C1, EnumMyK, WHITE_C1, 0);
    }      
  PAWN = MyAttackedPawns[opks];
  if (BitboardMyQ | BitboardMyR)
    ROOK = AttR (opks);
  if (BitboardMyQ | BitboardMyB)
    BISHOP = AttB (opks);
  for (U = FORWARD_SHIFT (BitboardMyP & SECOND_SIXTH_RANKS) & empty;
       U; BitClear (sq, U))
    {
      to = BSF (U);
      if (ON_THIRD_RANK (to) && POSITION->sq[FORWARD (to)] == 0)
	MoveAdd (LIST, (BACKWARD (to) << 6) | FORWARD (to),
		 EnumMyP, FORWARD (to), PAWN);
      MoveAdd (LIST, (BACKWARD (to) << 6) | to, EnumMyP, to, PAWN);
    }

  for (U = BitboardMyQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & empty;
      MovesTo (T, EnumMyQ, ROOK | BISHOP);
    }
  for (U = BitboardMyR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & empty;
      MovesTo (T, EnumMyR, ROOK);
    }
  for (U = BitboardMyB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & empty;
      MovesTo (T, ((SqSet[sq] & DARK) ? EnumMyBD : EnumMyBL), BISHOP);
    }
  sq = BSF (BitboardMyK);
  T = AttK[sq] & empty & (~OppAttacked);
  MovesTo (T, EnumMyK, 0);
  for (U = BitboardMyN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & empty;
      MovesTo (T, EnumMyN, AttN[opks]);
    }
  for (U = BitboardMyP & BITBOARD_SEVENTH_RANK; U; BitClear (sq, U))
    {
      sq = BSF (U);
      to = FORWARD (sq);
      if (POSITION->sq[to] == 0)
	UnderProm ();
      to = FORWARD_LEFT (sq);
      if (sq != WHITE_A7 && SqSet[to] & OppOccupied)
	UnderProm ();
      to = FORWARD_RIGHT (sq);
      if (sq != WHITE_H7 && SqSet[to] & OppOccupied)
	UnderProm ();
    }
  LIST->move = 0;
  return LIST;
}

typeMoveList* MyQuietChecks (typePOS* POSITION, typeMoveList* LIST, uint64 mask)
{
  int opks, king, sq, to, fr, pi;
  uint64 U, T, V;
  typeMoveList* list;
  uint32 move;
  uint64 gcm;
  gcm = ~MyXRAY;
  mask = (~mask) & ~MyOccupied;;
  list = LIST;
  king = OppKingSq;
  list = LIST;

  for (U = MyXRAY & MyOccupied; U; BitClear (fr, U))
    {
      fr = BSF (U);
      pi = POSITION->sq[fr];
      if (pi == EnumMyP)
	{
	  if (FILE (fr) != FILE (king) && !SEVENTH_RANK (fr)
	      && POSITION->sq[FORWARD (fr)] == 0)
	    {
	      (LIST++)->move = (fr << 6) | FORWARD (fr);
	      if (ON_SECOND_RANK (fr) && POSITION->sq[FORWARD2 (fr)] == 0)
		(LIST++)->move = (fr << 6) | FORWARD2 (fr);
	    }
          if (CAN_CAPTURE_RIGHT && RANK (fr) != NUMBER_RANK7)
	    (LIST++)->move = (fr << 6) | FORWARD_RIGHT (fr);
	  if (CAN_CAPTURE_LEFT && RANK (fr) != NUMBER_RANK7)
	    (LIST++)->move = (fr << 6) | FORWARD_LEFT (fr);
	}
      else if (pi == EnumMyN)
	{
	  V = AttN[fr] & mask;
	  while (V)
	    {
	      to = BSF (V);
	      (LIST++)->move = (fr << 6) | to;
	      BitClear (to, V);
	    }
	}
      else if (pi == EnumMyBL || pi == EnumMyBD)
	{
	  V = AttB (fr) & mask;
	  while (V)
	    {
	      to = BSF (V);
	      (LIST++)->move = (fr << 6) | to;
	      BitClear (to, V);
	    }
	}
      else if (pi == EnumMyR)
	{
	  V = AttR (fr) & mask;
	  while (V)
	    {
	      to = BSF (V);
	      (LIST++)->move = (fr << 6) | to;
	      BitClear (to, V);
	    }
	}
      else if (pi == EnumMyK)
	{
	  if (FILE (fr) == FILE (king) || RANK (fr) == RANK (king))
	    V = AttK[fr] & NON_ORTHO[king] & mask & (~OppAttacked);
	  else
	    V = AttK[fr] & NON_DIAG[king] & mask & (~OppAttacked);
	  while (V)
	    {
	      to = BSF (V);
	      (LIST++)->move = (fr << 6) | to;
	      BitClear (to, V);
	    }
	}
    }

  opks = OppKingSq;
  T = CAPTURE_LEFT & (~BITBOARD_EIGHTH_RANK) & mask & OppOccupied &
      MyAttackedPawns[opks];
  while (T)
    {
      to = BSF (T);
      (LIST++)->move = FROM_RIGHT (to) | to;
      BitClear (to, T);
    }
  T = CAPTURE_RIGHT & (~BITBOARD_EIGHTH_RANK) & mask & OppOccupied &
      MyAttackedPawns[opks];
  while (T)
    {
      to = BSF (T);
      (LIST++)->move = FROM_LEFT (to) | to;
      BitClear (to, T);
    }

  for (U = BitboardMyQ; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttQ (sq) & AttQ (king) & mask;
      while (T)
	{
	  to = BSF (T);
	  BitClear (to, T);
	  if ((OppAttackedPawns[to] & BitboardOppP & gcm) == 0
	      && (AttN[to] & BitboardOppN & gcm) == 0)
	    {
	      move = (sq << 6) | to;
	      if (MySEE (POSITION, move))
		(LIST++)->move = (sq << 6) | to;
	    }
	}
    }
  for (U = BitboardMyR; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttR (sq) & AttR (king) & mask;
      while (T)
	{
	  to = BSF (T);
	  BitClear (to, T);
	  if ((OppAttackedPawns[to] & BitboardOppP & gcm) == 0
	      && (AttN[to] & BitboardOppN & gcm) == 0)
	    {
	      move = (sq << 6) | to;
	      if (MySEE (POSITION, move))
		(LIST++)->move = (sq << 6) | to;
	    }
	}
    }
  for (U = BitboardMyB; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttB (sq) & AttB (king) & mask;
      while (T)
	{
	  to = BSF (T);
	  BitClear (to, T);
	  if ((OppAttackedPawns[to] & BitboardOppP & gcm) == 0)
	    {
	      move = (sq << 6) | to;
	      if (MySEE (POSITION, move))
		(LIST++)->move = (sq << 6) | to;
	    }
	}
    }
  for (U = BitboardMyN; U; BitClear (sq, U))
    {
      sq = BSF (U);
      T = AttN[sq] & AttN[king] & mask;
      while (T)
	{
	  to = BSF (T);
	  BitClear (to, T);
	  if ((OppAttackedPawns[to] & BitboardOppP & gcm) == 0)
	    {
	      move = (sq << 6) | to;
	      if (MySEE (POSITION, move))
		(LIST++)->move = (sq << 6) | to;
	    }
	}
    }

  if (BitboardOppK & FOURTH_EIGHTH_RANK_NOh
      && POSITION->sq[BACK_RIGHT (opks)] == 0)
    {
      if (POSITION->sq[BACK_RIGHT2 (opks)] == EnumMyP)
	{
	  fr = BACK_RIGHT2 (opks);
	  to = BACK_RIGHT (opks);
	  move = (fr << 6) | to;
	  if (PAWN_GUARD (to, fr) && MySEE (POSITION, move))
	    (LIST++)->move = move;
	}
      if (RANK (opks) == NUMBER_RANK5
	  && POSITION->sq[BACK_RIGHT2 (opks)] == 0
	  && POSITION->sq[BACK_RIGHT3 (opks)] == EnumMyP)
	{
	  to = BACK_RIGHT (opks);
	  fr = BACK_RIGHT3 (opks);
	  move = (fr << 6) | to;
	  if (PAWN_GUARD (to, fr) && MySEE (POSITION, move))
	    (LIST++)->move = move;
	}
    }
  if (BitboardOppK & FOURTH_EIGHTH_RANK_NOa
      && POSITION->sq[BACK_LEFT (opks)] == 0)
    {
      if (POSITION->sq[BACK_LEFT2 (opks)] == EnumMyP)
	{
	  fr = BACK_LEFT2 (opks);
	  to = BACK_LEFT (opks);
	  move = (fr << 6) | to;
	  if (PAWN_GUARD (to, fr) && MySEE (POSITION, move))
	    (LIST++)->move = move;
	}
      if (RANK (opks) == NUMBER_RANK5
	  && POSITION->sq[BACK_LEFT2 (opks)] == 0
	  && POSITION->sq[BACK_LEFT3 (opks)] == EnumMyP)
	{
	  to = BACK_LEFT (opks);
	  fr = BACK_LEFT3 (opks);
	  move = (fr << 6) | to;
	  if (PAWN_GUARD (to, fr) && MySEE (POSITION, move))
	    (LIST++)->move = move;
	}
    }
  LIST->move = MOVE_NONE;
  return LIST;
}

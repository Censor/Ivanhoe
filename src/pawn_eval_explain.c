
#ifdef UTILITIES
#include "RobboLito.h"
#include "pawn_eval.v"

#define EE printf
#define EXPLAIN TRUE
#define OpEg(x) \
  ((sint16) ((((x) & 0xffff) < 0) + ((x) >> 16))), (sint16) ((x) & 0xffff)

typedef struct
{
  uint64 EDGE, MIDDLE, CENTER;
  uint8 ShelterEdge[8], ShelterMiddle[8], ShelterCenter[8];
  uint8 StormEdge[8], StormMiddle[8], StormCenter[8],
    ShelterDiag[8], ZERO, VALU_ZERO;
} typePAWNptr;

extern typePAWNptr PAWNptr[8];

extern uint8 SHELTERaa[8];
extern uint8 SHELTERab[8];
extern uint8 SHELTERac[8];
extern uint8 STORMaa[8];
extern uint8 STORMab[8];
extern uint8 STORMac[8];

extern uint8 SHELTERba[8];
extern uint8 SHELTERbb[8];
extern uint8 SHELTERbc[8];
extern uint8 STORMba[8];
extern uint8 STORMbb[8];
extern uint8 STORMbc[8];

extern uint8 SHELTERcb[8];
extern uint8 SHELTERcc[8];
extern uint8 SHELTERcd[8];
extern uint8 STORMcb[8];
extern uint8 STORMcc[8];
extern uint8 STORMcd[8];

extern uint8 SHELTERdc[8];
extern uint8 SHELTERdd[8];
extern uint8 SHELTERde[8];
extern uint8 STORMdc[8];
extern uint8 STORMdd[8];
extern uint8 STORMde[8];

extern uint8 ShelterLongDiagA[8];
extern uint8 ShelterLongDiagB[8];
extern uint8 ShelterLongDiagC[8];
extern uint8 ShelterLongDiagD[8];

static int WhiteKingDanger (typePOS* POSITION, int wKs)
{
  int e, RankWa, RankWb, RankWc, RankBa, RankBb, RankBc, v, tr = RANK (wKs);
  uint64 T, A = wBitboardP & NotInFrontB[tr];
  typePAWNptr Z = PAWNptr[FILE (wKs)];
  T = A & Z.EDGE;
  RankWa = BSF (T);
  if (!T)
    RankWa = 0;
  RankWa >>= 3;
  T = A & Z.MIDDLE;
  RankWb = BSF (T);
  if (!T)
    RankWb = 0;
  RankWb >>= 3;
  T = A & Z.CENTER;
  RankWc = BSF (T);
  if (!T)
    RankWc = 0;
  RankWc >>= 3;
  T = bBitboardP & Z.EDGE;
  RankBa = BSF (T);
  if (!T)
    RankBa = 0;
  RankBa >>= 3;
  T = bBitboardP & Z.MIDDLE;
  RankBb = BSF (T);
  if (!T)
    RankBb = 0;
  RankBb >>= 3;
  T = bBitboardP & Z.CENTER;
  RankBc = BSF (T);
  if (!T)
    RankBc = 0;
  RankBc >>= 3;
  v = (Z.ShelterEdge)[RankWa] +
      (Z.ShelterMiddle)[RankWb] + (Z.ShelterCenter)[RankWc];
  if (v == Z.ZERO)
    v = Z.VALU_ZERO;
  T = A & LONG_DIAG[wKs];
  e = BSF (T);
  if (!T)
    e = 0;
  e >>= 3;
  v += (Z.ShelterDiag)[e];
  e = (Z.StormEdge)[RankBa];
  if (RankBa == (RankWa + 1))
    e >>= 1;
  v += e;
  e = (Z.StormMiddle)[RankBb];
  if (RankBb == (RankWb + 1))
    e >>= 1;
  v += e;
  e = (Z.StormCenter)[RankBc];
  if (RankBc == (RankWc + 1))
    e >>= 1;
  v += e;
  return v;
}

static int BlackKingDanger (typePOS* POSITION, int bKs)
{
  int e, RankWa, RankWb, RankWc, RankBa, RankBb, RankBc, v, tr = RANK (bKs);
  uint64 T, A = bBitboardP & NotInFrontW[tr];
  typePAWNptr Z = PAWNptr[FILE (bKs)];
  T = A & Z.EDGE;
  RankBa = BSR (T);
  if (!T)
    RankBa = 56;
  RankBa >>= 3;
  RankBa = 7 - RankBa;
  T = A & Z.MIDDLE;
  RankBb = BSR (T);
  if (!T)
    RankBb = 56;
  RankBb >>= 3;
  RankBb = 7 - RankBb;
  T = A & Z.CENTER;
  RankBc = BSR (T);
  if (!T)
    RankBc = 56;
  RankBc >>= 3;
  RankBc = 7 - RankBc;
  T = wBitboardP & Z.EDGE;
  RankWa = BSR (T);
  if (!T)
    RankWa = 56;
  RankWa >>= 3;
  RankWa = 7 - RankWa;
  T = wBitboardP & Z.MIDDLE;
  RankWb = BSR (T);
  if (!T)
    RankWb = 56;
  RankWb >>= 3;
  RankWb = 7 - RankWb;
  T = wBitboardP & Z.CENTER;
  RankWc = BSR (T);
  if (!T)
    RankWc = 56;
  RankWc >>= 3;
  RankWc = 7 - RankWc;
  v = (Z.ShelterEdge)[RankBa] + (Z.ShelterMiddle)[RankBb] +
      (Z.ShelterCenter)[RankBc];
  if (v == Z.ZERO)
    v = Z.VALU_ZERO;
  T = A & LONG_DIAG[bKs];
  e = BSR (T);
  if (!T)
    e = 56;
  e >>= 3;
  e = 7 - e;
  v += (Z.ShelterDiag)[e];
  e = (Z.StormEdge)[RankWa];
  if (RankWa == (RankBa + 1))
    e >>= 1;
  v += e;
  e = (Z.StormMiddle)[RankWb];
  if (RankWb == (RankBb + 1))
    e >>= 1;
  v += e;
  e = (Z.StormCenter)[RankWc];
  if (RankWc == (RankBc + 1))
    e >>= 1;
  v += e;
  return v;
}

void PawnEvalExplanation (typePOS* POSITION, typePawnEval* RESULT)
{
  int c, Value = 0, B, DistanceWhiteKing, DistanceBlackKing,
    BestWhiteKingDistance, BestBlackKingDistance;
  int wKs = POSITION->wKsq, bKs = POSITION->bKsq;
  int b, tr, co, v, ValuePassedPawn;
  uint64 T, U, V, CONNECTED;
  typePawnEval* ptr;

  RESULT->wPlight = RESULT->bPlight = RESULT->wPdark = RESULT->bPdark = 0;
  RESULT->wKdanger = RESULT->bKdanger = 0;
  RESULT->wPassedFiles = RESULT->bPassedFiles = 0;
  BestBlackKingDistance = BestWhiteKingDistance = 30000;
  CONNECTED = 0;

  c = 0;
  if (wBitboardP)
    EE ("wIslands ");
  for (co = FA; co <= FH; co++)
    {
      if ((wBitboardP & FileArray[co]) == 0)
	c = 0;
      else
	{
	  if (c == 0)
	    {
	      Value -= Islands;
	      EE ("-%d:-%d ", OpEg(Islands));
	    }
	  c = 1;
	}
    }
  if (wBitboardP)
    EE ("\n");

  T = wBitboardP;
  while (T)
    {
      b = BSF (T);
      BitClear (b, T);
      tr = RANK (b);
      co = FILE (b);
      EE ("wP%c%c%c%d:%d ", 'a' + co, '1' + tr,
	  AttPb[b] & AttK[bKs] ? '*' : ' ', OpEg (PST (wEnumP, b)));

      DistanceWhiteKing = WhiteKingPawnDistance (b, wKs);
      if (DistanceWhiteKing < BestWhiteKingDistance)
	BestWhiteKingDistance = DistanceWhiteKing;
      DistanceBlackKing = WhiteKingPawnDistance (b, bKs);
      if (DistanceBlackKing < BestBlackKingDistance)
	BestBlackKingDistance = DistanceBlackKing;

      if (SqSet[b] & LIGHT)
	{
	  RESULT->wPlight += BlockedPawnValue[b];
	  if (POSITION->sq[b + 8] == bEnumP)
	    RESULT->wPlight += BlockedPawnValue[b];
	}
      else
	{
	  RESULT->wPdark += BlockedPawnValue[b];
	  if (POSITION->sq[b + 8] == bEnumP)
	    RESULT->wPdark += BlockedPawnValue[b];
	}

      if (wBitboardP & LEFT2[b]
	  && (wBitboardP & InFrontW[tr - 1] & FileArray[co - 1]) == 0)
	{
	  Value -= Hole;
	  EE ("hole -%d:-%d ", OpEg (Hole));
	}

      if ((wBitboardP | bBitboardP) & OpenFileW[b])
	{
	  if (wBitboardP & DOUBLED[b])
	    {
	      Value -= DoubledClosed;
	      EE ("doub cl -%d:-%d ", OpEg (DoubledClosed));
	      if ((wBitboardP & IsolatedFiles[co]) == 0)
		{
		  Value -= DoubledClosedIsolated;
		  EE ("isol -%d:-%d ", OpEg (DoubledClosedIsolated));
		}
	    }
	  if ((wBitboardP & IsolatedFiles[co]) == 0)
	    {
	      Value -= IsolatedClosed;
	      EE ("isol cl -%d:-%d\n", OpEg (IsolatedClosed));
	      continue;
	    }
	  if ((wBitboardP & ProtectedPawnW[b]) == 0)
	    {
	      B = b + 8;
	      if ((wBitboardP & AttPb[b]) == 0)
		{
		  B += 8;
		  if ((wBitboardP & AttPb[b + 8]) == 0)
		    B += 8;
		}
	      if (bBitboardP & AttPb[B])
		{
		  Value -= BackwardClosed;
		  EE ("back cl -%d:-%d ", OpEg (BackwardClosed));
		}
	    }
	  EE ("\n");
	  continue;
	}

      if (wBitboardP & DOUBLED[b])
	{
	  Value -= DoubledOpen;
	  EE ("doub op -%d:-%d ", OpEg (DoubledOpen));
	  if ((wBitboardP & IsolatedFiles[co]) == 0)
	    {
	      Value -= DoubledOpenIsolated;
	      EE ("doub op isol -%d:-%d ", OpEg (DoubledOpenIsolated));
	    }
	}
      if ((wBitboardP & IsolatedFiles[co]) == 0)
	{
	  Value -= IsolatedOpen;
	  EE ("isol op -%d:-%d ", OpEg (IsolatedOpen));
	}
      else
	{
	  if ((wBitboardP & ProtectedPawnW[b]) == 0)
	    {
	      B = b + 8;
	      if ((wBitboardP & AttPb[b]) == 0)
		{
		  B += 8;
		  if ((wBitboardP & AttPb[b + 8]) == 0)
		    B += 8;
		}
	      if (bBitboardP & AttPb[B])
		{
		  Value -= BackwardOpen;
		  EE ("back op -%d:-%d ", OpEg (BackwardOpen));
		}
	    }
	}

      if ((bBitboardP & PassedPawnW[b]) == 0)
	goto PassedW;
      if (bBitboardP & PassedPawnW[b] & ~AttPb[b])
	{
	  Value += CandidatePawnValue[tr];
	  if (CandidatePawnValue[tr])
	    EE ("cand %d:%d\n", OpEg (CandidatePawnValue[tr]));
	  else
	    EE ("\n");
	  continue;
	}
      if (POPCNT (AttPb[b] & bBitboardP) > POPCNT (AttPw[b] & wBitboardP))
	{
	  Value += CandidatePawnValue[tr];
	  if (CandidatePawnValue[tr])
	    EE ("cand %d:%d\n", OpEg (CandidatePawnValue[tr]));
	  else
	    EE ("\n");
	  continue;
	}
    PassedW:
      ValuePassedPawn = PassedPawnValue[tr];
      EE ("\n    passed %d:%d ", OpEg (PassedPawnValue[tr]));
      if (wBitboardP & AttPw[b])
	{
	  ValuePassedPawn += ProtectedPassedPawnValue[tr];
	  if (ProtectedPassedPawnValue[tr])
	    EE ("prot %d:%d ", OpEg (ProtectedPassedPawnValue[tr]));
	}
      if ((bBitboardP & FilesLeft[co]) == 0
	  || (bBitboardP & FilesRight[co]) == 0)
	{
	  ValuePassedPawn += OutsidePassedPawnValue[tr];
	  if (OutsidePassedPawnValue[tr])
	    EE ("out %d:%d ", OpEg (OutsidePassedPawnValue[tr]));
	}
      V = ConnectedPawns[b] & CONNECTED;
      CONNECTED |= SqSet[b];
      if (V)
	{
	  int value = ConnectedPassedPawnValue[tr] +
	    ConnectedPassedPawnValue[RANK (BSF (V))];
	  ValuePassedPawn += value;
	  if (value)
	    EE ("conn %d:%d ", OpEg (value));
	  BitClear (0, V);
	  if (V)
	    {
	      int value = ConnectedPassedPawnValue[tr] +
		ConnectedPassedPawnValue[RANK (BSF (V))];
	      ValuePassedPawn += value;
	      if (value)
		EE ("conn %d:%d ", OpEg (value));
	    }
	}
      Value += ValuePassedPawn;
      if (ValuePassedPawn)
	EE ("= %d:%d ", OpEg (ValuePassedPawn));

      RESULT->wPassedFiles |= (uint8) (1 << co);
      if (b <= H3)
	{
	  EE ("\n");
	  continue;
	}
      Value += (WhiteKingPawnDistance (b + 8, bKs) *
		OppKingPawnDistance[RANK (b)]);
      EE (" oppK %d:%d ", OpEg (WhiteKingPawnDistance (b + 8, bKs) *
				OppKingPawnDistance[RANK (b)]));
      Value -= (WhiteKingPawnDistance (b + 8, wKs) *
		MyKingPawnDistance[RANK (b)]);
      EE (" myK -%d:-%d\n", OpEg (WhiteKingPawnDistance (b + 8, wKs) *
				  MyKingPawnDistance[RANK (b)]));
    }

  /******/ /******/  /******/ /******/   /******/ /******/  /******/ /******/
  c = 0;
  if (bBitboardP)
    EE ("bIslands ");
  for (co = FA; co <= FH; co++)
    {
      if ((bBitboardP & FileArray[co]) == 0)
	c = 0;
      else
	{
	  if (c == 0)
	    {
	      Value += Islands;
	      EE ("%d:%d ", OpEg(Islands));
	    }
	  c = 1;
	}
    }
  if (bBitboardP)
    EE ("\n");

  CONNECTED = 0;
  T = bBitboardP;
  while (T)
    {
      b = BSF (T);
      BitClear (b, T);
      tr = RANK (b);
      co = FILE (b);
      EE ("bP%c%c%c%d:%d ", 'a' + co, '1' + tr,
	  AttPw[b] & AttK[wKs] ? '*' : ' ', OpEg (PST (bEnumP, b)));
      DistanceBlackKing = BlackKingPawnDistance (b, bKs);
      if (DistanceBlackKing < BestBlackKingDistance)
	BestBlackKingDistance = DistanceBlackKing;
      DistanceWhiteKing = BlackKingPawnDistance (b, wKs);
      if (DistanceWhiteKing < BestWhiteKingDistance)
	BestWhiteKingDistance = DistanceWhiteKing;

      if (SqSet[b] & LIGHT)
	{
	  RESULT->bPlight += BlockedPawnValue[b];
	  if (POSITION->sq[b - 8] == wEnumP)
	    RESULT->bPlight += BlockedPawnValue[b];
	}
      else
	{
	  RESULT->bPdark += BlockedPawnValue[b];
	  if (POSITION->sq[b - 8] == wEnumP)
	    RESULT->bPdark += BlockedPawnValue[b];
	}

      if (bBitboardP & LEFT2[b]
	  && (bBitboardP & InFrontB[tr + 1] & FileArray[co - 1]) == 0)
	{
	  Value += Hole;
          EE ("hole %d:%d ", OpEg (Hole));
	}

      if ((wBitboardP | bBitboardP) & OpenFileB[b])
	{
	  if (bBitboardP & DOUBLED[b])
	    {
	      Value += DoubledClosed;
              EE ("doub cl %d:%d ", OpEg (DoubledClosed));
	      if ((bBitboardP & IsolatedFiles[co]) == 0)
		{
		  Value += DoubledClosedIsolated;
                  EE ("isol %d:%d ", OpEg (DoubledClosedIsolated));
		}
	    }
	  if ((bBitboardP & IsolatedFiles[co]) == 0)
	    {
	      Value += IsolatedClosed;
              EE ("isol cl %d:%d\n", OpEg (IsolatedClosed));
	      continue;
	    }
	  if ((bBitboardP & ProtectedPawnB[b]) == 0)
	    {
	      B = b - 8;
	      if ((bBitboardP & AttPw[b]) == 0)
		{
		  B -= 8;
		  if ((bBitboardP & AttPw[b - 8]) ==
		      0)
		    B -= 8;
		}
	      if (wBitboardP & AttPw[B])
		{
		  Value += BackwardClosed;
                  EE ("back cl %d:%d ", OpEg (BackwardClosed));
		}
	    }
          EE ("\n");
	  continue;
	}

      if (bBitboardP & DOUBLED[b])
	{
	  Value += DoubledOpen;
	  EE ("doub op %d:%d ", OpEg (DoubledOpen));
	  if ((bBitboardP & IsolatedFiles[co]) == 0)
	    {
	      Value += DoubledOpenIsolated;
              EE ("doub op isol %d:%d ", OpEg (DoubledOpenIsolated));
	    }
	}
      if ((bBitboardP & IsolatedFiles[co]) == 0)
	{
	  Value += IsolatedOpen;
          EE ("isol op %d:%d ", OpEg (IsolatedOpen));
	}
      else
	{
	  if ((bBitboardP & ProtectedPawnB[b]) == 0)
	    {
	      B = b - 8;
	      if ((bBitboardP & AttPw[b]) == 0)
		{
		  B -= 8;
		  if ((bBitboardP & AttPw[b - 8]) == 0)
		    B -= 8;
		}
	      if (wBitboardP & AttPw[B])
		{
		  Value += BackwardOpen;
                  EE ("back op %d:%d ", OpEg (BackwardOpen));
		}
	    }
	}

      if ((wBitboardP & PassedPawnB[b]) == 0)
	goto PassedB;
      if (wBitboardP & PassedPawnB[b] & ~AttPw[b])
	{
	  Value -= CandidatePawnValue[7 - tr];
	  if (CandidatePawnValue[7 - tr])
	    EE ("cand -%d:-%d\n", OpEg (CandidatePawnValue[7 - tr]));
	  else
	    EE ("\n");
	  continue;
	}
      if (POPCNT (AttPw[b] & wBitboardP) > POPCNT (AttPb[b] & bBitboardP))
	{
	  Value -= CandidatePawnValue[7 - tr];
	  if (CandidatePawnValue[7 - tr])
	    EE ("cand -%d:-%d\n", OpEg (CandidatePawnValue[7 - tr]));
	  else
	    EE ("\n");
	  continue;
	}
    PassedB:
      ValuePassedPawn = PassedPawnValue[7 - tr];
      EE ("\n    passed -%d:-%d ", OpEg (PassedPawnValue[7 - tr]));
      if (bBitboardP & AttPb[b])
	{
	  ValuePassedPawn += ProtectedPassedPawnValue[7 - tr];
	  if (ProtectedPassedPawnValue[7 - tr])
	    EE ("prot -%d:-%d ", OpEg (ProtectedPassedPawnValue[7 - tr]));
	}
      if ((wBitboardP & FilesLeft[co]) == 0
	  || (wBitboardP & FilesRight[co]) == 0)
	{
	  ValuePassedPawn += OutsidePassedPawnValue[7 - tr];
	  if (OutsidePassedPawnValue[7 - tr])
	    EE ("out -%d:-%d ", OpEg (OutsidePassedPawnValue[7 - tr]));
	}

      V = ConnectedPawns[b] & CONNECTED;
      CONNECTED |= SqSet[b];
      if (V)
	{
	  int value = ConnectedPassedPawnValue[7 - tr] +
	    ConnectedPassedPawnValue[7 - (BSF (V) >> 3)];
	  ValuePassedPawn -= value;
	  if (value)
	    EE ("conn -%d:-%d ", OpEg (value));
	  BitClear (0, V);
	  if (V)
	    {
	      value = ConnectedPassedPawnValue[7 - tr] +
                ConnectedPassedPawnValue[7 - (BSF (V) >> 3)];
	      if (value)
		ValuePassedPawn -= value;
	      EE ("conn -%d:-%d ", OpEg (value));
	    }
	}

      Value -= ValuePassedPawn;
      if (ValuePassedPawn)
	EE ("= -%d:-%d ", OpEg (ValuePassedPawn));
      RESULT->bPassedFiles |= (uint8) (1 << co);
      if (b >= A6)
	{
	  EE ("\n");
	  continue;
	}

      Value -= (BlackKingPawnDistance (b - 8, wKs) *
		OppKingPawnDistance[R8 - RANK (b)]);
      EE (" oppK -%d:-%d ", OpEg (BlackKingPawnDistance (b - 8, wKs) *
				  OppKingPawnDistance[R8 - RANK (b)]));
      Value += (BlackKingPawnDistance (b - 8, bKs) *
		MyKingPawnDistance[R8 - RANK (b)]);
      EE (" myK %d:%d\n", OpEg (BlackKingPawnDistance (b - 8, bKs) *
				MyKingPawnDistance[R8 - RANK (b)]));
    }

  /******/ /******/  /******/ /******/   /******/ /******/  /******/ /******/

  T = 0;
  for (tr = R2; tr <= R7; tr++)
    T |= ((wBitboardP >> (8 * tr)) & 0xff);
  U = 0;
  for (tr = R2; tr <= R7; tr++)
    U |= ((bBitboardP >> (8 * tr)) & 0xff);
  RESULT->wPfile_count = POPCNT (T);
  RESULT->bPfile_count = POPCNT (U);
  RESULT->OpenFileCount = 8 - POPCNT (T | U);
  RESULT->wDrawWeight = OpposingPawnsMult[POPCNT (T & ~U)] *
                         PawnCountMult[RESULT->wPfile_count];
  RESULT->bDrawWeight = OpposingPawnsMult[POPCNT (U & ~T)] *
                         PawnCountMult[RESULT->bPfile_count];
  EE ("DrawWeight white:%d black:%d\n",
      RESULT->wDrawWeight, RESULT->bDrawWeight);

  if (wBitboardP | bBitboardP)
    {
      EE ("Distance %d-%d -> 0:%d\n",
	  BestBlackKingDistance, BestWhiteKingDistance,
	  BestBlackKingDistance - BestWhiteKingDistance);
      Value += BestBlackKingDistance - BestWhiteKingDistance;
    }

  T = ((bBitboardP & (~FILEa)) >> 9) | ((bBitboardP & (~FILEh)) >> 7);
  EE ("wK ");
  if ((~T) & AttK[wKs] & bBitboardP)
    {
      EE ("attP %d:%d ", OpEg (KingAttPawn));
      Value += KingAttPawn;
    }
  if (POSITION->DYN->oo & 1)
    {
      EE ("OO %d:%d ", OpEg (KingOO));
      Value += KingOO;
    }
  if (POSITION->DYN->oo & 2)
    {
      EE ("OOO %d:%d ", OpEg (KingOOO));
      Value += KingOOO;
    }
  EE ("\n");
  T = ((wBitboardP & (~FILEa)) << 7) | ((wBitboardP & (~FILEh)) << 9);
  EE ("bK ");
  if ((~T) & AttK[bKs] & wBitboardP)
    {
      EE ("attP -%d:-%d ", OpEg (KingAttPawn));
      Value -= KingAttPawn;
    }
  if (POSITION->DYN->oo & 4)
    {
      EE ("OO -%d:-%d ", OpEg (KingOO));
      Value -= KingOO;
    }
  if (POSITION->DYN->oo & 8)
    {
      EE ("OOO -%d:-%d ", OpEg (KingOOO));
      Value -= KingOOO;
    }
  EE ("\n");
  RESULT->SCORE = Value;
  EE ("*** PAWNS *** (not with PST) %d:%d\n", OpEg (RESULT->SCORE));

  v = WhiteKingDanger (POSITION, wKs);
  EE ("wK shelter %d ", v);
  if (WhiteOO)
    {
      v = MIN (v, 5 + WhiteKingDanger (POSITION, G1));
      EE ("g1:%d ", 5 + WhiteKingDanger (POSITION, G1));
    }
  if (WhiteOOO)
    {
      v = MIN (v, 5 + WhiteKingDanger (POSITION, C1));
      EE ("c1:%d ", 5 + WhiteKingDanger (POSITION, C1));
    }
  EE ("\n");
  RESULT->wKdanger = SCORE (v, 0);
  v = BlackKingDanger (POSITION, bKs);
  EE ("bK shelter %d ", v);
  if (BlackOO)
    {
      v = MIN (v, 5 + BlackKingDanger (POSITION, G8));
      EE ("g8:%d ", 5 + BlackKingDanger (POSITION, G8));
    }
  if (BlackOOO)
    {
      v = MIN (v, 5 + BlackKingDanger (POSITION, C8));
      EE ("c8:%d ", 5 + BlackKingDanger (POSITION, C8));
    }
  EE ("\n");
  RESULT->bKdanger = SCORE (v, 0);

  RESULT->PAWN_HASH = POSITION->DYN->PAWN_HASH;
#if 1 /* HYATT */
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x1)[0];
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x2)[0];
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x3)[0];
#endif
  ptr = PawnHash + (POSITION->DYN->PAWN_HASH & (PawnHashSize - 1));
  memcpy (ptr, RESULT, sizeof(typePawnEval));
#if 1 /* HYATT */
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x1)[0];
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x2)[0];
  RESULT->PAWN_HASH ^= (((uint64*) (RESULT)) + 0x3)[0];
#endif
  return;
}
#endif /* UTILITIES */

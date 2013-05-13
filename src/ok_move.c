#ifndef BUILD_ok_move
#define BUILD_ok_move
#include "RobboLito.h"

#include "ok_move.c"
#include "white.h"
#else
#include "black.h"
#endif

/* attend Chess960 */
boolean MyOK (typePOS* POSITION, uint32 move)
{
  int fr, to, pi, cp, i;
  uint64 toSet;
  to = TO (move);
  toSet = SqSet[to];
  fr = FROM (move);
  pi = POSITION->sq[fr];
  if (pi == 0)
    return FALSE;
  if (PIECE_IS_OPP (pi))
    return FALSE;
  cp = POSITION->sq[to];
  if (cp == EnumOppK)
    return FALSE;
#ifdef CHESS_960
  if (UCI_OPTION_CHESS_960) /* attend Chess960 */
    {
      if (!MoveIsOO (move) && cp && PIECE_IS_MINE (cp))
	return FALSE;
      if (MoveIsOO (move))
	{
	  if (pi != EnumMyK || cp != EnumMyR || FILE (fr) != CHESS_960_KING_FILE)
	    return FALSE;
	  if (to > fr) /* OO */
	    {
	      if (!CastleOO || FILE (to) != CHESS_960_KR_FILE)
		return FALSE;
	      for (i = CHESS_960_KR_FILE + 1; i <= FF; i++)
		{
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] == EnumMyK)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	      for (i = CHESS_960_KR_FILE - 1; i >= FF; i--)
		{
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] == EnumMyK)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	      for (i = CHESS_960_KING_FILE + 1; i <= FG; i++)
		{
		  if (OppAttacked & SqSet[i + 8 * NUMBER_RANK1])
		    return FALSE;
		  if (i == CHESS_960_KR_FILE)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	    }
	  if (to < fr) /* ooo */
	    {
	      if (!CastleOOO || FILE (to) != CHESS_960_QR_FILE)
		return FALSE;
	      for (i = CHESS_960_QR_FILE + 1; i <= FD; i++)
		{
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] == EnumMyK)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	      for (i = CHESS_960_QR_FILE - 1; i >= FD; i--)
		{
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] == EnumMyK)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	      for (i = CHESS_960_KING_FILE + 1; i <= FC; i++)
		{
		  if (OppAttacked & SqSet[i + 8 * NUMBER_RANK1])
		    return FALSE;
		  if (i == CHESS_960_QR_FILE)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	      for (i = CHESS_960_KING_FILE - 1; i >= FC; i--)
		{
		  if (OppAttacked & SqSet[i + 8 * NUMBER_RANK1])
		    return FALSE;
		  if (i == CHESS_960_QR_FILE)
		    continue;
		  if (POSITION->sq[i + 8 * NUMBER_RANK1] != 0)
		    return FALSE;
		}
	    }
	  return TRUE;
	}
    }	  
#endif
  if (cp && PIECE_IS_MINE (cp))
    return FALSE;
  if (pi == EnumMyP)
    {
      if (EIGHTH_RANK (to) && !MoveIsProm (move))
	return FALSE;
      if (MoveIsEP (move) && to == POSITION->DYN->ep &&
	  (fr == BACK_LEFT (to) || fr == BACK_RIGHT (to)))
	return TRUE;
      if (fr == BACK_LEFT (to) || fr == BACK_RIGHT (to))
	{
	  if (toSet & OppOccupied)
	    return TRUE;
	  return FALSE;
	}
      if (fr == BACKWARD (to))
	{
	  if ((toSet & POSITION->OccupiedBW) == 0)
	    return TRUE;
	  return FALSE;
	}
      if (fr != BACKWARD2 (to) || RANK (fr) != NUMBER_RANK2)
	return FALSE;
      if (POSITION->OccupiedBW & SqSet[FORWARD (fr)])
	return FALSE;
      return TRUE;
    }
  if (MoveIsProm (move))
    return FALSE;
  if (pi == EnumMyN)
    {
      if (AttN[fr] & toSet)
	return TRUE;
      return FALSE;
    }
  if (pi == EnumMyBL || pi == EnumMyBD)
    {
      if (AttB (fr) & toSet)
	return TRUE;
      return FALSE;
    }
  if (MoveIsOO (move)) /* attend Chess960, unto the aboveing */
    {
      if (to == WHITE_G1)
	{
	  if (!CastleOO || POSITION->OccupiedBW & WHITE_F1G1
	      || OppAttacked & WHITE_F1G1)
	    return FALSE;
	  return TRUE;
	}
      if (to == WHITE_C1)
	{
	  if (!CastleOOO || POSITION->OccupiedBW & WHITE_B1C1D1
	      || OppAttacked & WHITE_C1D1)
	    return FALSE;
	  return TRUE;
	}
    }
  if (pi == EnumMyR)
    {
      if (AttR (fr) & toSet)
	return TRUE;
      return FALSE;
    }
  if (pi == EnumMyQ)
    {
      if (AttQ (fr) & toSet)
	return TRUE;
      return FALSE;
    }
  if (pi == EnumMyK)
    {
      if (AttK[fr] & toSet && (toSet & OppAttacked) == 0)
	return TRUE;
      return FALSE;
    }
  return FALSE;
}

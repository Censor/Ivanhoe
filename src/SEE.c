#ifndef BUILD_SEE
#define BUILD_SEE
#include "RobboLito.h"
typedef enum
{ ValueP = 100, ValueN = 325, ValueB = 325,
  ValueR = 500, ValueQ = 975, ValueKing = 12345 } EnumValues;
static const int Value[16] =
  { 0, ValueP, ValueN, 12345678, ValueB, ValueB, ValueR, ValueQ,
    0, ValueP, ValueN, 12345678, ValueB, ValueB, ValueR, ValueQ
  };

#include "SEE.c"
#include "white.h"
#else
#include "black.h"
#endif

boolean MySEE (typePOS* POSITION, uint32 move)
{
  int fr, to, PieceValue, CaptureValue, d, dir;
  uint64 bit, cbf, mask, TableIndex[4], gcm = 0, T;
  int b, w;
  T = MyXRAY & OppOccupied;
  fr = FROM (move);
  to = TO (move);
  while (T)
    {
      b = BSF (T);
      w = MyXrayTable[b];
      BitClear (b, T);
      if (fr != w && Line (to, b) != Line (b, OppKingSq))
	gcm |= SqSet[b];
    }
  gcm = ~gcm;
  PieceValue = Value[POSITION->sq[fr]];
  CaptureValue = Value[POSITION->sq[to]];
  if (PieceValue - CaptureValue > ValueP
      && OppAttackedPawns[to] & BitboardOppP & gcm)
    return FALSE;
  bit = (BitboardMyN | (BitboardOppN & gcm)) & AttN[to];
  d = PieceValue - CaptureValue;
  if (d > ValueN && BitboardOppN & bit)
    return FALSE;
  mask = BitboardMyQ | BitboardMyB | ((BitboardOppQ | BitboardOppB) & gcm);
  mask &= DIAG[to];
  TableIndex[Direction_h1a8] = TableIndex[Direction_a1h8] = mask;
  bit |= AttB (to) & mask;
  if (d > ValueB && (BitboardOppB & bit))
    return FALSE;
  mask = BitboardMyQ | BitboardMyR | ((BitboardOppQ | BitboardOppR) & gcm);
  mask &= ORTHO[to];
  TableIndex[Direction_horz] = TableIndex[Direction_vert] = mask;
  bit |= AttR (to) & mask;
  bit |= (BitboardMyK | BitboardOppK) & AttK[to];
  bit |= BitboardOppP & OppAttackedPawns[to] & gcm;
  bit |= BitboardMyP & MyAttackedPawns[to];
  cbf = ~(SqSet[fr] | SqSet[to]);
  bit &= cbf;
  dir = Line (fr, to);
  if (dir == Direction_h1a8 || dir == Direction_a1h8)
    bit |= AttB (fr) & TableIndex[dir] & cbf;
  if (dir == Direction_horz || dir == Direction_vert)
    bit |= AttR (fr) & TableIndex[dir] & cbf;
  CaptureValue -= PieceValue;
  do
    {
      cbf &= ~bit;
      mask = BitboardOppP & bit;
      if (mask)
	{
	  bit ^= (~(mask - 1)) & mask;
	  PieceValue = ValueP;
	}
      else
	{
	  mask = BitboardOppN & bit;
	  if (mask)
	    {
	      bit ^= (~(mask - 1)) & mask;
	      PieceValue = ValueN;
	    }
	  else
	    {
	      mask = BitboardOppB & bit;
	      if (mask)
		{
		  PieceValue = ValueB;
		  fr = BSF (mask);
		  mask = AttB (fr) & cbf & TableIndex[Direction_a1h8];
		  bit = mask | (SqClear[fr] & bit);
		}
	      else
		{
		  mask = BitboardOppR & bit;
		  if (mask)
		    {
		      PieceValue = ValueR;
		      fr = BSF (mask);
		      mask = AttR (fr) & cbf & TableIndex[Direction_horz];
		      bit = mask | (SqClear[fr] & bit);
		    }
		  else
		    {
		      mask = BitboardOppQ & bit;
		      if (mask)
			{
			  PieceValue = ValueQ;
			  fr = BSF (mask);
			  if (RANK (fr) == RANK (to) || FILE (fr) == FILE (to))
			    mask = AttR (fr) & cbf & TableIndex[Direction_horz];
			  else
			    mask = AttB (fr) & cbf & TableIndex[Direction_a1h8];
			  bit = mask | (SqClear[fr] & bit);
			}
		      else
			{
			  if (!(BitboardOppK & bit))
			    return TRUE;
			  PieceValue = 12345;
			}
		    }
		}
	    }
	}
      CaptureValue += PieceValue;
      if (CaptureValue < -60)
	return FALSE;
      mask = BitboardMyP & bit;
      if (mask)
	{
	  bit ^= (~(mask - 1)) & mask;
	  PieceValue = ValueP;
	}
      else
	{
	  mask = BitboardMyN & bit;
	  if (mask)
	    {
	      bit ^= (~(mask - 1)) & mask;
	      PieceValue = ValueN;
	    }
	  else
	    {
	      mask = BitboardMyB & bit;
	      if (mask)
		{
		  PieceValue = ValueB;
		  fr = BSF (mask);
		  mask = AttB (fr) & cbf & TableIndex[Direction_a1h8];
		  bit = mask | (SqClear[fr] & bit);
		}
	      else
		{
		  mask = BitboardMyR & bit;
		  if (mask)
		    {
		      PieceValue = ValueR;
		      fr = BSF (mask);
		      mask = AttR (fr) & cbf & TableIndex[Direction_horz];
		      bit = mask | (SqClear[fr] & bit);
		    }
		  else
		    {
		      mask = BitboardMyQ & bit;
		      if (mask)
			{
			  PieceValue = ValueQ;
			  fr = BSF (mask);
			  if (RANK (fr) == RANK (to) || FILE (fr) == FILE (to))
			    mask = AttR (fr) & cbf & TableIndex[Direction_horz];
			  else
			    mask = AttB (fr) & cbf & TableIndex[Direction_a1h8];
			  bit = mask | (SqClear[fr] & bit);
			}
		      else
			{
			  if (!(BitboardMyK & bit))
			    return FALSE;
			  if (CaptureValue > 6174)
			    return TRUE;
			  PieceValue = 23456;
			}
		    }
		}
	    }
	}
      CaptureValue -= PieceValue;
    }
  while (CaptureValue < -60);
  return TRUE;
}

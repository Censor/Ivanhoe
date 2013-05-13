
#include "RobboLito.h"

#ifdef CONCORD_REV_MOVES
int RevMoveSetup (int r)
{
  int t = REV_MOVES;
  int i;
  if (t > 0 && t < 8)
    t = 8;
  if (t > 8 && t < 16)
    t = 16;
  if (t > 16 && t < 32)
    t = 32;
  ZobristRev[0] = 0ULL;
  if (t == 0)
    for (i = 1; i < 16; i++)
      ZobristRev[i] = 0ULL;
  if (t == 8)
    for (i = 1; i < 16; i++)
      ZobristRev[i] = GET_RAND ();
  if (t == 16)
    {
      for (i = 2; i < 16; i += 2)
	ZobristRev[i] = GET_RAND ();
      for (i = 1; i < 16; i += 2)
	ZobristRev[i] = ZobristRev[i - 1];
    }
  if (t == 32)
    {
      for (i = 4; i < 16; i += 4)
	ZobristRev[i] = GET_RAND ();
      for (i = 1; i < 16; i += 4)
	ZobristRev[i] = ZobristRev[i - 1];
      for (i = 2; i < 16; i += 4)
	ZobristRev[i] = ZobristRev[i - 1];
      for (i = 3; i < 16; i += 4)
	ZobristRev[i] = ZobristRev[i - 1];
    }
  REV_MOVES = t;
  InitHash (CURRENT_HASH_SIZE);
  return t;
}
#endif

int Move50 (typePOS* POSITION)
{
  typeMoveList LIST[256];
  typeMoveList* p;
  EvasionMoves (POSITION, LIST, 0xffffffffffffffff);
  p = LIST;
  while (p->move)
    {
      Make (POSITION, p->move);
      Mobility (POSITION); /* jarkkop */
      if ( !POSITION->wtm ?
	   (wBitboardK & POSITION->DYN->bAtt) : 
	   (bBitboardK & POSITION->DYN->wAtt) )
	{
	  Undo (POSITION, p->move);
	  p++;
	  continue;
	}
      Undo (POSITION, p->move);
      return 0;
    }
  return 16 * HEIGHT (POSITION) - VALUE_MATE;
}

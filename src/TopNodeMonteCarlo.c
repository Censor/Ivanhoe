#ifdef MODE_ANALYSIS
#ifndef BUILD_top_node_monte_carlo
#define BUILD_top_node_monte_carlo
#include "RobboLito.h"
#include "MonteCarlo.h"

#define IsCheck \
 ( POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

#include "TopNodeMonteCarlo.c"
#include "white.h"
#else
#include "black.h"
#endif

int MyTopNodeMonteCarlo (typePOS* POSITION, int FIXED_DEPTH)
{
  int i, depth, A, L, U, v, Value = 0, trans_depth;
  int move_depth = 0, EXACT_DEPTH = 0;
  uint32 move, HASH_MOVE = 0, EXACT_MOVE = 0, to, fr;
  typeMoveList *mlx, *ml, ML[256];
  typeRootMoveList *p, *q, *list;
  typeRootMoveList ROOT_MOVE_LIST[256];
  TRANS_DECLARE();
  typeDYNAMIC* POS0 = POSITION->DYN;
  int PieceValue[16] = { 0, 1, 3, 0, 3, 3, 5, 9, 0, 1, 3, 0, 3, 3, 5, 9 };
  boolean tot, TRIPLE_PEEK = FALSE;
  int ROOT_VALUE;
  int TRIPLE_PEEK_VALUE = -VALUE_INFINITY;
  YUSUF_DECLARE ();

  Mobility (POSITION);
  if (IsCheck)
    ml = MyEvasion (POSITION, ML, 0xffffffffffffffff);
  else
    {
      mlx = MyCapture (POSITION, ML, OppOccupied);
      ml = MyOrdinary (POSITION, mlx);
      SortOrdinary (ml, mlx, 0, 0, 0);
    }

  Trans = HASH_POINTER (POSITION->DYN->HASH);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
	  trans_depth = trans->DepthLower;
	  move = trans->move;
	  if (IsExact (trans))
	    {
	      EXACT_DEPTH = trans_depth;
	      EXACT_MOVE = move;
	      Value = HashUpperBound (trans);
	    }
	  if (move && trans_depth > move_depth)
	    {
	      move_depth = trans_depth;
	      HASH_MOVE = move;
	    }
	}
    }

  for (i = 0; i < ml - ML; i++)
    ROOT_MOVE_LIST[i].move = ML[i].move;
  ROOT_MOVE_LIST[ml - ML].move = MOVE_NONE;
  list = ROOT_MOVE_LIST + (ml - ML);

#ifdef CON_ROBBO_BUILD
  if (ROBBO_LOAD && ROBBO_TRIPLE_CONDITION (POSITION) &&
      TRIPLE_VALUE (POSITION, &v, &tot, FALSE, FALSE, TRUE))
    {
      TRIPLE_PEEK = TRUE;
      TRIPLE_PEEK_VALUE = v;
    }
#endif
  q = ROOT_MOVE_LIST;
  for (p = ROOT_MOVE_LIST; p < list; p++)
    {
      move = p->move & 0x7fff;
      Make (POSITION, move);
      Mobility (POSITION);
      if (ILLEGAL_MOVE)
	{
	  UNDO (POSITION, move);
	  continue;
	}      
#ifdef CON_ROBBO_BUILD
      else if (ROBBO_LOAD && TRIPLE_PEEK &&
	       TRIPLE_VALUE (POSITION, &v, &tot, FALSE, FALSE, TRUE))
	{
	  v = -v; /* inverso */
	  if (TRIPLE_PEEK_VALUE > 0 && v > 0)
	    (q++)->move = move & 0x7fff;
	  if (TRIPLE_PEEK_VALUE == 0 && v >= 0)
	    {
	      if (v > 0)
		TRIPLE_PEEK_VALUE = 1;
	      (q++)->move = move & 0x7fff;
	    }
	  if (TRIPLE_PEEK_VALUE < 0)
	    {
	      (q++)->move = move & 0x7fff;
	      if (v > 0)
		TRIPLE_PEEK_VALUE = 1;
	      if (v == 0)
		TRIPLE_PEEK_VALUE = 0;
	    }
	}
#endif      
      else
	(q++)->move = move & 0x7fff;
      UNDO (POSITION, move);
    }
  q->move = 0;
  list = q;

#ifdef CON_ROBBO_BUILD
  q = ROOT_MOVE_LIST;
  for (p = ROOT_MOVE_LIST; p < list; p++)
    {
      move = p->move & 0x7fff;
      Make (POSITION, move);
      Mobility (POSITION);
      if (ROBBO_LOAD && TRIPLE_PEEK &&
	  TRIPLE_VALUE (POSITION, &v, &tot, FALSE, FALSE, TRUE))
	{
	  v = -v; /* inverso */
	  if (TRIPLE_PEEK_VALUE > 0 && v > 0)
	    (q++)->move = move & 0x7fff;
	  if (TRIPLE_PEEK_VALUE == 0 && v >= 0)
	    (q++)->move = move & 0x7fff;
	  if (TRIPLE_PEEK_VALUE < 0)
	    (q++)->move = move & 0x7fff;
	}
      else
	(q++)->move = move & 0x7fff;
      UNDO (POSITION, move);
    }
  q->move = 0;
  list = q;
#endif      

  for (p = ROOT_MOVE_LIST; p < list; p++)
    {
      if (POSITION->sq[TO (p->move)])
	{
	  to = POSITION->sq[TO (p->move)];
	  fr = POSITION->sq[FROM (p->move)];
	  p->move |=
	    0xff000000 + ((16 * PieceValue[to] - PieceValue[fr]) << 16);
	}
    }
  for (p = ROOT_MOVE_LIST; p < list; p++)
    if (p->move == HASH_MOVE)
      p->move |= 0xffff0000;
  for (p = list - 1; p >= ROOT_MOVE_LIST; p--)
    {
      move = p->move;
      for (q = p + 1; q < list; q++)
	{
	  if (move < q->move)
	    (q - 1)->move = q->move;
	  else
	    break;
	}
      q--;
      q->move = move;
    }

  L = -VALUE_MATE;
  U = VALUE_MATE;
  if (!ROOT_MOVE_LIST[0].move)
    {
      POSITION->DYN->move = MOVE_NONE;
      if (IsCheck)
	return -VALUE_MATE;
      else
	return 0;
    }
  ROOT_VALUE = 0;
  for (depth = 2; depth <= 2 * FIXED_DEPTH; depth += 2)
    {
      if (depth >= 14 && ROOT_VALUE <= 25000 && -25000 <= ROOT_VALUE)
	{
	  A = 8;
	  L = ROOT_VALUE - A;
	  U = ROOT_VALUE + A;
	  if (L < -25000)
	    L = -VALUE_MATE;
	  if (U > 25000)
	    U = VALUE_MATE;
	AGAIN:
	  v = MyRootNodeMonteCarlo (POSITION, ROOT_MOVE_LIST, L, U, depth);
	  if (MONTE_CARLO_CURTAIL)
	    goto CHECK_DONE;
	  if (v > L && v < U)
	    goto CHECK_DONE;
	  if (v <= L)
	    {
	      ROOT_VALUE = L;
	      L -= A;
	      A += A / 2;
	      goto AGAIN;
	    }
	  else
	    {
	      ROOT_VALUE = U;
	      U += A;
	      A += A / 2;
	      goto AGAIN;
	    }
	}
      else
	v = MyRootNodeMonteCarlo
	  (POSITION, ROOT_MOVE_LIST, -VALUE_MATE, VALUE_MATE, depth);
    CHECK_DONE:
      if (MONTE_CARLO_CURTAIL)
	return 0;
      ROOT_VALUE = v;
    }
  return ROOT_VALUE;
}
#endif

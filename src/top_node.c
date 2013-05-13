#ifndef BUILD_top_node
#define BUILD_top_node
#include "RobboLito.h"

#include "control.h"
#define IsCheck \
 ( POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

typeRootMoveList ROOT_MOVE_LIST[256];
#ifndef MINIMAL
#define AlwaysAnalyze() (ALWAYS_ANALYZE)
#else
#define AlwaysAnalyze() (FALSE)
#endif

#include "top_node.c"
#include "white.h"
#else
#include "black.h"
#endif

void MyTop (typePOS* POSITION)
{
  int i, depth, A, L, U, v, Value = 0, trans_depth;
  int move_depth = 0, EXACT_DEPTH = 0;
  uint32 move, HASH_MOVE = 0, EXACT_MOVE = 0, to, fr;
  typeMoveList *mlx, *ml, ML[256];
  typeRootMoveList *p, *q, *list;
  TRANS_DECLARE();
  typeDYNAMIC* POS0 = POSITION->DYN;
  int PieceValue[16] = { 0, 1, 3, 0, 3, 3, 5, 9, 0, 1, 3, 0, 3, 3, 5, 9 };
  boolean tot, TRIPLE_PEEK = FALSE;
  int TRIPLE_PEEK_VALUE = -VALUE_INFINITY;
  YUSUF_DECLARE ();

  if (ANALYSING || AlwaysAnalyze ())
    {
      MyTopAnalysis (POSITION);
      return;
    }
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

 /* "PREVIOUS_FAST" is inverse named plus don't too this with UCI_PONDER */
  if (EXACT_DEPTH >= PREVIOUS_DEPTH - 6 && EXACT_MOVE == HASH_MOVE
      && !UCI_PONDER && EXACT_MOVE && PREVIOUS_FAST && PREVIOUS_DEPTH >= 18
      && ALLOW_INSTANT_MOVE && MyOK (POSITION, EXACT_MOVE)
      && Value < 25000 && Value > -25000)
    {
      ROOT_SCORE = Value;
      ROOT_BEST_MOVE = EXACT_MOVE;
      ROOT_DEPTH = EXACT_DEPTH;
      PREVIOUS_FAST = FALSE;
      if (!IsCheck)
	v = MyExclude (POSITION, Value - 50, PREVIOUS_DEPTH - 6, EXACT_MOVE);
      else
	v = MyExcludeCheck (POSITION, Value - 50, PREVIOUS_DEPTH - 6, EXACT_MOVE);
      if (v < Value - 50)
	return;
    }

  PREVIOUS_FAST = TRUE;
  for (i = 0; i < ml - ML; i++)
    ROOT_MOVE_LIST[i].move = ML[i].move;
  ROOT_MOVE_LIST[ml - ML].move = MOVE_NONE;
  list = ROOT_MOVE_LIST + (ml - ML);

  ROBBO_TRIPLE_DRAW = FALSE;
#ifdef CON_ROBBO_BUILD
  if (ROBBO_LOAD && ROBBO_TRIPLE_CONDITION (POSITION)
      && TRIPLE_VALUE (POSITION, &v, &tot, FALSE, FALSE, TRUE))
    {
      TRIPLE_PEEK = TRUE;
      TRIPLE_PEEK_VALUE = v;
      if (v == 0)
        ROBBO_TRIPLE_DRAW = TRUE;
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
      if (IsCheck)
	ROOT_SCORE = L;
      else
	ROOT_SCORE = 0;
      ROOT_BEST_MOVE = 0;
      ROOT_DEPTH = 0;
      return;
    }
  for (depth = 2; depth <= 250; depth += 2)
    {
      BAD_MOVE = FALSE;
      BATTLE_MOVE = FALSE;
      if (depth >= 14 && ROOT_SCORE <= 25000 && -25000 <= ROOT_SCORE)
	{
	  A = 8;
	  L = ROOT_SCORE - A;
	  U = ROOT_SCORE + A;
	AGAIN:
	  if (L < -25000)
	    L = -VALUE_MATE;
	  if (U > 25000)
	    U = VALUE_MATE;
	  v = MyRootNode (POSITION, L, U, depth);
	  if (IVAN_ALL_HALT)
	    goto CHECK_DONE;
	  if (v > L && v < U)
	    goto CHECK_DONE;
	  if (v <= L)
	    {
	      ROOT_SCORE = L;
	      L -= A;
	      A += A / 2;
	      goto AGAIN;
	    }
	  else
	    {
	      ROOT_SCORE = U;
	      U += A;
	      A += A / 2;
	      goto AGAIN;
	    }
	}
      else
	v = MyRootNode (POSITION, -VALUE_MATE, VALUE_MATE, depth);
    CHECK_DONE:
      if (depth == 2)
	EASY_MOVE = TRUE;
      ROOT_PREVIOUS = ROOT_SCORE;
      if (IVAN_ALL_HALT)
	return;
      CheckDone (POSITION, depth / 2);
      if (IVAN_ALL_HALT)
	return;
    }
}

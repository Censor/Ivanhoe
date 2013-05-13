#ifndef BUILD_top_analysis
#define BUILD_top_analysis
#include "RobboLito.h"

#include "control.h"
#define IsCheck \
 ( POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

typeRootMoveList ROOT_MOVE_LIST[256];
#include "top_analysis.c"
#include "white.h"
#else
#include "black.h"
#endif

void MyTopAnalysis (typePOS* POSITION)
{
  int i, depth, A, L, U, v, Value = 0, trans_depth;
  int  move_depth = 0, EXACT_DEPTH = 0;
  uint32 move, HASH_MOVE = 0, EXACT_MOVE = 0, to, fr;
  typeMoveList *mlx, *ml, ML[256];
  typeRootMoveList *p, *q, *list;
  TRANS_DECLARE ();
  typeDYNAMIC* POS0 = POSITION->DYN;
  int PieceValue[16] = { 0, 1, 3, 0, 3, 3, 5, 9, 0, 1, 3, 0, 3, 3, 5, 9 };
  int sm = 0; /* searchmoves count */
  boolean tot, TRIPLE_PEEK = FALSE;
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

  ROBBO_TRIPLE_DRAW = FALSE;
#ifdef CON_ROBBO_BUILD
  if (ROBBO_LOAD && ROBBO_TRIPLE_CONDITION (POSITION) &&
      TRIPLE_VALUE (POSITION, &v, &tot, FALSE, FALSE, TRUE))
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
      if (ANALYSING && DO_SEARCH_MOVES)
	{
	  sm = 0;
	  while (SEARCH_MOVES[sm])
	    {
	      if (SEARCH_MOVES[sm] == move)
		{
		  (q++)->move = move & 0x7fff;
		  break;
		}
	      sm++;
	    }
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
	{
	  ROOT_SCORE = L;
	}
      else
	{
	  ROOT_SCORE = 0;
	}
      ROOT_BEST_MOVE = 0;
      ROOT_DEPTH = 0;
      return;
    }
  for (depth = 2; depth <= 250; depth += 2)
    {
      if (depth >= 14 && ROOT_SCORE <= 25000 && -25000 <= ROOT_SCORE && MULTI_PV == 1)
	{
	  A = 8;
	  L = ROOT_SCORE - A;
	  U = ROOT_SCORE + A;
	AGAIN:
	  if (L < -25000)
	    L = -VALUE_MATE;
	  if (U > 25000)
	    U = VALUE_MATE;
	  v = MyAnalysis (POSITION, L, U, depth);
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
	v = MyAnalysis (POSITION, -VALUE_MATE, VALUE_MATE, depth);
    CHECK_DONE:
      ROOT_PREVIOUS = ROOT_SCORE;
      if (IVAN_ALL_HALT)
	return;
      CheckDone (POSITION, depth / 2);
      if (IVAN_ALL_HALT)
	return;
    }
}

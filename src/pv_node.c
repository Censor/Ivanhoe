#ifndef BUILD_pv_node
#define BUILD_pv_node
#include "RobboLito.h"
#include "history.i"
#include "control.h"
#ifdef BUILD_ZOG_MP_COMPILE
void NEW_ZOG_NODE (typePOS*, int, int, int, type_zog_node*, uint32);
int TRY_ZOG_NODE (typePOS*, int, int, int, type_zog_node*, uint32);
typedef enum
  { NO_ZOG, ZOG_IS_BUSY, ZOG_MOVE_LESS, ZOG_MOVE_FORCED} zogs;
#define MIN_ZOG_DEPTH 10
#endif

#include "pv_node.c"
#include "white.h"
#else
#include "black.h"
#endif

#ifdef TRACE_COMPILE
#define RETURN(x) \
  { TRACE (TRACE_PV, \
    printf ("PV%cR %d dp:%d V:[%d,%d]\n", \
	    POSITION->wtm ? 'w' : 'b', (x), depth, ALPHA, BETA)); \
    return (x); }
#else
#define RETURN(x) return(x)
#endif

int MyPV (typePOS* POSITION, int ALPHA, int BETA,
	  int depth, int check, type_zog_node *parent)
{
  typeNEXT NextMove[1];
  TRANS_DECLARE();
  int good_move, v, Value, i, trans_depth, move,
    move_depth = 0, trans_move = 0, hash_depth;
  typeMoveList *list, *p, *q;
  int EXTEND, best_value, new_depth, move_is_check, to, fr;
  typeDYNAMIC* POS0 = POSITION->DYN;
  int SINGULAR, LMR, cnt, MEDIAN, MARGIN;
  boolean SPLIT, see;
  YUSUF_DECLARE ();
  TRACE (TRACE_PV,
         printf ("PV%c V:[%d,%d] dp:%d ev:%d\n", POSITION->wtm ? 'w' : 'b',
                 ALPHA, BETA, depth, POSITION->DYN->Value));

  if (BETA < -VALUE_MATE)
    RETURN (-VALUE_MATE);
  if (ALPHA > VALUE_MATE)
    RETURN (VALUE_MATE);
  MEDIAN = (BETA + ALPHA) / 2;
  MARGIN = (BETA - ALPHA);
  if (depth <= 1)
    {
      if (check)
	return MyPVQsearchCheck (POSITION, ALPHA, BETA, 1);
      else
	return MyPVQsearch (POSITION, ALPHA, BETA, 1);
    }
  CheckRepetition (check);
  NextMove->trans_move = 0;
  hash_depth = 0;
  NextMove->move = 0;
  NextMove->bc = 0;

  Trans = HASH_POINTER (POSITION->DYN->HASH);
  (POS0 + 1)->move = 0;
  /* usa PVHash ? */
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
          TRACE (TRACE_HASH && TRACE_PV, HASH_READ (trans));
	  if (trans->flags & FLAG_MOVE_LESS)
	    return (check) ? (16 * HEIGHT (POSITION) - VALUE_MATE) : 0;
	  trans_depth = trans->DepthLower; /* move_depth */
	  move = trans->move;
	  if (move && trans_depth > move_depth)
	    {
	      move_depth = trans_depth;
	      (POS0 + 1)->move = trans_move = move;
	    }
	  trans_depth = MAX (trans->DepthLower, trans->DepthUpper);
	  if (IsExact (trans) && trans_depth >= depth)
	    {
	      Value = HashLowerBound (trans); /* the either */
	      UPDATE_AGE();
	      if (!ANALYSING || !TRY_PV_IN_ANALYSIS)
		RETURN (Value);
	    }
	  if (trans_depth > hash_depth)
	    hash_depth = trans_depth;
	  if (trans->DepthLower > trans->DepthUpper)
	    {
	      Value = HashLowerBound (trans);
	      if (trans->flags & LOWER_FROM_PV &&
		  trans_depth >= depth && Value >= BETA)
		RETURN (BETA);
	    }
	  else
	    {
	      Value = HashUpperBound (trans);
	      if (trans->flags & UPPER_FROM_PV &&
		  trans_depth >= depth && Value <= ALPHA)
		RETURN (ALPHA);
	    }
	}
    }

#ifdef BUILD_ZOG_MP_COMPILE
  if (ZOG_AKTIV && trans_move && parent && POSITION->rp == 0)
    {
      int zog = TRY_ZOG_NODE (POSITION, ALPHA, BETA, depth, parent, trans_move);
      if (zog == NO_ZOG && parent->EXPANDED && depth >= MIN_ZOG_DEPTH)
        NEW_ZOG_NODE (POSITION, ALPHA, BETA, depth, parent, trans_move);
      if (zog == ZOG_MOVE_LESS)
	{
	  best_value = - VALUE_INFINITY;
	  goto MOVE_LESS;
	}
    }
#endif

  if (check && MoveIsOO (trans_move))
    trans_move = MOVE_NONE;
  if (!trans_move && depth >= 6) /* bad in if POSITION demurs legaled moves */
    {
      v = ALPHA;
      if (depth >= 10)
	{
	  v = MyPV (POSITION, ALPHA - depth, BETA + depth,
		    depth - 8, check, NULL);
	  CHECK_HALT ();
	  if (v > ALPHA - depth)
	    trans_move = (POS0 + 1)->move;
	}
      if (v > ALPHA - depth)
	v = MyPV (POSITION, ALPHA - depth, BETA + depth,
		  depth - 4, check, NULL);
      CHECK_HALT ();
      if (v > ALPHA - depth)
	trans_move = (POS0 + 1)->move;
    }
  else if (depth >= 10 && depth > hash_depth + 8)
    {
      v = MyPV (POSITION, ALPHA - depth, BETA + depth, depth - 8, check, NULL);
      CHECK_HALT ();
      if (v > ALPHA - depth)
	trans_move = (POS0 + 1)->move;
      if (v > ALPHA - depth)
	{
	  v = MyPV (POSITION, ALPHA - depth, BETA + depth, depth - 4, check, NULL);
	  CHECK_HALT ();
	  if (v > ALPHA - depth)
	    trans_move = (POS0 + 1)->move;
	}
    }

  NextMove->trans_move = trans_move;
  NextMove->phase = TRANS;
  EXTEND = 0;
  NextMove->TARGET = OppOccupied;

  SINGULAR = 0;
  if (check)
    {
      list = MyEvasion (POSITION, NextMove->LIST, 0xffffffffffffffff);
      NextMove->phase = EVADE_PHASE;
      for (p = list - 1; p >= NextMove->LIST; p--)
	{
	  if ((p->move & 0x7fff) == trans_move)
	    p->move |= 0xffff0000;
	  else if (p->move <= (0x80 << 24))
	    {
	      if ((p->move & 0x7fff) == POS0->killer1)
		p->move |= 0x7fff8000;
	      else if ((p->move & 0x7fff) == POS0->killer2)
		p->move |= 0x7fff0000;
	      else
		p->move |=
		  (p->move & 0x7fff) |
		  (HISTORY_VALUE (POSITION, p->move) << 15);
	    }
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
      if ((list - NextMove->LIST) <= 1)
	SINGULAR = 2;
      if ((list - NextMove->LIST) == 2)
	SINGULAR = 1;
      if ((list - NextMove->LIST) > 2)
	SINGULAR = 0;
    }

#define DEPTH_RED (MIN (12, depth / 2))
#define VALUE_RED1 (depth / 2)
#define VALUE_RED2 (depth)
  if (depth >= 16 && NextMove->trans_move && SINGULAR < 2
      && MyOK (POSITION, NextMove->trans_move))
    {
      move = NextMove->trans_move;
      if (check)
	v = MyExcludeCheck (POSITION, ALPHA - VALUE_RED1,
			    depth - DEPTH_RED, move & 0x7fff);
      else
	v = MyExclude (POSITION, ALPHA - VALUE_RED1,
		       depth - DEPTH_RED, move & 0x7fff);
      CHECK_HALT ();
      if (v < ALPHA - VALUE_RED1)
	{	
	  SINGULAR = 1;
	  if (check)
	    v = MyExcludeCheck (POSITION, ALPHA - VALUE_RED2,
				depth - DEPTH_RED, move & 0x7fff);
	  else
	    v = MyExclude (POSITION, ALPHA - VALUE_RED2,
			   depth - DEPTH_RED, move & 0x7fff);
	  CHECK_HALT ();
	  if (v < ALPHA - VALUE_RED2)
	    SINGULAR = 2;
	}
    }
  best_value = -VALUE_INFINITY;
  NextMove->move = 0;
  NextMove->bc = 0;
  good_move = 0;

  SPLIT = FALSE;
  cnt = 0;
  while (TRUE)
    {
#if 1 && defined (YUSUF_MULTICORE) /* nein check ? */ /* depth ya 10 ? */
      /* Personal Opinion: with the *increase* for the depth ! */
      /* now what is the best? decembrist testing for best params to find it! */
      if (TITANIC_MODE && SMP_FREE != 0 && !check &&
	  depth >= PV_SPLIT_DEPTH && !SPLIT && best_value != -VALUE_INFINITY)
	{
          int r;
	  boolean b;
          SPLIT = TRUE;
          b = IVAN_SPLIT
	    (POSITION, NextMove, depth, BETA, ALPHA, NODE_TYPE_PV, &r);
	  CHECK_HALT ();
	  if (b)
	    {
	      if (r > ALPHA || !good_move)
		return r;
	      move = good_move; /* SMP PV */
	      (POS0 + 1)->move = good_move & 0x7fff;
	      best_value = r;
	      goto IVAN;
	    }
        }
#endif
      move = MyNext (POSITION, NextMove);
      if (!move)
	break;
      to = TO (move);
      fr = FROM (move);
      if (ALPHA > 0 && POS0->reversible >= 2
	  && ((TO (move) << 6) | FROM (move)) == (POS0 - 1)->move
	  && POSITION->sq[TO (move)] == 0)
	{
	  best_value = MAX (0, best_value);
	  cnt++;
	  continue;
	}
      if (MoveIsEP (move) || POSITION->sq[fr] == EnumMyK)
	see = TRUE;
      else if (POSITION->sq[to] != 0)
	see = EasySEE (move) || MySEE (POSITION, move);
      else
	see = MySEE (POSITION, move);
      move &= 0x7fff;
      MAKE (POSITION, move);
      EVAL (move, depth);
      if (ILLEGAL_MOVE)
	{
	  UNDO (POSITION, move);
	  continue;
	}
      move_is_check = (MOVE_IS_CHECK != 0);

      EXTEND = 0;
      if (check)
	EXTEND = 1;
      if (see && (POS1->cp || move_is_check
		  || PassedPawnPush (to, FOURTH_RANK (to))))
	EXTEND = 1;
#if 0
      if ((((int) (POS1->OppKdanger)) - ((int) ((POS1 - 1)->OppKdanger))) > 5)
        EXTEND = 1;
      if ((((int) (POS1->OppKdanger)) - ((int) ((POS1 - 1)->OppKdanger))) > 10)
        EXTEND = 2;
#endif
      if (PassedPawnPush (to, SIXTH_RANK (to)))
	EXTEND = 2;
      if (see && POS1->cp && POS1->cp != EnumOppP)
	EXTEND = 2;
      if (POS1->cp && (POS1->material & 0xff) == 0 && POS1->cp != EnumOppP)
	EXTEND = 6; /* HACK */
      if (NextMove->trans_move != move)
	SINGULAR = 0;
      if (!EXTEND && cnt >= 3 && NextMove->phase == ORDINARY_MOVES
	  && MARGIN < 100 && depth >= 10)
	LMR = BSR (1 + cnt) - 1;
      else
	LMR = 0;
      new_depth = depth - 2 + MAX (EXTEND, SINGULAR) - LMR;
      if (IS_EXACT (POSITION->DYN))
        v = -POSITION->DYN->Value;
      else if (NextMove->trans_move != move && new_depth > 1)
	{
	  if (LOW_DEPTH_CONDITION_PV)
	    {
	      if (move_is_check)
		v = -OppLowDepthCheck (POSITION, -ALPHA, new_depth);
	      else
		v = -OppLowDepth (POSITION, -ALPHA, new_depth);
	    }
	  else
	    {
	      if (move_is_check)
		v = -OppCutCheck (POSITION, -ALPHA, new_depth);
	      else
		v = -OppCut (POSITION, -ALPHA, new_depth);
	    }
	  new_depth += LMR;
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -BETA, -ALPHA, new_depth, move_is_check, NULL);
	}
      else
	v = -OppPV (POSITION, -BETA, -ALPHA, new_depth, move_is_check, NULL);
      UNDO (POSITION, move);
      cnt++;
      CHECK_HALT ();
	  
      if (v <= ALPHA && POSITION->sq[TO (move)] == 0 && MoveHistory (move))
	HISTORY_BAD1 (move, depth);
      if (v <= best_value)
	continue;
      best_value = v;
      if (v <= ALPHA)
	continue;
      ALPHA = v;
      good_move = move;
      HashLowerPV (POSITION->DYN->HASH, move, depth, v);
      if (v >= BETA)
	{
	  if (POSITION->sq[TO (move)] == 0 && MoveHistory (move))
	    HISTORY_GOOD (move, depth);
	  RETURN (v);
	}
    }

  move = good_move;
  (POS0 + 1)->move = good_move & 0x7fff;
 MOVE_LESS:
  if (best_value == -VALUE_INFINITY) /* con SMP ? */
    {
      best_value = (check) ? (16 * HEIGHT (POSITION) - VALUE_MATE) : 0;
      HashExact (POSITION, MOVE_NONE, depth, best_value, FLAG_EXACT | FLAG_MOVE_LESS);
      RETURN (best_value);
    }
 IVAN:
  if (move)
    {
      if (POSITION->sq[TO (move)] == 0 && MoveHistory (move))
	HISTORY_GOOD (move, depth);
      HashExact (POSITION, move, depth, best_value, FLAG_EXACT);
      RETURN (best_value);
    }
  HashUpperPV (POSITION->DYN->HASH, depth, best_value);
  RETURN (best_value);
}

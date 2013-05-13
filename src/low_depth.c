#ifndef BUILD_low_depth
#define BUILD_low_depth
#include "RobboLito.h"

#include "history.i"
#include "null_move.i"
#include "low_depth.c"
#include "white.h"
#else
#include "black.h"
#endif

#ifdef TRACE_COMPILE
#define RETURN(x) \
  { TRACE (TRACE_LOW_DEPTH, \
    printf ("L%cR %d dp:%d V:%d\n", \
	    POSITION->wtm ? 'w' : 'b', (x), depth, VALUE)); \
    return (x); }
#else
#define RETURN(x) return(x)
#endif

int MyLowDepth (typePOS* POSITION, int VALUE, int depth)
{
  int cnt, Value, best_value, v, i, trans_move = 0;
  int move, move_depth = 0, trans_depth, to, fr;
  typeNEXT NextMove[1];
  typeDYNAMIC* POS0 = POSITION->DYN;
  TRANS_DECLARE ();
  YUSUF_DECLARE ();
  TRACE (TRACE_LOW_DEPTH,
         printf ("L%c V:%d dp:%d ev:%d\n", POSITION->wtm ? 'w' : 'b',
                 VALUE, depth, POSITION->DYN->Value));

  CHECK_FOR_MATE (VALUE);
  (POS0 + 1)->move = MOVE_NONE;
  CheckRepetition (FALSE);

  Trans = HASH_POINTER (POSITION->DYN->HASH);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
          TRACE (TRACE_HASH && TRACE_LOW_DEPTH, HASH_READ (trans));
	  if (trans->flags & FLAG_MOVE_LESS)
            return 0;
	  if (trans->DepthLower >= depth)
	    {
	      Value = HashLowerValue (trans);
              HashReBound (Value, VALUE);
	      if (Value >= VALUE)
		{
		  (POS0 + 1)->move = trans->move;
		  RETURN (Value);
		}
	    }
	  if (trans->DepthUpper >= depth)
	    {
	      Value = HashUpperValue (trans);
              HashReBound (Value, VALUE);
	      if (Value < VALUE)
		RETURN (Value);
	    }
	  trans_depth = trans->DepthLower;
	  move = trans->move;
	  if (move && trans_depth > move_depth)
	    {
	      move_depth = trans_depth;
	      trans_move = move;
	    }
	}
    }
  v = POS0->Value - (70 + 10 * depth); /* valid ? */
  if (v >= VALUE && VALUE >= -10000) /* consort MATEING */
    RETURN (POS0->Value);
  best_value = MIN (POS0->Value, VALUE - 1);
  if (NULL_MOVE_INTACT && POS0->Value >= VALUE && MyNull)
    {
      v = VALUE;
      if (v >= VALUE) /* TRUE */
	{
	  int new_depth = depth - NULL_REDUCTION;
	  new_depth -= SCORE_REDUCTION (POS0->Value - VALUE);
	  MakeNull (POSITION);
          if (QSEARCH_CONDITION) /* TRUE */
	    v = -OppQsearch (POSITION, 1 - VALUE, 0);
	  else
            v = -OppLowDepth (POSITION, 1 - VALUE, new_depth);
	  UndoNull (POSITION);
	  CHECK_HALT ();
	}
      if (VERIFY_NULL && v >= VALUE) /* value ? */
        {
          v = MyQsearch (POSITION, VALUE, 0); /* v >= VALUE, for the usual */
          CHECK_HALT ();
        }
      if (v >= VALUE)
	{
	  if (trans_move == MOVE_NONE)
	    HashLower (POSITION->DYN->HASH, 0, depth, v);
	  RETURN (v);
	}
    }
  NextMove->phase = TRANS;
  NextMove->TARGET = OppOccupied;
  if (POS0->Value + 50 + 8 * depth < VALUE) /* futile */
    {
      NextMove->phase = TRANS2; /* apply killer ? */
      if (VALUE >= POS0->Value + 75 + 32 * depth)
	{
	  NextMove->TARGET ^= BitboardOppP;
	  if (BitboardOppP & MyAttacked)
	    best_value += 125;
	  if (depth <= 3 && VALUE >= POS0->Value + 400 + 32 * depth)
	    {
	      NextMove->TARGET ^= (BitboardOppN | BitboardOppB);
	      best_value += 300;
	      if (VALUE >= POS0->Value + 600 + 32 * depth)
		{
		  NextMove->TARGET ^= BitboardOppR;
		  best_value += 200;
		}
	    }
	}
    }
  else if (depth <= 3 && POS0->Value + 4 * depth < VALUE)
    {
      NextMove->phase = TRANS3;
      NextMove->mask = (VALUE - POS0->Value) + 4 * depth + 5;
    }
  NextMove->bc = 0;
  NextMove->move = 0;
  NextMove->trans_move = trans_move;
  cnt = 0;
  while ((move = MyNext (POSITION, NextMove)))
    {
      to = TO (move);
      fr = FROM (move);
      if (IsRepetition (0))
	{
	  best_value = MAX (0, best_value);
	  cnt++;
	  continue;
	}
      if (cnt >= depth && NextMove->phase == ORDINARY_MOVES
	  && (move & 0xe000) == 0 && (SqSet[fr] & ~MyXRAY)
	  && MyOccupied ^ (BitboardMyP | BitboardMyK))
	{
	  if ((2 * depth) + MAX_POSITIONAL (move) + POS0->Value <
	      VALUE + 40 + 2 * cnt)
	    {
	      cnt++;
	      continue;
	    }
	}
      if (POSITION->sq[fr] != EnumMyK
	  && (POSITION->sq[to] == 0 || (depth <= 5 && !EasySEE (move)))
	  && (SqSet[fr] & ~MyXRAY) && !MoveIsEP (move)
	  && move != trans_move && !MySEE (POSITION, move))
	{
	  cnt++;
	  continue;
	}
      move &= 0x7fff;
      MAKE (POSITION, move);
      EvalLowDepth (VALUE, move);
      if (ILLEGAL_MOVE || (NextMove->phase == POSITIONAL_GAIN_PHASE && MOVE_IS_CHECK))
	{
	  UNDO (POSITION, move);
	  continue;
	}
      if (IS_EXACT (POSITION->DYN))
        v = -POSITION->DYN->Value;
      else if (MOVE_IS_CHECK)
	v = -OppLowDepthCheck (POSITION, 1 - VALUE, depth - 1);
      else
	{
	  int new_depth = depth - 2;
	  if (cnt >= depth && (2 * depth) - POS1->Value < VALUE + cnt)
	    {
	      UNDO (POSITION, move);
	      cnt++;
	      continue;
	    }
#if 0 /* LMR */
	  if (depth >= 5 && NextMove->phase == ORDINARY_MOVES)
	    {
	      v = -OppLowDepth (POSITION, 1 - VALUE, (new_depth + 1) / 2);
	      if (v < VALUE)
		goto DONE;
	    }
#endif	  
	  if (QSEARCH_CONDITION)
	    v = -OppQsearch (POSITION, 1 - VALUE, 0);
	  else
	    v = -OppLowDepth (POSITION, 1 - VALUE, new_depth);
	}
    DONE:
      UNDO (POSITION, move);
      CHECK_HALT ();
      cnt++;
      if (v >= VALUE)
	{
	  if ((POS0 + 1)->cp == 0 && MoveHistory (move))
	    HISTORY_GOOD (move, depth);
	  HashLower (POSITION->DYN->HASH, move, depth, v);
	  RETURN (v);
	}
      if (v >= best_value)
	best_value = v;
      if ((POS0 + 1)->cp == 0 && MoveHistory (move))
	HISTORY_BAD (move, depth);
    }
  if (!cnt && NextMove->phase <= TRANS2) /* HACK */
    RETURN (0);
  HashUpper (POSITION->DYN->HASH, depth, best_value);
  RETURN (best_value);
}

int MyLowDepthCheck (typePOS* POSITION, int VALUE, int depth)
{
  int ignored, trans_move = 0, trans_depth, move_depth = 0;
  int  Value, i, move, best_value, v, new_depth;
  boolean GEN;
  TRANS_DECLARE ();
  typeMoveList LIST[256], *list, *p, *q;
  typeDYNAMIC* POS0 = POSITION->DYN;
  YUSUF_DECLARE ();
  TRACE (TRACE_LOW_DEPTH,
         printf ("L%cC V:%d dp:%d ev:%d\n", POSITION->wtm ? 'w' : 'b',
                 VALUE, depth, POSITION->DYN->Value));

  CHECK_FOR_MATE (VALUE);
  CheckRepetition (TRUE);
  
  Trans = HASH_POINTER (POSITION->DYN->HASH);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
          TRACE (TRACE_HASH && TRACE_LOW_DEPTH, HASH_READ (trans));
	  if (trans->flags & FLAG_MOVE_LESS)
            return (16 * HEIGHT (POSITION) - VALUE_MATE);
	  if (trans->DepthLower && trans->DepthLower >= depth)
	    {
	      Value = HashLowerValue (trans);
              HashReBound (Value, VALUE);
	      if (Value >= VALUE)
		RETURN (Value);
	    }
	  if (trans->DepthUpper && trans->DepthUpper >= depth)
	    {
	      Value = HashUpperValue (trans);
              HashReBound (Value, VALUE);
	      if (Value < VALUE)
		RETURN (Value);
	    }
	  trans_depth = trans->DepthLower;
	  move = trans->move;
	  if (move && trans_depth > move_depth)
	    {
	      move_depth = trans_depth;
	      trans_move = move;
	    }
	}
    }
  if (MoveIsOO (trans_move))
    trans_move = MOVE_NONE;
  if (trans_move && !MyOK (POSITION, trans_move))
    trans_move = 0;

  best_value = 16 * HEIGHT (POSITION) - VALUE_MATE;
  p = LIST;
  LIST[0].move = trans_move;
  GEN = FALSE;
  LIST[1].move = 0;
  ignored = 0;
  while (p->move || !GEN)
    {
      if (!p->move)
	{
	  list = MyEvasion (POSITION, LIST + 1, 0xffffffffffffffff);
	  GEN = TRUE;
	  for (p = list - 1; p >= LIST + 1; p--)
	    {
	      if ((p->move & 0x7fff) == trans_move)
		p->move = 0;
	      else if (p->move <= (0x80 << 24))
		{
		  if ((p->move & 0x7fff) == POS0->killer1)
		    p->move |= 0x7fff8000;
		  else if ((p->move & 0x7fff) == POS0->killer2)
		    p->move |= 0x7fff0000;
		  else
		    p->move |= (p->move & 0xffff) |
		      ((HISTORY_VALUE (POSITION, p->move) >> 1) << 16);
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
	  p = LIST + 1;
	  continue;
	}
      move = p->move;
      p++;
      if (IsRepetition (0))
	{
	  best_value = MAX (0, best_value);
	  continue;
	}
      if (IsInterpose (move) && VALUE > -15000
	  && (move & 0x7fff) != trans_move && !MySEE (POSITION, move))
	{
	  ignored++;
	  continue;
	}
      move &= 0x7fff;
      MAKE (POSITION, move);
      EvalLowDepth (VALUE, move);
      if (ILLEGAL_MOVE)
	{
	  UNDO (POSITION, move);
	  continue;
	}
      if (IS_EXACT (POSITION->DYN))
        v = -POSITION->DYN->Value;
      else if (MOVE_IS_CHECK)
	v = -OppLowDepthCheck (POSITION, 1 - VALUE, depth - 1 + EXTEND_IN_CHECK);
      else
	{
	  new_depth = depth - 2 + EXTEND_IN_CHECK;
	  if (QSEARCH_CONDITION)
	    v = -OppQsearch (POSITION, 1 - VALUE, 0);
	  else
	    v = -OppLowDepth (POSITION, 1 - VALUE, new_depth);
	}
      UNDO (POSITION, move);
      CHECK_HALT ();
      if (v <= best_value)
	continue;
      best_value = v;
      if (v >= VALUE)
	{
	  HashLower (POSITION->DYN->HASH, move, MAX (1, depth), v);
	  RETURN (v);
	}
    }
  if (ignored && best_value < -15000)
    best_value = VALUE - 1;
  HashUpper (POSITION->DYN->HASH, MAX (1, depth), best_value);
  RETURN (best_value);
}

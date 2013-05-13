#ifndef BUILD_qsearch_pv
#define BUILD_qsearch_pv
#include "RobboLito.h"
#include "qsearch_pv.c"
#include "white.h"
#else
#include "black.h"
#endif

#ifdef TRACE_COMPILE
#define RETURN(x) \
  { TRACE (TRACE_PVQSEARCH, \
    printf ("PVQ%cR %d dp:%d V:[%d,%d]\n", \
	    POSITION->wtm ? 'w' : 'b', (x), depth, ALPHA, BETA)); \
    return (x); }
#else
#define RETURN(x) return(x)
#endif

int MyPVQsearch (typePOS* POSITION, int ALPHA, int BETA, int depth)
{
  int i;
  uint32 good_move = 0, trans_move = 0, move, BAD_CAPS[64];
  uint32 trans_depth, move_depth = 0;
  int best_value, Value;
  uint64 TARGET;
  typeMoveList LIST[256], *list, *p, *q;
  int TEMP, v;
  typeDYNAMIC* POS0 = POSITION->DYN;
  int bc = 0;
  TRANS_DECLARE ();
  YUSUF_DECLARE ();

  TRACE (TRACE_PVQSEARCH,
         printf ("PVQ%c V:[%d,%d] dp:%d ev:%d\n", POSITION->wtm ? 'w' : 'b',
                 ALPHA, BETA, depth, POSITION->DYN->Value));
  CheckRepetition (FALSE);
  if (BETA < -VALUE_MATE)
    RETURN (-VALUE_MATE);
  if (ALPHA > VALUE_MATE)
    RETURN (VALUE_MATE);
  
  Trans = HASH_POINTER (POSITION->DYN->HASH);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
          TRACE (TRACE_HASH && TRACE_PVQSEARCH, HASH_READ (trans));
	  if (trans->flags & FLAG_MOVE_LESS)
            return 0;
	  if (IsExact (trans))
	    {
	      Value = HashUpperBound (trans);
	      RETURN (Value);
	    }
	  if (trans->DepthLower)
	    {
	      Value = HashLowerBound (trans);
	      if (Value >= BETA)
		RETURN (Value);
	    }
	  if (trans->DepthUpper)
	    {
	      Value = HashUpperBound (trans);
	      if (Value <= ALPHA)
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
  best_value = POS0->Value + TempoValue2;
  TARGET = OppOccupied;
  if (best_value >= BETA)
    {
      RETURN (best_value);
    }
  else if (best_value > ALPHA)
    ALPHA = best_value;
  else
    {
      if (best_value < ALPHA - PrunePawn && HasPiece)
	{
	  TARGET ^= BitboardOppP;
	  if (best_value < ALPHA - PruneMinor)
	    {
	      TARGET ^= (BitboardOppN | BitboardOppB);
	      if (best_value < ALPHA - PruneRook)
		TARGET ^= BitboardOppR;
	    }
	  best_value += PrunePawn;
	}
    }

  list = MyCapture (POSITION, LIST, TARGET);
  p = LIST;
  while (p->move)
    {
      if ((p->move & 0x7fff) == trans_move)
	p->move |= 0xffff0000;
      p++;
    }
  p = LIST;
  while (p->move)
    {
      move = p->move;
      q = ++p;
      while (q->move)
	{
	  if (move < q->move)
	    {
	      TEMP = q->move;
	      q->move = move;
	      move = TEMP;
	    }
	  q++;
	}
      if (EasySEE (move) || (move & 0x7fff) == trans_move
	  || MySEE (POSITION, move))
	{
	  move &= 0x7fff;
	  MAKE (POSITION, move);
	  EVAL (move, depth);
	  if (ILLEGAL_MOVE)
	    {
	      UNDO (POSITION, move);
	      continue;
	    }
	  if (IS_EXACT (POSITION->DYN))
	      v = -POSITION->DYN->Value;
	  else if (MOVE_IS_CHECK)
	    v = -OppPVQsearchCheck (POSITION, -BETA, -ALPHA, depth - 1);
	  else
	    v = -OppPVQsearch (POSITION, -BETA, -ALPHA, depth - 1);
	  UNDO (POSITION, move);
	  CHECK_HALT ();
	  if (v <= best_value)
	    continue;
	  best_value = v;
	  if (v <= ALPHA)
	    continue;
	  ALPHA = v;
	  good_move = move;
	  if (v >= BETA)
	    {
	      HashLower (POSITION->DYN->HASH, move, 1, v);
	      RETURN (v);
	    }
	}
      else
	BAD_CAPS[bc++] = move;
    }
  if (depth > 0)
    for (i = 0; i < bc; i++)
      {
	move = BAD_CAPS[i] & 0x7fff;
	MAKE (POSITION, move);
	EVAL (move, depth);
	if (ILLEGAL_MOVE)
	  {
	    UNDO (POSITION, move);
	    continue;
	  }
	if (IS_EXACT (POSITION->DYN))
	  v = -POSITION->DYN->Value;
	else if (MOVE_IS_CHECK)
	  v = -OppPVQsearchCheck (POSITION, -BETA, -ALPHA, depth - 1);
	else
	  v = -OppPVQsearch (POSITION, -BETA, -ALPHA, depth - 1);
	UNDO (POSITION, move);
	CHECK_HALT ();
	if (v <= best_value)
	  continue;
	best_value = v;
	if (v <= ALPHA)
	  continue;
	ALPHA = v;
	good_move = move;
	if (v >= BETA)
	  {
	    HashLower (POSITION->DYN->HASH, move, 1, v);
	    RETURN (v);
	  }
      }
  if (depth >= -5 && POS0->Value >= ALPHA - (16 << (depth + 5)))
    {
      list = MyQuietChecks (POSITION, LIST, TARGET);
      for (i = 0; i < list - LIST; i++)
	{
	  move = LIST[i].move & 0x7fff;
	  MAKE (POSITION, move);
	  EVAL (move, depth);
	  if (ILLEGAL_MOVE)
	    {
	      UNDO (POSITION, move);
	      continue;
	    }
	  if (IS_EXACT (POSITION->DYN))
	    v = -POSITION->DYN->Value;
	  else
	    v = -OppPVQsearchCheck (POSITION, -BETA, -ALPHA, depth - 1);
	  UNDO (POSITION, move);
	  CHECK_HALT ();
	  if (v <= best_value)
	    continue;
	  best_value = v;
	  if (v <= ALPHA)
	    continue;
	  ALPHA = v;
	  good_move = move;
	  if (v >= BETA)
	    {
	      HashLower (POSITION->DYN->HASH, move, 1, v);
	      RETURN (v);
	    }
	}
      if (depth >= -4 && POS0->Value >= ALPHA - (2 << (4 + depth)))
	{
	  list = MyPositionalGain (POSITION, LIST, ALPHA - POS0->Value + 5);
	  for (i = 0; i < list - LIST; i++)
	    {
	      move = LIST[i].move & 0x7fff;
	      MAKE (POSITION, move);
	      EVAL (move, depth);
	      if (-POS1->Value < ALPHA)
		{
		  UNDO (POSITION, move);
		  continue;
		}
	      if (ILLEGAL_MOVE || MOVE_IS_CHECK)
		{
		  UNDO (POSITION, move);
		  continue;
		}
	      if (IS_EXACT (POSITION->DYN))
		v = -POSITION->DYN->Value;
	      else
		v = -OppPVQsearch (POSITION, -BETA, -ALPHA, depth - 1);
	      UNDO (POSITION, move);
	      CHECK_HALT ();
	      if (v <= best_value)
		continue;
	      best_value = v;
	      if (v <= ALPHA)
		continue;
	      ALPHA = v;
	      good_move = move;
	      HashLower (POSITION->DYN->HASH, move, 1, v);
	      if (v >= BETA)
		RETURN (v);
	    }
	}
    }
  if (good_move)
    {
      HashExact (POSITION, good_move, 1, best_value, FLAG_EXACT);
      RETURN (best_value);
    }
  HashUpper (POSITION->DYN->HASH, 1, best_value);
  RETURN (best_value);
}

int MyPVQsearchCheck (typePOS* POSITION, int ALPHA, int BETA, int depth)
{
  int i;
  uint32 trans_move = 0, good_move = 0, move, TEMP;
  int best_value, Value;
  uint64 TARGET;
  typeMoveList LIST[256], *list, *p, *q;
  int v, trans_depth, move_depth = 0;
  typeDYNAMIC* POS0 = POSITION->DYN;
  TRANS_DECLARE ();
  YUSUF_DECLARE ();

  TRACE (TRACE_PVQSEARCH,
         printf ("PVQ%c V:[%d,%d] dp:%d ev:%d\n", POSITION->wtm ? 'w' : 'b',
                 ALPHA, BETA, depth, POSITION->DYN->Value));
  CheckRepetition (TRUE);
  if (BETA < -VALUE_MATE)
    RETURN (-VALUE_MATE);
  if (ALPHA > VALUE_MATE)
    RETURN (VALUE_MATE);

  Trans = HASH_POINTER (POSITION->DYN->HASH);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (POSITION->DYN->HASH >> 32)) == 0)
	{
          TRACE (TRACE_HASH && TRACE_PVQSEARCH, HASH_READ (trans));
	  if (trans->flags & FLAG_MOVE_LESS)
            return (16 * HEIGHT (POSITION) - VALUE_MATE);
	  if (IsExact (trans))
	    {
	      Value = HashUpperBound (trans);
	      RETURN (Value);
	    }
	  if (trans->DepthLower)
	    {
	      Value = HashLowerBound (trans);
	      if (Value >= BETA)
		RETURN (Value);
	    }
	  if (trans->DepthUpper)
	    {
	      Value = HashUpperBound (trans);
	      if (Value <= ALPHA)
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
  best_value = 16 * HEIGHT (POSITION) - VALUE_MATE;
  TARGET = 0xffffffffffffffff;
  if (POS0->Value + PruneCheck < ALPHA && HasPiece)
    {
      best_value = POS0->Value + PruneCheck;
      v = ALPHA - 200;
      TARGET = OppOccupied;
      if (v > best_value)
	{
	  TARGET ^= BitboardOppP;
	  v = ALPHA - 500;
	  best_value += 200;
	  if (v > best_value)
	    TARGET ^= (BitboardOppN | BitboardOppB);
	}
    }
  list = MyEvasion (POSITION, LIST, TARGET);
  if ((list - LIST) != 1)
    depth--;
  p = LIST;
  while (p->move)
    {
      if ((p->move & 0x7fff) == trans_move)
	p->move |= 0xfff00000;
      p++;
    }
  p = LIST;
  while (p->move)
    {
      move = p->move;
      q = ++p;
      while (q->move)
	{
	  if (move < q->move)
	    {
	      TEMP = q->move;
	      q->move = move;
	      move = TEMP;
	    }
	  q++;
	}
      move &= 0x7fff;
      MAKE (POSITION, move);
      EVAL (move, depth);
      if (ILLEGAL_MOVE)
	{
	  UNDO (POSITION, move);
	  continue;
	}
      if (IS_EXACT (POSITION->DYN))
	v = -POSITION->DYN->Value;
      else if (MOVE_IS_CHECK)
	v = -OppPVQsearchCheck (POSITION, -BETA, -ALPHA, depth);
      else
	v = -OppPVQsearch (POSITION, -BETA, -ALPHA, depth);
      UNDO (POSITION, move);
      CHECK_HALT ();
      if (v <= best_value)
	continue;
      best_value = v;
      if (v <= ALPHA)
	continue;
      ALPHA = v;
      good_move = move;
      HashLower (POSITION->DYN->HASH, move, 1, v);
      if (v >= BETA)
	RETURN (v);
    }
  if (good_move)
    {
      HashExact (POSITION, good_move, 1, best_value, FLAG_EXACT);
      RETURN (best_value);
    }
  HashUpper (POSITION->DYN->HASH, 1, best_value);
  RETURN (best_value);
}

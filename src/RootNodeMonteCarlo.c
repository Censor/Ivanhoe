#ifdef MODE_ANALYSIS
#ifndef BUILD_root_node_monte_carlo
#define BUILD_root_node_monte_carlo
#include "RobboLito.h"
#define RETURN(x) return(x)

#include "RootNodeMonteCarlo.c"
#include "white.h"
#else
#include "black.h"
#endif

int MyRootNodeMonteCarlo
(typePOS* POSITION, typeRootMoveList * ROOT_MOVE_LIST, int ALPHA, int BETA, int depth)
{
  int CNT, origALPHA, best_value, cnt, move_is_check, new_depth, v;
  typeRootMoveList *p, *q;
  typeDYNAMIC* POS0 = POSITION->DYN;
  uint32 move, BEST_MOVE = MOVE_NONE;
  int EXTEND, LMR;
  int to, value;
  uint64 NODES, NODES_STORE, nodes;

  if (BETA > VALUE_MATE)
    BETA = VALUE_MATE;
  if (ALPHA < -VALUE_MATE)
    ALPHA = -VALUE_MATE;
  CNT = 0;
  for (p = ROOT_MOVE_LIST; p->move; p++)
    {
      CNT++;
      p->move &= 0x7fff;
    }
  origALPHA = ALPHA;
  p = ROOT_MOVE_LIST;
  v = best_value = -VALUE_INFINITY;
  cnt = 0; /* SMP ? */
  NODES = POSITION->nodes;
  NODES_STORE = NODES;
  cnt = 0;
  while ((move = p->move))
    {
      MAKE (POSITION, move);
      EVAL (move, depth);
      move_is_check = (MOVE_IS_CHECK != 0);
      EXTEND = 0;
      to = TO (move);
      if (POS1->cp || move_is_check || PassedPawnPush (to, FOURTH_RANK (to)))
	EXTEND = 1;
      LMR = 0;
      if (!EXTEND && cnt >= 3 && depth >= 10)
	LMR = 1;
      if (!EXTEND && cnt >= 6 && depth >= 10)
	LMR = 2;
      new_depth = depth - 2 + EXTEND - LMR;
      if (best_value == -VALUE_INFINITY || depth <= 2)
	v = -OppPV (POSITION, -BETA, -ALPHA, new_depth, move_is_check, NULL);
      else
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
#if 1
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -ALPHA - 1, -ALPHA,
			new_depth, move_is_check, NULL);
#endif
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -BETA, -ALPHA,
			new_depth, move_is_check, NULL);
	  if (v <= ALPHA)
	    v = ALPHA;
	}
      UNDO (POSITION, move);
      cnt++;
      CHECK_HALT ();
      if (v <= ALPHA)
	p->value = origALPHA;
      else
	p->value = v;
      NODES = POSITION->nodes;
      p->nodes = NODES - NODES_STORE;
      NODES_STORE = NODES;
      if (v > best_value)
	{
	  best_value = v;
	  if (best_value == -VALUE_INFINITY || v > ALPHA)
	    {
	      HashLowerPV (POSITION->DYN->HASH, move, depth, v);
	      BEST_MOVE = move;
	    }
	}
      if (v > ALPHA)
	ALPHA = v;
      if (v < BETA)
	{
	  p++;
	  continue;
	}
      break;
    }
  for (p = ROOT_MOVE_LIST + (CNT - 1); p >= ROOT_MOVE_LIST; p--)
    {
      move = p->move;
      value = p->value;
      nodes = p->nodes;
      for (q = p + 1; q < ROOT_MOVE_LIST + CNT; q++)
	{
	  if (value < q->value)
	    {
	      (q - 1)->move = q->move;
	      (q - 1)->value = q->value;
	      (q - 1)->nodes = q->nodes;
	    }
	  else
	    break;
	}
      q--;
      q->move = move;
      q->value = value;
      q->nodes = nodes;
    }
#if 1
  for (p = ROOT_MOVE_LIST + CNT; p > ROOT_MOVE_LIST; p--)
    {
      move = p->move;
      value = p->value;
      nodes = p->nodes;
      for (q = p + 1; q < ROOT_MOVE_LIST + CNT; q++)
	{
	  if (nodes < q->nodes)
	    {
	      (q - 1)->move = q->move;
	      (q - 1)->value = q->value;
	      (q - 1)->nodes = q->nodes;
	    }
	  else
	    break;
	}      
      (q - 1)->move = move;
      (q - 1)->value = value;
      (q - 1)->nodes = nodes;
    }
#endif
  if (best_value <= origALPHA)
    HashUpperPV (POSITION->DYN->HASH, depth, origALPHA);
  else if (best_value < BETA)
    HashExact (POSITION, BEST_MOVE, depth, best_value, FLAG_EXACT);
  POSITION->DYN->move = BEST_MOVE;
  return best_value;
}
#endif


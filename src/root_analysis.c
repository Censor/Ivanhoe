#ifndef BUILD_root_analysis
#define BUILD_root_analysis
#include "RobboLito.h"
#define RETURN(x) return(x)

#include "control.h"
typeRootMoveList ROOT_MOVE_LIST[256];
#include "root_analysis.c"
#include "white.h"
#else
#include "black.h"
#endif

int MyAnalysis (typePOS* POSITION, int ALPHA, int BETA, int depth)
{
  int CNT, origALPHA, best_value, cnt, move_is_check, new_depth, v;
  typeRootMoveList *p, *q;
  typeDYNAMIC* POS0 = POSITION->DYN;
  uint32 move;
  int EXTEND, LMR;
  int to, value;
  uint64 NODES, NODES_STORE, nodes;

#ifndef MINIMAL
  if (MULTI_PV != 1)
    return MyMultiPV (POSITION, depth);
#endif
  if (BETA > VALUE_MATE)
    BETA = VALUE_MATE;
  if (ALPHA < -VALUE_MATE)
    ALPHA = -VALUE_MATE;
  if (DO_OUTPUT)
    SEND ("info depth %d\n", depth / 2);
#ifdef BUILD_ZOG_MP_COMPILE
  if (ZOG_AKTIV) /* && depth >= 15 */
    return zog_root_node (POSITION, ALPHA, BETA, depth);
#endif
  CNT = 0;
  for (p = ROOT_MOVE_LIST; p->move; p++)
    {
      CNT++;
      p->move &= 0x7fff;
    }
  origALPHA = ALPHA;
  p = ROOT_MOVE_LIST;
  v = best_value = -VALUE_INFINITY;
  if (TITANIC_MODE)
    {
      int cpu, rp;
      NODES = 0;
      for (cpu = 0; cpu < CPUS_SIZE; cpu++)
        for (rp = 0; rp < RP_PER_CPU ; rp++)
          NODES += ROOT_POSITION[cpu][rp].nodes;
    }
  else
    NODES = ROOT_POSIZIONE0->nodes;
  NODES_STORE = NODES;
  cnt = 0; /* SMP ? */
  while ((move = p->move))
    {
      MAKE (POSITION, move);
      EVAL (move, depth);
      move_is_check = (MOVE_IS_CHECK != 0);
      EXTEND = 0;
      to = TO (move);
      LMR = 0;
#if 1
      if (!EXTEND && cnt >= 3 && depth >= 10)
        LMR = 1;
      if (!EXTEND && cnt >= 6 && depth >= 10)
        LMR = 2;
#endif
      new_depth = depth - 2 + EXTEND - LMR;
      if (POS1->cp || move_is_check || PassedPawnPush (to, FOURTH_RANK (to)))
	EXTEND = 1;
      new_depth = depth - 2 + EXTEND;
      if (DO_OUTPUT && ANALYSING && depth >= 24)
	SEND ("info currmove %s currmovenumber %d\n",
	      Notate (move, STRING1[POSITION->cpu]), (p - ROOT_MOVE_LIST) + 1);
      if (IS_EXACT (POSITION->DYN))
        v = -POSITION->DYN->Value;
      else if (best_value == -VALUE_INFINITY || depth <= 2)
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
	      if (new_depth >= 16 && ANALYSING) /* ANALYSING es TRUE */
		{
		  int an = new_depth - 12;
		  v = VALUE_INFINITY;
		  while (an <= new_depth && v > ALPHA)
		    {
		      v = -OppPV (POSITION, -ALPHA - 1, -ALPHA,
				  an, move_is_check, NULL);
		      an += 4;
		    }
		  if (an > new_depth)
		    {
		      goto DEC;
		      new_depth += LMR;
		    }
		}
	      if (move_is_check)
		v = -OppCutCheck (POSITION, -ALPHA, new_depth);
	      else
		v = -OppCut (POSITION, -ALPHA, new_depth);
	    }
	  new_depth += LMR;
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -ALPHA - 1, -ALPHA,
			new_depth, move_is_check, NULL);
	DEC:
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -BETA, -ALPHA, new_depth, move_is_check, NULL);
	  if (v <= ALPHA)
	    v = ALPHA;
	} /* NORMALE */
      UNDO (POSITION, move);
      CHECK_HALT ();
      cnt++;
      if (v <= ALPHA)
        p->value = origALPHA;
      else
        p->value = v;
      if (TITANIC_MODE)
        {
          int cpu, rp;
          NODES = 0;
          for (cpu = 0; cpu < CPUS_SIZE; cpu++)
            for (rp = 0; rp < RP_PER_CPU ; rp++)
              NODES += ROOT_POSITION[cpu][rp].nodes;
        }
      else
	NODES = ROOT_POSIZIONE0->nodes;
      p->nodes = NODES - NODES_STORE;
      NODES_STORE = NODES;
      if (v > best_value)
	{
	  best_value = v;
	  if (best_value == -VALUE_INFINITY || v > ALPHA)
	    {
	      HashLowerPV (POSITION->DYN->HASH, move, depth, v);
	      ROOT_BEST_MOVE = move;
	      ROOT_SCORE = v;
	      ROOT_DEPTH = depth;
	      if (v > ALPHA && v < BETA)
		Information (POSITION, GetClock () - START_CLOCK, origALPHA, v, BETA);
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
  ROOT_DEPTH = depth;
  if (!DO_SEARCH_MOVES) /* analysis specificated */
    {
      if (best_value <= origALPHA)
	HashUpperPV (POSITION->DYN->HASH, depth, origALPHA);
      else if (best_value < BETA)
	HashExact (POSITION, ROOT_BEST_MOVE, depth, best_value, FLAG_EXACT);
    }
  Information (POSITION, GetClock () - START_CLOCK, origALPHA, best_value, BETA);
  return best_value;
}

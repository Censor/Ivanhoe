#ifndef MINIMAL
#ifndef BUILD_root_multipv
#define BUILD_root_multipv
#include "RobboLito.h"
#define RETURN(x) return(x)

#include "control.h"
typeRootMoveList ROOT_MOVE_LIST[256];
void APPLY_SORT (int n, typeMPV* mpv)
{
  typeMPV *p;
  int s, x;
  uint64 y;
  for (s = 0; s < n && mpv[s].move; s++)
    {
      if (s == 0)
	continue;
      p = &mpv[s];
      while (p != mpv && p->Value > (p-1)->Value)
	{
	  x = p->Value;
	  p->Value = (p - 1)->Value;
	  (p - 1)->Value = x;
	  x = p->move;
	  p->move = (p - 1)->move;
	  (p - 1)->move = x;
	  x = p->depth;
	  p->depth = (p - 1)->depth;
	  (p - 1)->depth = x;
	  x = p->alpha;
	  p->alpha = (p - 1)->alpha;
	  (p - 1)->alpha = x;
	  x = p->beta;
	  p->beta = (p - 1)->beta;
	  (p - 1)->beta = x;
	  y = p->nodes;
	  p->nodes = (p - 1)->nodes;
	  (p - 1)->nodes = y;
	  p--;
	}	
    }
}

#include "root_multipv.c"
#include "white.h"
#else
#include "black.h"
#endif

#define CHECK_HALT_MULTI() { if (IVAN_ALL_HALT || POSITION->stop) goto UNDO_LABEL; }

int MyMultiPV (typePOS* POSITION, int depth)
{
  int CNT, cnt, best_value, move_is_check, new_depth, v;
  typeRootMoveList *p, *q;
  typeDYNAMIC* POS0 = POSITION->DYN;
  uint32 move;
  int EXTEND, LMR, value;
  int to;
  int i, j, x, moveno;
  int ALPHA = -VALUE_MATE, TARGET, DELTA, ALPHA2, LOWER;
  int GOOD_MOVES = 0;
  uint64 NODES, NODES_STORE, nodes, y;

  if (depth < 14) /* 7 plies */
    for (i = 0; i < 0x100; i++)
      MPV[i].move = MPV[i].Value = 0;
  if (DO_OUTPUT)
    SEND ("info depth %d\n", depth / 2);
  CNT = 0;
  for (p = ROOT_MOVE_LIST; p->move; p++)
    {
      CNT++;
      p->move &= 0x7fff;
    }
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
      TARGET = -VALUE_INFINITY;
      DELTA = VALUE_MATE;
      for (moveno = 0; MPV[moveno].move; moveno++)
	if (MPV[moveno].move == move)
	  {
	    TARGET = MPV[moveno].Value;
	    break;
	  }
      if (TARGET == -VALUE_INFINITY)
	TARGET = 0;
      else
	DELTA = 8;
      if (GOOD_MOVES > 0)
	ALPHA2 = MPV[0].Value - MULTI_CENTI_PAWN_PV;
      else
	ALPHA2 = -VALUE_INFINITY;
      MAKE (POSITION, move);
      EVAL (move, depth);
      move_is_check = (MOVE_IS_CHECK != 0);
      EXTEND = 0;
      to = TO (move);
      if (POS1->cp || move_is_check || PassedPawnPush (to, FOURTH_RANK (to)))
	EXTEND = 1;
      LMR = 0;
      if (!EXTEND && cnt >= 2 * GOOD_MOVES + 3 && depth >= 10)
	LMR = 1;
      if (!EXTEND && cnt >= 2 * GOOD_MOVES + 6 && depth >= 10)
	LMR = 2;
      new_depth = depth - 2 + EXTEND - LMR;
      if (DO_OUTPUT && ANALYSING && depth >= 24)
	SEND ("info currmove %s currmovenumber %d\n",
	      Notate (move, STRING1[POSITION->cpu]), (p - ROOT_MOVE_LIST) + 1);
      if (GOOD_MOVES < MULTI_PV || depth <= 2)
	{
	  while (TRUE)
	    {
	      LOWER = MAX (TARGET - DELTA, ALPHA2);
	      if (IS_EXACT (POSITION->DYN))
		v = -POSITION->DYN->Value;
	      else
		v = -OppPV (POSITION, -TARGET - DELTA, -LOWER,
			    new_depth, move_is_check, NULL);
	      CHECK_HALT_MULTI ();
	      if (v >= VALUE_MATE)
		v = VALUE_MATE - 1;
	      if (v <= -VALUE_MATE)
		v = -VALUE_MATE + 1;
	      if (v < TARGET + DELTA && v > LOWER)
		break;
	      if (GOOD_MOVES && v <= ALPHA2)
		break;
	      new_depth += LMR;
	      LMR = 0;
	      /* UCI bounds */
	      MPV[moveno].move = move;
	      MPV[moveno].depth = depth;
	      MPV[moveno].alpha = LOWER;
	      MPV[moveno].beta = TARGET + DELTA;
	      MPV[moveno].Value = v;
	      UNDO (POSITION, move);
	      Information (POSITION, GetClock () - START_CLOCK,
			   MPV[moveno].alpha, MPV[moveno].Value, MPV[moveno].beta);
	      MAKE (POSITION, move);
	      EVAL (move, depth);
	      if (v >= TARGET + DELTA)
		TARGET = TARGET + DELTA / 2;
	      else
		TARGET = TARGET - DELTA / 2;
	      if (TARGET + DELTA >= 25000 || TARGET - DELTA <= -25000)
		{
		  TARGET = 0;
		  DELTA = VALUE_MATE;
		}
	      else
		DELTA += DELTA / 2;	      
	    }
	}
      else
	{
	  if (LOW_DEPTH_CONDITION_PV)
	    {
	      if (IS_EXACT (POSITION->DYN))
		v = -POSITION->DYN->Value;
	      else if (move_is_check)
		v = -OppLowDepthCheck (POSITION, -ALPHA, new_depth);
	      else
		v = -OppLowDepth (POSITION, -ALPHA, new_depth);
	      CHECK_HALT_MULTI ();
	    }
	  else
	    {
	      if (new_depth >= 16 && ANALYSING) /* ANALYSING es TRUE */
		{
		  int an = new_depth - 12;
		  v = VALUE_INFINITY;
		  while (an <= new_depth && v > ALPHA)
		    {
		      if (IS_EXACT (POSITION->DYN))
			v = -POSITION->DYN->Value;
		      else
			v = -OppPV (POSITION, -ALPHA - 1, -ALPHA,
				    an, move_is_check, NULL);
		      CHECK_HALT_MULTI ();
		      an += 4;
		    }
		  if (an > new_depth)
		    {
		      new_depth += LMR;
		      goto EN_CIRCLE;
		    }
		}
	      if (IS_EXACT (POSITION->DYN))
		v = -POSITION->DYN->Value;
	      else if (move_is_check)
		v = -OppCutCheck (POSITION, -ALPHA, new_depth);
	      else
		v = -OppCut (POSITION, -ALPHA, new_depth);
	      CHECK_HALT_MULTI ();
	    }
	  new_depth += LMR;
	  if (v >= VALUE_MATE)
	    v = VALUE_MATE - 1;
	  if (v > ALPHA)
	    v = -OppPV (POSITION, -ALPHA - 1, -ALPHA,
			new_depth, move_is_check, NULL);
	  if (v >= VALUE_MATE)
	    v = VALUE_MATE - 1;
	EN_CIRCLE:
	  CHECK_HALT_MULTI ();
	  DELTA = 8;
	  while (v > ALPHA)
	    {
	      if (IS_EXACT (POSITION->DYN))
		v = -POSITION->DYN->Value;
	      else
		v = -OppPV (POSITION, -ALPHA - DELTA, -ALPHA,
			    new_depth, move_is_check, NULL);
	      if (IVAN_ALL_HALT)
		break;
	      if (v >= VALUE_MATE)
		v = VALUE_MATE - 1;
	      if (v <= -VALUE_MATE)
		v = -VALUE_MATE + 1;
	      if (v < ALPHA + DELTA)
		break;
	      DELTA += DELTA / 2;
	      if (ALPHA + DELTA >= 25000)
		DELTA = VALUE_MATE - ALPHA;
	    }
	} /* NORMALE */
    UNDO_LABEL:
      UNDO (POSITION, move);
      CHECK_HALT ();
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
      if (v > ALPHA)
	{
	  p->value = v;
	  if (v > best_value)
	    HashLowerPV (POSITION->DYN->HASH, move, depth, v);
	  MPV[GOOD_MOVES].move = move;
	  MPV[GOOD_MOVES].depth = depth;
	  MPV[GOOD_MOVES].alpha = -VALUE_MATE;
	  MPV[GOOD_MOVES].beta = VALUE_MATE;
	  MPV[GOOD_MOVES++].Value = v;
	  APPLY_SORT (GOOD_MOVES, MPV);
	  if (GOOD_MOVES >= MULTI_PV)
	    ALPHA = MPV[MULTI_PV - 1].Value;
	  else
	    ALPHA = MAX (MPV[0].Value - MULTI_CENTI_PAWN_PV, -VALUE_MATE);
	  ROOT_BEST_MOVE = MPV[0].move;
	  best_value = ROOT_SCORE = MPV[0].Value;
	  Information (POSITION, GetClock () - START_CLOCK,
		       -VALUE_MATE, MPV[0].Value, VALUE_MATE);
	}
      else
	p->value = ALPHA;
      p++;
    }
  MPV[GOOD_MOVES].move = MOVE_NONE;
  for (i = 0; i < GOOD_MOVES; i++)
    {
      for (j = 0; j < CNT; j++)
	{
	  if (ROOT_MOVE_LIST[j].move == MPV[i].move)
	    {
	      x = ROOT_MOVE_LIST[i].move;
	      ROOT_MOVE_LIST[i].move = ROOT_MOVE_LIST[j].move;
	      ROOT_MOVE_LIST[j].move = x;
	      x = ROOT_MOVE_LIST[i].value;
	      ROOT_MOVE_LIST[i].value = ROOT_MOVE_LIST[j].value;
	      ROOT_MOVE_LIST[j].value = x;
	      y = ROOT_MOVE_LIST[i].nodes;
	      ROOT_MOVE_LIST[i].nodes = ROOT_MOVE_LIST[j].nodes;
	      ROOT_MOVE_LIST[j].nodes = y;
	      break;
	    }
	}
    }
#if 1
  for (p = ROOT_MOVE_LIST + CNT; p >= ROOT_MOVE_LIST + GOOD_MOVES; p--)
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
  if (!DO_SEARCH_MOVES)
    HashExact (POSITION, MPV[0].move, depth, MPV[0].Value, FLAG_EXACT);
  Information (POSITION, GetClock () - START_CLOCK,
	       -VALUE_MATE, MPV[0].Value, VALUE_MATE);
  return MPV[0].Value;
}
#endif /* MINIMAL */

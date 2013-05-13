#include "RobboLito.h"
#include "control.h"
#include "null_move.i"
#include <string.h>

#define LEGAL (POSITION->wtm ? !BLACK_IN_CHECK : !WHITE_IN_CHECK)
#define IN_CHECK (POSITION->wtm ? WHITE_IN_CHECK : BLACK_IN_CHECK)

void OutputBestMove (typePOS* POSITION)
{
  int i, k;
  boolean b;
  TRANS_PV_DECLARE ();
  int PONDER_MOVE = MOVE_NONE;
  YUSUF_PV_DECLARE ();
  if (!ROOT_BEST_MOVE)
    {
      SEND ("bestmove NULL\n");
      return;
    }
  Make (POSITION, ROOT_BEST_MOVE);
  Mobility (POSITION);
  if (SEND_HASH)
    SEND ("hash %lld\n", POSITION->DYN->HASH ^ ZOB_REV (POSITION)); /* %llx ? */
  k = POSITION->DYN->HASH & PVHashMask;
  for (i = 0; i < 4; i++)
    {
      Trans_pv = PVHashTable + (k + i);
      HYATT_HASH(Trans_pv, trans_pv);
      if (trans_pv->hash == POSITION->DYN->HASH)
	{
	  PONDER_MOVE = trans_pv->move;
	  break;
	}
    }
#if 1 /* PUZZLE */
  if (UCI_PONDER && !PONDER_MOVE)
    {
      int depth;
      for (depth = 2; depth <= 10; depth +=2)
	POSITION->wtm ?
	  PVNodeWhite (POSITION, -VALUE_INFINITY, VALUE_INFINITY,
		       depth, POSITION->DYN->bAtt & wBitboardK, NULL) :
	  PVNodeBlack (POSITION, -VALUE_INFINITY, VALUE_INFINITY,
		       depth, POSITION->DYN->wAtt & bBitboardK, NULL);
      PONDER_MOVE = (POSITION->DYN + 1)->move;
    }
#endif
  if ((POSITION->wtm ?
       !WhiteOK (POSITION, PONDER_MOVE) : !BlackOK (POSITION, PONDER_MOVE)))
    PONDER_MOVE = MOVE_NONE;
  else
    {
      Make (POSITION, PONDER_MOVE);
      Mobility (POSITION);
      b = (LEGAL);
      Undo (POSITION, PONDER_MOVE);
      if (!b)
	PONDER_MOVE = MOVE_NONE;
    }
  Undo (POSITION, ROOT_BEST_MOVE);
  SEND ("bestmove %s ponder %s\n",
	Notate (ROOT_BEST_MOVE, STRING1[POSITION->cpu]),
	Notate (PONDER_MOVE, STRING2[POSITION->cpu]));
}

static char* MODIFIER (int ALPHA, int Value, int BETA, char* s)
{
  s[0] = 0;
  if (ALPHA <= -VALUE_MATE)
    ALPHA = -VALUE_MATE;
  if (BETA >= VALUE_MATE)
    BETA = VALUE_MATE;
  if (Value > VALUE_MATE - 16)
    Value = VALUE_MATE - 16;
  if (Value < -VALUE_MATE + 32)
    Value = -VALUE_MATE + 32;
  if (Value <= ALPHA)
    strcpy (s, " upperbound");
  else if (Value >= BETA)
    strcpy (s, " lowerbound");
  else
    strcpy (s, "");
  return s;
}

#define MAX_MATE_PLY 256
static char* cp_mate (int Value, char* s)
{
  if (Value > VALUE_MATE - 16)
    Value = VALUE_MATE - 16;
  if (Value < -VALUE_MATE + 32)
    Value = -VALUE_MATE + 32;
  if (Value > VALUE_MATE - 16 * MAX_MATE_PLY)
    sprintf (s, "mate %d", (VALUE_MATE + 16 - Value) / 32);
  else if (Value < -VALUE_MATE + 16 * MAX_MATE_PLY)
    sprintf (s, "mate %d", (-VALUE_MATE - Value) / 32);
  else
    sprintf (s, "cp %d", ROBBO_TRIPLE_DRAW ? 0 : Value);
  return s;
}

void Information (typePOS* POSITION, sint64 x, int ALPHA, int Value, int BETA)
{
  uint64 t, nps, NODES = 0;
  int cpu, rp;
  int sd, k, move;
  char pv[0x100 * 8], *q;
  TRANS_PV_DECLARE ();
  TRANS_DECLARE ();
  uint64 HashStack[256];
  int i;
  int cnt = 0;
  boolean B;
  int mpv;
  uint64 TBHITS = 0;
  char send_str[4096];
  YUSUF_DECLARE ();
  YUSUF_PV_DECLARE ();

  if (MONTE_CARLO)
    return; /* HACK */
  if (x < 1000ULL * OUTPUT_DELAY)
    return;
  DO_OUTPUT = TRUE;
  if (!TITANIC_MODE && ZOG_AKTIV)
    {
      ROOT_POSIZIONE0->nodes = ROOT_POSITION[0][0].nodes;
      ROOT_POSIZIONE0->tbhits = ROOT_POSITION[0][0].tbhits;
    }
  if (TITANIC_MODE)
    for (cpu = 0; cpu < CPUS_SIZE; cpu++)
      for (rp = 0; rp < RP_PER_CPU ; rp++)
	NODES += ROOT_POSITION[cpu][rp].nodes;
  else
    NODES = ROOT_POSIZIONE0->nodes;
  if (TITANIC_MODE)
    for (cpu = 0; cpu < CPUS_SIZE; cpu++)
      for (rp = 0; rp < RP_PER_CPU ; rp++)
	TBHITS += ROOT_POSITION[cpu][rp].tbhits;
  else
    TBHITS = ROOT_POSIZIONE0->tbhits;

#if 0 /* SELdepth non funzione direct con SMP */
  /* typeDYNAMIC *p; */
  for (p = POSITION->DYN_ROOT + 1; p->HASH; p++);
  sd = p - (POSITION->DYN_ROOT + 1);
#else
  sd = 0;
#endif
  memset (HashStack, 0, 256 * sizeof (uint64));
  t = x / 1000;
  if (t == 0)
    nps = 0;
  else
    nps = NODES / t;
  if (MULTI_PV == 1)
    {
      MPV[0].move = ROOT_BEST_MOVE;
      MPV[0].Value = Value;
      MPV[0].alpha = ALPHA;
      MPV[0].beta = BETA;
      MPV[0].depth = ROOT_DEPTH;
    }
  for (mpv = 0; mpv < MULTI_PV; mpv++)
    {
      move = MPV[mpv].move;
      if (move == MOVE_NONE) 
	break;
      q = pv;
      cnt = 0;
      HashStack[cnt++] = POSITION->DYN->HASH;
      Notate (move, STRING1[POSITION->cpu]);
      strcpy (q, STRING1[POSITION->cpu]);
      q += strlen (STRING1[POSITION->cpu]);
      strcpy (q, " ");
      q++;  
      while (move)
	{
	  Make (POSITION, move);
	  Mobility (POSITION);
	  B = FALSE;
	  for (i = 0; i < cnt; i++)
	    if (HashStack[i] == POSITION->DYN->HASH)
	      B = TRUE;
	  if (B)
	    break;
	  HashStack[cnt++] = POSITION->DYN->HASH;
	  move = 0;
	  k = POSITION->DYN->HASH & PVHashMask;
	  for (i = 0; i < 4; i++)
	    {
	      Trans_pv = PVHashTable + (k + i);
	      HYATT_HASH (Trans_pv, trans_pv);
	      if (trans_pv->hash == POSITION->DYN->HASH)
		{
		  move = trans_pv->move;
		  break;
		}
	    }
#if 1
	  if (!move)
	    {      
	      Trans = HASH_POINTER (POSITION->DYN->HASH);
	      for (i = 0; i < 4; i++, Trans++)
		{
		  HYATT_HASH (Trans, trans);
		  if (trans->hash == POSITION->DYN->HASH)
		    {
		      move = trans->move;
		      break; /* depth? */
		    }
		}
	    }
#endif
	  if (!move || (POSITION->wtm ?
			!WhiteOK (POSITION, move) : !BlackOK (POSITION, move)))
	    break;
	  if (cnt > 250 || POSITION->DYN->reversible > 100)
	    break;
	  Notate (move, STRING1[POSITION->cpu]);
	  strcpy (q, STRING1[POSITION->cpu]);
	  q += strlen (STRING1[POSITION->cpu]);
	  strcpy (q, " ");
	  q++;
	}
      q--;
      *q = 0;
      while (POSITION->DYN != (POSITION->DYN_ROOT + 1))
	{
	  if (!POSITION->DYN->move)
	    UndoNull (POSITION);
	  else
	    Undo (POSITION, POSITION->DYN->move);
	}
      sprintf (send_str,
	       "info multipv %d time " TYPE_64_BIT " nodes " TYPE_64_BIT
	       " nps " TYPE_64_BIT, mpv + 1, t, NODES, nps * 1000);
      if (TBHITS)
	sprintf (send_str + strlen (send_str), " tbhits " TYPE_64_BIT, TBHITS);
      sprintf (send_str + strlen (send_str), " score %s%s depth %d pv %s\n",
	       cp_mate (MPV[mpv].Value, STRING2[POSITION->cpu]),
	       MODIFIER (MPV[mpv].alpha, MPV[mpv].Value, MPV[mpv].beta, STRING3[POSITION->cpu]),
	       MPV[mpv].depth / 2, pv);
      SEND (send_str); /* HACK */
    }
}

#include <string.h>
volatile boolean WEAK_PROBE_PONDER;
boolean EmergeBackGroundLoader ();
boolean DesistBackGroundLoader ();

void Search (typePOS* POSITION)
{
  int z, i;
  typeDYNAMIC *p, *q, *S;
  typePOS* POS;
  SEARCH_IS_DONE = FALSE;
  SUPPRESS_INPUT = FALSE; /* redundant con JUMP_IS_SET? */
  if (NEW_GAME)
    SINCE_NEW_GAME = 0;
  else
    SINCE_NEW_GAME++;
  NEW_GAME = FALSE;
  START_CLOCK = GetClock ();
  INIT_CLOCK = clock ();
  DO_OUTPUT = FALSE;
  PONDER_HIT = FALSE;
  NODE_CHECK = 0;
  ROOT_POSIZIONE0->nodes = 0;
  ROOT_BEST_MOVE = ROOT_DEPTH = ROOT_SCORE = 0;
  POSITION->StackHeight = 0;
  if (POSITION->DYN->reversible > 110)
    goto SKIP_REP_CHECK;
  POSITION->StackHeight = -1;
  S = MAX (POSITION->DYN_ROOT + 1, POSITION->DYN - POSITION->DYN->reversible);
  for (p = S; p <= POSITION->DYN; p++)
    POSITION->STACK[++(POSITION->StackHeight)] = p->HASH;
  if (ANALYSING)
    {
      boolean REPETITION;
      for (p = S; p < POSITION->DYN; p++)
	{
	  REPETITION = FALSE;
	  for (q = p + 2; q < POSITION->DYN; q += 2)
	    if (p->HASH == q->HASH)
	      {
		REPETITION = TRUE;
		break;
	      }
	  if (!REPETITION)
	    POSITION->STACK[p - POSITION->DYN + POSITION->DYN->reversible] = 0;
	  (p + 1)->move = 0;
	}
    }
  if (POSITION->StackHeight == -1)
    POSITION->StackHeight = 0;
 SKIP_REP_CHECK:
  memcpy (POSITION->DYN_ROOT + 1, POSITION->DYN, sizeof (typeDYNAMIC));
  memset (POSITION->DYN_ROOT + 2, 0, 254 * sizeof (typeDYNAMIC));
  memset (POSITION->DYN_ROOT, 0, sizeof (typeDYNAMIC));
  POSITION->DYN = POSITION->DYN_ROOT + 1;
  POSITION->height = 0;
  if (FIXED_AGE_ANALYSIS && ANALYSING)
    GLOBAL_AGE = 0;
  else if (!ICI_HASH_MIX)
    IncrementAge ();
  else if (DO_HASH_FULL)
    {
      int u = DoHashFull (POSITION->DYN->HASH); /* HACK, random seed */
      if (u)
	SEND ("info hashfull %d\n", u);
    }
  for (i = 0; i < 256; i++)
    (POSITION->DYN_ROOT + i)->age = GLOBAL_AGE;
#ifdef CON_ROBBO_BUILD
  SEARCH_ROBBO_BASES = TRUE;
  SUPPRESS_INPUT = TRUE;
  TRIPLE_MAX_USAGE = MaximumTripleUsage ();
  if (RobboFare (POSITION))
    goto INFINITY;
  SUPPRESS_INPUT = FALSE;
  if (DO_PONDER && !WEAK_PROBE_PONDER)
    DesistBackGroundLoader ();
  else
    EmergeBackGroundLoader (); /* yet: LOAD_ON_WEAK_PROBE avails the fail */
#endif
  ROOT_PREVIOUS = -VALUE_MATE;
  EASY_MOVE = FALSE;
  JUMP_IS_SET = TRUE;
  POS = POSITION;
  ROOT_POSIZIONE0->tbhits = 0;
#ifdef YUSUF_MULTICORE
  if (TITANIC_MODE)
    {
      STUB_IVAN ();
      POS = &ROOT_POSITION[0][0];
    }
#endif
  z = setjmp (J);
  if (!z)
    {
      if (POS->wtm)
	TopWhite (POS);
      else
	TopBlack (POS);
    }
  SUPPRESS_INPUT = TRUE;
  if (TITANIC_MODE)
    {
      LOCK (SMP_IVAN);
      SMP_HAS_AKTIV = FALSE;
      UNLOCK (SMP_IVAN);
    }
  JUMP_IS_SET = FALSE;
  PREVIOUS_DEPTH = ROOT_DEPTH;
  if (POS == POSITION)
    {
      while (POS->DYN != (POS->DYN_ROOT + 1))
	{
	  if (!POS->DYN->move)
	    UndoNull (POS);
	  else
	    Undo (POS, POS->DYN->move);
	}
    }
  if (!SEARCH_ROBBO_BASES)
    POSITION->DYN->HASH ^= SQUISH_SPLAT;
  Information (POSITION, GetClock () - START_CLOCK, -32767, ROOT_SCORE, 32767);
 INFINITY:
  SUPPRESS_INPUT = FALSE;
  if (DO_INFINITE && !STOP)
    {
      INFINITE_LOOP = TRUE;
      while (!STOP)
	Input (POSITION);      
    }
#if 1
  if (DO_PONDER && !STOP && !PONDER_HIT)
    {
      INFINITE_LOOP = TRUE;
      while (!STOP && !PONDER_HIT)
	Input (POSITION);
    }
#endif
  INFINITE_LOOP = FALSE;
  SUPPRESS_INPUT = TRUE;
  OutputBestMove (POSITION);
  SUPPRESS_INPUT = FALSE;
  SEARCH_IS_DONE = TRUE;
  if (UCI_NEW_GAME)
    NewGame (POSITION, TRUE);
  if (STALL_MODE)
    SEND ("complete-search\n"); /* HACK */
  while (STALL_MODE)
    NANO_SLEEP (1000000);
}

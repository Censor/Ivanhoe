
#include "RobboLito.h"
#include <string.h>
#include "control.h"

static sint64 LAST_MESSAGE;
static sint64 ABSOLUTE_TIME, DESIRED_TIME, INCREMENT;
static int DEPTH;
static boolean NEW_PONDERHIT;

jmp_buf J;

static sint64 BATTLE_TIME, EASY_TIME, ORDINARY_TIME;

void ponderhit () /* ABSOLUTE_TIME se cambia ? */
{
  if (!DO_PONDER)
    return;
  PONDER_HIT = TRUE;
  DO_PONDER = FALSE;
  NEW_PONDERHIT = TRUE;
  ABSOLUTE_TIME += GetClock() - START_CLOCK;
#if 0
  EASY_TIME += EASY_TIME / 4;
  BATTLE_TIME += BATTLE_TIME / 4;
  ORDINARY_TIME += ORDINARY_TIME / 4;
#endif
}

void HaltSearch (int d, int tr)
{
  STOP = TRUE;
  DO_PONDER = FALSE;
  DO_INFINITE = FALSE;
  SUPPRESS_INPUT = TRUE;
#ifdef YUSUF_MULTICORE
  if (TITANIC_MODE)
    {
      IVAN_ALL_HALT = TRUE;
      if (d == 0)
	ivan_end_smp ();
    }
  else if (JUMP_IS_SET)
    longjmp (J, 1);
#else
  if (JUMP_IS_SET)
    longjmp (J, 1);
#endif
}

static float math_power (float x, int n)
{
  int i;
  float ans = 1.0f;
  for (i = 0; i < n; i++)
    ans *= x;
  return ans;
}

#define sqrt2 (1.4142135623730950488)
int DoHashFull (uint64 x)
{
  int d, u;
  float c = 0.499; /* HACK */
  typeHash* Trans;
  for (d = 0; d < 1000; d++)
    {
      x= (0xeca97530f2468bd1) * x + 0x43218765edcb09af; /* sample 1000 */
      Trans = HASH_POINTER0 (x);
      if (ICI_HASH_MIX)
	{
	  u = MAX (trans->DepthLower, trans->DepthUpper)
	           - 3 * MIN (((uint8) (Trans->age - GLOBAL_AGE)),
			      ((uint8) (GLOBAL_AGE - Trans->age)));
	  if (u > 1)
	    c += 1.0 - (sqrt2 - 1.0) / (math_power (sqrt2, u) - 1.0);
	}
      else
	if (Trans->hash && Trans->age == GLOBAL_AGE)
	  c += 1.0f;
    }
  return (int) c;
}

void Info (sint64 x)
{
  uint64 t, nps, NODES = 0;
  int cpu, rp, hash_full = 0;
  clock_t u;
  uint64 TBHITS = 0;
  char send_str[4096];

  if (x < 1000ULL * OUTPUT_DELAY)
    return;
  DO_OUTPUT = TRUE;
  if (!TITANIC_MODE && ZOG_AKTIV) /* fix 1 only cpu con ZOG */
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
  u = clock () - INIT_CLOCK; /* CLOCKS_PER_SEC always 1000 ? */
  if (DO_HASH_FULL)
    hash_full = DoHashFull (((sint64) u) + x + NODES); /* HACK, random seed */
  LAST_MESSAGE = x;
  t = x / 1000;
  if (t == 0)
    nps = 0;
  else
    nps = NODES / t;
  sprintf (send_str,
	   "info time " TYPE_64_BIT " nodes " TYPE_64_BIT " nps " TYPE_64_BIT,
	   t, NODES, nps * 1000);
  if (hash_full)
    sprintf (send_str + strlen (send_str), " hashfull %d", hash_full);
  if (TBHITS)
    sprintf (send_str + strlen (send_str), " tbhits " TYPE_64_BIT, TBHITS);
  sprintf (send_str + strlen (send_str), 
	   " cpuload %d\n", (int) ((double) u / (double) x * 1000.0));
  SEND (send_str);
}

/********************************/ /********************************/

static int moves_in_control, total_white_time, total_black_time, old_mtg;
static boolean INCREASED_DEPTH;
static float time_factor;

void CheckDoneAlternative (typePOS* POSITION, int d)
{
  sint64 x;
  if (IVAN_ALL_HALT)
    {
      HaltSearch (d, 1);
      return;
    }
  x = GetClock () - START_CLOCK;
#ifdef UTILITIES
  if (DEBUG_TIME_MANAGEMENT)
    {
      SEND ("DebugTimeA: time: " TYPE_64_BIT " depth %d ", x, d);
      SEND ("DO_PONDER %d ABSOLUTE TIME " TYPE_64_BIT "\n",
	    DO_PONDER, ABSOLUTE_TIME);
    }
#endif
  if (d && d == DEPTH)
    {
      HaltSearch (d, 1);
      return;
    }
  if (x - LAST_MESSAGE > 1000000)
    Info (x);
  if (DO_PONDER)
    goto END; /* Decembrist */
  if (DO_INFINITE)
    goto END; /* Decembrist */
  if (d >= 1 && d < 8) /* porque? */
    goto END;
  if (x > ABSOLUTE_TIME)
    {
      HaltSearch (d, 1);
      return;
    }
  if (d == 0 && !NEW_PONDERHIT)
    goto END;
#if 1 || defined (UTILITIES)
  if (1 || DEBUG_TIME_MANAGEMENT)
    {
      SEND ("DebugTimeA: NEW_PONDERHIT %d INCREASED_DEPTH %d time_factor %f ",
	      NEW_PONDERHIT, INCREASED_DEPTH, time_factor);
      SEND ("MOVES: EASY %d BATTLE %d BAD %d ",
	      EASY_MOVE, BATTLE_MOVE, BAD_MOVE);
      SEND (" TIMES: EASY " TYPE_64_BIT " BATTLE " TYPE_64_BIT
	    " ORDINARY " TYPE_64_BIT " CLOCK " TYPE_64_BIT "\n",
	    EASY_TIME, BATTLE_TIME, ORDINARY_TIME, x);
    }
#endif
  NEW_PONDERHIT = FALSE;
  /* Alternative TimeUsage */
#define INCREASE_DEPTH() { INCREASED_DEPTH = TRUE; goto END; }
#define CHECK_INCREASE_DEPTH() \
  { if (!INCREASED_DEPTH && time_factor > 1.05) INCREASE_DEPTH (); \
    HaltSearch (d, 2); return; }
  if (!BAD_MOVE && x >= BATTLE_TIME)
    CHECK_INCREASE_DEPTH();
  if (EASY_MOVE && x >= EASY_TIME)
    CHECK_INCREASE_DEPTH();
  if (!BATTLE_MOVE && x >= ORDINARY_TIME && !BAD_MOVE)
    CHECK_INCREASE_DEPTH();
 END:
  if (d && TITANIC_MODE)
    return;
  while (TryInput ())
    Input (POSITION);
}

void CheckDone (typePOS* POSITION, int d)
{
  sint64 x;
  if (MONTE_CARLO)
    return;
  if (!ROOT_BEST_MOVE)
    return; /* HACK */ /* Vlad0 */
  if (IVAN_ALL_HALT)
    {
      HaltSearch (d, 1);
      return;
    }
  if (SUPPRESS_INPUT)
    return;
  if (!JUMP_IS_SET)
    return;
  if (USE_ALTERNATIVE_TIME)
    {
      CheckDoneAlternative (POSITION, d);
      return;
    }
  x = GetClock () - START_CLOCK;
#if 1
  if (DEBUG_TIME_MANAGEMENT)
    {
      SEND ("DebugTime: time: " TYPE_64_BIT " depth %d ", x, d);
      SEND ("DO_PONDER %d ABSOLUTE TIME " TYPE_64_BIT "\n",
	    DO_PONDER, ABSOLUTE_TIME);
    }
#endif
  if (d && d == DEPTH)
    {
      HaltSearch (d, 1);
      return;
    }
  if (x - LAST_MESSAGE > 1000000)
    Info (x);
  if (DO_PONDER)
    goto END; /* Decembrist */
  if (DO_INFINITE)
    goto END; /* Decembrist */
  if (d >= 1 && d < 8)
    goto END;
  if (x > ABSOLUTE_TIME)
    {
      HaltSearch (d, 1);
      return;
    }
  if (d == 0 && !NEW_PONDERHIT)
    goto END;
#if defined (UTILITIES)
  if (DEBUG_TIME_MANAGEMENT)
    {
      SEND ("DebugTime: NEW_PONDERHIT %d INCREASED_DEPTH %d time_factor %f ",
	      NEW_PONDERHIT, INCREASED_DEPTH, time_factor);
      SEND ("MOVES: EASY %d BATTLE %d BAD %d ",
	      EASY_MOVE, BATTLE_MOVE, BAD_MOVE);
      SEND (" TIMES: EASY " TYPE_64_BIT " BATTLE " TYPE_64_BIT
	    " ORDINARY " TYPE_64_BIT " CLOCK " TYPE_64_BIT "\n",
	    EASY_TIME, BATTLE_TIME, ORDINARY_TIME, x);
    }
#endif
  NEW_PONDERHIT = FALSE;
  /* Default TimeUsage */
  if (!BAD_MOVE && x >= BATTLE_TIME)
    {
      HaltSearch (d, 2);
      return;
    }
  if (EASY_MOVE && x >= EASY_TIME)
    {
      HaltSearch (d, 3);
      return;
    }
  if (!BATTLE_MOVE && x >= ORDINARY_TIME && !BAD_MOVE)
    {
      HaltSearch (d, 4);
      return;
    }
 END:
  if (d && TITANIC_MODE)
    return;
  while (TryInput ())
    {
      Input (POSITION);
      if (d == 0 && TITANIC_MODE && !SMP_HAS_AKTIV)
	return;
    }
}

/********************************/ /********************************/

void TimeManagerAlternative
(sint64 TOTAL_TIME, sint64 TIME, sint64 INCREMENT, int mtg)
{
  TIME = MAX (TIME - 500000, (9 * TIME) / 10);
  if (mtg)
    {
      old_mtg = mtg;
      time_factor = (float) (TIME * moves_in_control) / (float) (TOTAL_TIME * mtg);
      if (time_factor < 1.0)
	DESIRED_TIME = TIME / mtg + INCREMENT;
      else
	DESIRED_TIME =
	  MIN (TIME * time_factor * time_factor / mtg + INCREMENT, TIME);
      if (time_factor < 1.2 && mtg > 2)
	ABSOLUTE_TIME =
	  MIN ((TIME * time_factor * (mtg + 2)) / (mtg + 3),
	       6 * DESIRED_TIME);
      else
	ABSOLUTE_TIME = (TIME * (mtg + 1)) / (mtg + 2);
      ABSOLUTE_TIME = MAX (10000, ABSOLUTE_TIME);
    }
#define NUMBER (UCI_PONDER ? 6 : 0)
  else /* no mtg */
    {
      int temp = TIME / 20;
      float sf;
      if (temp > INCREMENT)
	{
	  DESIRED_TIME =
	    INCREMENT + (temp * TIME) / ((50 - NUMBER) * temp - 20 * INCREMENT);
	  ABSOLUTE_TIME = (temp * TIME) / (5 * temp - 3 * INCREMENT);
	}
      else
	{
	  ABSOLUTE_TIME = TIME >> 1;
	  DESIRED_TIME = INCREMENT + TIME / (30 - NUMBER);
	}
      sf = (float) (TIME + 20 * INCREMENT) / (float) (TOTAL_TIME);
      if (6.0 * sf < 1.0)
	ABSOLUTE_TIME /= 2;
      else if (3.0 * sf < 1.0)
	ABSOLUTE_TIME *= (3.0 * sf);
      if (TIME < 500000)
	{
	  ABSOLUTE_TIME = 10000;
	  DESIRED_TIME = 5000;
	}
      if (TIME >= 500000 && TIME < 1000000 && INCREMENT < 500000)
	{
	  ABSOLUTE_TIME = (TIME * 5) / 100;
	  DESIRED_TIME = (TIME * 5) / 400;
	}
      if (TIME >= 1000000 && TIME < 2000000 && INCREMENT < 500000)
	{
	  ABSOLUTE_TIME = TIME / 10;
	}
    }
  DESIRED_TIME = MAX (5000, DESIRED_TIME);
  EASY_TIME = DESIRED_TIME >> 2;
  ORDINARY_TIME = 3 * EASY_TIME;
  BATTLE_TIME = DESIRED_TIME;
}

void TimeManager (sint64 TIME, sint64 OPP_TIME, sint64 INCREMENT, int mtg)
{  
  double rat;
  uint64 mult;
  if (mtg)
    {
      if (mtg > 25)
	mtg = 25;
      DESIRED_TIME = TIME / mtg + INCREMENT;
      ABSOLUTE_TIME =
	(TIME * mtg) / (4 * mtg - 3) - MIN (1000000, TIME / 10);
      if (mtg == 1)
	ABSOLUTE_TIME -= MIN (1000000, ABSOLUTE_TIME / 10);
      if (ABSOLUTE_TIME < 1000) /* porque? */
	ABSOLUTE_TIME = 1000;
    }
  else
    {
      ABSOLUTE_TIME = (TIME * ABSOLUTE_PERCENT) / 100 - 10000;
      if (ABSOLUTE_TIME < 1000)
	ABSOLUTE_TIME = 1000;
      DESIRED_TIME = (TIME * DESIRED_MILLIS) / 1000 + INCREMENT;
    }
  if (SINCE_NEW_GAME < BOOK_EXIT_MOVES)
    DESIRED_TIME +=
      (DESIRED_TIME * (BOOK_EXIT_MOVES - SINCE_NEW_GAME)) / BOOK_EXIT_MOVES;
#if 1
  if (TIME_IMITATE && !mtg)
    {
      rat = ((double) (TIME + 1)) / ((double) (OPP_TIME + 1));
      if (rat > 2.0)
	rat = 2.0;
      if (rat < 0.5)
	rat = 0.5;
      if (rat > 1.0)
	{
	  mult = ((int) (1024.0 * rat)) - 1024;
	  DESIRED_TIME += (mult * DESIRED_TIME) >> (10 + 2); /* max 1/4 */ 
	}
      else
	{
	  mult = ((int) (1024.0 / rat)) - 1024;
	  DESIRED_TIME -= (mult * DESIRED_TIME) >> (10 + 2); /* max 1/4 */ 
	}
    }
#endif
#if 1
  if (ROOT_DEPTH && TIME_LOSE_MORE && DESIRED_TIME > 1000000)
    {
      if (ROOT_SCORE < -25 && ROOT_SCORE >= -50)
	DESIRED_TIME += DESIRED_TIME >> 3;
      if (ROOT_SCORE < -50 && ROOT_SCORE >= -75)
	DESIRED_TIME += DESIRED_TIME >> 2;
      if (ROOT_SCORE < -75 && ROOT_SCORE >= -100)
	DESIRED_TIME += DESIRED_TIME >> 3;
    }
#endif
#if 1
  if (ROOT_DEPTH && TIME_WIN_MORE && DESIRED_TIME > 1000000)
    {
      if (ROOT_SCORE > 25 && ROOT_SCORE <= 50)
	DESIRED_TIME += DESIRED_TIME >> 3;
      if (ROOT_SCORE > 50 && ROOT_SCORE <= 75)
	DESIRED_TIME += DESIRED_TIME >> 2;
      if (ROOT_SCORE > 75 && ROOT_SCORE <= 100)
	DESIRED_TIME += DESIRED_TIME >> 3;
    }
#endif
  if (DESIRED_TIME > ABSOLUTE_TIME)
    DESIRED_TIME = ABSOLUTE_TIME;
  if (DESIRED_TIME < 1000)
    DESIRED_TIME = 1000;
  EASY_TIME = (DESIRED_TIME * EASY_FACTOR) / 100;
  if (UCI_PONDER)
    EASY_TIME = (DESIRED_TIME * EASY_FACTOR_PONDER) / 100;
  BATTLE_TIME = (DESIRED_TIME * BATTLE_FACTOR) / 100;
  ORDINARY_TIME = (DESIRED_TIME * ORDINARY_FACTOR) / 100;
}

/************************************************************/

#define INFINITY 0x7ffffffffffffff /* sint64 */
#define STRTOK(p) p = strtok (NULL, " ")

void InitSearch (typePOS* POSITION, char* str)
{
  char *p;
  sint64 wtime = INFINITY, btime = INFINITY,
    winc = 0, binc = 0, TIME, OPP_TIME, mtg = 0;
  int sm = 0;

  MONTE_CARLO = FALSE;
  DEPTH = 255;
  ABSOLUTE_TIME = DESIRED_TIME = INFINITY;
  STOP = FALSE;
  DO_INFINITE = FALSE;
  DO_PONDER = FALSE;
  NEW_PONDERHIT = FALSE;
  DO_SEARCH_MOVES = FALSE;
  LAST_MESSAGE = 0;
#ifdef MODE_ANALYSIS
  p = strstr (str, "montecarlo");
  if (p)
    {
      MonteCarlo (POSITION, p + 11);
      MONTE_CARLO = TRUE;
      SEND ("MonteCarlo finds completion\n");
      return;
    }
#endif
  p = strtok (str, " ");
  for (STRTOK (p); p != NULL; STRTOK (p))
    {
      if (!strcmp (p, "depth"))
	{
	  STRTOK (p);
	  DEPTH = MAX (1, atoi (p));
	}
      else if (!strcmp (p, "movetime"))
	{
	  STRTOK (p);
	  ABSOLUTE_TIME = MAX (1, atoll (p)) * 1000; /* - 10000 ? */
	}
      else if (!strcmp (p, "wtime"))
	{
	  STRTOK (p);
	  wtime = atoll (p) * 1000;
	}
      else if (!strcmp (p, "winc"))
	{
	  STRTOK (p);
	  winc = atoll (p) * 1000;
	}
      else if (!strcmp (p, "btime"))
	{
	  STRTOK (p);
	  btime = atoll (p) * 1000;
	}
      else if (!strcmp (p, "binc"))
	{
	  STRTOK (p);
	  binc = atoll (p) * 1000;
	}
      else if (!strcmp (p, "movestogo"))
	{
	  STRTOK (p);
	  mtg = atoi (p);
	}
      else if (!strcmp (p, "infinite"))
	DO_INFINITE =  TRUE;
      else if (!strcmp (p, "ponder"))
	DO_PONDER = TRUE;
      else if (!strcmp (p, "searchmoves"))
	DO_SEARCH_MOVES = TRUE;
      else if (DO_SEARCH_MOVES)
	SEARCH_MOVES[sm++] = numeric_move (POSITION, p);
      else
	ERROR_END ("go string: %s", p);
    }
  BATTLE_TIME = INFINITY;
  ORDINARY_TIME = INFINITY;
  EASY_TIME = INFINITY;

  /* TimeManagerAlternative */
  time_factor = 1.0;
  INCREASED_DEPTH = FALSE;
  if (NEW_GAME || mtg > old_mtg) /* reset */
    {
      moves_in_control = mtg;
      total_white_time = MAX (wtime - 500000, (95 * wtime) / 100);
      total_black_time = MAX (btime - 500000, (95 * btime) / 100);
    }

  TIME = POSITION->wtm ? wtime : btime;
  OPP_TIME = POSITION->wtm ?  btime : wtime;
  TIME = TIME - BUFFER_TIME * 1000ULL;
  if (TIME < 0)
    TIME = 0;  
  if (TIME == INFINITY)
    goto END;
  INCREMENT = POSITION->wtm ? winc : binc;
  if (USE_ALTERNATIVE_TIME)
    TimeManagerAlternative (POSITION->wtm ? total_white_time : total_black_time,
			    TIME, INCREMENT, mtg); /* alternativo */
  else
    TimeManager (TIME, OPP_TIME, INCREMENT, mtg);
 END:
  if (TIME == INFINITY)
    ANALYSING = TRUE;
  else
    ANALYSING = FALSE;
  if (DO_SEARCH_MOVES)
    SEARCH_MOVES[sm] = MOVE_NONE;
}

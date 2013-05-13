#ifdef MODE_ANALYSIS
#include "RobboLito.h"
#include <string.h>
#include "MonteCarlo.h"

void COPY_POSITION (typePOS* CHILD, typePOS* PARENT) /* import */
{
  int h;
  memcpy (CHILD, PARENT, NUM_BYTES_TO_COPY);
  memcpy (CHILD->DYN_ROOT, PARENT->DYN - 1, 2 * sizeof (typeDYNAMIC));
  CHILD->DYN = CHILD->DYN_ROOT + 1;
  h = PARENT->StackHeight;
  memcpy (CHILD->STACK, PARENT->STACK, h * sizeof (uint64));
  CHILD->StackHeight = h;
}

#define IS_ILLEGAL \
  ( !POSITION->wtm ? \
    (wBitboardK & POSITION->DYN->bAtt) :   (bBitboardK & POSITION->DYN->wAtt) )

#define IS_IN_CHECK \
  ( POSITION->wtm ? \
    (wBitboardK & POSITION->DYN->bAtt) :   (bBitboardK & POSITION->DYN->wAtt) )

static void AssuranceMoveList (typePOS * POSITION, type_Monte_Carlo * MC)
{
  typeMoveList * ml;
  typeMoveList * p;
  if (MC->mv == 0)
    {
      ml = CaptureMoves (POSITION, MC->SEARCH, 0xffffffffffffffff);
      ml = OrdinaryMoves (POSITION, ml);
    }
  else
    ml = MC->SEARCH + MC->mv;
  for (p = MC->SEARCH; p < ml; p++)
    {
      if ( ! (POSITION->wtm ?
	      WhiteOK (POSITION, p->move) : BlackOK (POSITION, p->move)))
	{
	  p->move = (p + 1)->move;
	  ml--;
	  p--;
	  continue;
	}
	Make (POSITION, p->move);
	Mobility (POSITION);
	if (IS_ILLEGAL)
	  {
	    Undo (POSITION, p->move);
	    p->move = (p + 1)->move;
	    ml--;
	    p--;
	    continue;
	  }
	else
	  Undo (POSITION, p->move);
    }
  MC->mv = ml - MC->SEARCH;
}

static void ParseMonteCarlo (typePOS * POSITION, char * str, type_Monte_Carlo * MC)
{
  char* p;
  boolean MOVE_LIST = FALSE;
  int i;
  LOCK_INIT (MC->SEND_LOCK);
  LOCK_INIT (MC->TABLE_LOCK);
  for (i = 0; i < 256; i++)
    MC->WIN[i] = MC->LOSS[i] = MC->SCORE[i] = MC->DO_COUNT[i] = 0;
  MC->length = 0xffff;
  MC->verbose_length = 0x0;
  MC->max = 10000;
  MC->min = -10000;
  MC->fixed_depth = 10;
  MC->cpus = 1;
  MC->verbose = FALSE;
  MC->mv = 0;
  p = strtok (str, " ");
  for (; p != NULL; STRTOK (p))
    {
      if (!strcmp (p, "length"))
	{
	  STRTOK (p);
	  MC->length = MAX (1, atoi (p));
	}
      else if (!strcmp (p, "vlength"))
	{
	  STRTOK (p);
	  MC->verbose_length = MAX (1, atoi (p));
	}
      else if (!strcmp (p, "depth"))
	{
	  STRTOK (p);
	  MC->fixed_depth = MAX (5, atoi (p));
	}
      else if (!strcmp (p, "max"))
	{
	  STRTOK (p);
	  MC->max = MAX (MIN (atoi (p), 10000), -9000);
	}
      else if (!strcmp (p, "min"))
	{
	  STRTOK (p);
	  MC->min = MIN (MAX (atoi (p), -10000), 9000);
	}
      else if (!strcmp (p, "cpus"))
	{
	  STRTOK (p);
	  MC->cpus = MIN (MAX (1, atoi (p)), MAX_CPUS);
	}
      else if (!strcmp (p, "verbose"))
	MC->verbose = TRUE;
      else if (!strcmp (p, "moves"))
	MOVE_LIST = TRUE;
      else if (MOVE_LIST)
	MC->SEARCH[MC->mv++].move = numeric_move (POSITION, p);
    }  
}

void MC_StoreResult (type_Monte_Carlo * MC, int moveno, int value)
{
  LOCK (MC->TABLE_LOCK);
  if (value >= MC->max)
    MC->WIN[moveno]++;
  else if (value <= MC->min)
    MC->LOSS[moveno]++;
  else
    {
      MC->SCORE[moveno] += value;
      MC->DO_COUNT[moveno]++;
    }
  UNLOCK (MC->TABLE_LOCK);
}

int DecideMove (type_Monte_Carlo * MC)
{
  return (GET_RAND () % (MC->mv));
}

#define RETURN(x) return (x)
int MonteCarloForward (type_Monte_Carlo * MC, typePOS * POSITION)
{
  int apply = 0;
  uint32 move;
  int value, i, cpu;
  POSITION->height = 0;
  POSITION->DYN->age = GET_RAND () & 0xff;
  cpu = POSITION->cpu; /* HACK */
  while (TRUE)
    {
      if (POSITION->wtm)
	value = TopWhiteMC (POSITION, MC->fixed_depth);
      else
	value = TopBlackMC (POSITION, MC->fixed_depth);
      if (POSITION->wtm != MC->ROOT_POSITION->wtm)
	value = - value;
      if (MONTE_CARLO_CURTAIL)
	return 0;
      if (value > MC->max || value < MC->min || apply >= MC->length)
	return value;
      apply++;
      move = POSITION->DYN->move;
      if (move == MOVE_NONE)
	return 0; /* patt */
      if (MC->verbose)
	MC->ML[cpu][MC->MLcnt[cpu]++] = move & 0x7fff;
      if (POSITION->sq[FROM (move)] == (POSITION->wtm ? wEnumP : bEnumP)
	  || POSITION->sq[TO (move)] != 0)
	POSITION->StackHeight = -1;
      Make (POSITION, move);
      POSITION->height = 0;
      POSITION->DYN->age++;
      memcpy (POSITION->DYN_ROOT + 1, POSITION->DYN, sizeof (typeDYNAMIC));
      POSITION->DYN = POSITION->DYN_ROOT + 1;
      CheckRepetition ((IS_IN_CHECK));
    }
  return 0;
}


#ifdef WINDOWS
#define MonteCarloThread(x) DWORD WINAPI monte_carlo_cpu (LPVOID x)
HANDLE MC_Thread[64];
#else
#define MonteCarloThread(x) void* monte_carlo_cpu (void * x)
pthread_t MC_Thread[64];
#endif

int ValueCpu;
MonteCarloThread(x)
{
  type_Monte_Carlo * MC;
  int result, cpu;
  int move ,i;
  char STRING[16];
  char SEND_STRING[65536];
  uint64 r;
  MC = (type_Monte_Carlo * ) (x);
  LOCK (MC->SEND_LOCK);
  cpu = ValueCpu;
  ValueCpu++; /* increment */
  UNLOCK (MC->SEND_LOCK);
  while (TRUE)
    {
      COPY_POSITION (MC->POSITION[cpu], MC->ROOT_POSITION);
      MC->POSITION[cpu]->cpu = cpu;
      r = GET_RAND ();
      MC->POSITION[cpu]->DYN->HASH ^= r;
      for (i = 0; i < MC->ROOT_POSITION->StackHeight; i++)
	MC->POSITION[cpu]->STACK[i] ^= r; /* move-list */
      move = DecideMove (MC);
      MC->MLcnt[cpu] = 0;
      if (MC->verbose)
	MC->ML[cpu][MC->MLcnt[cpu]++] = MC->SEARCH[move].move & 0x7fff;
      Make (MC->POSITION[cpu], MC->SEARCH[move].move);
      result = MonteCarloForward (MC, MC->POSITION[cpu]);
      if (MONTE_CARLO_CURTAIL)
	return NULL;
      MC_StoreResult (MC, move, result);
      sprintf (SEND_STRING, "MCresult %s %d %d",
	       Notate (MC->SEARCH[move].move, STRING), result, cpu);
      if (MC->verbose)
	for (i = 0; i < MC->MLcnt[cpu] && i < MC->verbose_length; i++)
	  {
	    strcat (SEND_STRING, " ");
	    strcat (SEND_STRING, Notate (MC->ML[cpu][i], STRING));
	  }
      strcat (SEND_STRING, "\n");
      LOCK (MC->SEND_LOCK); /* ensure */
      SEND (SEND_STRING);
      UNLOCK (MC->SEND_LOCK);
    }
}

void EffectMonteCarlo (typePOS * POSITION, type_Monte_Carlo * MC)
{
  int cpu;
  boolean REPETITION;
  typeDYNAMIC* p;
  typeDYNAMIC* S;
  typeDYNAMIC* q;

  POSITION->height = 0;
  POSITION->StackHeight = -1;
  S = MAX (POSITION->DYN_ROOT + 1, POSITION->DYN - POSITION->DYN->reversible);
  for (p = S; p <= POSITION->DYN; p++)
    POSITION->STACK[++(POSITION->StackHeight)] = p->HASH;
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
  if (POSITION->StackHeight == -1)
    POSITION->StackHeight = 0;

  memset ( (void*) (MC->ROOT_POSITION), 0, sizeof (typePOS));
  MC->ROOT_POSITION->DYN_ROOT = malloc (MAXIMUM_PLY * sizeof (typeDYNAMIC));
  COPY_POSITION (MC->ROOT_POSITION, POSITION);
  MC->ROOT_POSITION->DYN =  MC->ROOT_POSITION->DYN_ROOT + 1;
  for (cpu = 0; cpu < MC->cpus; cpu++)
    {
      memset ( (void*) (MC->POSITION[cpu]), 0, sizeof (typePOS));
      MC->POSITION[cpu]->DYN_ROOT = malloc (MAXIMUM_PLY * sizeof (typeDYNAMIC));
      MC->POSITION[cpu]->DYN =  MC->POSITION[cpu]->DYN_ROOT + 1;
    }

  ValueCpu = 0;
  MONTE_CARLO_CURTAIL = FALSE;
  for (cpu = 0; cpu < MC->cpus; cpu++)
    PTHREAD_CREATE (&MC_Thread[cpu], NULL, monte_carlo_cpu, MC);
  while (TRUE)
    {
      NANO_SLEEP (1000000);
      if (TryInput ()) /* HACK */ /* input for the any */
	{
	  MONTE_CARLO_CURTAIL = TRUE;
	  for (cpu = 0; cpu < MC->cpus; cpu++)
	    PTHREAD_JOIN (MC_Thread[cpu]);
	  break;
	}
    }
  free (MC->ROOT_POSITION->DYN_ROOT);
  for (cpu = 0; cpu < MC->cpus; cpu++)
    free (MC->POSITION[cpu]->DYN_ROOT);
}

void MonteCarlo (typePOS * POSITION, char * str)
{
  boolean tm = TITANIC_MODE;
  int rc = RANDOM_COUNT, rb = RANDOM_BITS;
  type_Monte_Carlo MC[1];
  SEND ("Fact: MonteCarlo notes occurrence\n");
  MONTE_CARLO = TRUE;
  TITANIC_MODE = FALSE; /* HACK */
  if (rc == 0)
    {
      RANDOM_COUNT = 8;
      RANDOM_BITS = 1; /* annual */
    }
  ParseMonteCarlo (POSITION, str, MC);  
  AssuranceMoveList (POSITION, MC);
  if (MC->mv != 0)  
    EffectMonteCarlo (POSITION, MC);
  else
    SEND ("info string No legal moves for MonteCarlo\n");
  TITANIC_MODE = tm;
  RANDOM_COUNT = rc;
  RANDOM_BITS = rb;
}
#endif

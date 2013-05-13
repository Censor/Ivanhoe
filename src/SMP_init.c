
#include "RobboLito.h"
#include <time.h>
void NANO_SLEEP (int x)
{
#ifdef WINDOWS
  Sleep (x / 1000000);
#else
  struct timespec TS[1];
  TS->tv_sec = 0;
  TS->tv_nsec = x;
  nanosleep (TS, NULL);
#endif
}


#ifdef YUSUF_MULTICORE
#define DO_LOCKED(x) { LOCK (SMP_IVAN); (x); UNLOCK (SMP_IVAN); }
#define MAX_SPLIT 8

typedef struct
{
  int cpu;
}
t_args;

t_args ARGS[MAX_CPUS];

volatile int init_threads;
volatile int io_init;
typePOS* volatile WORKING[MAX_CPUS];

void thread_stall (typePOS*, int);

void ivan_end_smp()
{
  int cpu, rp, sp;
  IVAN_ALL_HALT = TRUE;
  while (TRUE)
    {
      for (cpu = 0; cpu < CPUS_SIZE; cpu++)
	SIGNAL_CON_LOCK (PTHREAD_COND_WAIT[cpu], PTHREAD_COND_MUTEX[cpu]);
      for (cpu = 0; cpu < CPUS_SIZE; cpu++)
	for (rp = 0; rp < RP_PER_CPU ; rp++)
	  ROOT_POSITION[cpu][rp].stop = TRUE;
      LOCK (SMP_IVAN);
      if (!SMP_HAS_AKTIV)
	break;
      UNLOCK (SMP_IVAN);
    }
  UNLOCK (SMP_IVAN);
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    for (rp = 0; rp < RP_PER_CPU ; rp++)
      ROOT_POSITION[cpu][rp].used = FALSE;
  for (sp = 0; sp < MAX_SP; sp++)
    ROOT_SP[sp].aktiv = FALSE;
}

IVAN_THREAD(A)
{
  t_args* B;
  B = (t_args*) A;
  DO_LOCKED (init_threads++);
#ifdef BUILD_ZOG_MP_COMPILE
  if (ZOG_AKTIV)
    ZOG_TRAP (B->cpu);
  else
#endif
    thread_stall (NULL_PARENT, B->cpu);
  return (VOID_STAR_TYPE) NULL;
}

static boolean volatile DESTROY, DESTROYED;
IO_THREAD(A)
{
  while (TRUE)
    {
      IO_AWAKE = FALSE;
      STALL_MODE = TRUE;
      while (!SEARCH_IS_DONE)
	NANO_SLEEP (1000000);
      STALL_MODE = FALSE;
      io_init = TRUE;
      SUPPRESS_INPUT = FALSE;
    WAIT_AGAIN:
      WAIT_CON_LOCK (*WAKEUP_IO, *WAKEUP_LOCK_IO);
      if (DESTROY)
	break;
      if (!SMP_HAS_AKTIV) /* predicate */
	goto WAIT_AGAIN;
      IO_AWAKE = TRUE;
      while (SMP_HAS_AKTIV)
	{
	  STALL_MODE = TRUE;
	  NANO_SLEEP (1000000);
	  if (SMP_HAS_AKTIV)
	    CheckDone (ROOT_POSIZIONE0, 0);
	}
    }
  DESTROYED = TRUE;
  /* EXIT in WINDOWS? */
  return (VOID_STAR_TYPE) NULL;
}

static int CURR_CPUS = 0;
static void ivan_cleanup()
{
  int cpu;
  for (cpu = 0; cpu < CURR_CPUS; cpu++)
    {
#ifndef WINDOWS
      LOCK_DESTROY (&PTHREAD_COND_MUTEX[cpu]);
#endif
      if (cpu > 0)
	{
	  DIE[cpu] = TRUE;
	  SIGNAL_CON_LOCK (PTHREAD_COND_WAIT[cpu], PTHREAD_COND_MUTEX[cpu]);
	  PTHREAD_JOIN (PTHREAD[cpu]);
	  DIE[cpu] = FALSE;
	}
#ifndef WINDOWS
      LOCK_INIT (&PTHREAD_COND_MUTEX[cpu]);
#endif
    }
  DESTROY = TRUE;
  DESTROYED = FALSE;
  SIGNAL_CON_LOCK (*WAKEUP_IO, *WAKEUP_LOCK_IO);  
  while (!DESTROYED)
    {
    }
#ifndef WINDOWS
  LOCK_DESTROY (WAKEUP_LOCK_IO);
#endif
  DESTROYED = FALSE;
}

int ivan_init_smp()
{
  int cpu, rp, sp;

  DESTROY = FALSE;
  if (CURR_CPUS)
    ivan_cleanup();
  DESTROY = FALSE;
  IVAN_ALL_HALT = FALSE;
  COND_INIT (*WAKEUP_IO, *WAKEUP_LOCK_IO);
  SMP_FREE = 0;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    for (rp = 0; rp < RP_PER_CPU ; rp++)
      {
	ROOT_POSITION[cpu][rp].used = FALSE;
	ROOT_POSITION[cpu][rp].nodes = 0;
	ROOT_POSITION[cpu][rp].tbhits = 0;
      }
  for (sp = 0; sp < MAX_SP; sp++)
    ROOT_SP[sp].aktiv = FALSE;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    WORKING[cpu] = NULL;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    DIE[cpu] = FALSE;
  NULL_PARENT->child_count = 123;
  io_init = FALSE;
  PTHREAD_CREATE (&PTHREAD_IO, NULL, io_thread, NULL);
  init_threads = 1;
  for (cpu = 1; cpu < CPUS_SIZE; cpu++)
    {
      ARGS[cpu].cpu = cpu;
      PTHREAD_CREATE (&PTHREAD[cpu], NULL, ivan_thread, &ARGS[cpu]);
    }
  while (init_threads < CPUS_SIZE || !io_init)
    NANO_SLEEP (1000000);
  CURR_CPUS = CPUS_SIZE;
  NANO_SLEEP (1000000);
  return CPUS_SIZE;
}

static void sp_init ()
{
  int sp;
  for (sp = 0; sp < MAX_SP; sp++)
    {
      ROOT_SP[sp].aktiv = FALSE;
      LOCK_INIT (ROOT_SP[sp].splock);
    }
}

static boolean IVAN_INIT = FALSE;
void rp_init ()
{
  int cpu;
  int rp;
  if (IVAN_INIT)
    return;
  TITANIC_MODE = FALSE; /* init */  
  LOCK_INIT (SMP_IVAN);
  for (cpu = 0; cpu < MAX_CPUS; cpu++)
    for (rp = 0; rp < RP_PER_CPU ; rp++)
    {
      memset( (void*) &ROOT_POSITION[cpu][rp], 0, sizeof (typePOS));
      ROOT_POSITION[cpu][rp].DYN_ROOT =
	malloc (MAXIMUM_PLY * sizeof(typeDYNAMIC)); /* SLAB_malloc */
      ROOT_POSITION[cpu][rp].used =  FALSE;
      ROOT_POSITION[cpu][rp].parent = NULL;
      ROOT_POSITION[cpu][rp].DYN = ROOT_POSITION[cpu][rp].DYN_ROOT;
      ROOT_POSITION[cpu][rp].cpu = cpu;
      ROOT_POSITION[cpu][rp].rp = rp;
      LOCK_INIT (ROOT_POSITION[cpu][rp].padlock);
    }
  for (cpu = 0; cpu < MAX_CPUS; cpu++)
    COND_INIT (PTHREAD_COND_WAIT[cpu], PTHREAD_COND_MUTEX[cpu]);
  sp_init ();
  IVAN_INIT = TRUE;
}
#endif


#define YUSUF_MULTICORE
#ifdef YUSUF_MULTICORE
#include "RobboLito.h"
#define DO_LOCKED(x) { LOCK (SMP_IVAN); (x); UNLOCK (SMP_IVAN); }
#define MAX_SPLIT 8
#include <signal.h>

volatile uint64 SMP_FREE;
typePOS* volatile WORKING[MAX_CPUS];

#ifdef BUILD_ZOG_MP_COMPILE
#include "ZOG_NODE.h"
#endif

void STUB_IVAN ()
{
  typePOS* RP00;
  int h, cpu, rp, sp;
  int volatile x;
  RP00 = &ROOT_POSITION[0][0];
  RP00->used = TRUE;
  RP00->stop = FALSE;
  memcpy (RP00, ROOT_POSIZIONE0, NUM_BYTES_TO_COPY);
  memcpy (RP00->DYN_ROOT, ROOT_POSIZIONE0->DYN_ROOT, 2 * sizeof (typeDYNAMIC));
  RP00->DYN = RP00->DYN_ROOT + 1;
  h = ROOT_POSIZIONE0->StackHeight;
  memcpy (RP00->STACK, ROOT_POSIZIONE0->STACK, h * sizeof (uint64));
  RP00->StackHeight = h;
  RP00->child_count = 0;
  RP00->parent = NULL;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    RP00->children[cpu] = NULL;
  IVAN_ALL_HALT = FALSE;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    for (rp = 0; rp < RP_PER_CPU ; rp++)
      {
        ROOT_POSITION[cpu][rp].used = FALSE;
        ROOT_POSITION[cpu][rp].stop = FALSE;
        ROOT_POSITION[cpu][rp].nodes = 0;
        ROOT_POSITION[cpu][rp].tbhits = 0;
      }
  ROOT_POSITION[0][0].used = TRUE;
  for (sp = 0; sp < MAX_SP; sp++)
    ROOT_SP[sp].aktiv = FALSE;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    WORKING[cpu] = NULL;
  NULL_PARENT->child_count = 123;
  RP00->SplitPoint = NULL;
  WORKING[0] = RP00;
  SMP_HAS_AKTIV = TRUE;
  SIGNAL_CON_LOCK (*WAKEUP_IO, *WAKEUP_LOCK_IO);
  for (x = 0; x < 10000; x++);
  SEARCH_IS_DONE = TRUE; /* HACK */
  while (!IO_AWAKE)
    {
      SIGNAL_CON_LOCK (*WAKEUP_IO, *WAKEUP_LOCK_IO);
      NANO_SLEEP (1000000);
    }
  SEARCH_IS_DONE = FALSE; /* HACK */
}

static void SMP_GOOD_HISTORY (typePOS* POS, uint32 m, SPLITPUNKT* sp)
{
  int sv = HISTORY_VALUE (POS, m);
  HISTORY_VALUE (POS, m) = sv + (( (0xff00 - sv) * sp->depth) >> 8);
}

void ivan_fail_high (SPLITPUNKT* sp, typePOS* POSITION, uint32 m)
{
  int cpu;
  LOCK (sp->splock);
  if (sp->tot || POSITION->stop)
    {
      UNLOCK (sp->splock);
      return;
    }
  sp->tot = TRUE;
  sp->move = m;
  sp->value = sp->beta;
  UNLOCK (sp->splock);
  if (POSITION->sq[TO (m)] == 0 && MoveHistory (m))
    SMP_GOOD_HISTORY (POSITION, m, sp);
  if (sp->node_type == NODE_TYPE_ALL)
    HashLowerALL (POSITION, m, sp->depth, sp->beta);
  else
    HashLower (POSITION->DYN->HASH, m, sp->depth, sp->beta);
  LOCK (SMP_IVAN);
  LOCK (POSITION->parent->padlock);
  if (!POSITION->stop)
    {
      for (cpu = 0; cpu < CPUS_SIZE; cpu++)
	if (POSITION->parent->children[cpu] && cpu != POSITION->cpu)
	  thread_halt (POSITION->parent->children[cpu]);
    }
  UNLOCK (POSITION->parent->padlock);
  UNLOCK (SMP_IVAN);
}

static INLINE void SMP_BAD_HISTORY (typePOS* POSITION, uint32 m, SPLITPUNKT* sp)
{
  if ((POSITION->DYN + 1)->cp == 0 && MoveHistory (m))
    {
      int sv = HISTORY_VALUE (POSITION, m);
      if (POSITION->DYN->Value > sp->alpha - 50)
	HISTORY_VALUE (POSITION, m) = sv - ((sv * sp->depth) >> 8);
    }
}

static void ivan_search_cut_node (typePOS* POSITION)
{
  SPLITPUNKT* sp;
  sp = POSITION->SplitPoint;
  LOCK (sp->splock);
  sp->childs++;
  UNLOCK (sp->splock);
  POSITION->wtm ? WhiteCutSMP (POSITION) : BlackCutSMP (POSITION);
  LOCK (sp->splock);
  sp->childs--;
  if (!sp->tot && !sp->childs && !POSITION->stop)
    HashUpperCUT (POSITION, sp->depth, sp->value);
  UNLOCK (sp->splock);
}

static void ivan_search_all_node (typePOS* POSITION)
{
  SPLITPUNKT* sp;
  sp = POSITION->SplitPoint;
  LOCK (sp->splock);
  sp->childs++;
  UNLOCK (sp->splock);
  POSITION->wtm ? WhiteAllSMP (POSITION) : BlackAllSMP (POSITION);
  LOCK (sp->splock);
  sp->childs--;
  if (!sp->tot && !sp->childs && !POSITION->stop)
    HashUpper (POSITION->DYN->HASH, sp->depth, sp->value);
  UNLOCK (sp->splock);
}

void ivan_search (typePOS* POSITION)
{
  SPLITPUNKT* sp;
  sp = POSITION->SplitPoint;
  if (sp->node_type == NODE_TYPE_ALL)
    {
      ivan_search_all_node (POSITION);
      return;
    }
  if (sp->node_type == NODE_TYPE_CUT)
    {
      ivan_search_cut_node (POSITION);
      return;
    }
  LOCK (sp->splock);
  sp->childs++;
  UNLOCK (sp->splock);
  POSITION->wtm ? WhitePVNodeSMP (POSITION) : BlackPVNodeSMP (POSITION);
  LOCK (sp->splock);
  sp->childs--;
  if (!sp->tot && !sp->childs && !POSITION->stop)
    {
      uint32 m = sp->good_move;
      if (m)
	{
	  HashExact (POSITION, m, sp->depth, sp->value, FLAG_EXACT);
	  if (POSITION->sq[TO (m)] == 0 && MoveHistory (m))
	    SMP_GOOD_HISTORY (POSITION, m, sp);
	}
      else
	  HashUpper (POSITION->DYN->HASH, sp->depth, sp->value);
    }
  UNLOCK (sp->splock);
}

static void COPY_FROM_CHILD (typePOS* PARENT, typePOS* CHILD)
{
  if (CHILD->SplitPoint->value >= CHILD->SplitPoint->beta)
    PARENT->DYN->move = CHILD->SplitPoint->move;
  else
    PARENT->DYN->move = 0;
}

void thread_stall (typePOS *PARENT, int cpu)
{
  typePOS* W;
  while (TRUE)
    {
      DO_LOCKED (SMP_FREE |= (1 << cpu));
      while (!WORKING[cpu] && PARENT->child_count && !ZOG_AVAILABLE)
	{
	  if (DIE[cpu])
	    return;
#ifdef WINDOWS
          WAIT_CON_LOCK (PTHREAD_COND_WAIT[cpu], PTHREAD_COND_MUTEX[cpu]);
#else
          LOCK (&PTHREAD_COND_MUTEX[cpu]);
          if (WORKING[cpu] || !PARENT->child_count || ZOG_AVAILABLE)
            {
              UNLOCK (&PTHREAD_COND_MUTEX[cpu]);
              break; // doble wakeup ?
            }
          WAIT (&PTHREAD_COND_WAIT[cpu], &PTHREAD_COND_MUTEX[cpu]);
          UNLOCK (&PTHREAD_COND_MUTEX[cpu]);
#endif
	  if (DIE[cpu])
	    return;
	}
#ifdef BUILD_ZOG_MP_COMPILE
      if (ZOG_AKTIV && ZOG_ROOT_DONE && cpu == 0)
	{
	  WORKING[0] = NULL;
	  longjmp (JMP_BUF[0], (1 << 29));
	}
#endif
      LOCK (SMP_IVAN); /* ZOG_REGISTRY ? */
      SMP_FREE &= ~(1 << cpu);
      W = WORKING[cpu];
      if (!W && PARENT != NULL_PARENT && !PARENT->child_count)
	{
	  WORKING[cpu] = PARENT;
	  UNLOCK (SMP_IVAN); 
	  return; /* IVAN_THREAD */
	}
#ifdef BUILD_ZOG_MP_COMPILE
      if (!W && ZOG_AVAILABLE && !ROOT_POSITION[cpu][0].used)
	{
	  WORKING[cpu] = NULL_PARENT; /* temporary */
	  UNLOCK (SMP_IVAN);
	  STUB_Znode (cpu); /* ZOG_AVAILABLE if fails? */
	  LOCK (SMP_IVAN);
	  WORKING[cpu] = NULL;
	  UNLOCK (SMP_IVAN);
	  continue;
	}
#endif
      UNLOCK (SMP_IVAN);
      if (!W)
	continue;
      ivan_search (W);
      LOCK (SMP_IVAN);
      LOCK (W->parent->padlock);
      COPY_FROM_CHILD (W->parent, W);
      W->parent->child_count--;

      if (W->parent->child_count == 0)
	{
	  int icpu = W->parent->cpu;
	  SIGNAL_CON_LOCK (PTHREAD_COND_WAIT[icpu], PTHREAD_COND_MUTEX[icpu]);
	}

      W->parent->children[cpu] = NULL;
      UNLOCK (W->parent->padlock);
      WORKING[cpu] = NULL;
      W->used = FALSE;
      UNLOCK (SMP_IVAN);
    }
}

void thread_halt (typePOS* POS)
{
  int n;  
  LOCK (POS->padlock);
  POS->stop = TRUE;
  for (n = 0; n < CPUS_SIZE; n++)
    {
      if (POS->children[n] != NULL)
	thread_halt (POS->children[n]);
    }
  UNLOCK (POS->padlock);
}

static typePOS* GetPosition (int cpu)
{
  int u;
  for (u = 0; u < RP_PER_CPU; u++)
    if (!ROOT_POSITION[cpu][u].used)
      break;
  if (u == RP_PER_CPU)
    return NULL;
  ROOT_POSITION[cpu][u].used = TRUE;
  ROOT_POSITION[cpu][u].stop = FALSE;
  return &ROOT_POSITION[cpu][u];
}

static void COPY_POSITION (typePOS* CHILD, typePOS* PARENT)
{
  int h;
  memcpy (CHILD, PARENT, NUM_BYTES_TO_COPY);
  memcpy (CHILD->DYN_ROOT, PARENT->DYN - 1, 2 * sizeof (typeDYNAMIC));
  CHILD->DYN = CHILD->DYN_ROOT + 1;
  h = PARENT->StackHeight;
  memcpy (CHILD->STACK, PARENT->STACK, h * sizeof (uint64));
  CHILD->StackHeight = h;
}

typePOS* COPY_TO_CHILD (int icpu, typePOS* PARENT)
{
  typePOS* CHILD;
  int cpu;
  CHILD = GetPosition (icpu);
  if (!CHILD)
    return NULL;
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    CHILD->children[cpu] = NULL;
  COPY_POSITION (CHILD, PARENT);
  return CHILD;
}

static void end_splitpunkt (SPLITPUNKT* sp) /* LOCKed ? */
{
  sp->aktiv = FALSE;
}

static SPLITPUNKT* new_splitpunkt()
{
  int sp;
  for (sp = 0; sp < MAX_SP; sp++)
    if (!ROOT_SP[sp].aktiv)
      return &ROOT_SP[sp];
  return NULL;
}

boolean IVAN_SPLIT (typePOS* POSITION, typeNEXT* NextMove,
		   int depth, int beta, int alpha, int NODE_TYPE, int* r)
{
  int cpu;
  int split;
  typePOS* CHILD;
  SPLITPUNKT* sp;

  LOCK (SMP_IVAN);
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    if (!WORKING[cpu])
      break;
  if (POSITION->stop || cpu == CPUS_SIZE)
    {
      UNLOCK (SMP_IVAN);
      return FALSE;
    }
  WORKING[POSITION->cpu] = NULL;
  POSITION->child_count = 0;
  sp = new_splitpunkt ();
  if (sp == NULL)
    {
      WORKING[POSITION->cpu] = POSITION;      
      UNLOCK (SMP_IVAN);
      return FALSE;
    }
  LOCK (sp->splock); /* ? */
  sp->alpha = alpha;
  sp->beta = beta;
  sp->depth = depth;
  sp->node_type = NODE_TYPE;
  if (NODE_TYPE != NODE_TYPE_PV)
    sp->value = sp->beta - 1;
  else
    sp->value = sp->alpha;
  sp->move = MOVE_NONE;
  sp->good_move = MOVE_NONE;
  sp->childs = 0;
  sp->MOVE_PICK = NextMove;
  sp->tot = FALSE;
  sp->aktiv = TRUE;
  UNLOCK (sp->splock); /* ? */
  split = 0;
  for (cpu = 0; cpu < CPUS_SIZE && split < MAX_SPLIT; cpu++)
    {
      POSITION->children[cpu] = NULL;
      if (WORKING[cpu] == NULL)
	{
	  CHILD = COPY_TO_CHILD (cpu, POSITION);
	  if (!CHILD)
	    continue;
	  split++;
	  POSITION->children[cpu] = CHILD;
	  CHILD->cpu = cpu;
	  CHILD->parent = POSITION;
	  CHILD->stop = FALSE;
	  CHILD->SplitPoint = sp;
	  POSITION->child_count++;
	}
    }
  if (split == 0)
    {
      WORKING[POSITION->cpu] = POSITION;
      LOCK (sp->splock);
      end_splitpunkt (sp);
      UNLOCK (sp->splock);
      UNLOCK (SMP_IVAN);
      return FALSE;
    }
  /* cuando "split" es uno ? */
  for (cpu = 0; cpu < CPUS_SIZE; cpu++)
    {
      if (POSITION->children[cpu])
	{
	  WORKING[cpu] = POSITION->children[cpu];
	  SIGNAL_CON_LOCK (PTHREAD_COND_WAIT[cpu], PTHREAD_COND_MUTEX[cpu]);
	}
    }
  UNLOCK (SMP_IVAN);
  thread_stall (POSITION, POSITION->cpu);
  LOCK (SMP_IVAN);
  LOCK (sp->splock); /* ? */
  *r = sp->value;
  end_splitpunkt (sp);
  UNLOCK (sp->splock); /* ? */
  UNLOCK (SMP_IVAN);
  return TRUE;
}
#endif


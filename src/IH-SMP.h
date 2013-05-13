
/* SLAB ALL? likely small enough */

MUTEX_TYPE PTHREAD_COND_MUTEX[MAX_CPUS], WAKEUP_LOCK_IO[1], SMP_IVAN[1];;
COND_TYPE PTHREAD_COND_WAIT[MAX_CPUS], WAKEUP_IO[1];
boolean volatile DIE[MAX_CPUS];
boolean volatile SMP_HAS_AKTIV;
boolean volatile IVAN_ALL_HALT;
boolean volatile IO_AWAKE;
boolean volatile INFINITE_LOOP;
boolean volatile SEARCH_IS_DONE;
boolean volatile STALL_MODE;
boolean TITANIC_MODE;
int volatile CPUS_SIZE;

typedef struct
{
  int alpha;
  int beta;
  int depth;
  int node_type;
  int value;
  uint32 move;
  uint32 good_move;
  uint32 childs;
  typeNEXT* MOVE_PICK;
  boolean tot;
  boolean aktiv;
  MUTEX_TYPE splock[1];
}
SPLITPUNKT;

SPLITPUNKT ROOT_SP[MAX_SP];
volatile uint64 SMP_FREE;
#define NODE_TYPE_PV 1
#define NODE_TYPE_ALL 2
#define NODE_TYPE_CUT 3
#include <setjmp.h>

#ifdef USER_SPLIT
boolean SPLIT_AT_CUT;
int CUT_SPLIT_DEPTH, ALL_SPLIT_DEPTH, PV_SPLIT_DEPTH;
#else
#define SPLIT_AT_CUT TRUE
#define CUT_SPLIT_DEPTH 16
#define ALL_SPLIT_DEPTH 14
#define PV_SPLIT_DEPTH 14
#endif

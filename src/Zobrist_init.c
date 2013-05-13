
#include "RobboLito.h"
#include <string.h>

#define MAX_AGE 256

#define FREE_MEM_NULL(x, y, z) { FREE_MEM (x, y, z); x = NULL; }

/****************************** PV HASH ******************************/

static uint64 PV_HASH_SIZE = 0x10000;
static boolean PV_FLAG_HASH_INIT = FALSE;

static boolean LARGE_PV_HASH = FALSE;
static int LARGE_PV = -1;

int PVHashClear ()
{
  memset (PVHashTable, 0, PV_HASH_SIZE * sizeof (typePVHash));
  return FALSE; /* HACK */
}

int InitPVHash (int mb)
{
  if (mb > 1024)
    mb = 1024;
  if (mb < 1)
    mb = 1;
  PV_HASH_SIZE = ((1ULL << BSR (mb)) << 20) / sizeof (typePVHash);
  mb = (PV_HASH_SIZE * sizeof (typePVHash)) >> 20;
  PVHashMask = PV_HASH_SIZE - 4;
  if (PV_FLAG_HASH_INIT)
    FREE_MEM_NULL (PVHashTable, &LARGE_PV, &LARGE_PV_HASH);
  PV_FLAG_HASH_INIT = TRUE;
  CREATE_MEM (&PVHashTable, 64, PV_HASH_SIZE * sizeof (typePVHash),
	      &LARGE_PV, &LARGE_PV_HASH, "PVHash");
  PVHashClear ();
  return mb;
}

void QUIT_PV () { FREE_MEM_NULL (PVHashTable, &LARGE_PV, &LARGE_PV_HASH); }

/****************************** PAWNS HASH ******************************/

static boolean LARGE_PAWNS_HASH = FALSE;
static int LARGE_PAWNS = -1;
static boolean PAWN_HASH_WRAPPER = FALSE;
int PawnHashReset ()
{
  memset (PawnHash, 0, PawnHashSize * sizeof (typePawnEval));
  return FALSE; /* HACK */
}

int InitPawnHash (int mb)
{
  mb = UCI_PAWNS_HASH;
  if (mb > 1024)
    mb = 1024;
  if (mb < 1)
    mb = 1;
  if (PAWNS_HASH_ONE_EIGHTH)
    mb = CURRENT_HASH_SIZE >> 3; /* one eighth */
  PawnHashSize = ((1ULL << BSR (mb)) << 20) / sizeof (typePawnEval);
  mb = (PawnHashSize * sizeof (typePawnEval)) >> 20;
  if (PawnHash)
    FREE_MEM_NULL (PawnHash, &LARGE_PAWNS, &LARGE_PAWNS_HASH);
  CREATE_MEM (&PawnHash, 64, PawnHashSize * sizeof (typePawnEval),
              &LARGE_PAWNS, &LARGE_PAWNS_HASH, "PawnsHash");
  PawnHashReset ();
  return mb;
}

int InitPawnHashWrapper (int mb)
{
  PAWN_HASH_WRAPPER = TRUE;
  return InitPawnHash (mb);
}

void QUIT_LARGE_PAWNS()
{ FREE_MEM_NULL (PawnHash, &LARGE_PAWNS, &LARGE_PAWNS_HASH); }

/****************************** TRIPLE HASH ******************************/

#ifdef CON_ROBBO_BUILD
static boolean LARGE_TRIPLE_HASH = FALSE;
static int LARGE_TRIPLE_NUMBER = -1;
void TripleHashClear ()
{
  int c;
  for (c = 0; c < TripleHashSize; c++)
    TripleHash[c] = 0;
}

int InitTripleHash (int mb)
{
  uint64 size;
  if (mb > 4096)
    mb = 4096;
  if (mb < 1)
    mb = 1;
  TripleHashSize = ((1ULL << BSR (mb)) << 20) / sizeof (uint64);
  TRIPLE_HASH_MASK = TripleHashSize - 1;
  mb = (TripleHashSize * sizeof (uint64)) >> 20;
  if (TripleHash)
    FREE_MEM_NULL (TripleHash, &LARGE_TRIPLE_NUMBER, &LARGE_TRIPLE_HASH);
  size = TripleHashSize * sizeof (uint64);
  if (TRY_LARGE_PAGES) /* HACK */
    size = MAX ((1 << 21), size);
  CREATE_MEM (&TripleHash, 64, size,
	      &LARGE_TRIPLE_NUMBER, &LARGE_TRIPLE_HASH, "TripleHash");
  TripleHashClear ();
  return mb; /* enquire to size ? */
}

void QUIT_TRIPLE_HASH ()
{ FREE_MEM_NULL (TripleHash, &LARGE_TRIPLE_NUMBER, &LARGE_TRIPLE_HASH); }
#endif

/****************************** EVAL HASH ******************************/

static boolean LARGE_EVAL_HASH = FALSE;
static int LARGE_EVAL_NUMBER = -1;
void EvalHashClear ()
{
  int c;
  for (c = 0; c < EvalHashSize; c++)
    EvalHash[c] = 0;
}

int InitEvalHash (int kb)
{
  uint64 size;
  if (kb > 1048576)
    kb = 1048576;
  if (kb < 1)
    kb = 1;
  EvalHashSize = ((1ULL << BSR (kb)) << 10) / sizeof (uint64);
  EVAL_HASH_MASK = EvalHashSize - 1;
  kb = (EvalHashSize * sizeof (uint64)) >> 10;
  if (EvalHash)
    FREE_MEM_NULL (EvalHash, &LARGE_EVAL_NUMBER, &LARGE_EVAL_HASH);
  size = EvalHashSize * sizeof (uint64);
  if (TRY_LARGE_PAGES) /* HACK */
    size = MAX ((1 << 21), size);
  CREATE_MEM (&EvalHash, 64, size,
	      &LARGE_EVAL_NUMBER, &LARGE_EVAL_HASH, "EvalHash");
  EvalHashClear ();
  return kb; /* enquire to size ? */
}

void QUIT_EVAL_HASH ()
{ FREE_MEM_NULL (EvalHash, &LARGE_EVAL_NUMBER, &LARGE_EVAL_HASH); }

/******************************** HASH ********************************/

static boolean FLAG_HASH_INIT = FALSE;

#ifdef HYPER_HASH
void HashClear ()
{
  int i;
  uint64 j; /* 64-BIT */
  for (i = 0; i < 3; i++)
    if (TABLE_ARR[i])
      {
	memset (TABLE_ARR[i], 0, (MASK_ARR[i] + 4) * sizeof (typeHash));
	for (j = 0; j < MASK_ARR[i] + 4; j++)
	  (TABLE_ARR[i] + j)-> age = (MAX_AGE / 2);
      }
  GLOBAL_AGE = 0;
}


static int LARGE[4] = {-1, -1, -1, -1};
static boolean USE[4] = {FALSE, FALSE, FALSE, FALSE};
void DETACH_ALL_MEMORY()
{
  int i;
  for (i = 0; i < 3; i++)
    FREE_MEM_NULL (TABLE_ARR[i], &LARGE[i], &USE[i]);
}

int InitHash (int mb)
{
  int i, w, MB;
  int b0, b1, b2;
  int PAWN_HASH_MB;
  mb = CURRENT_HASH_SIZE; /* HACK */
  w = BSR (mb);
  GLOBAL_AGE = 0;
  if (FLAG_HASH_INIT)
    {
      for (i = 0; i < 3 && TABLE_ARR[i]; i++)
	FREE_MEM_NULL (TABLE_ARR[i], &LARGE[i], &USE[i]);
    }
  else
    SETUP_PRIVILEGES ();
  FLAG_HASH_INIT = TRUE;
  b0 = b1 = b2 = 0;
  TABLE_ARR[0] = TABLE_ARR[1] = TABLE_ARR[2] = NULL;
  MASK_ARR[0] = MASK_ARR[1] = MASK_ARR[2] = 0;
  for (i = 0; i < 16; i++)
    HASH_WHICH[i] = 0;
  if (w >= 2)
    mb = (mb >> (w - 2)) << (w - 2);
  MB = mb;
  b0 = BSR (mb);
  MASK_ARR[0] = ((1ULL << (20 + b0)) / sizeof (typeHash)) - 4;
  mb -= (1 << b0);
  if (mb)
    {
      b1 = BSR (mb);
      MASK_ARR[1] = ((1ULL << (20 + b1)) / sizeof (typeHash)) - 4;
      mb -= (1 << b1);
    }
  if (mb)
    {
      b2 = BSR (mb);
      MASK_ARR[2] = ((1ULL << (20 + b2)) / sizeof (typeHash)) - 4;
      mb -= (1 << b2);
    }
#define WHICH_SPLIT(a, b, c) \
  for (i = 0; i < 16; i++) \
    HASH_WHICH[i] = (i < a) ? 0 : ((i < a + b) ? 1 : 2);
  if (b2) /* 4 + 2 + 1 */
    WHICH_SPLIT (9, 5, 2);
  if (!b2 && b1 && b1 == b0 - 1) /* 2 + 1 */
    WHICH_SPLIT (11, 5, 0);
  if (!b2 && b1 && b1 == b0 - 2) /* 4 + 1 */
    WHICH_SPLIT (13, 3, 0);
  if (!b2 && !b1)
    WHICH_SPLIT (16, 0, 0);
  for (i = 0; i < 3 && MASK_ARR[i]; i++)
    CREATE_MEM (&TABLE_ARR[i], 128, (MASK_ARR[i] + 4) * sizeof (typeHash),
		&LARGE[i], &USE[i], "Hash");
  HashClear ();
  CURRENT_HASH_SIZE = MB;
  if (!PAWN_HASH_WRAPPER)
    PAWN_HASH_MB = (2 * MB - 1) / 8;
  else
    PAWN_HASH_MB = (PawnHashSize * sizeof (typePawnEval)) >> 20;
  InitPawnHash (PAWN_HASH_MB);
  InitPVHash ((PV_HASH_SIZE * sizeof (typePVHash)) >> 20); /* HACK */
  InitEvalHash ((EvalHashSize * sizeof (uint64)) >> 10); /* HACK */
  InitSlab (4); /* HACK */
  return MB;
}

#else /* legacy */ /* Yet: 1% plus for the speeding! */
static uint64 HASH_SIZE;
static int LARGE = -1;
static boolean USE = FALSE;
void HashClear ()
{
  uint64 i;
  memset (HashTable, 0, HASH_SIZE * sizeof (typeHash));
  for (i = 0; i < HASH_SIZE; i++)
    (HashTable + i)->age = (MAX_AGE / 2);
  GLOBAL_AGE = 0;
}

int InitHash (int mb)
{
  int PAWN_HASH_MB;
  GLOBAL_AGE = 0;
  mb = CURRENT_HASH_SIZE; /* HACK, UCI */
  HASH_SIZE = ((1ULL << BSR (mb)) << 20) / sizeof (typeHash);
  if (HASH_SIZE > 0x100000000)
    HASH_SIZE = 0x100000000;
  mb = (HASH_SIZE * sizeof (typeHash)) >> 20;
  HashMask = HASH_SIZE - 4;
  if (FLAG_HASH_INIT)
    {
      FREE_MEM_NULL (HashTable, &LARGE, &USE); /* macro */
    }
  else
    SETUP_PRIVILEGES ();
  FLAG_HASH_INIT = TRUE;
  CREATE_MEM (&HashTable, 128, HASH_SIZE * sizeof (typeHash), &LARGE, &USE, "Hash");
  HashClear ();
  CURRENT_HASH_SIZE = mb;
  if (!PAWN_HASH_WRAPPER)
    PAWN_HASH_MB = (2 * mb - 1) / 8;
  else
    PAWN_HASH_MB = (PawnHashSize * sizeof (typePawnEval)) >> 20;
  InitPawnHash (PAWN_HASH_MB);
  InitPVHash ((PV_HASH_SIZE * sizeof (typePVHash)) >> 20); /* HACK */
  InitEvalHash ((EvalHashSize * sizeof (uint64)) >> 10); /* HACK */
  InitSlab (4); /* HACK */
  return mb;
}

void DETACH_ALL_MEMORY () { FREE_MEM_NULL (HashTable, &LARGE, &USE); }
#endif

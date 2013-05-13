
#include "RobboLito.h"
#include "RobboBaseLibUsage.h"

#define wEnumB wEnumBL
#define bEnumB bEnumBL
#define BlockedPawn 8

static char RIMAP[16] =
  { 0, wEnumP, wEnumN, wEnumK, wEnumB, wEnumB, wEnumR, wEnumQ,
    8, bEnumP, bEnumN, bEnumK, bEnumB, bEnumB, bEnumR, bEnumQ };

boolean Squeeze (typePOS * POSITION, Type_PiSq * PiSq, boolean w)
{
  uint64 T, U, V, W, BLOCKED, A;
  int i, n = 0, c, sq;
  PiSq->wtm = POSITION->wtm;
  PiSq->wK = POSITION->wKsq;
  PiSq->bK = POSITION->bKsq;
  PiSq->pedone = (wBitboardP || bBitboardP);
  U = wBitboardP & (bBitboardP >> 8);
  V = (U << 1) & U & ~FILEa;
  W = (V << 1) & V;
  BLOCKED = (U & ~V) | (W & ~((W << 1) & W));
  PiSq->ep = POSITION->DYN->ep;
  for (i = 0; i < 4; i++)
    PiSq->pi[i] = PiSq->sq[i] = 0;
  T = POSITION->OccupiedBW ^ (wBitboardK | bBitboardK);
  T ^= BLOCKED | (BLOCKED << 8);  
  if (POPCNT (T) + POPCNT (BLOCKED) > 4)
    return FALSE;
  while (T)
    {
      sq = BSF (T);
      PiSq->pi[n] = RIMAP[POSITION->sq[sq]];
      PiSq->sq[n++] = sq;
      BitClear (sq, T);
    }
  c = n;
  A = BLOCKED;
  while (A)
    {
      sq = BSF (A);
      PiSq->pi[n] = BlockedPawn;
      PiSq->sq[n++] = sq;
      BitClear (sq, A);
    }
  PiSq->n = n;
  if (w ? FUNC_CALL (HAS_TotalBase) (PiSq) : FUNC_CALL (HAS_TripleBase) (PiSq))
    return TRUE;
  while (BLOCKED && n < 4)
    {
      sq = BSF (BLOCKED);
      PiSq->pi[c] = wEnumP;
      PiSq->sq[c++] = sq; /* redundancy */
      PiSq->pi[n] = bEnumP;
      PiSq->sq[n++] = sq + 8;
      PiSq->n = n;
      if (w ? FUNC_CALL (HAS_TotalBase) (PiSq) : FUNC_CALL (HAS_TripleBase) (PiSq))
        return TRUE;
    }
  return FALSE;
}

#define IS_ILLEGAL \
  (!POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

boolean TRIPLE_VALUE
(typePOS * POSITION, int * v, boolean * total,
 boolean DEMAND_TOTAL, boolean WEAK, boolean IMPALE)
{
  Type_PiSq PiSq[1];
  boolean b;
  uint64 H;
  if (POSITION->DYN->oo || IS_ILLEGAL)
    return FALSE;
  H = TripleHash[POSITION->DYN->HASH & TRIPLE_HASH_MASK];
  if (((H ^ POSITION->DYN->HASH) & 0xfffffffffffffff0) == 0)
    {
      *v = (H & 0x7) - 1;
      *total = (H & 0x8) >> 3;
      if (!DEMAND_TOTAL || *total)
	{
	  POSITION->tbhits++;
	  return TRUE;
	}
    }
  if (!ROBBO_LOAD || !Squeeze (POSITION, PiSq, 0))
    return FALSE;
  *total = FUNC_CALL (HAS_TotalBase) (PiSq); /* novel, NewBuy */
  if (DEMAND_TOTAL && !(*total))
    return FALSE;
  b = FUNC_CALL (RobboTripleLibScore) (PiSq, v, IMPALE ? 2 : (WEAK ? 0 : 1));
  if (!b)
    return FALSE;
  TripleHash[POSITION->DYN->HASH & TRIPLE_HASH_MASK] =
    (POSITION->DYN->HASH & 0xfffffffffffffff0) | ((*v) + 1 + 8 * (*total));
  POSITION->nodes += PiSq->nodes; /* admire */
  POSITION->tbhits++;
  return TRUE;
}

boolean RobboTotalBaseScore (typePOS * POSITION, int * v)
{
  Type_PiSq PiSq[1];
  boolean b;
  uint8 v8;
  if (POSITION->DYN->oo || IS_ILLEGAL)
    return FALSE;
  if (!ROBBO_TOTAL_LOAD || !Squeeze (POSITION, PiSq, 1))
    return FALSE;
  b = FUNC_CALL (RobboTotalLibScore) (PiSq, &v8, 1); /* strong in IvanHoe */
  *v = v8;
  if (b && (*v != 0))
    POSITION->tbhits++;
  return b && (*v != 0);
}

#ifndef STATIC_LINKERY
static boolean DYNAMIC_LIBRARY_LOADED = FALSE;
static boolean ROBBO_LIBRARY_INIT = FALSE;
char DYNAMIC_LIBRARY_NAME[1024];
#define GAIN(x) DL_ERROR (); /* clear */ \
  x = DL_SYM (so_file, #x); /* load */ \
  err = DL_ERROR (); /* again */ \
  if (err) { printf ("%s\n", err); return FALSE; } 

#ifndef WINDOWS
#include <dlfcn.h> /* dynamic load */
#define HMODULE void *
#endif

boolean LoadDynamicLibrary (char * STRING)
{
  HMODULE so_file; /* void * unto Linux */
  char * err;

  DYNAMIC_LIBRARY_LOADED = FALSE;
  ROBBO_LIBRARY_INIT = FALSE;
  so_file = DL_OPEN (STRING, RTLD_LAZY); /* local */
  if (!so_file)
    {
      SEND ("%s not detected\n", STRING);
      return FALSE;
    }
  SEND ("%s found (file name), to do dynamic load functions\n", STRING);
  GAIN (RobboTotalLibScore);
  GAIN (RobboTripleLibScore);
  GAIN (GetRobboBaseLibraryVersion);

  GAIN (RegisterTotalBases);
  GAIN (DeRegisterTotalBases);
  GAIN (RegisterRobboTotalBasesInDirectory);
  GAIN (SetCacheTotalBase);
  GAIN (HAS_TotalBase);

  GAIN (SetCacheTripleBase);
  GAIN (SetLoadOnWeakProbeBackGround);
  GAIN (AllowBackGroundLoader);
  GAIN (PauseBackGroundLoader);

  GAIN (LoadTripleBases);
  GAIN (UnLoadTripleBases);
  GAIN (LoadRobboTripleBasesInDirectory);
  GAIN (HAS_TripleBase);
  
  GAIN (RobboLibBulkLoadFile);
  GAIN (RobboLibBulkDetachFile);
  GAIN (RobboLibBulkLoadDirectory);
  GAIN (RobboLibBulkDetachDirectory);

  GAIN (RobboBaseLibStartUp);
  GAIN (RobboBaseLibExit);

  GAIN (TotalBase_FileNames);
  GAIN (TripleBase_FileNames);
  GAIN (FilesInfo_TotalBase);
  GAIN (FilesInfo_TripleBase);

  GAIN (GetMaximumTriplePieces);
  GAIN (ResetTripleCounters);
  GAIN (SendTripleCounters);

  DYNAMIC_LIBRARY_LOADED = TRUE;
  ROBBO_LIBRARY_INIT = TRUE;
  SetLoadOnWeakProbe (TRUE); /* default */
  strcpy (DYNAMIC_LIBRARY_NAME, ROBBO_DYNAMIC_LIBRARY);
  return TRUE;
}

int LoadRobboDynamicLibrary (int x)
{
  if (!strcmp (ROBBO_DYNAMIC_LIBRARY, DYNAMIC_LIBRARY_NAME))
    return TRUE;
  return LoadDynamicLibrary (ROBBO_DYNAMIC_LIBRARY);
}
#else
static boolean ROBBO_LIBRARY_INIT = TRUE; /* STATIC_LINKERY */
#endif

volatile boolean WEAK_PROBE_PONDER = TRUE;

int MaximumTripleUsage ()
{
  if (!ROBBO_LOAD)
    return 0;
  return FUNC_CALL (GetMaximumTriplePieces) ();
}

#define VERIFY_ROBBO_LIBRARY() \
  if (!ROBBO_LIBRARY_INIT) \
    { SEND ("RobboBaseLibrary buys not in consist\n"); return FALSE; }  

int LoadRobboTripleBases ()
{
  VERIFY_ROBBO_LIBRARY ();
  ROBBO_LOAD = TRUE;
  FUNC_CALL (LoadTripleBases) (ROBBO_TRIPLE_DIR);
  return TRUE;
}

int UnLoadRobboTripleBases ()
{
  VERIFY_ROBBO_LIBRARY ();
  ROBBO_LOAD = FALSE;
  FUNC_CALL (UnLoadTripleBases) ();
  return TRUE;
}

int RegisterRobboTotalBases ()
{
  VERIFY_ROBBO_LIBRARY ();
  ROBBO_TOTAL_LOAD = TRUE;
  FUNC_CALL (RegisterTotalBases) (ROBBO_TOTAL_DIR);
  return TRUE;
}

int DeRegisterRobboTotalBases ()
{
  VERIFY_ROBBO_LIBRARY ();
  ROBBO_TOTAL_LOAD = FALSE;
  FUNC_CALL (DeRegisterTotalBases) ();
  return TRUE;
}

int SetTotalBaseCache (int mb)
{
  VERIFY_ROBBO_LIBRARY ();
  return FUNC_CALL (SetCacheTotalBase) (mb);
}

int SetTripleBaseCache (int mb)
{
  VERIFY_ROBBO_LIBRARY ();
  return FUNC_CALL (SetCacheTripleBase) (mb);
}

int TripleEnumerateReset ()
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (ResetTripleCounters) ();
  return TRUE; /* HACK */
}

int AkinRobboInformatory (int x)
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (SendTripleCounters) ();
  return TRUE; /* HACK */
}

int RobboBulkDirectory ()
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (RobboLibBulkLoadDirectory) (ROBBO_BULK_DIRECTORY);
  return TRUE; /* HACK */
}

int RobboBulkDirectoryDetach ()
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (RobboLibBulkDetachDirectory) (ROBBO_BULK_DIRECTORY);
  return TRUE; /* HACK */
}

int RobboBulkLoad ()
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (RobboLibBulkLoadFile) (ROBBO_BULK_NAME);
  return TRUE; /* HACK */
}

int RobboBulkDetach ()
{
  VERIFY_ROBBO_LIBRARY ();
  FUNC_CALL (RobboLibBulkDetachFile) (ROBBO_BULK_NAME);
  return TRUE; /* HACK */
}

char WEAK_LOAD_USAGE[16];
int ChangeWeakLoadUsage (int x)
{
  if (!strcmp (WEAK_LOAD_USAGE, "AllWays"))
    {
      WEAK_PROBE_PONDER = TRUE;
      SetLoadOnWeakProbe (TRUE);
    }
  if (!strcmp (WEAK_LOAD_USAGE, "UnlessPondered"))
    {
      WEAK_PROBE_PONDER = FALSE;
      SetLoadOnWeakProbe (TRUE);
    }
  if (!strcmp (WEAK_LOAD_USAGE, "Refrain"))
    {
      WEAK_PROBE_PONDER = FALSE;
      SetLoadOnWeakProbe (FALSE);
    }
  return 0; /* compiler */
}

int SetLoadOnWeakProbe (boolean b)
{
  VERIFY_ROBBO_LIBRARY ();
  return FUNC_CALL (SetLoadOnWeakProbeBackGround) (b);
}

boolean EmergeBackGroundLoader ()
{
  VERIFY_ROBBO_LIBRARY ();
  return FUNC_CALL (AllowBackGroundLoader) ();
}

boolean DesistBackGroundLoader ()
{
  VERIFY_ROBBO_LIBRARY ();
  return FUNC_CALL (PauseBackGroundLoader) ();
}

char UTIL_STRING[32];
int UtilRobboBases (int x)
{
  if (!strcmp (UTIL_STRING, "DeRegisterTotal"))
    DeRegisterRobboTotalBases (0);
  if (!strcmp (UTIL_STRING, "UnLoadTriple"))
    UnLoadRobboTripleBases (0);
  return 0;
}


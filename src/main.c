
#include "RobboLito.h"
#include "mios.h"

void ResetPositionalGain ()
{
  int p, m;
#ifdef MULTIPLE_POS_GAIN
  typePOS POSITION[1]; /* HACK */
  for (POSITION->cpu = 0; POSITION->cpu < MAX_CPUS; POSITION->cpu++)
#endif
    for (p = 0; p < 0x10; p++)
      for (m = 0; m < 010000; m++)
	MAX_POS_GAIN (p, m) = 0;
}

void ResetHistory ()
{
  int pi, sq;
#ifdef MULTIPLE_HISTORY
  typePOS POSITION[1]; /* HACK */
  for (POSITION->cpu = 0; POSITION->cpu < MAX_CPUS; POSITION->cpu++)
#endif
    for (pi = 0; pi < 0x10; pi++)
      for (sq = A1; sq <= H8; sq++)
	HISTORY_PI_TO (POSITION, pi, sq) = 0x800;
}

static void Banner ()
{
#ifdef MODE_ANALYSIS
  printf ("%s VERSION %s Analysis\n", EPONYM, VERSION);
#endif
#ifdef MODE_GAME_PLAY
  printf ("%s VERSION %s GamePlay\n", EPONYM, VERSION);
#endif
#ifdef WINDOWS
  printf ("WINDOWS capitalist mode in severance\n");
#else
  printf ("Workers mode enabled! (no WINDOWS)\n");
#ifdef NOME_WINDOWS
  printf ("Yet: RobboBases accrue WINDOWS namingsakes\n");
#endif
#endif
#ifdef MODE_ANALYSIS
  printf ("MODE_ANALYSIS\n");
  NULL_MOVE_INTACT = TRUE;
#endif
#ifdef MODE_GAME_PLAY
  printf ("Battle Mode\n");
#endif
#ifdef HAS_PREFETCH
  printf ("compiled with PREFETCH\n");
#endif
#ifdef HAS_POPCNT
  printf ("compiled with POPCNT\n");
#endif
#ifdef YUSUF_MULTICORE
  printf ("Capitalist multicore mode compiled\n");
#endif
#ifdef TRACE_COMPILE
  printf ("Trace mode compile\n");
#endif
#ifdef UTILITIES
  printf ("Mode with UTILITIES compiled\n");
#endif
#ifdef MINIMAL
  printf ("MINIMAL Mode compiled\n");
#endif
#ifdef CON_ROBBO_BUILD
  printf ("RobboBases enabled by compile\n");
#endif
#ifdef HYPER_HASH
  printf ("HYPER_HASH in use\n");
#endif
#ifdef ZUGZWANG_DETECT
  printf ("ZUGZWANG_DETECT intact\n");
#endif
#ifdef BUILD_ZOG_MP_COMPILE
  printf ("compiled with ZOG MP, sizeof %d+%d\n",
	  (int) (sizeof (type_zog_node)), (int) (2 * sizeof (typeDYNAMIC)));
#endif
}

#ifdef CON_ROBBO_BUILD
#include "RobboBaseLibUsage.h"
#ifdef STATIC_LINKERY
void RobboIniz ()
{
  char STR[16];
  printf ("RobboBaseLib STATIC_LINKERY\n");
  FUNC_CALL (RobboBaseLibStartUp) (TRUE); /* background */
  FUNC_CALL (GetRobboBaseLibraryVersion) (STR);
  SetLoadOnWeakProbe (TRUE); /* default */
  printf ("RobboBaseLib Version %s\n");
}
#else
boolean LoadDynamicLibrary (char *);
void RobboIniz ()
{
  char STR[16];
  printf ("RobboBaseLib Dynamic_Link\n");
  if (LoadDynamicLibrary (LIB_NAME0) || LoadDynamicLibrary (LIB_NAME1))
    {
      FUNC_CALL (RobboBaseLibStartUp) (TRUE); /* background */
      FUNC_CALL (GetRobboBaseLibraryVersion) (STR);
      printf ("RobboBaseLib Version %s (dynamic)\n", STR);
    }
}
#endif /* STATIC_LINKERY */
#endif

#ifdef TRACE_COMPILE
void TraceOn ()
{
  SEND ("TraceOn\n");
  TRACE_PVQSEARCH = TRUE;
  TRACE_QSEARCH = TRUE;
  TRACE_HASH = TRUE;
  TRACE_EVAL = TRUE;
  TRACE_LOW_DEPTH = TRUE;
  TRACE_CUT = TRUE;
  TRACE_ALL = TRUE;
  TRACE_EXCLUDE = TRUE;
  TRACE_PV = TRUE;
  TRACE_MAKE_UNMAKE = TRUE;
}

void TraceOff ()
{
  SEND ("TraceOff\n");
  TRACE_PVQSEARCH = FALSE;
  TRACE_QSEARCH = FALSE;
  TRACE_HASH = FALSE;
  TRACE_EVAL = FALSE;
  TRACE_LOW_DEPTH = FALSE;
  TRACE_CUT = FALSE;
  TRACE_ALL = FALSE;
  TRACE_EXCLUDE = FALSE;
  TRACE_PV = FALSE;
  TRACE_MAKE_UNMAKE = FALSE;
}
#endif

int main ()
{
  Banner ();
  InitSend ();
  fflush (stdout);
  ICI = FALSE;
  PVHashTable = NULL;
  EvalHash = NULL;
  PawnHash = NULL; /* HACK */
  PawnHashSize = (1 << 16); /* HACK */
  EvalHashSize = (1 << 15); /* HACK */
#ifdef CON_ROBBO_BUILD
  TripleHash = NULL;
  TripleHashSize = (1 << 17);
#endif
  memset ( (void*) ROOT_POSIZIONE0, 0, sizeof (typePOS));
  ROOT_POSIZIONE0->DYN_ROOT = malloc (MAXIMUM_PLY * sizeof (typeDYNAMIC));
  ROOT_POSIZIONE0->DYN = ROOT_POSIZIONE0->DYN_ROOT + 1;
  ROOT_POSIZIONE0->stop = FALSE;
  CPUS_SIZE = 1;
#ifdef YUSUF_MULTICORE
  rp_init ();
#endif
  SMP_HAS_AKTIV = FALSE;
  UCI_White_Bishops_Scale = 45;
  UCI_White_Pawn_Scale = 100;
  UCI_White_Knight_Scale = 300;
  UCI_White_Light_Scale = 310;
  UCI_White_Dark_Scale = 310;
  UCI_White_Rook_Scale = 500;
  UCI_White_Queen_Scale = 950;
  UCI_Black_Bishops_Scale = 45;
  UCI_Black_Pawn_Scale = 100;
  UCI_Black_Knight_Scale = 300;
  UCI_Black_Light_Scale = 310;
  UCI_Black_Dark_Scale = 310;
  UCI_Black_Rook_Scale = 500;
  UCI_Black_Queen_Scale = 950;
  UCI_MATERIAL_WEIGHTING = 1024;
  UCI_PAWNS_WEIGHTING = 1024;
  UCI_KING_SAFETY_WEIGHTING = 1024;
  UCI_STATIC_WEIGHTING = 1024;
  UCI_MOBILITY_WEIGHTING = 1024;

  PAWNS_HASH_ONE_EIGHTH = TRUE;
  UCI_PAWNS_HASH = 4;
  CURRENT_HASH_SIZE = 32;
  InitHash (32);
  InitPawnHash (4);
  InitPVHash (2);
  InitEvalHash (256);
#ifdef SLAB_MEMORY
  InitSlab (SLAB_SIZE);
#else
  InitSlab (0);
#endif
  InitCaptureValues (); /* SLAB_MEM */
  InitPawns (); /* SLAB_MEM? */

#ifdef USER_SPLIT
  CUT_SPLIT_DEPTH = 16;
  ALL_SPLIT_DEPTH = 14;
  PV_SPLIT_DEPTH = 14;
  SPLIT_AT_CUT = TRUE;
#endif
  SEND_CURR_MOVE = FALSE;
  USE_ALTERNATIVE_TIME = FALSE;
  TIME_IMITATE = FALSE;
  TIME_LOSE_MORE = FALSE;
  TIME_WIN_MORE = FALSE;
  BUFFER_TIME = 0;
  OUTPUT_DELAY = 0;
  UCI_PONDER = FALSE;
  VERIFY_NULL = TRUE;
  ROBBO_LOAD = ROBBO_TOTAL_LOAD = FALSE; 
  SEARCH_ROBBO_BASES = TRUE;
  MULTI_PV = 1;
  SEND_HASH = FALSE; /* aid GUI */
  DO_HASH_FULL = TRUE;
  ALLOW_INSTANT_MOVE = TRUE;
  ZOG_AKTIV = FALSE;
  strcpy (ROBBO_TRIPLE_DIR, "");
  strcpy (ROBBO_TOTAL_DIR, "");
  SEARCH_IS_DONE = TRUE;
  INFINITE_LOOP = FALSE;
#ifndef MINIMAL
  MULTI_CENTI_PAWN_PV = 65535;
  RANDOM_COUNT = 0;
  RANDOM_BITS = 1;
  ALWAYS_ANALYZE = FALSE;
  TRY_PV_IN_ANALYSIS = TRUE;
  FIXED_AGE_ANALYSIS = FALSE;
#endif

#ifdef TRACE_COMPILE
  TraceOn ();
#endif

#ifdef CON_ROBBO_BUILD
  RobboIniz ();
  InitTripleHash (1); /* ensure */
#endif
  TRY_LARGE_PAGES = FALSE;
  SUPPRESS_INPUT = FALSE;
  INPUT_BUFFER = malloc (65536);
  input_ptr = INPUT_BUFFER;
  STALL_MODE = FALSE;
  SINCE_NEW_GAME = 0;
  EASY_FACTOR = 15;
  EASY_FACTOR_PONDER = 33;
  BATTLE_FACTOR = 100;
  ORDINARY_FACTOR = 75;
  ABSOLUTE_PERCENT = 25;
  DESIRED_MILLIS =  40;
  BOOK_EXIT_MOVES = 0;
  EXTEND_IN_CHECK = FALSE;
  TRIPLE_WEAK_PROBE_DEPTH = -10;
  TRIPLE_DEFINITE_PROBE_DEPTH = 40;
  TRIPLE_DEFINITE_PROBE_HEIGHT = 5;
  LOAD_ON_WEAK_PROBE = TRUE;
  STALL_INPUT = FALSE;
  UCI_OPTION_CHESS_960 = FALSE;
#ifdef LINUX_LARGE_PAGES
  LINUX_handle_signals ();
#endif
#ifdef MODE_ANALYSIS
  ICI_HASH_MIX = FALSE;
#endif
  NewGame (ROOT_POSIZIONE0, TRUE);
  while (1)
    Input (ROOT_POSIZIONE0);
  return 0;
}

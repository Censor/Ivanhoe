#include "RobboLito.h"
#include <string.h>
#include "control.h"

#if 1 /* UCI? */
int GetNumCPU() /*Get number of CPUs */
{
#ifdef WINDOWS /*Windows*/
  SYSTEM_INFO si;
  GetSystemInfo( &si );
  CPUS_SIZE = si.dwNumberOfProcessors;
#else /*LINUX */
#include <unistd.h>
  CPUS_SIZE = sysconf( _SC_NPROCESSORS_CONF );
#endif
  /*
#ifdef APPLE
#include <sys/sysctl.h>
CPUS_SIZE = 1;
size_t size=sizeof(CPUS_SIZE) ;
sysctlbyname("hw.ncpu",&CPUS_SIZE,&size,NULL,0);
#endif
  */
  return CPUS_SIZE;
} /*End Get number of CPUs*/
#endif

#if defined(UTILITIES)
#define SWAP_TWO(x, y) { (x) ^= (y); (y) ^= (x); (x) ^= (y); }
#define SHUFFLE_TWO(x) { (x) = ((((x) & 0x3) << 2) | (((x) & 0xc) >> 2)); }
static void InvertBoard (typePOS* POSITION)
{
  int f, r, sq;
  for (f = FA; f <= FH; f++)
    for (r = R1; r <= R4; r++)
      SWAP_TWO (POSITION->sq[8 * r + f], POSITION->sq[8 * (7 - r) + f]);
  for (sq = A1; sq <= H8; sq++)
    {
      if (POSITION->sq[sq])
	POSITION->sq[sq] ^= 8;
      if ((POSITION->sq[sq] & 0x6) == 0x4)
	POSITION->sq[sq] ^= 1; /* HACK, color bishop */
    }
  POSITION->wtm ^= 1;
  if (POSITION->DYN->ep)
    POSITION->DYN->ep ^= 0x7 << 3; /* HACK, flip 3rd/6th */
  SHUFFLE_TWO (POSITION->DYN->oo);
  InitBitboards (POSITION);
}

static void EvalTest (typePOS* POSITION)
{
  int v;
  EvalExplanation (POSITION, -32767, 32767, 0);
  v = POSITION->DYN->Value;
  Eval (POSITION, -32767, 32767, 0, 0);
  if (v != POSITION->DYN->Value)
    {
      printf ("ERROR with EvalExplanation compared to EvalEvaluation %d %d\n",
	      v, POSITION->DYN->Value);
    }
  InvertBoard (POSITION);
  Eval (POSITION, -32767, 32767, 0, 0);
  if (v != POSITION->DYN->Value)
    {
      printf ("ERROR in InvertBoard Eval compared to Invert %d %d\n",
	      v, POSITION->DYN->Value);
      printf ("FEN is: %s\n", EmitFen (ROOT_POSIZIONE0, STRING1[0]));
    }
  InvertBoard (POSITION);
}

static int UCI_fen ()
{
  SEND ("info string %s\n", EmitFen (ROOT_POSIZIONE0, STRING1[0]));
  return TRUE;
}
#endif

#if 0
#define READYOK() readyok ()
#else
#define READYOK() /* */
#endif

typedef enum
{ UCI_SPIN, UCI_CHECK, UCI_BUTTON, UCI_STRING, UCI_COMBO, UCI_IGNORE } enum_UCI;
typedef enum
 { ICI_SPIN, ICI_BINARY, ICI_CHECK, ICI_BUTTON,
   ICI_STRING, ICI_DIRECTORY, ICI_DIRECTORY_MULTI_RESET, /* HACK */
   ICI_FILE, ICI_COMBO, UCI_ONLY } enum_ICI;

typedef struct
{
  char name[0x100];
  char sub_class[0x40];
  int type, ici_type; sint32 min, max, def;
  void* var;
  int (*action)(int);
} UCI_type;

#define MAX_VAR 8
char COMBO[2][MAX_VAR][32] =
  {
    { "AllWays", "UnlessPondered", "Refrain" },
    { "Inert", "DeRegisterTotal", "UnLoadTriple" }
  };
char WEAK_LOAD_USAGE[16], UTIL_STRING[32];

#define NUM_SUB_CLASS 5
char SUB_CLASS_LIST[NUM_SUB_CLASS][16] =
  { "General", "RobboBases", "Evaluation", "Timing", "Other" };

UCI_type UCI_OPTIONS[256]=
  {
    { "Hash", "General", UCI_SPIN, ICI_SPIN, /* partial binary */
      1, 65536, 32, &CURRENT_HASH_SIZE, &InitHash },
    { "PawnsHash", "General", UCI_SPIN, ICI_BINARY,
      1, 1024, 4, &UCI_PAWNS_HASH, &InitPawnHashWrapper },
    { "PawnsHashOneEighth", "General", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &PAWNS_HASH_ONE_EIGHTH, &InitPawnHash }, /* HACK */
    { "PVHash", "General", UCI_SPIN, ICI_BINARY,
      1, 1024, 2, NULL, &InitPVHash }, /* 2 for LARGE */
    { "EvalHash", "General", UCI_SPIN, ICI_BINARY,
      1, 1048576, 256, NULL, &InitEvalHash },
#ifdef YUSUF_MULTICORE
    { "TitanicMode", "General", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TITANIC_MODE, &ivan_init_smp },
    { "TitanicCPUsSizings", "General", UCI_SPIN, ICI_SPIN,
      1, MAX_CPUS, 1, &CPUS_SIZE, &ivan_init_smp }, /* volatile ? */
#endif
#ifdef MODE_GAME_PLAY /* HACK */
    { "Ponder", "Other", UCI_CHECK, UCI_ONLY, 0, 0, FALSE, &UCI_PONDER, NULL },
#endif
#ifdef MODE_ANALYSIS
    { "AllowNullMove", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &NULL_MOVE_INTACT, NULL },
    { "ComradesMonteCarlo", "Other", UCI_BUTTON, UCI_ONLY,
      0, 0, 0, NULL, NULL},
    { "HashMixICI", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &ICI_HASH_MIX, NULL },
#endif
    { "VerifyNullMove", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &VERIFY_NULL, NULL },
    { "AlternativeTimeUsage", "Timing", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &USE_ALTERNATIVE_TIME, NULL },
    { "AllowInstantMoveFromHash", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &ALLOW_INSTANT_MOVE, NULL },
#ifdef MODE_GAME_PLAY
    { "BufferTime", "Timing", UCI_SPIN, ICI_SPIN,
      0, 10000, 0, &BUFFER_TIME, NULL },
#endif
    { "OutputDelay", "Other", UCI_SPIN, ICI_SPIN,
      0, 60000, 0, &OUTPUT_DELAY, NULL },
#ifdef CON_ROBBO_BUILD
#ifndef STATIC_LINKERY
    { "RobboBaseDynamicLibraryFile", "RobboBases", UCI_STRING, ICI_STRING,
      0, 0, 0, ROBBO_DYNAMIC_LIBRARY, &LoadRobboDynamicLibrary },
#endif
    { "RobboTripleBaseDirectory", "RobboBases", UCI_STRING, ICI_DIRECTORY,
      0, 0, 0, ROBBO_TRIPLE_DIR, &LoadRobboTripleBases },
    { "RobboTotalBaseDirectory", "RobboBases", UCI_STRING, ICI_DIRECTORY,
      0, 0, 0, ROBBO_TOTAL_DIR, &RegisterRobboTotalBases },

    { "RobboTotalBaseCacheSize", "RobboBases", UCI_SPIN, ICI_BINARY,
      1, 1024, 1, NULL, &SetTotalBaseCache },
    { "DynamicLoadTripleBaseCacheSize", "RobboBases", UCI_SPIN, ICI_BINARY,
      1, 65536, 1, NULL, &SetTripleBaseCache },
    { "TripleHashSize", "RobboBases", UCI_SPIN, ICI_BINARY,
      1, 4096, 1, NULL, &InitTripleHash },
    { "TripleWeakProbeDepthHalfPly", "RobboBases", UCI_SPIN, ICI_SPIN,
      -10, 255, -10, &TRIPLE_WEAK_PROBE_DEPTH, NULL },
    { "TripleDefiniteProbeDepthHalfPly", "RobboBases", UCI_SPIN, ICI_SPIN,
      -10, 255, 40, &TRIPLE_DEFINITE_PROBE_DEPTH, NULL },
    { "TripleDefiniteProbeHeight", "RobboBases", UCI_SPIN, ICI_SPIN,
      1, 255, 5, &TRIPLE_DEFINITE_PROBE_HEIGHT, NULL },
    { "LoadOnWeakProbe", "RobboBases", UCI_COMBO, ICI_COMBO,
      0, 3, 0, &WEAK_LOAD_USAGE, &ChangeWeakLoadUsage },
    { "UnloadDeregisterInform", "RobboBases", UCI_COMBO, ICI_COMBO,
      1, 3, 0, &UTIL_STRING, &UtilRobboBases },
    { "RobboTripleBulkLoadThisDirectory", "RobboBases", UCI_STRING, ICI_DIRECTORY_MULTI_RESET,
      0, 0, 0, ROBBO_BULK_DIRECTORY, &RobboBulkDirectory },
    { "RobboTripleBulkLoadThisName", "RobboBases", UCI_STRING, ICI_STRING,
      0, 0, 0, ROBBO_BULK_NAME, &RobboBulkLoad },
    { "RobboTripleBulkDetachThisDirectory", "RobboBases", UCI_STRING, ICI_DIRECTORY_MULTI_RESET,
      0, 0, 0, ROBBO_BULK_DIRECTORY, &RobboBulkDirectoryDetach },
    { "RobboTripleBulkDetachThisName", "RobboBases", UCI_STRING, ICI_STRING,
      0, 0, 0, ROBBO_BULK_NAME, &RobboBulkDetach },
    { "RobboInformatory", "RobboBases", UCI_BUTTON, ICI_BUTTON, /* in still */
      0, 0, 0, NULL, &AkinRobboInformatory },
#endif
    { "MultiPV", "General", UCI_SPIN, ICI_SPIN,
	1, 250, 1, &MULTI_PV, NULL },
    { "MultiCentiPawnPV", "General", UCI_SPIN, ICI_SPIN,
      1, 65535, 65535, &MULTI_CENTI_PAWN_PV, NULL },
    { "RandomCount", "Other", UCI_SPIN, ICI_SPIN,
      0, 8, 0, &RANDOM_COUNT, NULL },
    { "RandomBits", "Other", UCI_SPIN, ICI_SPIN,
      1, 3, 1, &RANDOM_BITS, NULL },
#ifdef USER_SPLIT
    { "SplitAtCUT", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &SPLIT_AT_CUT, NULL },
    { "SplitDepthCUT", "Other", UCI_SPIN, ICI_SPIN,
      16, 20, 16, &CUT_SPLIT_DEPTH, NULL },
    { "SplitDepthALL", "Other", UCI_SPIN, ICI_SPIN,
      12, 20, 14, &ALL_SPLIT_DEPTH, NULL },
    { "SplitDepthPV", "Other", UCI_SPIN, ICI_SPIN,
      10, 20, 14, &PV_SPLIT_DEPTH, NULL },
#endif
    { "WhiteBishopPair", "Evaluation", UCI_SPIN, ICI_SPIN,
      1, 150, 45, &UCI_White_Bishops_Scale, &InitMaterialValue },
    { "WhitePawn", "Evaluation", UCI_SPIN, ICI_SPIN,
      50, 200, 100, &UCI_White_Pawn_Scale, &InitMaterialValue },
    { "WhiteKnight", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 300, &UCI_White_Knight_Scale, &InitMaterialValue },
    { "WhiteLightBishop", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 310, &UCI_White_Light_Scale, &InitMaterialValue },
    { "WhiteDarkBishop", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 310, &UCI_White_Dark_Scale, &InitMaterialValue },
    { "WhiteRook", "Evaluation", UCI_SPIN, ICI_SPIN,
      300, 700, 500, &UCI_White_Rook_Scale, &InitMaterialValue },
    { "WhiteQueen", "Evaluation", UCI_SPIN, ICI_SPIN,
      500, 1500, 950, &UCI_White_Queen_Scale, &InitMaterialValue },
    { "BlackBishopPair", "Evaluation", UCI_SPIN, ICI_SPIN,
      1, 150, 45, &UCI_Black_Bishops_Scale, &InitMaterialValue },
    { "BlackPawn", "Evaluation", UCI_SPIN, ICI_SPIN,
      50, 200, 100, &UCI_Black_Pawn_Scale, &InitMaterialValue },
    { "BlackKnight", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 300, &UCI_Black_Knight_Scale, &InitMaterialValue },
    { "BlackLightBishop", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 310, &UCI_Black_Light_Scale, &InitMaterialValue },
    { "BlackDarkBishop", "Evaluation", UCI_SPIN, ICI_SPIN,
      150, 500, 310, &UCI_Black_Dark_Scale, &InitMaterialValue },
    { "BlackRook", "Evaluation", UCI_SPIN, ICI_SPIN,
      300, 700, 500, &UCI_Black_Rook_Scale, &InitMaterialValue },
    { "BlackQueen", "Evaluation", UCI_SPIN, ICI_SPIN,
      500, 1500, 950, &UCI_Black_Queen_Scale, &InitMaterialValue },
    { "MaterialWeighting", "Evaluation", UCI_SPIN, ICI_SPIN,
      512, 2048, 1024, &UCI_MATERIAL_WEIGHTING, &InitMaterialValue },
    { "KingSafetyWeighting", "Evaluation", UCI_SPIN, ICI_SPIN,
      512, 2048, 1024, &UCI_KING_SAFETY_WEIGHTING, NULL },
    { "StaticWeighting", "Evaluation", UCI_SPIN, ICI_SPIN,
      512, 2048, 1024, &UCI_STATIC_WEIGHTING, NULL },
    { "MobilityWeighting", "Evaluation", UCI_SPIN, ICI_SPIN,
      512, 2048, 1024, &UCI_MOBILITY_WEIGHTING, NULL },
    { "PawnsWeighting", "Evaluation", UCI_SPIN, ICI_SPIN,
      512, 2048, 1024, &UCI_PAWNS_WEIGHTING, &PawnHashReset },
    { "AlwaysAnalyze", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &ALWAYS_ANALYZE, NULL },
    { "TryPVinAnalysis", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &TRY_PV_IN_ANALYSIS, NULL },
    { "FixedAgeAnalysis", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &FIXED_AGE_ANALYSIS, NULL },
#ifdef MODE_GAME_PLAY
    { "SendCurrmove", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &SEND_CURR_MOVE, NULL },
#endif
    { "DoHashFull", "General", UCI_CHECK, ICI_CHECK,
      0, 0, TRUE, &DO_HASH_FULL, NULL },
#ifdef UTILITIES
    { "GetFEN", "Other", UCI_BUTTON, UCI_ONLY, 0, 0, 0, NULL, &UCI_fen },
    { "DebugTimeManagement", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &DEBUG_TIME_MANAGEMENT, NULL },
#endif
#ifdef TIMING_OPTIONS
    { "TimeImitateOpponent", "Timing", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TIME_IMITATE, NULL },
    { "TimeMoreWhenLosing", "Timing", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TIME_LOSE_MORE, NULL },
    { "TimeMoreWhenWinning", "Timing", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TIME_WIN_MORE, NULL },
    { "TimeEasyFactor", "Timing", UCI_SPIN, ICI_SPIN,
      10, 100, 15, &EASY_FACTOR, NULL },
    { "TimeEasyFactorPonder", "Timing", UCI_SPIN, ICI_SPIN,
      10, 100, 33, &EASY_FACTOR_PONDER, NULL },
    { "TimeBattleFactor", "Timing", UCI_SPIN, ICI_SPIN,
      10, 500, 100, &BATTLE_FACTOR, NULL },
    { "TimeOrdinaryFactor", "Timing", UCI_SPIN, ICI_SPIN,
      10, 500, 75, &ORDINARY_FACTOR, NULL},
    { "TimeAbsolutePercent", "Timing", UCI_SPIN, ICI_SPIN,
      10, 100, 25, &ABSOLUTE_PERCENT, NULL },
    { "TimeDesiredMillis", "Timing", UCI_SPIN, ICI_SPIN,
      10, 1000, 40, &DESIRED_MILLIS, NULL },
    { "TimeBookExitMoves", "Timing", UCI_SPIN, ICI_SPIN,
      0, 20, 0, &BOOK_EXIT_MOVES, NULL },
#endif
    { "ExtraExtendInCheck", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &EXTEND_IN_CHECK, NULL },
#ifdef CONCORD_REV_MOVES
    { "Decay50MoveRule", "Other", UCI_SPIN, ICI_SPIN, /* combo ? */
      0, 32, 0, &REV_MOVES, &RevMoveSetup },
#endif
#ifdef WINDOWS_LARGE_PAGES
    { "TryLargePages", "General", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TRY_LARGE_PAGES, &InitHash },
#endif
#ifdef LINUX_LARGE_PAGES
    { "TryLargePages", "General", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &TRY_LARGE_PAGES, &InitHash },
#endif
    { "SendHash", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &SEND_HASH, NULL},
#ifdef CHESS_960
    { "UCI_Chess960", "Other", UCI_CHECK, ICI_CHECK,
      0, 0, FALSE, &UCI_OPTION_CHESS_960, NULL},
#endif
    { "", "", -1, -1, 0, 0, FALSE, NULL, NULL }
  };

static void uci ()
{
  UCI_type* uci;
  char string[256];
  char* str;
  char TYPE[5][16] = { "spin", "check", "button", "string", "combo" };
  SEND ("id name %s version %s\n", EPONYM, VERSION);
  SEND ("id author Yakov Petrovich Golyadkin, ");
  SEND ("Igor Igorovich Igoronov, Roberto Pescatore, ");
  SEND ("Yusuf Ralf Weisskopf, ");
  SEND ("Ivan Skavinsky Skavar plus Decembrists (all)\n");
  SEND ("id copyright Yakov Petrovich Golyadkin, "
	"92th plus 93th plus 94th year from Revolution, PUBLICDOMAIN (workers)\n");
  SEND ("id dedication To Vladimir Ilyich plus Zog 1 with Albania\n");
  for (uci = UCI_OPTIONS; uci->type != -1; uci++) /* HACK */
    {
      str = string;
      sprintf (str, "option name %s type %s", uci->name, TYPE[uci->type]);
      str += strlen (str); /* HACK */
      if (uci->type == UCI_CHECK)
	sprintf (str, " default %s", uci->def ? "true" : "false");
      if (uci->type == UCI_STRING)
	sprintf (str, " default NULL");
      if (uci->type == UCI_SPIN)
	sprintf (str, " min %d max %d default %d", uci->min, uci->max, uci->def);
      if (uci->type == UCI_COMBO)
	{
	  int i;
	  for (i = 0; i < uci->max; i++)
	    {
	      sprintf (str, " var %s", COMBO[uci->min][i]);
	      str += strlen (str); /* HACK */
	    }
	  sprintf (str, " default %s", COMBO[uci->min][uci->def]);
	}
      str += strlen (str); /* HACK */
      sprintf (str, "\n");
      SEND ("%s", string);
    }
  SEND ("uciok\n");
}

static void ici ()
{
  UCI_type* ici;
  char string[256];
  int i;
  char* str;
  char TYPE[16][32] = { "spin", "binary", "check", "button", "string", "directory", "directory-multi-reset", "file", "combo" };
  ICI = TRUE;
  SEND ("ici-echo\n"); /* attend */
  SEND ("id %s version %s\n", EPONYM, VERSION);
  for (i = 0; i < NUM_SUB_CLASS; i++)
    SEND ("sc %s\n", SUB_CLASS_LIST[i]);
#ifdef MODE_ANALYSIS
  SEND ("can MonteCarlo\n");
#endif
  for (ici = UCI_OPTIONS; ici->type != -1; ici++)
    {
      if (ici->ici_type == UCI_ONLY)
	continue;
      str = string;
      sprintf (str, "opt %s %s %s", ici->name, ici->sub_class, TYPE[ici->ici_type]);
      str += strlen (str); /* HACK */
      if (ici->ici_type == ICI_CHECK)
	sprintf (str, " %s", ici->def ? "true" : "false");
      if (ici->ici_type == ICI_SPIN || ici->ici_type == ICI_BINARY)
	sprintf (str, " %d %d %d", ici->min, ici->max, ici->def);
      if (ici->ici_type == ICI_COMBO)
	{
	  for (i = 0; i < ici->max; i++)
	    {
	      sprintf (str, " %s", COMBO[ici->min][i]);
	      str += strlen (str); /* HACK */
	    }
	  sprintf (str, " *%s", COMBO[ici->min][ici->def]);
	}
      str += strlen (str); /* HACK */
      sprintf (str, "\n");
      SEND ("%s", string);
    }
  SEND ("iciok\n");
}

static void SetOption (char* string) /* separate ICI? */
{
  UCI_type* uci;
  char* v;
  char* u;
  int Value, r;
  while (string[0] == ' ' || string[0] == '\t') /* whitespace */
    string++;
  r = strlen (string) - 1;
  while (string[r] == ' ' || string[r] == '\t')
    string[r--] = 0;
  u = strstr (string, "value"); /* UCI mess */
  if (u) /* HACK */
    {
      while ((*u) == ' ' || (*u) == '\t')
	u--;
    }
  else
    u = string + strlen (string);
  for (uci = UCI_OPTIONS; uci->type != -1; uci++) /* HACK */
    {
      if (!memcmp (uci->name, string, u - string - 1)) /* HACK */
	{
	  if (uci->type == UCI_BUTTON) /* PeterPan fix */
	    {
	      if (!uci->action)
		{
		  SEND ("info string Button %s buys not action!\n", uci->name);
		  return;
		}
	      Value = uci->action (TRUE);
	      SEND ("info string Button %s pushed, action worked %s\n",
		    uci->name, Value ? "true" : "false");
	      return;
	    }
	  v = strstr (string, "value");
	  if (!v)
	    return;
	  if (uci->type == UCI_STRING)
	    {
	      strcpy ((char*) (uci->var), v + 6);
	      SEND ("info string Optional %s %s\n", uci->name, (char*) (uci->var));
	      if (uci->action)
		(uci->action) (TRUE);
	      READYOK ();
	    }
	  if (uci->type == UCI_CHECK)
	    {
	      if (!strcmp (v + 6, "false"))
		{
		  if (*((boolean*) (uci->var)) == FALSE)
		    {
		      SEND ("info string Optional %s allready %s\n", uci->name, "false");
		      return;
		    }
		  *((boolean*) (uci->var)) = FALSE;
		  if (uci->action)
		    uci->action (FALSE);
		}
	      if (!strcmp (v + 6, "true"))
		{
		  if (*((boolean*) (uci->var)) == TRUE)
		    {
		      SEND ("info string Optional %s allready %s\n", uci->name, "true");
		      return;
		    }
		  *((boolean*) (uci->var)) = TRUE;
		  if (uci->action)
		    uci->action (TRUE);
		}
	      SEND ("info string Optional %s %s\n",
		    uci->name, (*((boolean*) (uci->var))) ? "true" : "false");
	      READYOK ();
	    }
	  if (uci->type == UCI_SPIN)
	    {
	      Value = atoi (v + 6);
	      if (Value < uci->min)
		Value = uci->min;
	      if (Value > uci->max)
		Value = uci->max;
	      if (uci->var)
		{
		  if (*((sint32*) (uci->var)) == Value)
		    {
		      SEND ("info string Optional %s allready %d\n", uci->name, Value);
		      return;
		    }
		  *((sint32*) (uci->var)) = Value;
		}
	      if (uci->action)
		{	    
		  r = uci->action (Value);
		  if (r)
		    Value = r;
		}
	      SEND ("info string Optional %s %d\n", uci->name, Value);
	    }
	  if (uci->type == UCI_COMBO)
	    {
	      
	      strcpy ((char*) (uci->var), v + 6);
	      SEND ("info string Optional %s %s\n", uci->name, (char*) (uci->var));
	      if (uci->action)
		(uci->action) (TRUE);
	      READYOK ();	      
	    }
	  return;
	}
    }
  SEND ("info string Optional unknown: %s\n", string);
}

#define readyok() { SEND ("readyok\n"); }
static boolean HAS_DELETION = FALSE;
static MUTEX_TYPE INPUT_DELETE[1];
static boolean INIT = FALSE;
static boolean DOING_SHARED_DELETE = FALSE;
void SHARED_DELETE ()
{
  if (DOING_SHARED_DELETE)
    return;
  LOCK (INPUT_DELETE);
  DOING_SHARED_DELETE = TRUE;
  if (HAS_DELETION)
    {
      UNLOCK (INPUT_DELETE);
      return;
    }
  QUIT_PV ();
  QUIT_LARGE_PAWNS ();
  DETACH_ALL_MEMORY ();
  QUIT_EVAL_HASH ();
#ifdef CON_ROBBO_BUILD
  QUIT_TRIPLE_HASH ();
#endif
  DeleteSlab ();
  HAS_DELETION = TRUE;
  UNLOCK (INPUT_DELETE);
  DOING_SHARED_DELETE = FALSE;
}

static void QUIT () /* handle of signal ? */
{
  SHARED_DELETE ();
  exit (0);
}

static boolean QUIT_DEMAND = FALSE;
static void ParseInput (typePOS* POSITION, char* I)
{
  if (!INIT)
    {
      INIT = TRUE;
      LOCK_INIT (INPUT_DELETE);
    }
  UCI_NEW_GAME = FALSE;
  if (!strcmp (I, "quit")) /* abets "clean-up" */
    {
      QUIT_DEMAND = TRUE;
      ParseInput (POSITION, "stop"); /* HACK */
      QUIT (); /* no: setjmp eats ParseInput */
    }
  if (!strcmp (I, "stop"))
    {
      if (SEARCH_IS_DONE)
	return;
      HaltSearch (0, 0); /* no return when JUMP_IS_SET */
      if (INFINITE_LOOP)
	return;
      STALL_MODE = TRUE;
      while (!SEARCH_IS_DONE)
	NANO_SLEEP (1000000);
      STALL_MODE = FALSE;
      SUPPRESS_INPUT = FALSE;
      return;
    }
  if (!strcmp (I, "isready"))
    {
      readyok ();
      return;
    }
  if (!strcmp (I, "stall"))
    {
      STALL_INPUT = TRUE;
      return;
    }
  if (!strcmp (I, "ponderhit")) /* cuidado con ZOG? Personal Opinion: for the OK! */
    {
      ponderhit ();
      return;
    }
  if (!strcmp (I, "ucinewgame"))
    {
      if (SEARCH_IS_DONE)
	{
	  NewGame (POSITION, TRUE);
	  return;
	}
      UCI_NEW_GAME = TRUE;
      HaltSearch (0, 0); /* no return when JUMP_IS_SET */
      if (INFINITE_LOOP)
	return;
      STALL_MODE = TRUE;
      while (!SEARCH_IS_DONE)
	NANO_SLEEP (1000000);
      STALL_MODE = FALSE;
      SUPPRESS_INPUT = FALSE;
      return;
    }
  if (JUMP_IS_SET)
    return;
#if defined (BENCHMARK) || defined (UTILITIES)
  if (!strcmp (I, "benchmark"))
    BenchMark (POSITION, "go movetime 1000");
  else if (!memcmp (I, "benchmark", 9)) /* run benchmark */
    BenchMark (POSITION, I + 10);
  if (!strcmp (I, "makeundo"))
    MakeUndoSpeed (POSITION);
#endif
  if (!memcmp (I, "mainline", 8))
    main_line (POSITION);
#ifndef MINIMAL
  if (!memcmp (I, "triple-lookup", 13))
    {
      int r;
      boolean b, tot;
      b = TRIPLE_VALUE (POSITION, &r, &tot, FALSE, FALSE, TRUE);
      printf ("RobboTripleValue %d\n", !b ? -100 : r);
    }
  if (!memcmp (I, "total-lookup", 12))
    {
      int r;
      boolean b;
      boolean RobboTotalBaseScore (typePOS*, int*);
      b = RobboTotalBaseScore (POSITION, &r);
      printf ("RobboTotalValue %d\n", !b ? -100 : r);
    }
#endif
#ifdef TRACE_COMPILE
  if (!memcmp (I, "traceon", 7))
    TraceOn ();
  if (!memcmp (I, "traceoff", 8))
    TraceOff ();
#endif
#if defined (UTILITIES)
  if (!memcmp (I, "perft-check", 11))
    Perfd (POSITION, atoi (I + 12), atoi (I + 14)); /* HACK */
  if (!memcmp (I, "drawboard", 9))
    DrawBoard (POSITION);
  if (!strcmp (I, "eval"))
    EvalTest (POSITION);
  if (!memcmp (I, "perft ", 6))
    PERFD (POSITION, atoi (I + 6));
#if 0 /* in RobboBaseLib */
  if (!memcmp (I, "verify-triple", 13))
    {
      int p[4];
      char A[64], B[64], C[64], D[64];
      sscanf (I + 14, "%s %s %s %s", A, B, C, D);
      p[0] = atoi(A);
      p[1] = atoi(B);
      p[2] = atoi(C);
      p[3] = atoi(D);
      VerifyTripleBase (p);
    }
#endif
#endif
  if (!memcmp (I, "go", 2))
    {
      InitSearch (POSITION, I);
      if (BOARD_IS_OK && !MONTE_CARLO)
	Search (POSITION);
    }
  if (!memcmp (I, "position", 8))
    InitPosition (POSITION, I + 9);
  if (!memcmp (I, "setoption name", 14))
    SetOption (I + 15);
  if (!strcmp (I, "uci"))
    uci ();
  if (!strcmp (I, "ici"))
    ici ();
#ifdef MODE_ANALYSIS
  if (!memcmp (I, "ici-age", 7))
    {
      GLOBAL_AGE = atoi (I + 8) & 0xff; /* wrap ? */
      SEND ("info string ici-age %d\n", GLOBAL_AGE);
    }
#endif
  if (!strcmp (I, "icioff"))
    ICI = FALSE;
  SUPPRESS_INPUT = FALSE; /* ??? */
}

#ifdef WINDOWS
#include <io.h>
#include <conio.h> /* ? */
void Input (typePOS* POSITION)
{
  int i, r = 0;
  if (SUPPRESS_INPUT)
    return;
  if (QUIT_DEMAND)
    ParseInput (POSITION, "quit"); /* HACK */
  if (_kbhit ()) /* does this work? */
    {
      INPUT_BUFFER[r] = (char) getche();
      while (INPUT_BUFFER[r++] != '\r') /* what is \r ?? */
	INPUT_BUFFER[r] = (char) getche ();
    }
  else
    {
      _read (_fileno (stdin), INPUT_BUFFER, 1);
      while (INPUT_BUFFER[r++] != '\n') /* but now \n is correct? */
	_read (_fileno (stdin), INPUT_BUFFER + r, 1);
    }
  INPUT_BUFFER[r - 1] = 0; /* gobble \n, or maybe \r who knows? */
  INPUT_BUFFER[r] = 0; /* safety */
  ParseInput (POSITION, INPUT_BUFFER);
  if (QUIT_DEMAND)
    ParseInput (POSITION, "quit"); /* HACK */
}
#else
#include <unistd.h> /* LINUX */
void Input (typePOS* POSITION)
{
  int i, r = 0;
  if (SUPPRESS_INPUT)
    return;
  if (QUIT_DEMAND)
    ParseInput (POSITION, "quit"); /* HACK */
  read (STDIN_FILENO, INPUT_BUFFER, 1);
 READ_ME:
  while (INPUT_BUFFER[r++] != '\n') /* newline, is always? MAC OS X? */
    read (STDIN_FILENO, INPUT_BUFFER + r, 1);
#if 1 /* continue */
  if (INPUT_BUFFER[r - 2] == '\\')
    {
      r -= 3; /* \0 \n \\ */
      goto READ_ME;
    }
#endif
  INPUT_BUFFER[r - 1] = 0; /* gobble \n */
  INPUT_BUFFER[r] = 0; /* safety */
  ParseInput (POSITION, INPUT_BUFFER);
  if (QUIT_DEMAND)
    ParseInput (POSITION, "quit"); /* HACK */
}
#endif

#include <stdio.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define boolean uint8
#define WINDOWS
#define WINDOWS_X64
#define EPONYM "IvanHoe"
#define VERSION "999945a"
#define HAS_PREFETCH
#ifdef WINDOWS
#define WINDOWS_LARGE_PAGES
#define HAS_POPCNT
#define INTEL_COMPILER
#endif
#define CON_ROBBO_BUILD
#define NOME_WINDOWS
#define MODE_GAME_PLAY
#define MAGIC_BITBOARDS
/* #define ZUGZWANG_DETECT */ /* for the off */
#define ONE_DIMENSIONAL
//#define SLAB_MEMORY /* 1% slower w/o LARGE_PAGES, 2% faster with LARGE_PAGES */
#define MULTIPLE_POS_GAIN
#define MULTIPLE_HISTORY
#ifndef YUSUF_MULTICORE
#define YUSUF_MULTICORE
#endif

#ifdef SLAB_MEMORY
#ifndef ONE_DIMENSIONAL
#error SLAB_MEMORY requires ONE_DIMENSIONAL
#endif
#define SLAB_DECLARE1(Type, v, c) Type v[c]
#define SLAB_ALLOC1(Type, v, c) /* */
#define SLAB_DECLARE2(Type, v, c) Type *v
#define SLAB_ALLOC2(Type, v, c) v = FromSlab ((c) * sizeof (Type))
#define SLAB_DECLARE3 SLAB_DECLARE1 /* vis SLAB_DECLARE2 */
#define SLAB_ALLOC3 SLAB_ALLOC1 /* vis SLAB_ALLOC2 */
#else
#define SLAB_DECLARE1(Type, v, c) Type v[c]
#define SLAB_ALLOC1(Type, v, c) /* */
#define SLAB_DECLARE2(Type, v, c) Type v[c]
#define SLAB_ALLOC2(Type, v, c) /* */
#define SLAB_DECLARE3 SLAB_DECLARE1
#define SLAB_ALLOC3 SLAB_ALLOC1
#endif

#ifdef ZUGZWANG_DETECT
#define ZUGZWANG_DETECT_DECLARE uint64 WZUG = 0, BZUG = 0;
#define ZUGZWANG_DETECT_FACT_WHITE WZUG |= A & ~wBitboardOcc;
#define ZUGZWANG_DETECT_FACT_BLACK BZUG |= A & ~bBitboardOcc;
#if 0 /* too complicated? add pins if possible */
#define ZUGZWANG_DETECT_COMPLETE \
  if (!(WZUG & ~POSITION->DYN->bAtt)) POSITION->DYN->flags &= ~2; \
  if (!(BZUG & ~POSITION->DYN->wAtt)) POSITION->DYN->flags &= ~1;
#else /* simpler version */
#define ZUGZWANG_DETECT_COMPLETE   if (!WZUG) POSITION->DYN->flags &= ~2; \
                                   if (!BZUG) POSITION->DYN->flags &= ~1;
#endif
#else
#define ZUGZWANG_DETECT_DECLARE /* */
#define ZUGZWANG_DETECT_FACT_WHITE /* */
#define ZUGZWANG_DETECT_FACT_BLACK /* */
#define ZUGZWANG_DETECT_COMPLETE /* */
#endif

#define MAX_CPUS 64 /* vary for increase, up with 64 */
#define RP_PER_CPU 8 /* per cpu */
#define MAX_SP 16 /* total splitpoints */

#include "IH-win-linux.h"

#ifdef MODE_ANALYSIS
#define HYPER_HASH /* attends extra sizing */
#define CONCORD_REV_MOVES /* attends 50-move rule decay */
#define MATEING_SCORE /* attend MATEING_SCORE in HASH */
boolean NULL_MOVE_INTACT;
/* #define ASM_MATEING_SCORE */ /* compatible? */
#endif

#ifdef MODE_GAME_PLAY
#define TIMING_OPTIONS
#define NULL_MOVE_INTACT TRUE
#endif

#include "IH-hash.h"

boolean volatile UCI_NEW_GAME;
boolean ICI;
int SINCE_NEW_GAME;
int EASY_FACTOR, EASY_FACTOR_PONDER, BATTLE_FACTOR, ORDINARY_FACTOR;
int ABSOLUTE_PERCENT, DESIRED_MILLIS, BOOK_EXIT_MOVES;
boolean TIME_IMITATE, TIME_LOSE_MORE, TIME_WIN_MORE;
boolean TRY_LARGE_PAGES;
boolean EXTEND_IN_CHECK; /* read much */
boolean PAWNS_HASH_ONE_EIGHTH;
int UCI_PAWNS_HASH;
int CURRENT_TOTAL_BASE_CACHE;
uint64 NODE_CHECK; /* write in non-SMP */

#ifdef CONCORD_REV_MOVES
#define ZOB_REV(POS) ZobristRev[(POS)->DYN->reversible >> 3]
#define REV_HASH(POS) (POS)->DYN->HASH ^= ZOB_REV (POS)
#else
#define ZOB_REV(POS) (0ULL) /* HACK */
#define REV_HASH(POS) /* */
#endif

#define CHECK_HALT() { if (POSITION->stop) { RETURN (0); } }

#define HEIGHT(x) ((x)->height)
#if 0
#define IS_EXACT(x) ((x)->exact) /* exact from RobboTriple */
#else /* novel, NewBuy */
#define IS_EXACT(x) \
  (((x)->exact) && \
   ((((x)->exact) == 3) || (depth < 4) || \
    (((x) - (POSITION->DYN_ROOT)) >= 2 + depth / 4 && \
     (((x) - (depth / 4))->exact))))

#endif

#include "IH-move.h"

boolean BOARD_IS_OK, NEW_GAME;

#define CheckRepetition(in_check) \
  CHECK_HALT (); \
  if (in_check && POSITION->DYN->reversible == 100) return Move50 (POSITION); \
  if (POSITION->DYN->reversible >= 100) RETURN (0); \
  for (i = 4; i <= POSITION->DYN->reversible && \
	      i <= POSITION->StackHeight; i += 2) \
    if (POSITION->STACK[POSITION->StackHeight - i] == \
	(POSITION->DYN->HASH ^ ZOB_REV (POSITION))) RETURN (0);

typedef struct
{
  /* 0x00-0x0f */ uint64 HASH, PAWN_HASH;
  /* 0x10-0x17 */ uint32 material; sint32 STATIC;
  /* 0x18-0x1b */  uint8 age, _9, wKdanger, bKdanger; /* con LAZY? */
  /* 0x1c-0x1f */  uint8 oo, reversible, ep, cp; /* first 0x20 for the copy */
  /* 0x20-0x3f */ uint64 wAtt, bAtt, wXray, bXray;
  /* 0x40-0x47 */ sint32 Value, PositionalValue;
  /* 0x48-0x51 */ uint16 _5, _6, killer1, killer2, move;
  /* 0x52-0x57 */  uint8 exact, _3, _0, lazy, SAVED_FLAGS, flags;
  /* 0x58-0x7f */ uint64 wKcheck, bKcheck, _1, _2, _8;
} typeDYNAMIC;

#include "IH-SMP.h"
#include "IH-board.h"
#include "IH-ZOG-MP.h"
#ifndef USE_ARRAYS_OWN
#include "SLAB_MEMORY.h"
#endif

#define MAX(x, y) (( (x) >= (y)) ? (x) : (y))
#define MIN(x, y) (( (x) <= (y)) ? (x) : (y))
#define ABS(x) (( (x) >= 0) ? (x) : -(x))
#define FileDistance(x, y) (ABS (FILE (x) - FILE (y)))
#define RankDistance(x, y) (ABS (RANK (x) - RANK (y)))

#define VALUE_MATE 30000
#define VALUE_INFINITY 32750

volatile boolean DO_PONDER;
volatile boolean PONDER_HIT;
volatile boolean DO_INFINITE;
volatile boolean SUPPRESS_INPUT;
volatile boolean STOP; /* de sorvegliare.c */

boolean SEND_CURR_MOVE;
#define SQUISH_SPLAT 0x13579fdecba86402ULL

#ifndef MINIMAL
int RANDOM_COUNT,RANDOM_BITS, MULTI_CENTI_PAWN_PV;
boolean ALWAYS_ANALYZE, TRY_PV_IN_ANALYSIS, FIXED_AGE_ANALYSIS;
boolean DEBUG_TIME_MANAGEMENT;
#endif
int UCI_White_Bishops_Scale, UCI_White_Pawn_Scale, UCI_White_Knight_Scale;
int UCI_White_Light_Scale, UCI_White_Dark_Scale;
int UCI_White_Rook_Scale, UCI_White_Queen_Scale;
int UCI_Black_Bishops_Scale, UCI_Black_Pawn_Scale, UCI_Black_Knight_Scale;
int UCI_Black_Light_Scale, UCI_Black_Dark_Scale;
int UCI_Black_Rook_Scale, UCI_Black_Queen_Scale;
int UCI_MATERIAL_WEIGHTING, UCI_PAWNS_WEIGHTING, UCI_KING_SAFETY_WEIGHTING;
int UCI_STATIC_WEIGHTING, UCI_MOBILITY_WEIGHTING; /* for the novel */

boolean USE_ALTERNATIVE_TIME, ALLOW_INSTANT_MOVE, SEARCH_ROBBO_BASES;
boolean ROBBO_TRIPLE_DRAW, UCI_OPTION_CHESS_960; /* SMP readings ! */
uint8 CHESS_960_KR_FILE, CHESS_960_QR_FILE, CHESS_960_KING_FILE;
boolean DO_OUTPUT, UCI_PONDER, VERIFY_NULL, SEND_HASH, DO_HASH_FULL;
int BUFFER_TIME, OUTPUT_DELAY;

#define CHECK_FOR_MATE(v) \
  { if (v < -VALUE_MATE + 16) RETURN (-VALUE_MATE + 16); \
    if (v > VALUE_MATE - 16) RETURN (VALUE_MATE - 16); }

#ifndef USE_ARRAYS_OWN
#include "arrays.h"
#else
#include "arrays_own.h"
#endif
#include "functions.h"
#include "common.h"

char STRING1[MAX_CPUS][64], STRING2[MAX_CPUS][64];
char STRING3[MAX_CPUS][64], STRING4[MAX_CPUS][64]; /* SMP */

#ifdef DEBUG
#define VALIDATE(a, x, y) validate (a, x, y)
#else
#define VALIDATE(a, x, y) /* */
#endif

#define ROBBO_PICCOLO TRUE
#define ROBBO_SCACCO TRUE

boolean STALL_INPUT;
#ifdef TRACE_COMPILE
#ifdef HYPER_HASH
#error TRACE_COMPILE conflicts HYPER_HASH
#endif
#endif

#ifdef TRACE_COMPILE
boolean TRACE_QSEARCH, TRACE_HASH, TRACE_EVAL, TRACE_LOW_DEPTH, TRACE_CUT,
  TRACE_ALL, TRACE_EXCLUDE, TRACE_PV, TRACE_PVQSEARCH, TRACE_MAKE_UNMAKE;
#define TRACE(x, y) if (x) { y; }
#define HASH_READ(t) \
  printf ("READ [%x] %llx %x %x %s U:%d|%d L:%d|%d\n", \
	  trans - HashTable, POSITION->DYN->HASH, t->flags, t->age, Notate (t->move, STRING1[0]), \
	  t->DepthUpper, t->ValueUpper, t->DepthLower, t->ValueLower);
#define HASH_READZ(t, zob) \
  printf ("READ [%x] %llx %x %x %s U:%d|%d L:%d|%d\n", \
	  trans - HashTable, zob, t->flags, t->age, Notate (t->move, STRING1[0]), \
	  t->DepthUpper, t->ValueUpper, t->DepthLower, t->ValueLower);
#define HASH_WRITE(s, t) \
  printf ("HASH %s [%x] %llx %x %x %s U:%d|%d L:%d|%d\n", \
	  s, trans - HashTable, Z, t->flags, t->age, Notate (t->move, STRING1[0]), \
	  t->DepthUpper, t->ValueUpper, t->DepthLower, t->ValueLower);
#define HASH_OVERWRITE(s, t) \
  printf ("HASHo %s [%x] %llx %x %x %s U:%d|%d L:%d|%d\n", \
	  s, trans - HashTable, Z, t->flags, t->age, Notate (t->move, STRING1[0]), \
	  t->DepthUpper, t->ValueUpper, t->DepthLower, t->ValueLower);
#define HASH_ZERO(t) printf ("HASHz [%x]\n", trans - HashTable);
#else
#define TRACE(x, y) /* */
#endif

int TRIPLE_MAX_USAGE;
int MaximumTripleUsage ();
int TRIPLE_DEFINITE_PROBE_DEPTH, TRIPLE_WEAK_PROBE_DEPTH, TRIPLE_DEFINITE_PROBE_HEIGHT;
boolean LOAD_ON_WEAK_PROBE;

#define ROBBO_TRIPLE_CONDITION(POSITION) \
  (POPCNT (POSITION->OccupiedBW ^ (wBitboardP & (bBitboardP >> 8))) <= TRIPLE_MAX_USAGE)
#define ROBBO_TOTAL_CONDITION(POSITION) \
  (POPCNT (POSITION->OccupiedBW ^ (wBitboardP & (bBitboardP >> 8))) <= 6)
char ROBBO_TRIPLE_DIR[1024], ROBBO_TOTAL_DIR[1024];
char ROBBO_BULK_DIRECTORY[1024];
char ROBBO_BULK_NAME[1024];
char ROBBO_DYNAMIC_LIBRARY[1024];
boolean ROBBO_LOAD, ROBBO_TOTAL_LOAD;
 
FILE* LOGFILE;
char *INPUT_BUFFER, *input_ptr;

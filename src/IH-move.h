
#define WHITE_IN_CHECK (POSITION->DYN->bAtt & wBitboardK)
#define BLACK_IN_CHECK (POSITION->DYN->wAtt & bBitboardK)
#define POS1 (POS0 + 1)
#define MOVE_IS_CHECK_WHITE (POS1->wKcheck)
#define MOVE_IS_CHECK_BLACK (POS1->bKcheck)
#define HEIGHT(x) ((x)->height)

typedef struct { uint32 move; } typeMoveList;
typedef struct { uint32 move; sint32 value; uint64 nodes; } typeRootMoveList;

typedef struct
{
  int phase, mask, bc;
  uint32 trans_move, move, exclude;
  uint64 TARGET;
  typeMoveList LIST[256];
  uint32 BAD_CAPS[64];
} typeNEXT;

#define MAXIMUM_PLY 2048
#define WhiteOO (POSITION->DYN->oo & 0x1)
#define WhiteOOO (POSITION->DYN->oo & 0x2)
#define BlackOO (POSITION->DYN->oo & 0x4)
#define BlackOOO (POSITION->DYN->oo & 0x8)

typedef enum
{ TRANS, CAPTURE_GEN, CAPTURE_MOVES,
    KILLER1, KILLER2, ORDINARY_MOVES, BAD_CAPS,
  TRANS2, CAPTURE_PGEN2, CAPTURE_MOVES2, QUIET_CHECKS, EVADE_PHASE, TRANS3,
  CAPTURE_GEN3, CAPTURE_MOVES3, QUIET_CHECKS3, POSITIONAL_GAIN_PHASE, FASE_0
} EnumPhases;

#define FlagEP 030000
#define FlagOO 010000
#define FLAG_MASK 070000
#define FlagPromQ 070000
#define FlagPromR 060000
#define FlagPromB 050000
#define FlagPromN 040000
#define MoveIsEP(x) (((x) & FLAG_MASK) == FlagEP)
#define MoveIsProm(x) (((x) & FLAG_MASK) >= FlagPromN)
#define MoveIsOO(x) (((x) & FLAG_MASK) == FlagOO)
#define MoveHistory(x) (((x) & 060000) == 0)

#define Direction_h1a8 0
#define Direction_a1h8 1
#define Direction_horz 2
#define Direction_vert 3
#define BAD_DIRECTION 37
#define MOVE_NONE 0

typedef struct
{ uint32 move; sint32 Value, alpha, beta; uint32 depth, _0; uint64 nodes; } typeMPV;
typeMPV MPV[256]; /* SLAB? */

int MULTI_PV;
boolean DO_SEARCH_MOVES;
uint32 SEARCH_MOVES[256]; /* SLAB? */

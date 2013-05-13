
typedef struct
{
  uint64 volatile PAWN_HASH;
  uint8 wPfile_count, bPfile_count, OpenFileCount;
  boolean locked;
  uint32 wKdanger, bKdanger;
  uint8 wPlight, wPdark, bPlight, bPdark,
    wPassedFiles, bPassedFiles, wDrawWeight, bDrawWeight;
  uint32 SCORE;
} typePawnEval; /* 32 bytes */
typePawnEval *PawnHash;

#define CHECK_HALT() { if (POSITION->stop) { RETURN (0); } }

#define FLAG_LOWER 1
#define FLAG_UPPER 2
#define FLAG_CUT 4
#define FLAG_ALL 8
#define FLAG_EXACT 16
#define FLAG_MOVE_LESS 32
#define UPPER_FROM_PV 64
#define LOWER_FROM_PV 128

#define IsCUT(tr) ((tr->flags) & FLAG_CUT)
#define IsALL(tr) ((tr->flags) & FLAG_ALL)
#define IsExact(tr) ((tr)->flags & FLAG_EXACT)

typedef struct
{
  /* 00-03 */ uint32 hash;
  /* 04-07 */ uint8 flags, age, DepthUpper, DepthLower;
  /* 08-0b */ sint16 ValueLower, ValueUpper; /* overlap hash in lockless */
  /* 0c-0d */ uint16 move;
  /* 0e-0f */ uint8 reversibles, sticky; /* unused in the currency */
} typeHash;

#ifdef MODE_GAME_PLAY
#define STICKY_CONDITION(T) TRUE
#endif
#ifdef MODE_ANALYSIS
#define STICKY_CONDITION(T) (!(T)->sticky)
#endif

typeHash *HashTable;
uint64 HashMask, GLOBAL_AGE;

#ifndef HYPER_HASH
#define HASH_POINTER(x) (HashTable + ((x) & HashMask))
#define HASH_POINTER0(x) (HashTable + ((x) & (3 + HashMask)))
#else
uint8 HASH_WHICH[16];
typeHash* TABLE_ARR[3];
uint64 MASK_ARR[3]; /* 64 bytes, but yet for the align? */
#define HASH_POINTER(x) \
  (TABLE_ARR[HASH_WHICH[(x) >> 60]] + ((x) & MASK_ARR[HASH_WHICH[(x) >> 60]]))
#define HASH_POINTER0(x) \
  (TABLE_ARR[HASH_WHICH[(x) >> 60]] + ((x) & (3 + MASK_ARR[HASH_WHICH[(x) >> 60]])))
#endif

#define NEW_HASH
#ifdef NEW_HASH
#define VAS(v) VALUE_ADJUST_STORE ((v), (HEIGHT (POSITION)))
#if 0 /* novel */
#define AgePass (GLOBAL_AGE)
#else
#define AgePass (POSITION->DYN->age)
#endif
#define HashLowerALL(POS, mv, dp, v) \
  HashLowerALLNew (POS->DYN->HASH, mv, dp, v, HEIGHT (POS), AgePass)
#define HashUpperCUT(POS, dp, v) \
  HashUpperCUTNew (POS->DYN->HASH, dp, v, HEIGHT (POS), AgePass)
#define HashLower(Z, mv, dp, v) \
  HashLowerNew (Z, mv, dp, v, 0, HEIGHT (POSITION), AgePass)
#define HashUpper(Z, dp, v) \
  HashUpperNew (Z, dp, v, 0, HEIGHT (POSITION), AgePass)
#define HashLowerPV(Z, mv, dp, v) \
  HashLowerNew (Z, mv, dp, v, LOWER_FROM_PV, HEIGHT (POSITION), AgePass)
#define HashUpperPV(Z, dp, v) \
  HashUpperNew (Z, dp, v, UPPER_FROM_PV, HEIGHT (POSITION), AgePass)
#define HashExact(POS, mv, dp, v, FL) \
  HashExactNew (POS->DYN->HASH, mv, dp, v, FL, HEIGHT (POSITION), AgePass)
#endif

#ifdef MATEING_SCORE
#define VALUE_ADJUST_STORE(val, ht) \
  ((val) + (((val) > 15000) ? (16 * ht) : 0) - (((val) < -15000) ? (16 * ht) : 0))
#define HashUpperBound(tr) \
  (tr->ValueUpper - ((tr->ValueUpper > 15000) ? (16 * HEIGHT (POSITION)) : 0) \
                  + ((tr->ValueUpper < -15000) ? (16 * HEIGHT (POSITION)) : 0))
#define HashLowerBound(tr) \
  (tr->ValueLower - ((tr->ValueLower > 15000) ? (16 * HEIGHT (POSITION)) : 0) \
                  + ((tr->ValueLower < -15000) ? (16 * HEIGHT (POSITION)) : 0))
#define HashUpperValue(tr) (tr->ValueUpper)
#define HashLowerValue(tr) (tr->ValueLower)
#define HashReBound(Va, VA)  { if (Va > 15000) Va -= 16 * HEIGHT (POSITION); \
                               if (Va < -15000) Va += 16 * HEIGHT (POSITION); }

#else /* nein MATEING_SCORE */
#define VALUE_ADJUST_STORE(Value, height) (Value)
#define HashUpperValue(tr) (tr->ValueUpper)
#define HashLowerValue(tr) (tr->ValueLower)
#define HashUpperBound(tr) (tr->ValueUpper)
#define HashLowerBound(tr) (tr->ValueLower)
#define HashReBound(Va, VA) /* */
#endif

#ifdef LOCKLESS_HASHING  /* Fact: 2-3% slowed, for the why? */
#define TRANS_DECLARE() typeHash trans[1]
#define YUSUF_DECLARE() typeHash *Trans
#define TRANS_PV_DECLARE() typePVHash trans_pv[1]
#define YUSUF_PV_DECLARE() typePVHash *Trans_pv
#if 0 /* on/off */
#define READ_WRITE_BARRIER() asm volatile ("" ::: "memory")
#define HYATT_HASH(T, t) \
  { READ_WRITE_BARRIER(); \
    ((uint64*) (t))[0] = ((uint64*) (T))[0] ^ ((uint64*) (T))[1]; \
    ((uint64*) (t))[1] = ((uint64*) (T))[1]; READ_WRITE_BARRIER(); }
#define HYATT_HASH_WRITE(t, T) \
  { READ_WRITE_BARRIER(); \
    ((uint64*) (T))[0] = ((uint64*) (t))[0] ^ ((uint64*) (t))[1]; \
    ((uint64*) (T))[1] = ((uint64*) (t))[1]; READ_WRITE_BARRIER(); }
#else /* dummy */ /* Fact: 2-3% snailed in the already with the this */
#define HYATT_HASH(T, t) { memcpy (t, T, 16); }
#define HYATT_HASH_WRITE(T, t) { memcpy (t, T, 16); }
#endif
#define UPDATE_AGE() /* ? */ /* Need for the think */
#else /* Personal Opinion: in the forget for the LOCKLESS_HASHING */
#define Trans trans
#define Trans_pv trans_pv
#define TRANS_DECLARE() typeHash *trans
#define YUSUF_DECLARE() /* */
#define TRANS_PV_DECLARE() typePVHash *trans_pv
#define YUSUF_PV_DECLARE() /* */
#define HYATT_HASH(x, y) /* */
#define HYATT_HASH_WRITE(x, y) /* */
#if 1
#define UPDATE_AGE() trans->age = (AgePass); /* purposeful? */
#else
#define UPDATE_AGE() /* */
#endif
#endif

typedef struct
{
  uint64 hash;
  sint32 Value;
  uint16 move;
  uint8 depth, age;
} typePVHash;
typePVHash *PVHashTable; /* multicore ? */
uint64 PVHashMask;

sint64 PawnHashSize; /* uci option ahora */
sint64 EvalHashSize;
uint64 EVAL_HASH_MASK;
uint64* EvalHash;
sint64 TripleHashSize;
uint64 TRIPLE_HASH_MASK;
uint64* TripleHash;

#ifdef MODE_GAME_PLAY
#define ICI_HASH_MIX FALSE
#endif
#ifdef MODE_ANALYSIS
boolean ICI_HASH_MIX;
#endif

int CURRENT_HASH_SIZE; /* HACK */
int REV_MOVES;

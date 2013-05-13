
#define FILE(s) ((s) & 7)
#define RANK(s) ((s) >> 3)
#define FROM(s) (((s) >> 6) & 077)
#define TO(s) ((s) & 077)

typedef enum
{ wEnumOcc, wEnumP, wEnumN, wEnumK, wEnumBL, wEnumBD, wEnumR, wEnumQ,
  bEnumOcc, bEnumP, bEnumN, bEnumK, bEnumBL, bEnumBD, bEnumR, bEnumQ
} EnumPieces;
typedef enum
{ A1, B1, C1, D1, E1, F1, G1, H1, A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3, A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5, A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7, A8, B8, C8, D8, E8, F8, G8, H8
} EnumSquares;
typedef enum { R1, R2, R3, R4, R5, R6, R7, R8 } EnumRanks;
typedef enum { FA, FB, FC, FD, FE, FF, FG, FH } EnumFiles;

#define wBitboardK POSITION->bitboard[wEnumK]
#define wBitboardQ POSITION->bitboard[wEnumQ]
#define wBitboardR POSITION->bitboard[wEnumR]
#define wBitboardBL POSITION->bitboard[wEnumBL]
#define wBitboardBD POSITION->bitboard[wEnumBD]
#define wBitboardB (wBitboardBL|wBitboardBD)
#define wBitboardN POSITION->bitboard[wEnumN]
#define wBitboardP POSITION->bitboard[wEnumP]
#define wBitboardOcc POSITION->bitboard[wEnumOcc]
#define bBitboardK POSITION->bitboard[bEnumK]
#define bBitboardQ POSITION->bitboard[bEnumQ]
#define bBitboardR POSITION->bitboard[bEnumR]
#define bBitboardBL POSITION->bitboard[bEnumBL]
#define bBitboardBD POSITION->bitboard[bEnumBD]
#define bBitboardB (bBitboardBL|bBitboardBD)
#define bBitboardN POSITION->bitboard[bEnumN]
#define bBitboardP POSITION->bitboard[bEnumP]
#define bBitboardOcc POSITION->bitboard[bEnumOcc]

#ifdef ONE_DIMENSIONAL /* 1-dim */
#define ShiftLeft45 (LineShift + 0100 * Direction_h1a8)
#define ShiftRight45 (LineShift + 0100 * Direction_a1h8)
#define ShiftAttack (LineShift + 0100 * Direction_horz)
#define ShiftLeft90 (LineShift + 0100 * Direction_vert)
#define AttLeft45 (LineMask + 0100 * 0100 * Direction_h1a8)
#define AttRight45 (LineMask + 0100 * 0100 * Direction_a1h8)
#define AttNormal (LineMask + 0100 * 0100 * Direction_horz)
#define AttLeft90 (LineMask + 0100 * 0100 * Direction_vert)
#else
#define ShiftLeft45 LineShift[Direction_h1a8]
#define ShiftRight45 LineShift[Direction_a1h8]
#define ShiftAttack LineShift[Direction_horz]
#define ShiftLeft90 LineShift[Direction_vert]
#define AttLeft45 LineMask[Direction_h1a8]
#define AttRight45 LineMask[Direction_a1h8]
#define AttNormal LineMask[Direction_horz]
#define AttLeft90 LineMask[Direction_vert]
#endif

#ifdef MAGIC_BITBOARDS
typedef struct { uint64 mask, mult, shift; uint64* index; } type_MM;
#define AttRocc(sq, OCC) ROOK_MM[sq].index\
  [((OCC & ROOK_MM[sq].mask) * ROOK_MM[sq].mult) >> ROOK_MM[sq].shift]
#define AttBocc(sq, OCC) BISHOP_MM[sq].index\
  [((OCC & BISHOP_MM[sq].mask) * BISHOP_MM[sq].mult) >> BISHOP_MM[sq].shift]
#define AttB(sq) AttBocc (sq, POSITION->OccupiedBW)
#define AttR(sq) AttRocc (sq, POSITION->OccupiedBW)
#define AttFile(sq) (AttR (sq) & FileArray[FILE (sq)])

#else /* not MAGIC_BITBOARDS */

#ifdef ONE_DIMENSIONAL
#define AttLEFT45(x, y) AttLeft45[0100 * (x) + (y)]
#define AttRIGHT45(x, y) AttRight45[0100 * (x) + (y)]
#define AttNORMAL(x, y) AttNormal[0100 * (x) + (y)]
#define AttLEFT90(x, y) AttLeft90[0100 * (x) + (y)]
#define Att_h1a8(fr) \
  AttLEFT45 (fr, (POSITION->OccupiedL45 >> ShiftLeft45[fr]) & 077)
#define Att_a1h8(fr) \
  AttRIGHT45 (fr, (POSITION->OccupiedR45 >> ShiftRight45[fr]) & 077)
#define AttRank(fr) \
  AttNORMAL (fr, (POSITION->OccupiedBW >> ShiftAttack[fr]) & 077)
#define AttFile(fr) \
  AttLEFT90 (fr, (POSITION->OccupiedL90 >> ShiftLeft90[fr]) & 077)
#else
#define AttLEFT45(x, y) AttLeft45[x][y]
#define AttRIGHT45(x, y) AttRight45[x][y]
#define AttNORMAL(x, y) AttNormal[x][y]
#define AttLEFT90(x, y) AttLeft90[x][y]
#define Att_h1a8(fr) \
 AttLeft45[fr][(POSITION->OccupiedL45 >> ShiftLeft45[fr]) & 077]
#define Att_a1h8(fr) \
 AttRight45[fr][(POSITION->OccupiedR45 >> ShiftRight45[fr]) & 077]
#define AttRank(fr) \
 AttNormal[fr][(POSITION->OccupiedBW >> ShiftAttack[fr]) & 077]
#define AttFile(fr) \
 AttLeft90[fr][(POSITION->OccupiedL90 >> ShiftLeft90[fr]) & 077]
#endif

#define AttB(fr) (Att_a1h8(fr) | Att_h1a8(fr))
#define AttR(fr) (AttRank(fr) | AttFile(fr))
#endif

#define AttQ(fr) (AttR(fr) | AttB(fr))

#define MAX_STACK 256
#define NUM_BYTES_TO_COPY (64 + 128 + 32 + 128 + 4)
struct TP
{
  uint8 sq[64];
  uint64 bitboard[16];
#ifndef MAGIC_BITBOARDS
  uint64 OccupiedBW, OccupiedL90, OccupiedL45, OccupiedR45;
#else
  uint64 OccupiedBW, _0, _1, _2;
#endif
  uint8 XRAYw[64], XRAYb[64];
  uint8 wtm, wKsq, bKsq, height;
  /* Above bytes are copied, exactly 356 of them */
  /* 64 + 128 + 32 + 128 + 4 = 356 */
  typeDYNAMIC *DYN, *DYN_ROOT;
  uint64 STACK[MAX_STACK], StackHeight;
  uint64 nodes, tbhits;
  uint8 cpu, rp;
  boolean stop, used;
  MUTEX_TYPE padlock[1];
  int child_count;
  struct TP *parent, *children[MAX_CPUS];
  SPLITPUNKT *SplitPoint;
};

typedef struct TP typePOS;


typedef struct { sint16 Value; uint8 token, flags; } typeMATERIAL;
SLAB_DECLARE2 (typeMATERIAL, MATERIAL, 419904); /* SMP read */ /* SLAB 1.68mb */



#ifdef MAGIC_BITBOARDS
SLAB_DECLARE1 (type_MM, ROOK_MM, 64);
SLAB_DECLARE1 (type_MM, BISHOP_MM, 64);
SLAB_DECLARE2 (uint64, MM_ORTHO, 102400); /* SLAB 800k */
SLAB_DECLARE2 (uint64, MM_DIAG, 5248);
#endif

typePOS ROOT_POSITION[MAX_CPUS][RP_PER_CPU]; /* SLAB? */ /* 1-dim */
typePOS ROOT_POSIZIONE0[1], NULL_PARENT[1];

#ifdef ONE_DIMENSIONAL
#ifdef MULTIPLE_POS_GAIN
SLAB_DECLARE2 (sint16, MAX_POSITIONAL_GAIN, MAX_CPUS * 0x10 * 010000); /* SLAB 1mb */
#define MAX_POS_GAIN(pez, mos) \
  MAX_POSITIONAL_GAIN[POSITION->cpu * 0x10 * 010000 + (pez) * 010000 + (mos)]
#else
SLAB_DECLARE2 (sint16, MAX_POSITIONAL_GAIN, 0x10 * 010000);
#define MAX_POS_GAIN(pez, mos) MAX_POSITIONAL_GAIN[(pez) * 010000 + (mos)]
#endif

#define HISTORY_VALUE(P, M) HISTORY_PI_TO (P, P->sq[FROM (M)],TO (M))
#ifdef MULTIPLE_HISTORY
SLAB_DECLARE2 (uint16, HISTORY, MAX_CPUS * 0x10 * 0100); /* SLAB 64k */
#define HISTORY_PI_TO(P, PI, To) HISTORY[P->cpu * 0x10 * 0100 + 0100 * (PI) + (To)]
#else
SLAB_DECLARE2 (uint16, HISTORY, 0x10 * 0100);  /* SLAB 8k */
#define HISTORY_PI_TO(P, PI, To) HISTORY[(PI) * 0100 + (To)]
#endif

SLAB_DECLARE2 (sint32, PieceSquareValue, 0x10 * 0100); /* SMP read SLAB 16k */
#define PST(pi, sq) PieceSquareValue[(pi) * 0100 + (sq)]

#else /* nein ONE_DIMENSIONAL */

#ifdef MULTIPLE_POS_GAIN
sint16 MAX_POSITIONAL_GAIN[MAX_CPUS][0x10][010000];
#define MAX_POS_GAIN(pez, mos) MAX_POSITIONAL_GAIN[POSITION->cpu][pez][mos]
#else
sint16 MAX_POSITIONAL_GAIN[0x10][010000];
#define MAX_POS_GAIN(pez, mos) MAX_POSITIONAL_GAIN[pez][mos]
#endif

#define HISTORY_VALUE(P, M) HISTORY_PI_TO (P, P->sq[FROM (M)],TO (M))
#ifdef MULTIPLE_HISTORY
uint16 HISTORY[MAX_CPUS][0x10][0100]; /* SLAB 64k */ /* SMP cashline ? */
#define HISTORY_PI_TO(P, PI, To) HISTORY[P->cpu][PI][To]
#else
uint16 HISTORY[0x10 * 0100]; /* SLAB 8k */
#define HISTORY_PI_TO(P, PI, To) HISTORY[PI][To]
#endif

sint32 PieceSquareValue[0x10][0100];
#define PST(pi, sq) PieceSquareValue[pi][sq]

#endif


uint8 lanc_pedone[4][64];

#define TRIANGOLO 0x00000000080c0e0f
#define MARGINE 0x0000000008040201
#define TRI_MAR 0x80c0e0f0f8fcfeff

#define MAX_TABLES 4096 /* RobboTotalBase size: 2721 */ /* MAX is 65536 */
#define TRIPLE_MAX 4096
#define MASSIMO_FOPEN 64 /* max open files */

uint16 inverso_normale_re[462], inverso_pedone_re[1806];
uint16 Sq2Blocked[780], Sq2Pawn[1128], Sq2Normal[2016];
uint32 Sq3blocked[9880], Sq3pedone[17296], Sq3normale[41664];
uint32 Sq4blocked[91390],Sq4pedone[194580], Sq4normale[635376];
sint16 RE_normale[4096], RE_pedone[4096];

int STESSO4[64][64][64][64]; /* BUILD */
int STESSO3[64][64][64];
int STESSO2[64][64]; /* uint16 ? */
int TOTAL_VERBOSO;

typedef struct
{
  uint8 pi[4], sq[4], wK, bK;
  boolean wtm, pedone; /* 0xc */
  uint32 n;
  uint64 Blocked; /* 0x18 */
  boolean DTR;
} type_PiSq;

int RB_CPUS;
uint64 LIST_SIZE;
#ifndef WINDOWS
#define USE_SPINLOCKS TRUE /* spinlocks: 4 bytes */
#endif
#ifdef USE_SPINLOCKS
#define ROBBO_LOCK_TYPE pthread_spinlock_t
#define ROBBO_LOCK_IT(x) pthread_spin_lock (x)
#define ROBBO_UNLOCK_IT(x) pthread_spin_unlock (x)
#define ROBBO_LOCK(tb, ind) pthread_spin_lock (&(tb->locks)[ind]);
#define ROBBO_UNLOCK(tb, ind) pthread_spin_unlock (&(tb->locks)[ind]);
#define ROBBO_LOCK_INIT(x) pthread_spin_init (x, 1) /* HACK */
#define ROBBO_LOCK_DESTROY(x) pthread_spin_destroy (x)
#else
#define ROBBO_LOCK_TYPE MUTEX_TYPE
#define ROBBO_LOCK_IT(x) LOCK (x)
#define ROBBO_UNLOCK_IT(x) UNLOCK (x)
#define ROBBO_LOCK(tb, ind) LOCK (&(tb->locks)[ind]);
#define ROBBO_UNLOCK(tb, ind) UNLOCK (&(tb->locks)[ind]);
#define ROBBO_LOCK_INIT(x) LOCK_INIT (x) /* HACK */
#define ROBBO_LOCK_DESTROY(x) LOCK_DESTROY (x)
#endif
MUTEX_TYPE RB_SMP[1];
MUTEX_TYPE FWRITE_LOCK[1];
boolean DISK_SWAP;
typedef struct
{
  uint64 m[4];
  uint8 *data, *MAR, *VIT, *PER;
  uint8 p[4];
  uint64 pro, PawnMult, PawnMult8, supp;
  int num, massimo_rima, shift, num_indici;
  uint32 *indici, BLOCCO_pro, BWT; /* 32-bit indici? */
  boolean PEDONE, simm, MEMORIA, TYPE, is_uno, DTR;
  sint8 efgh1, efgh2, _0;
  uint16 efgh[4];
  uint8 efgh_shift[4], efgh_file[4];
  FILE *Fdata, *Frima, *Fvit, *Fmar, *Fper, *Fsupp;
  int *rima_pro, *vit_pro, *per_pro, *mar_pro;
  uint32 supp_indice[8];
  char DIR_NOME[256];
  char string[16];
  ROBBO_LOCK_TYPE* locks;
}
RobboTotalBase;

typeMoveList* GenCapturesTotal (typePOS*, typeMoveList*);
typeMoveList* WhiteQueenPromotions (typePOS*, typeMoveList*, uint64);
void load_triple (char*, char*);
void GetTripleBase (char*);

#define emarco(A,w) (A[(w) >> 3] & (1 << ((w) & 7)))
#define Set(A,w) (A[(w) >> 3] |= (1 << ((w) & 7)))
#define UnSet(A,w) (A[(w) >> 3] &= ~(1 << ((w) & 7)))

int CountWhite (typePOS *, uint64 *);
void INIT_TOTAL_BASE ();

void DISCO_FREAD1 (uint8*, uint64, uint64, FILE*, uint64);
void DISCO_FREAD2 (uint8*, uint64, uint64, FILE*, uint64);
int DISCO_FWRITE1 (uint8*, uint64, uint64, FILE*);
int DISCO_FWRITE2 (uint8*, uint64, uint64, FILE*);

void un_windows (char*);

/* #define DIV(a,b) ((a)/(b)) */
#define DIV(X,a,b) \
  { if ((b) == 1) X = (a); \
    else if ((b) == 10) X = (a) / 10; \
    else if ((b) == 12) X = (a) / 12; \
    else if ((b) == 24) X = (a) / 24; \
    else if ((b) == 40) X = (a) / 40; \
    else if ((b) == 48) X = (a) / 48; \
    else if ((b) == 64) X = (a) / 64; \
    else if ((b) == 780) X = (a) / 780; \
    else if ((b) == 1128) X = (a) / 1128; \
    else if ((b) == 2016) X = (a) / 2016; \
    else if ((b) == 9880) X = (a) / 9880; \
    else if ((b) == 17296) X = (a) / 17296; \
    else if ((b) == 41664) X = (a) / 41664; \
    else if ((b) == 91390) X = (a) / 91390; \
    else if ((b) == 194580) X = (a) / 194580; \
    else if ((b) == 635376) X = (a) / 635376; }

uint8 lanc_oriz[64], lanc_vert[64], lanc_diag[64];

typedef struct
{
  uint8 sq[4], pi[4], rip[4], rip_ai[4], efgh_shift[4], wK, bK;
  boolean capW[4], capB[4];
  uint64 m[4], re_fetta_molt, Double, ind, Occupied, PEDONE, re_fetta,
    PawnOnFourth;
} tipo_fPOS;

boolean tot_ind_ott (type_PiSq*, int*, uint64*, uint64*, int*);
uint64 fareMARCO (RobboTotalBase*, int, RobboTotalBase*);
uint64 fareRIMA (RobboTotalBase*, int, RobboTotalBase*);
void iniz_vario ();
int compressa (unsigned char*, int, int);
int blocco_decompressa (unsigned char*, unsigned char*, int, int);
void InitTotalBaseCache (uint64);
boolean BITBORDO_PQ (typePOS*, type_PiSq*);

char TRIPLE_DIR[1024];
uint64 TOT_INDICI;

#define wEnumB wEnumBL
#define bEnumB bEnumBL
#define BlockedPawn 8

#define MAX_NUM 4
int nome_canonico (char*, boolean*);
void RobboTotal_sott (char*);

char TOTAL_BASE_DIRECTORY[1024];
char COMPRESSA_PREFISSO[1024];
int RobboTotal_scop (char *);
void GetTotalBase (char *);

#define DISCO_COMPRESSO 0x05
#define DISCO_HUFFMAN 0x03

RobboTotalBase* TABLE_TOTAL_BASES;
int NUM_TOTAL_BASES;
int TotalBaseTable[16][16][16][16];
int col_scamb[16];
#define DISCO_NON 1

#define IN_CHECK \
  (POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt))
#define IS_ILLEGAL \
  (!POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt))

#define MASSIMO_RIMA 125
#define VALU_ROTTO (MASSIMO_RIMA + 1)
#define VALU_INCOG (MASSIMO_RIMA + 2)
#define VALU_VITTORIA (MASSIMO_RIMA + 3)
#define VALUE_DRAW (MASSIMO_RIMA + 4)
#define PRIMERA_PERDITA (MASSIMO_RIMA + 5)
#define PERD_IN(x) (PRIMERA_PERDITA + (x))
#define eVITTORIA(Value) ((Value) == VALU_VITTORIA)
#define ePATTA(Value) ((Value) == VALUE_DRAW)
#define ePERDITA(Value) ((Value) >= PRIMERA_PERDITA)
#define PATTA_PERDITA(Value) ((Value) >= VALUE_DRAW)
#define LARGH (240 - MASSIMO_RIMA)
#define PERD_FINE ((PRIMERA_PERDITA) + (LARGH))

#define dROTT 0x00
#define dVITT 0x01
#define dPATT 0x02
#define dPERD0 (dPATT + 1)
#define DISCO_PATT_PERD(x) ((x) >= dPATT)
#define DISCO_VITTORIA(x) ((x) == dVITT)
#define DISCO_PATTA(x) ((x) == dPATT)
#define DISCO_PERDITA(x) ((x) >= dPERD0)

boolean ott_indice (type_PiSq*, uint64*, uint64*, int*);
boolean RobboTotalBaseScore (typePOS*, int*);
int tot_valu (RobboTotalBase*, uint64);
void re_fetta (RobboTotalBase*, int);
void RobboTotal_add (RobboTotalBase*, char*, char*);
boolean RobboTotal_registro (char*, char*);
void InitTotalBaseUtility (boolean);
void InitTotalBaseCache ();
void INIT_TRIPLE_BASE ();

#define SCAMB(x,y) { x ^= y; y ^= x; x ^= y; }
#define LANCpez(u,x) x = lanc_pedone[u][x]
#define LANCnor(u,x) x = lanc_norm[u][x]

uint8 oLANC[64][64], lanc_norm[8][64];

void SEND (char* fmt, ...);

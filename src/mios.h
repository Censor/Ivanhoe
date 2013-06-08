//#define EVAL_OPEN_END
//#define MOVIL
//#define BUENATAQUE
//#define ATAQUEBUENO
#define SEE_VALUE
//#define EVAL_PINS
//#define EVAL_ROOK_FIX
//#define STATIC256
//#define POTENJUEGO
#ifdef POTENJUEGO

int PowerGambling;
#define ASSERT(a)
#define TIME5
#endif
#ifdef SEE_VALUE
#define QueenMatVal 950
#define RookMatVal 500
#define BishopMatVal 310
#define KnightMatVal 300
#define PawnMatVal 100
#define BishopPairMatVal 45
#endif


#include <string.h>
#define TempoValue (4 + ((POSITION->DYN->material & 0xff) >> 2))
#define TempoValue2 (4 + ((POSITION->DYN->material & 0xff) >> 2))
#define PrunePawn 160
#define PruneMinor 500
#define PruneRook 800
#define PruneCheck 10

#define LazyValue 150
#define LazyValue2 300
#define EVAL(m, dp) Eval (POSITION, -0x7fff0000, 0x7fff0000, m, dp)
#define EvalLazy(B, A, p, m) Eval (POSITION, (B) - (p), (A) + (p), m, depth)
#define EvalQsearch(v, m) EvalLazy ((v), (v), LazyValue, (m))
#define EvalLowDepth(v, m) EvalLazy ((v), (v), LazyValue, (m))
#if 1
#define EvalCutAll(v, m) EvalLazy ((v), (v), LazyValue2, (m))
#else
#define EvalCutAll(v, m) EVAL (m)
#endif

#define MAX_POSITIONAL(m) MAX_POS_GAIN (POSITION->sq[FROM (m)], (m) & 07777)

#define IsRepetition(x) \
  (VALUE > x && POS0->reversible >= 2 && \
   ((TO(move) << 6) | FROM(move)) == (POS0 - 1)->move && \
   POSITION->sq[TO(move)] == 0)

#define IsInterpose(x) (x & (1 << 15))
#define EasySEE(x) (x & 0x300000)

#define PassedPawnPush(to, x) \
  (POSITION->sq[to] == EnumMyP && x && (BitboardOppP & IsPassedPawn[to]) == 0)

#define PIECE_IS_WHITE(pi) (pi <= 7)
#define PIECE_IS_BLACK(pi) (pi >= 8)

#define QSEARCH_CONDITION (new_depth <= 1)
#define LOW_DEPTH_CONDITION (new_depth <= 9)
#define LOW_DEPTH_CONDITION_PV (new_depth <= 9)

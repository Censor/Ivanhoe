
uint32 ROOT_BEST_MOVE;
int ROOT_SCORE, ROOT_PREVIOUS;
int ROOT_DEPTH, PREVIOUS_DEPTH, PREVIOUS_FAST;

#include <setjmp.h>
#include <time.h>
extern jmp_buf J; /* extern ? */
volatile boolean JUMP_IS_SET;
boolean EASY_MOVE, BAD_MOVE, BATTLE_MOVE;
uint64 START_CLOCK, INIT_CLOCK;
boolean ANALYSING, MONTE_CARLO;

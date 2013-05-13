
extern boolean MONTE_CARLO;
volatile boolean MONTE_CARLO_CURTAIL;
#define STRTOK(p) p = strtok (NULL, " ")

#define MAX_MC 64
#define MAX_MC_MOVES 1024
typedef struct
{
  int fixed_depth, length, max, min, cpus, verbose_length;
  boolean verbose; /* null action */
  MUTEX_TYPE SEND_LOCK[1];
  MUTEX_TYPE TABLE_LOCK[1];
  int mv; /* count */
  typeMoveList SEARCH[256];
  typePOS ROOT_POSITION[1];
  typePOS POSITION[MAX_MC][1];
  int WIN[256], LOSS[256], SCORE[256], DO_COUNT[256];
  int MLcnt[MAX_MC];
  uint16 ML[MAX_MC][MAX_MC_MOVES];
} type_Monte_Carlo;

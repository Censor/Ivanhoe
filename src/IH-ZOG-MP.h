
/***************************** ZOG MP *******************************/

typedef struct {uint16 move; sint16 score;} type_ZOG_ML;
struct ZN
{
  typePOS POS[1]; /* STACK has 2048 bytes */
  MUTEX_TYPE lock[1];
  int alpha, beta, depth, orig_alpha, expected_value, best_value;
  struct ZN *PARENT;
  int USERS, AKTIV_MOVES_COUNT;
  int zm, phase;
  int ZOG_MOVE_COUNT, TOTAL_MOVE_COUNT, extension;
  boolean cpus_used[MAX_CPUS];
  struct ZN *children[MAX_CPUS];
  uint32 MOVE_FROM_PARENT, best_move;
  boolean is_busy, dead, EXPANDED, join_able, is_registered, is_book_keep;
  uint64 hash, when;
  int num, previous_result;
  type_ZOG_ML MOVE_LIST[256]; /* 1024 bytes */
}; /* volatile? */

boolean ZOG_AKTIV;
uint64 ZOG_SIZE, ZOG_HEFT; /* int ? */
boolean volatile ZOG_AVAILABLE;
boolean volatile ZOG_AVAILABLE_OTHER;
boolean volatile ZOG_ROOT_DONE;
typedef struct ZN type_zog_node;
#define MIN_ZOG_DEPTH 10

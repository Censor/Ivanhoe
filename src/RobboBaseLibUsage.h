
#define uint8 unsigned char
#define boolean uint8

typedef struct
{
  uint8 pi[4], sq[4], wK, bK;
  boolean wtm, pedone;
  uint8 ep, n, nodes, _2; /* Gaviota comply, ep = 64  */
} Type_PiSq;

/* declare, unto static linkery */

#ifdef STATIC_LINKERY
#define FUNC(x, y) x y
#define FUNC_CALL(x) x
#else
#define FUNC(x, y) x (*y) /* demand, consort */
#define FUNC_CALL(x) (*x)
#endif

/************** Score */

FUNC (boolean, RobboTotalLibScore) (Type_PiSq* PiSq, uint8* Value, int type);
FUNC (boolean, RobboTripleLibScore) (Type_PiSq* PiSq, int* Value, int type);
FUNC (void, GetRobboBaseLibraryVersion) (char * STRING);

/*************** Total set up */

FUNC (int, RegisterTotalBases) (char* DIRECTORY_MASTER);
FUNC (int, DeRegisterTotalBases) (); /* RobboTotal_iniz.c */
FUNC (void, RegisterRobboTotalBasesInDirectory) (char* DIRECTORY);
FUNC (int, SetCacheTotalBase) (int mega_bytes); /* Default 1 */
FUNC (int, HAS_TotalBase) (Type_PiSq*);

/************ Triple set up */

/* Cache and Weak Load */

FUNC (int, SetCacheTripleBase) (int mega_bytes); /* Default 1 */
FUNC (boolean, SetLoadOnWeakProbeBackGround) (boolean type);
FUNC (boolean, AllowBackGroundLoader) ();
FUNC (boolean, PauseBackGroundLoader) ();

/* Load and UnLoad */

FUNC (int, LoadTripleBases) (char* DIRECTORY_MASTER); /* RobboTriple.c */
FUNC (int, UnLoadTripleBases) (); /* RobboTriple.c */
FUNC (void, LoadRobboTripleBasesInDirectory) (char* DIRECTORY);
FUNC (int, HAS_TripleBase) (Type_PiSq*);

/* Bulk */

FUNC (void, RobboLibBulkLoadFile) (char*); /* one file, RobboTriple_iniz.c */
FUNC (void, RobboLibBulkDetachFile) (char*); /* one file, RobboTriple_iniz.c */
FUNC (void, RobboLibBulkLoadDirectory) (char*); /* RobboTriple_iniz.c */
FUNC (void, RobboLibBulkDetachDirectory) (char*); /* RobboTriple_iniz.c */

/*************** start up, exit */

FUNC (void, RobboBaseLibStartUp) (boolean back_ground);
FUNC (void, RobboBaseLibExit) ();

/**** Utility */

FUNC (void, TotalBase_FileNames) (Type_PiSq *, char * BUFFER);
FUNC (void, TripleBase_FileNames) (Type_PiSq *, char * BUFFER);
FUNC (void, FilesInfo_TotalBase) (Type_PiSq *, char * BUFFER);
FUNC (void, FilesInfo_TripleBase) (Type_PiSq *, char * BUFFER);

/** Others **/
FUNC (int, GetMaximumTriplePieces) ();
FUNC (void, ResetTripleCounters) ();
FUNC (void, SendTripleCounters) ();



void NANO_SLEEP (int);
int InitPawnHash (int);
int InitPawnHashWrapper (int);
void InitArrays ();
void InitCaptureValues ();
void InitSlab (int);

void CREATE_MEM (void**, int, uint64, int*, boolean*, char*);
void FREE_MEM (void*, int*, boolean*);

uint64 GET_RAND ();
void HaltSearch (int, int);
void CheckDone (typePOS *, int);
void InitSearch (typePOS *, char *);
void Info (sint64);
void MonteCarlo (typePOS *, char *);

void Eval (typePOS *, int, int, int, int);
void Mobility (typePOS *);
void EvalHashClear ();
int InitEvalHash (int);
int DoHashFull (uint64);

#ifdef CON_ROBBO_BUILD
int TripleEnumerateReset ();
void TripleHashClear ();
int InitTripleHash (int);
int AkinRobboInformatory (int);
int ChangeWeakLoadUsage (int);
int SetLoadOnWeakProbe (boolean);
int LoadRobboDynamicLibrary (int);
int UtilRobboBases (int);
#endif

typeMoveList* CaptureMoves (typePOS *, typeMoveList *, uint64);
typeMoveList* OrdinaryMoves (typePOS *, typeMoveList *);
typeMoveList* EvasionMoves (typePOS *, typeMoveList*, uint64);
void InitCaptureValues ();

typeMoveList* WhiteCaptures (typePOS *, typeMoveList*, uint64);
typeMoveList* BlackCaptures (typePOS *, typeMoveList*, uint64);
typeMoveList* WhiteOrdinary (typePOS *, typeMoveList*);
typeMoveList* BlackOrdinary (typePOS *, typeMoveList*);
void SortOrdinary
(typeMoveList*, typeMoveList*, uint32, uint32, uint32);

typeMoveList* QuietChecksWhite (typePOS *, typeMoveList*, uint64);
typeMoveList* QuietChecksBlack (typePOS *, typeMoveList*, uint64);

typeMoveList* PositionalMovesWhite (typePOS *, typeMoveList*, int);
typeMoveList* PositionalMovesBlack (typePOS *, typeMoveList*, int);
typeMoveList* BlackEvasions (typePOS *, typeMoveList*, uint64);
typeMoveList* WhiteEvasions (typePOS *, typeMoveList*, uint64);

void IncrementAge ();
void HashClear ();
int InitHash (int);

void HashLowerALLNew (uint64, int, int, int, int, int);
void HashUpperCUTNew (uint64, int, int, int, int);
void HashLowerNew (uint64, int, int, int, int, int, int);
void HashUpperNew (uint64, int, int, int, int, int);
void HashExactNew (uint64, int, int, int, int, int, int);
int InitPVHash (int);
int PVHashClear ();

void Input (typePOS *);

void ResetHistory ();
void ResetPositionalGain ();
void ReFuel960Castle ();

void Make (typePOS *, uint32);
void Undo (typePOS *, uint32);
void MakeWhite (typePOS *, uint32);
void UndoWhite (typePOS *, uint32);
void MakeBlack (typePOS *, uint32);
void UndoBlack (typePOS *, uint32);

int InitMaterialValue ();

uint32 NextWhite (typePOS *,typeNEXT*);
uint32 NextBlack (typePOS *,typeNEXT*);

boolean WhiteOK (typePOS *,uint32);
boolean BlackOK (typePOS *,uint32);

void InitPawns ();
void PawnEval (typePOS *, typePawnEval*);

int Move50 (typePOS *);

int PVQsearchWhite (typePOS *,int, int, int);
int PVQsearchWhiteCheck (typePOS *,int, int, int);
int PVQsearchBlack (typePOS *,int, int, int);
int PVQsearchBlackCheck (typePOS *,int, int, int);

void TopWhite (typePOS *);
void TopBlack (typePOS *);
int TopWhiteMC (typePOS *, int);
int TopBlackMC (typePOS *, int);

int RootWhite (typePOS *, int, int, int);
int RootBlack (typePOS *, int, int, int);
int RootWhiteMC (typePOS *, typeRootMoveList *, int, int, int);
int RootBlackMC (typePOS *, typeRootMoveList *, int, int, int);

int PVNodeWhite (typePOS *, int, int, int, int, type_zog_node*);
int PVNodeBlack (typePOS *, int, int, int, int, type_zog_node*);

int ExcludeWhite (typePOS *, int, int, uint32);
int ExcludeWhiteCheck (typePOS *, int, int, uint32);
int ExcludeBlack (typePOS *, int, int, uint32);
int ExcludeBlackCheck (typePOS *, int, int, uint32);

int CutNodeWhite (typePOS *, int, int);
int CutNodeBlack (typePOS *, int, int);
int CutNodeWhiteCheck (typePOS *, int, int);
int CutNodeBlackCheck (typePOS *, int, int);

int AllNodeWhite (typePOS *, int, int);
int AllNodeBlack (typePOS *, int, int);
int AllNodeWhiteCheck (typePOS *, int, int);
int AllNodeBlackCheck (typePOS *, int, int);

int LowDepthWhite (typePOS *, int, int);
int LowDepthBlack (typePOS *, int, int);
int LowDepthWhiteCheck (typePOS *, int, int);
int LowDepthBlackCheck (typePOS *, int, int);

int QsearchWhite (typePOS *, int, int);
int QsearchBlack (typePOS *, int, int);
int QsearchWhiteCheck (typePOS *, int, int);
int QsearchBlackCheck (typePOS *, int, int);

void OutputBestMove ();
void Search (typePOS *);
void Information (typePOS *, sint64, int, int, int);

boolean WhiteSEE (typePOS *, uint32);
boolean BlackSEE (typePOS *, uint32);

char*ReadFEN (typePOS *, char*);
void InitPosition (typePOS *, char*);

void InitStatic ();

uint32 FullMove (typePOS *, uint32);
uint32 numeric_move (typePOS *, char *);
void ERROR_END (char *, ...);
void FEN_ERROR (char *, ...);
void SEND (char *, ...);
char* Notate (uint32, char *);
uint64 GetClock ();
void InitBitboards (typePOS *);
void NewGame (typePOS *, boolean);
boolean TryInput ();

boolean RobboFare (typePOS *);
void InitTotalBaseUtility (boolean);
void total_iniz ();
int SetTotalBaseCache (int);
int SetTripleBaseCache (int);
void InitInitTotalBaseCache (uint64);
void InitInitTripleBaseCache (uint64);

boolean TRIPLE_VALUE (typePOS *, int*, boolean *, boolean, boolean, boolean);
void INIT_TOTAL_BASE ();
void triple_statistica ();

void CleanupTriple ();
/* uci struct attends (int) for parameter ? */
int LoadRobboTripleBases ();
int RegisterRobboTotalBases ();
int UnLoadRobboTripleBases ();
int DeRegisterRobboTotalBases ();
int RobboBulkDirectory ();
int RobboBulkLoad ();
int RobboBulkDirectoryDetach ();
int RobboBulkDetach ();

boolean IVAN_SPLIT (typePOS *, typeNEXT*, int, int, int, int, int*);
void ivan_fail_high (SPLITPUNKT*, typePOS *, uint32);
void STUB_IVAN ();
void thread_halt (typePOS *);
void thread_stall (typePOS *, int);

void WhitePVNodeSMP (typePOS *);
void BlackPVNodeSMP (typePOS *);
void WhiteAllSMP (typePOS *);
void BlackAllSMP (typePOS *);
void WhiteCutSMP (typePOS *);
void BlackCutSMP (typePOS *);
int ivan_init_smp ();
void ivan_end_smp ();
void rp_init ();
int PawnHashReset ();
void ponderhit ();

uint64 PERFD (typePOS *, int);
void Perfd (typePOS *, int, uint64);
void validate (typePOS *, int, uint32);

void WhiteTopAnalysis (typePOS *);
void BlackTopAnalysis (typePOS *);
int WhiteAnalysis (typePOS *, int, int, int);
int BlackAnalysis (typePOS *, int, int, int);

int WhiteMultiPV (typePOS *, int);
int BlackMultiPV (typePOS *, int);
boolean SubsumeTripleSMP ();

#ifdef BENCHMARK
void BenchMark (typePOS *, char*);
void MakeUndoSpeed (typePOS *);
#endif
#ifdef UTILITIES
void BenchMark (typePOS *, char*);
void DrawBoard (typePOS *);
void EvalExplanation (typePOS *, int, int, int);
void PawnEvalExplanation (typePOS *,typePawnEval*);
void MaterialValueExplain (int);
void VerifyTripleBase (int*);
#endif
#ifndef MINIMAL
char* EmitFen (typePOS *, char*);
#endif

#ifdef BUILD_ZOG_MP_COMPILE
void ZOG_ON ();
void ZOG_OFF ();
void ZOG_init ();
void ZOG_TRAP (int);
void ENTER_ZOG_NODE (type_zog_node*, int, boolean);
int zog_root_node (typePOS *, int, int, int);
#endif

#ifdef CONCORD_REV_MOVES
int RevMoveSetup (int);
#endif
int STAT (char *);



#if 0
#define Bitboard2(x, y) (1ULL << (x))|((1ULL << (y))
#define F1H1 Bitboard2(F1, H1)
#define F1H1Left90 Bitboard2(Left90[F1], Left90[H1])
#define F1H1Left45 Bitboard2(Left45[F1], Left45[H1])
#define F1H1Right45 Bitboard2(Right45[F1], Right45[H1])
#define A1D1 Bitboard2(A1, D1)
#define A1D1Left90 Bitboard2(Left90[A1], Left90[D1])
#define A1D1Left45 Bitboard2(Left45[A1], Left45[D1])
#define A1D1Right45 Bitboard2(Right45[A1], Right45[D1])
#define F8H8 Bitboard2(F8, H8)
#define F8H8Left90 Bitboard2(Left90[F8], Left90[H8])
#define F8H8Left45 Bitboard2(Left45[F8], Left45[H8])
#define F8H8Right45 Bitboard2(Right45[F8], Right45[H8])
#define A8D8 Bitboard2(A8, D8)
#define A8D8Left90 Bitboard2(Left90[A8], Left90[D8])
#define A8D8Left45 Bitboard2(Left45[A8], Left45[D8])
#define A8D8Right45 Bitboard2(Right45[A8], Right45[D8])
#else
#define Bitboard2(x, y) (1ULL << (x))|(1ULL << (y))
#define F1H1 Bitboard2 (F1, H1)
#define F1H1Left90 Bitboard2 (47, 63)
#define F1H1Left45 Bitboard2 (20, 35)
#define F1H1Right45 Bitboard2 (3, 0)
#define A1D1 Bitboard2 (A1, D1)
#define A1D1Left90 Bitboard2 (7, 31)
#define A1D1Left45 Bitboard2 (0, 9)
#define A1D1Right45 Bitboard2 (28, 10)
#define F8H8 Bitboard2 (F8, H8)
#define F8H8Left90 Bitboard2 (40, 56)
#define F8H8Left45 Bitboard2 (58, 63)
#define F8H8Right45 Bitboard2 (48, 35)
#define A8D8 Bitboard2 (A8, D8)
#define A8D8Left90 Bitboard2 (0, 24)
#define A8D8Left45 Bitboard2 (28, 49)
#define A8D8Right45 Bitboard2 (63, 57)
#endif

#ifdef MAGIC_BITBOARDS
#define ClearOccupied(M, x) { POSITION->OccupiedBW &= M; }
#define SetOccupied(M, x) { POSITION->OccupiedBW |= M; }
#else
#define ClearOccupied(M, x) \
  { POSITION->OccupiedBW &= M; \
    POSITION->OccupiedL90 &= ClearL90[x]; \
    POSITION->OccupiedL45 &= ClearL45[x]; \
    POSITION->OccupiedR45 &= ClearR45[x]; }
#define SetOccupied(M, x) \
  { POSITION->OccupiedBW |= M; POSITION->OccupiedL90 |= SetL90[x]; \
    POSITION->OccupiedL45 |= SetL45[x]; POSITION->OccupiedR45 |= SetR45[x]; }
#endif

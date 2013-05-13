#define MyPVNodeSMP BlackPVNodeSMP
#define OppPVNodeSMP WhitePVNodeSMP
#define MyAllSMP BlackAllSMP
#define OppAllSMP WhiteAllSMP
#define MyCutSMP BlackCutSMP
#define OppCutSMP WhiteCutSMP

#define MyMultiPV BlackMultiPV

#define MyPVQsearch PVQsearchBlack
#define MyPVQsearchCheck PVQsearchBlackCheck
#define MyQsearchCheck QsearchBlackCheck
#define MyQsearch QsearchBlack
#define MyLowDepth LowDepthBlack
#define MyLowDepthCheck LowDepthBlackCheck
#define MyExclude ExcludeBlack
#define MyExcludeCheck ExcludeBlackCheck
#define MyCut CutNodeBlack
#define MyCutCheck CutNodeBlackCheck
#define MyAll AllNodeBlack
#define MyAllCheck AllNodeBlackCheck
#define MyPV PVNodeBlack
#define MyRootNode RootBlack
#define MyRootNodeMonteCarlo RootBlackMC
#define MyAnalysis BlackAnalysis
#define MyTop TopBlack
#define MyTopNodeMonteCarlo TopBlackMC
#define MyTopAnalysis BlackTopAnalysis
#define MyAttacked POSITION->DYN->bAtt
#define OppAttacked POSITION->DYN->wAtt
#define MyAttackedPawns AttPb
#define OppAttackedPawns AttPw
#define OppPVQsearch PVQsearchWhite
#define OppPVQsearchCheck PVQsearchWhiteCheck
#define OppQsearch QsearchWhite
#define OppQsearchCheck QsearchWhiteCheck
#define OppLowDepth LowDepthWhite
#define OppLowDepthCheck LowDepthWhiteCheck
#define OppExclude ExcludeWhite
#define OppExcludeCheck ExcludeWhiteCheck
#define OppCut CutNodeWhite
#define OppCutCheck CutNodeWhiteCheck
#define OppAll AllNodeWhite
#define OppAllCheck AllNodeWhiteCheck
#define OppPV PVNodeWhite
#define OppRootNode RootWhite
#define OppAnalysis WhiteAnalysis
#define OppTop TopWhite
#define OppTopAnalysis WhiteTopAnalysis
#define OppOccupied wBitboardOcc
#define BitboardOppP wBitboardP
#define BitboardOppN wBitboardN
#define BitboardOppBL wBitboardBL
#define BitboardOppBD wBitboardBD
#define BitboardOppB (BitboardOppBL | BitboardOppBD)
#define BitboardOppR wBitboardR
#define BitboardOppQ wBitboardQ
#define BitboardOppK wBitboardK
#define MyOccupied bBitboardOcc
#define BitboardMyP bBitboardP
#define BitboardMyN bBitboardN
#define BitboardMyBL bBitboardBL
#define BitboardMyBD bBitboardBD
#define BitboardMyB (BitboardMyBL | BitboardMyBD)
#define BitboardMyR bBitboardR
#define BitboardMyQ bBitboardQ
#define BitboardMyK bBitboardK
#define MyKingSq POSITION->bKsq
#define OppKingSq POSITION->wKsq
#define EnumOppP wEnumP
#define EnumOppN wEnumN
#define EnumOppBL wEnumBL
#define EnumOppBD wEnumBD
#define EnumOppR wEnumR
#define EnumOppQ wEnumQ
#define EnumOppK wEnumK
#define EnumMyP bEnumP
#define EnumMyN bEnumN
#define EnumMyBL bEnumBL
#define EnumMyBD bEnumBD
#define EnumMyR bEnumR
#define EnumMyQ bEnumQ
#define EnumMyK bEnumK
#define OppKingCheck POSITION->DYN->wKcheck
#define MyCapture BlackCaptures
#define MyOrdinary BlackOrdinary
#define MyPositionalGain PositionalMovesBlack
#define MyEvasion BlackEvasions
#define MyQuietChecks QuietChecksBlack
#define MySEE BlackSEE
#define MAKE MakeBlack
#define UNDO UndoBlack
#define ILLEGAL_MOVE MOVE_IS_CHECK_BLACK
#define MOVE_IS_CHECK MOVE_IS_CHECK_WHITE
#define MyOK BlackOK
#define MyNext NextBlack
#define MyNull ((POSITION->DYN->flags) & 1)
#define HasPiece (bBitboardOcc ^ bBitboardK ^ bBitboardP)
#define SIXTH_RANK(x) ((x) <= H3)
#define FOURTH_RANK(x) ((x) <= H5)
#define IsPassedPawn PassedPawnB
#define MyXRAY (POSITION->DYN->bXray)
#define OppXRAY (POSITION->DYN->wXray)
#define MyXrayTable POSITION->XRAYb
#define OppXrayTable POSITION->XRAYw
#define MyKingCheck POSITION->DYN->bKcheck
#define MY_COUNT CountBlack
#define MY_COUNT_FLEE conto_nero_fuggio
#define IN_CHECK BLACK_IN_CHECK
#define OppKdanger wKdanger

#define EIGHTH_RANK(x) (x <= H1)
#define CAPTURE_RIGHT ((bBitboardP & ~FILEh) >> 7)
#define FROM_LEFT(x) (((x) + 7) << 6)
#define CAPTURE_LEFT ((bBitboardP & ~FILEa) >> 9)
#define FROM_RIGHT(x) (((x) + 9) << 6)
#define FORWARD(x) ((x) - 8)
#define FORWARD_LEFT(x) ((x) - 9)
#define FORWARD_RIGHT(x) ((x) - 7)
#define BACKWARD(x) ((x) + 8)
#define FORWARD_SHIFT(x) ((x) >> 8)
#define BACK_SHIFT(x) ((x) << 8)
#define SEVENTH_RANK(x) (x <= H2)
#define BACK_SHIFT2(x) ((x) << 16)
#define FORWARD2(x) ((x) - 16)
#define SECOND_RANK RANK7
#define ON_THIRD_RANK(x) ((x & 070) == 050)
#define FOURTH_RANK_BITBOARD RANK5
#define SECOND_SIXTH_RANKS Ranks3to7
#define BITBOARD_SEVENTH_RANK RANK2
#define BITBOARD_EIGHTH_RANK RANK1
#define WHITE_A7 A2
#define WHITE_H7 H2

#define CaptureEP CAPTURE_VALUE[bEnumP][wEnumP]
#define PromQueenCap ((0x28 << 24) + CAPTURE_VALUE[bEnumP][c])
#define PromKnightCap ((0x1a << 24) + CAPTURE_VALUE[bEnumP][c])

#define CastleOO BlackOO
#define CastleOOO BlackOOO
#define WHITE_F1G1 F8G8
#define WHITE_C1D1 C8D8
#define WHITE_B1C1D1 B8C8D8
#define WHITE_E1 E8
#define WHITE_C1 C8
#define WHITE_G1 G8
#define UnderProm UnderPromBlack

#define ON_SECOND_RANK(x) ((x & 070) == 060)
#define CAN_CAPTURE_RIGHT (((SqSet[fr] & NOTh) >> 7) & wBitboardOcc & mask)
#define CAN_CAPTURE_LEFT (((SqSet[fr] & NOTa) >> 9) & wBitboardOcc & mask)
#define PAWN_GUARD(x,y) \
 (POSITION->DYN->bAtt & SqSet[x] || AttFile((y)) & (bBitboardR | bBitboardQ))
#define FOURTH_EIGHTH_RANK_NOh (0x0000007f7f7f7f7f)
#define FOURTH_EIGHTH_RANK_NOa (0x000000fefefefefe)
#define BACK_RIGHT(x) ((x) + 9)
#define BACK_RIGHT2(x) ((x) + 17)
#define BACK_RIGHT3(x) ((x) + 25)
#define BACK_LEFT(x) ((x) + 7)
#define BACK_LEFT2(x) ((x) + 15)
#define BACK_LEFT3(x) ((x) + 23)
#define NUMBER_RANK5 R4

#define NUMBER_RANK1 R8
#define NUMBER_RANK2 R7
#define NUMBER_RANK7 R2
#define PIECE_IS_MINE PIECE_IS_BLACK
#define PIECE_IS_OPP PIECE_IS_WHITE
#define BACKWARD2(x) ((x) + 16)

#define PIECE_IS_OPP_PAWN(x) ((x == wEnumP) ? 0xffffffffffffffff : 0)

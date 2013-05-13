#include "undef.h"
#define MyPVNodeSMP WhitePVNodeSMP
#define OppPVNodeSMP BlackPVNodeSMP
#define MyAllSMP WhiteAllSMP
#define OppAllSMP BlackAllSMP
#define MyCutSMP WhiteCutSMP
#define OppCutSMP BlackCutSMP

#define MyMultiPV WhiteMultiPV

#define MyPVQsearch PVQsearchWhite
#define MyPVQsearchCheck PVQsearchWhiteCheck
#define MyQsearch QsearchWhite
#define MyQsearchCheck QsearchWhiteCheck
#define MyLowDepth LowDepthWhite
#define MyLowDepthCheck LowDepthWhiteCheck
#define MyExclude ExcludeWhite
#define MyExcludeCheck ExcludeWhiteCheck
#define MyCut CutNodeWhite
#define MyCutCheck CutNodeWhiteCheck
#define MyAll AllNodeWhite
#define MyAllCheck AllNodeWhiteCheck
#define MyPV PVNodeWhite
#define MyRootNode RootWhite
#define MyRootNodeMonteCarlo RootWhiteMC
#define MyAnalysis WhiteAnalysis
#define MyTop TopWhite
#define MyTopNodeMonteCarlo TopWhiteMC
#define MyTopAnalysis WhiteTopAnalysis
#define MyAttacked POSITION->DYN->wAtt
#define OppAttacked POSITION->DYN->bAtt
#define MyAttackedPawns AttPw
#define OppAttackedPawns AttPb
#define OppPVQsearch PVQsearchBlack
#define OppPVQsearchCheck PVQsearchBlackCheck
#define OppQsearch QsearchBlack
#define OppQsearchCheck QsearchBlackCheck
#define OppLowDepth LowDepthBlack
#define OppLowDepthCheck LowDepthBlackCheck
#define OppExclude ExcludeBlack
#define OppExcludeCheck ExcludeBlackCheck
#define OppCut CutNodeBlack
#define OppCutCheck CutNodeBlackCheck
#define OppAll AllNodeBlack
#define OppAllCheck AllNodeBlackCheck
#define OppPV PVNodeBlack
#define OppRootNode RootBlack
#define OppAnalysis BlackAnalysis
#define OppTop TopBlack
#define OppTopAnalysis BlackTopAnalysis
#define OppOccupied bBitboardOcc
#define BitboardOppP bBitboardP
#define BitboardOppN bBitboardN
#define BitboardOppBL bBitboardBL
#define BitboardOppBD bBitboardBD
#define BitboardOppB (BitboardOppBL | BitboardOppBD)
#define BitboardOppR bBitboardR
#define BitboardOppQ bBitboardQ
#define BitboardOppK bBitboardK
#define MyOccupied wBitboardOcc
#define BitboardMyP wBitboardP
#define BitboardMyN wBitboardN
#define BitboardMyBL wBitboardBL
#define BitboardMyBD wBitboardBD
#define BitboardMyB (BitboardMyBL | BitboardMyBD)
#define BitboardMyR wBitboardR
#define BitboardMyQ wBitboardQ
#define BitboardMyK wBitboardK
#define MyKingSq POSITION->wKsq
#define OppKingSq POSITION->bKsq
#define EnumOppP bEnumP
#define EnumOppN bEnumN
#define EnumOppBL bEnumBL
#define EnumOppBD bEnumBD
#define EnumOppR bEnumR
#define EnumOppQ bEnumQ
#define EnumOppK bEnumK
#define EnumMyP wEnumP
#define EnumMyN wEnumN
#define EnumMyBL wEnumBL
#define EnumMyBD wEnumBD
#define EnumMyR wEnumR
#define EnumMyQ wEnumQ
#define EnumMyK wEnumK
#define OppKingCheck POSITION->DYN->bKcheck
#define MyCapture WhiteCaptures
#define MyOrdinary WhiteOrdinary
#define MyPositionalGain PositionalMovesWhite
#define MyEvasion WhiteEvasions
#define MyQuietChecks QuietChecksWhite
#define MySEE WhiteSEE
#define MAKE MakeWhite
#define UNDO UndoWhite
#define ILLEGAL_MOVE MOVE_IS_CHECK_WHITE
#define MOVE_IS_CHECK MOVE_IS_CHECK_BLACK
#define MyOK WhiteOK
#define MyNext NextWhite
#define MyNull ((POSITION->DYN->flags) & 2)
#define HasPiece (wBitboardOcc ^ wBitboardK ^ wBitboardP)
#define FOURTH_RANK(x) ((x) >= A4)
#define SIXTH_RANK(x) ((x) >= A6)
#define FOURTH_RANK_BITBOARD RANK4
#define IsPassedPawn PassedPawnW
#define MyXRAY (POSITION->DYN->wXray)
#define OppXRAY (POSITION->DYN->bXray)
#define MyXrayTable POSITION->XRAYw
#define OppXrayTable POSITION->XRAYb
#define MyKingCheck POSITION->DYN->wKcheck
#define MY_COUNT CountWhite
#define MY_COUNT_FLEE conto_bianco_fuggio
#define IN_CHECK WHITE_IN_CHECK
#define OppKdanger bKdanger

#define EIGHTH_RANK(x) (x >= A8)
#define CAPTURE_RIGHT ((wBitboardP & ~FILEh) << 9)
#define FROM_LEFT(x) (((x) - 9) << 6)
#define CAPTURE_LEFT ((wBitboardP & ~FILEa) << 7)
#define FROM_RIGHT(x) (((x) - 7) << 6)
#define FORWARD(x) ((x) + 8)
#define FORWARD_LEFT(x) ((x) + 7)
#define FORWARD_RIGHT(x) ((x) + 9)
#define BACKWARD(x) ((x) - 8)
#define FORWARD_SHIFT(x) ((x) << 8)
#define BACK_SHIFT(x) ((x) >> 8)
#define SEVENTH_RANK(x) (x >= A7)
#define BACK_SHIFT2(x) ((x) >> 16)
#define FORWARD2(x) ((x) + 16)
#define SECOND_RANK RANK2
#define ON_THIRD_RANK(x) ((x & 070) == 020)
#define SECOND_SIXTH_RANKS Ranks2to6
#define BITBOARD_SEVENTH_RANK RANK7
#define BITBOARD_EIGHTH_RANK RANK8
#define WHITE_A7 A7
#define WHITE_H7 H7

#define CaptureEP CAPTURE_VALUE[wEnumP][bEnumP]
#define PromQueenCap ((0x28 << 24) + CAPTURE_VALUE[wEnumP][c])
#define PromKnightCap ((0x1a << 24) + CAPTURE_VALUE[wEnumP][c])

#define CastleOO WhiteOO
#define CastleOOO WhiteOOO
#define WHITE_F1G1 F1G1
#define WHITE_C1D1 C1D1
#define WHITE_B1C1D1 B1C1D1
#define WHITE_E1 E1
#define WHITE_C1 C1
#define WHITE_G1 G1
#define UnderProm UnderPromWhite

#define ON_SECOND_RANK(x) ((x & 070) == 010)
#define CAN_CAPTURE_RIGHT (((SqSet[fr] & NOTh) << 9) & bBitboardOcc & mask)
#define CAN_CAPTURE_LEFT (((SqSet[fr] & NOTa) << 7) & bBitboardOcc & mask)
#define PAWN_GUARD(x,y) \
 (POSITION->DYN->wAtt & SqSet[x] || AttFile((y)) & (wBitboardR|wBitboardQ))
#define FOURTH_EIGHTH_RANK_NOh (0x7f7f7f7f7f000000)
#define FOURTH_EIGHTH_RANK_NOa (0xfefefefefe000000)
#define BACK_RIGHT(x) ((x) - 7)
#define BACK_RIGHT2(x) ((x) - 15)
#define BACK_RIGHT3(x) ((x) - 23)
#define BACK_LEFT(x) ((x) - 9)
#define BACK_LEFT2(x) ((x) - 17)
#define BACK_LEFT3(x) ((x) - 25)
#define NUMBER_RANK5 R5

#define NUMBER_RANK1 R1
#define NUMBER_RANK2 R2
#define NUMBER_RANK7 R7
#define PIECE_IS_MINE PIECE_IS_WHITE
#define PIECE_IS_OPP PIECE_IS_BLACK
#define BACKWARD2(x) ((x) - 16)

#define PIECE_IS_OPP_PAWN(x) ((x == bEnumP) ? 0xffffffffffffffff : 0)

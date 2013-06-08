
#ifdef DEBUG
#include "RobboLito.h"
#define TWEAK (0x74d3c012a8bf965e)
#include <signal.h>
#include "material_value.h"
void validate (typePOS* POSITION, int x, uint32 move)
{
  int sq, pez;
  boolean OK = TRUE;
  uint32 STATICO = 0; /* en mejor sint32 ? */
  uint32 MATERIAL = 0;
  uint64 zZOBRIST = 0, ZOBRIST_PEDONE = 0;
  uint64 WHITE_OCCUPIED = 0, BLACK_OCCUPIED = 0, OCCUPIED;
  uint64 bitboard[16];
  int wKsq, bKsq;
  uint64 LEFT_45 = 0;
  uint64 RIGHT_45 = 0;
  uint64 LEFT_90 = 0;
  int b;
  for (pez = 0; pez < 16; pez++)
    bitboard[pez] = 0;
  for (sq = A1; sq <= H8; sq++)
    if ((pez = POSITION->sq[sq]))
      {
	BitSet (sq, bitboard[pez]);
	STATICO += PST (pez, sq);
	zZOBRIST ^= Zobrist (pez, sq);
	if (pez == wEnumP || pez == bEnumP)
	  ZOBRIST_PEDONE ^= Zobrist (pez, sq);
	if (pez == wEnumK)
	  wKsq = sq;
	if (pez == bEnumK)
	  bKsq = sq;
	MATERIAL += MATERIAL_VALUE[pez];
      }
  if ((MATERIAL & 0x7fffffff) != (POSITION->DYN->material & 0x7fffffff))
    {
      OK = FALSE;
      printf ("Material: POSIZIONE %x board: %x\n", POSITION->DYN->material, MATERIAL);
    }
  if (STATICO != POSITION->DYN->STATIC)
    {
      OK = FALSE;
      printf ("Statico: POSIZIONE %x board: %x\n", POSITION->DYN->STATIC, STATICO);
    }
    if (POPCNT (bitboard[wEnumK]) != 1 || BSF (bitboard[wEnumK]) != wKsq ||
	POSITION->wKsq != wKsq)
    {
      OK = FALSE;
      printf ("White King: POSIZIONE: %llx %d board: %llx %d\n",
	      POSITION->bitboard[wEnumK],
	      POSITION->wKsq,
	      bitboard[wEnumK],
	      wKsq);
    }
    if (POPCNT (bitboard[bEnumK]) != 1 || BSF (bitboard[bEnumK]) != bKsq ||
	POSITION->bKsq != bKsq)
    {
      OK = FALSE;
      printf ("Black King: POSIZIONE: %llx %d board: %llx %d\n",
	      POSITION->bitboard[bEnumK],
	      POSITION->bKsq,
	      bitboard[bEnumK],
	      bKsq);
    }
    
  ZOBRIST_PEDONE ^= ZobristCastling[POSITION->DYN->oo] ^ TWEAK;
  ZOBRIST_PEDONE ^=
    Zobrist (wEnumK, POSITION->wKsq) ^ Zobrist (bEnumK, POSITION->bKsq);
  if (ZOBRIST_PEDONE != POSITION->DYN->PAWN_HASH)
    {
      OK = FALSE;
      printf ("Zobrist Pedone: POSIZIONE: %llx board: %llx\n",
	      POSITION->DYN->PAWN_HASH, ZOBRIST_PEDONE);      
    }
  for (pez = 0; pez < 8; pez ++)
    WHITE_OCCUPIED |= bitboard[pez];
  for (pez = 8; pez < 0x10; pez ++)
    BLACK_OCCUPIED |= bitboard[pez];
  if (WHITE_OCCUPIED != wBitboardOcc)
    {
      OK = FALSE;
      printf ("White Occupied: POSIZIONE: %llx board: %llx\n",
	      wBitboardOcc, WHITE_OCCUPIED);
    }
  if (BLACK_OCCUPIED != bBitboardOcc)
    {
      OK = FALSE;
      printf ("Black Occupied: POSIZIONE: %llx board: %llx\n",
	      bBitboardOcc, BLACK_OCCUPIED);
    }
  if (bitboard[wEnumP] != wBitboardP)
    {
      OK = FALSE;
      printf ("White Pawns: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumP], wBitboardP);
    }
  if (bitboard[wEnumN] != wBitboardN)
    {
      OK = FALSE;
      printf ("White Knights: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumN], wBitboardN);
    }
  if (bitboard[wEnumBL] != wBitboardBL)
    {
      OK = FALSE;
      printf ("White Light Bishops: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumBL], wBitboardBL);
    }
  if (bitboard[wEnumBD] != wBitboardBD)
    {
      OK = FALSE;
      printf ("White Dark Bishops: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumBD], wBitboardBD);
    }
  if (bitboard[wEnumR] != wBitboardR)
    {
      OK = FALSE;
      printf ("White Rooks: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumR], wBitboardR);
    }
  if (bitboard[wEnumQ] != wBitboardQ)
    {
      OK = FALSE;
      printf ("White Queens: POSIZIONE: %llx board: %llx\n",
	      bitboard[wEnumQ], wBitboardQ);
    }
  if (bitboard[bEnumP] != bBitboardP)
    {
      OK = FALSE;
      printf ("Black Pawns: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumP], bBitboardP);
    }
  if (bitboard[bEnumN] != bBitboardN)
    {
      OK = FALSE;
      printf ("Black Knights: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumN], bBitboardN);
    }
  if (bitboard[bEnumBL] != bBitboardBL)
    {
      OK = FALSE;
      printf ("Black Light Bishops: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumBL], bBitboardBL);
    }
  if (bitboard[bEnumBD] != bBitboardBD)
    {
      OK = FALSE;
      printf ("Black Dark Bishops: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumBD], bBitboardBD);
    }
  if (bitboard[bEnumR] != bBitboardR)
    {
      OK = FALSE;
      printf ("Black Rooks: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumR], bBitboardR);
    }
  if (bitboard[bEnumQ] != bBitboardQ)
    {
      OK = FALSE;
      printf ("Black Queens: POSIZIONE: %llx board: %llx\n",
	      bitboard[bEnumQ], bBitboardQ);
    }
  OCCUPIED = WHITE_OCCUPIED | BLACK_OCCUPIED;
  if (OCCUPIED != POSITION->OccupiedBW)
    {
      OK = FALSE;
      printf ("Occupied: POSIZIONE: %llx board: %llx\n",
	      POSITION->OccupiedBW, OCCUPIED);
    }
#ifndef MAGIC_BITBOARDS
  while (OCCUPIED)
    {
      b = BSF (OCCUPIED); BitClear (b, OCCUPIED);
      BitSet (Left45[b], LEFT_45);
      BitSet (Left90[b], LEFT_90);
      BitSet (Right45[b], RIGHT_45);
    }
  if (LEFT_45 != POSITION->OccupiedL45)
    {
      OK = FALSE;
      printf ("Left 45: POSIZIONE: %llx board: %llx\n",
	      POSITION->OccupiedL45, LEFT_45);
    }
  if (LEFT_90 != POSITION->OccupiedL90)
    {
      OK = FALSE;
      printf ("Left 90: POSIZIONE: %llx board: %llx\n",
	      POSITION->OccupiedL90, LEFT_90);
    }
  if (RIGHT_45 != POSITION->OccupiedR45)
    {
      OK = FALSE;
      printf ("Right 45: POSIZIONE: %llx board: %llx\n",
	      POSITION->OccupiedR45, RIGHT_45);
    }
#endif
  zZOBRIST ^= ZobristCastling[POSITION->DYN->oo];
  if (POSITION->DYN->ep)
    zZOBRIST ^= ZobristEP[POSITION->DYN->ep & 7];
  if (POSITION->wtm)
    zZOBRIST ^= ZobristWTM;
  zZOBRIST ^= ZOB_REV (POSITION);
  if (zZOBRIST != POSITION->DYN->HASH)
    {
      OK = FALSE;
      printf ("Zobrist: POSIZIONE: %llx board: %llx\n",
	      POSITION->DYN->HASH, zZOBRIST);
    }
  if (!OK)
    {
      printf ("x: %s mossa: %s\n", x ? "Undo" : "Make",
	      Notate (move, STRING1[POSITION->cpu]));
      raise (SIGTRAP);
    }
}
#endif

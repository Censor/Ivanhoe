
#include "RobboLito.h"

static uint8* SLAB_ROOT_LOC = NULL;
static uint8* CURRENT_SLAB_LOC;
static sint64 CURRENT_SLAB_SIZE = 0;

#define FREE_MEM_NULL(x, y, z) { FREE_MEM (x, y, z); x = NULL; }
void* FromSlab (uint64 sz)
{
  void* A;
  uint64 pageset = ((uint64) (CURRENT_SLAB_LOC)) & 0xfff;
  if (sz >= 0x1000 && pageset & 0xfff) /* align 0x1000 4k page */
    CURRENT_SLAB_LOC += (0x1000 - pageset);
  A = CURRENT_SLAB_LOC;
  if (sz & 0x3f)
    sz = sz + (0x40 - (sz & 0x3f)); /* align 0x40 cache */
  CURRENT_SLAB_LOC += sz;
  if (CURRENT_SLAB_LOC - SLAB_ROOT_LOC > CURRENT_SLAB_SIZE)
    {
      CURRENT_SLAB_LOC -= sz;
      SEND ("FromSlab failed: CURRENT_SLAB_LOC=%d, SLAB_ROOT_LOC=%d, CURRENT_SLAB_SIZE=%d, sz=%d \n", (int)CURRENT_SLAB_LOC,(int)SLAB_ROOT_LOC,(int)CURRENT_SLAB_SIZE,(int) (sz));
      return NULL;
    }
  return A;
}

static int SLAB_NUMBER = -1;
static boolean LARGE_SLAB = FALSE;
void DeleteSlab ()
{
  FREE_MEM_NULL (SLAB_ROOT_LOC, &SLAB_NUMBER, &LARGE_SLAB);
}

void FillSlab ();
void InitSlab (int mb)
{
#ifdef SLAB_MEMORY
  if (SLAB_ROOT_LOC)
    DeleteSlab ();
  CURRENT_SLAB_SIZE = mb * 0x1000000; // Yuri Censor added a 0 here, 6/7/2013
  CREATE_MEM
    (&SLAB_ROOT_LOC, 64, CURRENT_SLAB_SIZE, &SLAB_NUMBER, &LARGE_SLAB, "SLAB");
  CURRENT_SLAB_LOC = SLAB_ROOT_LOC + (0x40 - ((uint64) (SLAB_ROOT_LOC) & 0x3f));
#endif
  FillSlab ();
}

/****/ /****/  /****/   /****/    /****/    /****/   /****/  /****/ /****/

void FillSlab ()
{
  SLAB_ALLOC1 (uint64, AttN, 64);
  SLAB_ALLOC1 (uint64, AttK, 64);
  SLAB_ALLOC1 (uint64, AttPw, 64);
  SLAB_ALLOC1 (uint64, AttPb, 64);
#ifndef MAGIC_BITBOARDS
  SLAB_ALLOC3 (uint64, LineMask, 4 * 0100 * 0100);
  SLAB_ALLOC3 (int, LineShift, 4 * 0100);
  SLAB_ALLOC1 (uint64, ClearL90, 64);
  SLAB_ALLOC1 (uint64, ClearL45, 64);
  SLAB_ALLOC1 (uint64, ClearR45, 64);
  SLAB_ALLOC1 (uint64, SetL90, 64);
  SLAB_ALLOC1 (uint64, SetL45, 64);
  SLAB_ALLOC1 (uint64, SetR45, 64);
#else
  SLAB_ALLOC1 (type_MM, ROOK_MM, 64);
  SLAB_ALLOC1 (type_MM, BISHOP_MM, 64);
  SLAB_ALLOC2 (uint64, MM_ORTHO, 102400); /* SLAB 800k */
  SLAB_ALLOC2 (uint64, MM_DIAG, 5248);
#endif
  SLAB_ALLOC1 (uint64, SqSet, 64);
  SLAB_ALLOC1 (uint64, SqClear, 64);
  SLAB_ALLOC1 (uint64, NON_DIAG, 64);
  SLAB_ALLOC1 (uint64, NON_ORTHO, 64);
  SLAB_ALLOC1 (uint64, ORTHO, 64);
  SLAB_ALLOC1 (uint64, DIAG, 64);
  SLAB_ALLOC1 (uint64, ORTHO_DIAG, 64);
  SLAB_ALLOC1 (uint64, OpenFileW, 64);
  SLAB_ALLOC1 (uint64, OpenFileB, 64);
  SLAB_ALLOC1 (uint64, PassedPawnW, 64);
  SLAB_ALLOC1 (uint64, PassedPawnB, 64);
  SLAB_ALLOC1 (uint64, ProtectedPawnW, 64);
  SLAB_ALLOC1 (uint64, ProtectedPawnB, 64);
  SLAB_ALLOC1 (uint64, IsolatedPawnW, 64);
  SLAB_ALLOC1 (uint64, IsolatedPawnB, 64);
  SLAB_ALLOC1 (uint64, ConnectedPawns, 64);
  SLAB_ALLOC1 (uint64, InFrontW, 8);
  SLAB_ALLOC1 (uint64, NotInFrontW, 8);
  SLAB_ALLOC1 (uint64, InFrontB, 8);
  SLAB_ALLOC1 (uint64, NotInFrontB, 8);
  SLAB_ALLOC1 (uint64, IsolatedFiles, 8);
  SLAB_ALLOC1 (uint64, FilesLeft, 8);
  SLAB_ALLOC1 (uint64, FilesRight, 8);
  SLAB_ALLOC1 (uint64, DOUBLED, 64);
  SLAB_ALLOC1 (uint64, LEFT2, 64);
  SLAB_ALLOC1 (uint64, RIGHT2, 64);
  SLAB_ALLOC1 (uint64, LEFT1, 64);
  SLAB_ALLOC1 (uint64, RIGHT1, 64);
  SLAB_ALLOC1 (uint64, ADJACENT, 64);
  SLAB_ALLOC1 (uint64, LONG_DIAG, 64);
  SLAB_ALLOC1 (uint64, NORTHWEST, 64);
  SLAB_ALLOC1 (uint64, SOUTHWEST, 64);
  SLAB_ALLOC1 (uint64, NORTHEAST, 64);
  SLAB_ALLOC1 (uint64, SOUTHEAST, 64);
  SLAB_ALLOC1 (uint64, QuadrantWKwtm, 64);
  SLAB_ALLOC1 (uint64, QuadrantBKwtm, 64);
  SLAB_ALLOC1 (uint64, QuadrantWKbtm, 64);
  SLAB_ALLOC1 (uint64, QuadrantBKbtm, 64);
  SLAB_ALLOC1 (uint64, ShepherdWK, 64);
  SLAB_ALLOC1 (uint64, ShepherdBK, 64);
  SLAB_ALLOC3 (uint64, INTERPOSE, 0100 * 0100);
  SLAB_ALLOC3 (uint64, EVADE, 0100 * 0100);
  SLAB_ALLOC3 (uint64, ZOBRIST, 0x10 * 0100);
  SLAB_ALLOC3 (sint8, LINE, 0100 * 0100);
  SLAB_ALLOC1 (uint64, ZobristCastling, 16);
  SLAB_ALLOC1 (uint64, ZobristEP, 8);
  SLAB_ALLOC1 (uint64, ZobristRev, 16);
#ifdef MULTIPLE_POS_GAIN
  SLAB_ALLOC2 (sint16, MAX_POSITIONAL_GAIN, MAX_CPUS * 0x10 * 010000);
#else
  SLAB_ALLOC2 (sint16, MAX_POSITIONAL_GAIN, 0x10 * 010000); /* SLAB 1mb */
#endif
#ifdef MULTIPLE_HISTORY
  SLAB_ALLOC2 (uint16, HISTORY, MAX_CPUS * 0x10 * 0100); /* SLAB 64k */
#else
  SLAB_ALLOC2 (uint16, HISTORY, 0x10 * 0100);
#endif
  SLAB_ALLOC2 (sint32, PieceSquareValue, 0x10 * 0100); /* SMP read SLAB 16k */
  SLAB_ALLOC2 (typeMATERIAL, MATERIAL, 419904); /* SMP read SLAB 1.68mb */

  ResetHistory ();
  ResetPositionalGain ();
  InitArrays ();
  InitMaterialValue ();
  InitStatic ();
}

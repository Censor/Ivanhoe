
#ifdef UTILITIES
#include "RobboLito.h"

void DrawBoard (typePOS* POSITION)
{
  char C[16] = "0PNKBBRQ";
  char ARR[256];
  int t, c, u;
  printf ("      +---+---+---+---+---+---+---+---+\n");
  for (t = R8; t >= R1; t--)
    {
      printf("   %i  |", t+1);
      for (c = FA; c <= FH; c++)
       {
	 u = POSITION->sq[c + 8 * t];
	 if (u >= 8)
	   printf ("<%c>", C[u - 8]);
	 else if (u > 0)
	   printf (" %c ", C[u]);
	 else if ((c + t) & 1)
	   printf ("   ");
	 else
	   printf (" . ");
	 printf ("|");
       }
      printf ("\n      +---+---+---+---+---+---+---+---+\n");
    }
  printf ("        a   b   c   d   e   f   g   h\n");
  EmitFen (POSITION, ARR);
  printf ("%s\n", ARR);
}

uint64 CNT[64];
#define IN_CHECK \
  (POSITION->wtm ? POSITION->DYN->wKcheck : POSITION->DYN->bKcheck)
#define ILLEGAL \
  (!POSITION->wtm ? POSITION->DYN->wKcheck : POSITION->DYN->bKcheck)

static void perft (typePOS* POSITION, int d, int h)
{
  typeMoveList LM[256], *lm;
  int i;
  if (IN_CHECK)
    lm = EvasionMoves (POSITION, LM, 0xffffffffffffffff);
  else
    {
      lm = CaptureMoves (POSITION, LM, POSITION->OccupiedBW);
      lm = OrdinaryMoves (POSITION, lm);
    }
  for (i = 0; i < lm - LM; i++)
    {
      Make (POSITION, LM[i].move);
      Mobility (POSITION);
      if (!ILLEGAL)
	{
	  CNT[h]++;
	  if (d > 1)
	    perft (POSITION, d - 1, h + 1);
	}
      Undo (POSITION, LM[i].move);
    }
}

void PERFT (typePOS* POSITION, int d)
{
  int i;
  Mobility (POSITION);
  for (i = 0; i < 64; i++)
    CNT[i] = 0;
  perft (POSITION, d, 1);
  for (i = d; i >= 2; i--)
    printf ("%lld/", CNT[i]);
  printf ("%lld\n", CNT[1]);
}

uint64 PERFD (typePOS* POSITION, int n)
{
  int i;
  uint64 TOTAL = 0, TIME;
  typeMoveList LM[256], *lm;
  DrawBoard (POSITION);
  TIME = GetClock();
  Mobility (POSITION);
  if (IN_CHECK)
    lm = EvasionMoves (POSITION, LM, 0xffffffffffffffff);
  else
    {
      lm = CaptureMoves (POSITION, LM, POSITION->OccupiedBW);
      lm = OrdinaryMoves (POSITION, lm);
    }
  for (i = 0; i < lm - LM; i++)
    {
      Make (POSITION, LM[i].move);
      Mobility (POSITION);
      if (!ILLEGAL)
	{
	  printf ("%s ",Notate (LM[i].move, STRING1[POSITION->cpu]));
	  PERFT (POSITION, n - 1);
	  TOTAL += CNT[n - 1];
	}
      Undo (POSITION, LM[i].move);
    }
  printf ("TOTAL %lld  moves %ld  time: %lld us\n",
	  TOTAL, lm - LM, GetClock() - TIME);
  return TOTAL;
}

void Perfd (typePOS* POSITION, int d, uint64 n)
{
  uint64 u = PERFD (POSITION, d);
  if (u != n)
    ERROR_END ("Perfd failed: computed: %lld given: %lld", u, n);
}
#endif


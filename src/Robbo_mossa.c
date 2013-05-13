
#include "RobboLito.h"
#include "RobboBaseLibUsage.h"

#define dROTT 0x00
#define dVITT 0x01
#define dPATT 0x02
#define dPERD0 (dPATT + 1)
#define DISCO_PATT_PERD(x) ((x) >= dPATT)
#define DISCO_VITTORIA(x) ((x) == dVITT)
#define DISCO_PATTA(x) ((x) == dPATT)
#define DISCO_PERDITA(x) ((x) >= dPERD0)

#define IsCheck \
 ( POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

#define IS_ILLEGAL \
 ( !POSITION->wtm ? \
   (wBitboardK & POSITION->DYN->bAtt) : (bBitboardK & POSITION->DYN->wAtt) )

boolean RobboTotalBaseScore (typePOS *, int *);

boolean Robbo_mossa (typePOS* POSITION, uint32* am, int* v, int* cap)
{
  int va, v2, av = 0xff, zv = 0x00, cp;
  typeMoveList mossa_lista[256];
  typeMoveList* q;
  uint32 m;
  if (POSITION->DYN->oo)
    return FALSE;
  if (!RobboTotalBaseScore (POSITION, &va) || va == dPERD0)
    return FALSE;
  Mobility (POSITION);
  if (IsCheck)
    EvasionMoves (POSITION, mossa_lista, 0xffffffffffffffff);
  else
    {
      q = CaptureMoves (POSITION, mossa_lista, 0xffffffffffffffff);
      OrdinaryMoves (POSITION, q);
    }
  for (q = mossa_lista; q->move; q++)
    {
      m = q->move & 0x7fff;
      POSITION->StackHeight = 0; /* HACK */
      Make (POSITION, m);
      POSITION->StackHeight = 1; /* HACK */
      Mobility (POSITION);
      if (IS_ILLEGAL)
	{
	  Undo (POSITION, m);
	  POSITION->StackHeight = 1; /* HACK */
	  continue;
	}
      if (!RobboTotalBaseScore (POSITION, &v2))
	{
	  Undo (POSITION, m);
	  POSITION->StackHeight = 1; /* HACK */
	  return FALSE;
	}
      if (DISCO_PERDITA (va))
	{
	  uint32 m3;
	  int v3;
	  if (!Robbo_mossa (POSITION, &m3, &v3, &cp))
	    {
	      Undo (POSITION, m);
	      POSITION->StackHeight = 1; /* HACK */
	      return FALSE;
	    }
	  if (v3 > zv &&
	      ((!POSITION->DYN->cp && !(m & FLAG_MASK) &&
		((!cp && (!(m3 & FLAG_MASK)))))
	       || va <= dPERD0 + 1))
	    {
	      *am = m;
	      *v = zv = v3;
	      *cap = POSITION->DYN->cp;
	    }
	}
      Undo (POSITION, m);
      POSITION->StackHeight = 1; /* HACK */
      if (DISCO_PATTA (va) && DISCO_PATTA (v2))
	{
	  *am = m;
	  *v = dPATT;
	  *cap = FALSE; /* HACK */
	  return TRUE;
	}
      if (DISCO_VITTORIA (va) && DISCO_PERDITA (v2))
	{
	  if (POSITION->sq[TO (m)] || (m & FLAG_MASK))
	    {
	      *am = m;
	      *v = v2;
	      *cap = TRUE; /* HACK */
	      return TRUE;
	    }
	  if (v2 < av)
	    {
	      *am = m;
	      *v = av = v2;
	      *cap = FALSE; /* HACK */
	    }
	}
    }
  return TRUE;
}

extern uint32 ROOT_BEST_MOVE;
#define HASH_XOR 0xfa73e65b089c41d2ULL
boolean RobboFare (typePOS* POSITION)
{
  int va, v2, cp;
  uint32 m, tbhits;
  POSITION->DYN->HASH ^= HASH_XOR;
  SEARCH_ROBBO_BASES = TRUE;
  Eval (POSITION, -0x7fff0000, 0x7fff0000, 0, 0xff);
  POSITION->DYN->HASH ^= HASH_XOR;
  if (POSITION->DYN->exact)
    {
      SEARCH_ROBBO_BASES = FALSE;
      POSITION->DYN->HASH ^= SQUISH_SPLAT;
    }
  else
    SEARCH_ROBBO_BASES = TRUE;
  if (!ROBBO_TOTAL_LOAD || !ROBBO_TOTAL_CONDITION (POSITION)
      || !RobboTotalBaseScore (POSITION, &va))
    return FALSE;
  if (!Robbo_mossa (POSITION, &m, &v2, &cp))
    return FALSE;
  tbhits = POSITION->tbhits; /* 32-bits */ /* HACK */
  if (DISCO_PATTA (va))
    {
      SEND ("info multipv 1 depth 0 score cp 0 tbhits %d pv %s\n",
	    tbhits, Notate (m, STRING1[POSITION->cpu]));
      ROOT_BEST_MOVE = m;
      return TRUE;
    }
  if (DISCO_PERDITA (va))
    {
      SEND ("info multipv 1 depth 0 score cp %d tbhits %d pv %s\n",
	    -28000 + 1000 * POPCNT (POSITION->OccupiedBW) + v2,
	    tbhits, Notate (m, STRING1[POSITION->cpu]));
      ROOT_BEST_MOVE = m;
      return TRUE;
    }
  if (DISCO_VITTORIA (va))
    {
      SEND ("info multipv 1 depth 0 score cp %d tbhits %d pv %s\n",
	    28000 - 1000 * POPCNT (POSITION->OccupiedBW) - v2,
	    tbhits, Notate (m, STRING1[POSITION->cpu]));
      ROOT_BEST_MOVE = m;
      return TRUE;
    }
  return FALSE;
}

static void main_line_iterate (typePOS* POSITION)
{
  int va, v2, cp;
  uint32 m;
  (POSITION->DYN + 1)->move = MOVE_NONE;
  Mobility (POSITION);
  if (!RobboTotalBaseScore (POSITION, &va))
    {
      SEND ("info string Failure in main_line\n");      
      (POSITION->DYN + 1)->move = MOVE_NONE;
      POSITION->StackHeight = 0; /* HACK */
      return;
    }
  if (!Robbo_mossa (POSITION, &m, &v2, &cp))
    {
      (POSITION->DYN + 1)->move = MOVE_NONE;
      POSITION->StackHeight = 0; /* HACK */
      return;
    }
  if (va == dPATT)
    {
      POSITION->StackHeight = 0; /* HACK */
      Make (POSITION, m);
      POSITION->StackHeight = 1; /* HACK */
      Undo (POSITION, m);
      return;
    }
  m &= 0x7fff;
  POSITION->StackHeight = 0; /* HACK */
  Make (POSITION, m);
  POSITION->StackHeight = 1; /* HACK */
  main_line_iterate (POSITION);
  Undo (POSITION, m);
  POSITION->StackHeight = 1; /* HACK */
}

void main_line (typePOS* POSITION) /* extra */
{
  typeDYNAMIC* p;
  char STRING[16];
  int va;
  main_line_iterate (POSITION);
  if (!RobboTotalBaseScore (POSITION, &va) || va == dROTT)
    {
      SEND ("info string Failure in main_line\n");
      return;
    }
  sprintf (STRING, "%d", va - dPERD0);
  printf ("mainline (%c%s): ", (va == dVITT) ? 'W' : ((va == dPATT) ? 'D' : 'L'),
	  (va < dPERD0) ? "" : STRING);
  for (p = POSITION->DYN + 1; p->move; p++)
    printf ("%s ", Notate (p->move & 0x7fff, STRING));
  SEND ("\n");
}

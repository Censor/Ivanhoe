#include "RobboLito.h"
#include <string.h>

char* Notate (uint32 move, char* M)
{
  int fr, to, pr;
  char c[16] = "0123nbrq";
  fr = FROM (move);
  to = TO (move);
  if (move == MOVE_NONE)
    {
      M[0] = 'N';
      M[1] = 'U';
      M[2] = M[3] = 'L';
      M[4] = 0;
      return M;
    }
  sprintf (M, "%c%c%c%c", 'a' + (fr & 7), '1' + ((fr >> 3) & 7),
	   'a' + (to & 7), '1' + ((to >> 3) & 7));
  if (MoveIsProm (move))
    {
      pr = (move & FLAG_MASK) >> 12;
      sprintf (M + 4, "%c", c[pr]);
    }
  return M;
}

uint32 FullMove (typePOS* POSITION, uint32 x)
{
  int pi, to = TO (x), fr = FROM (x);
  if (!x)
    return x;
  pi = POSITION->sq[fr];
  if (pi == wEnumK || pi == bEnumK)
    {
      if (UCI_OPTION_CHESS_960) /* attend Chess960 */
	{
	  if (POSITION->wtm && POSITION->sq[to] == wEnumR)
	    x |= FlagOO;
	  if (!POSITION->wtm && POSITION->sq[to] == bEnumR)
	    x |= FlagOO;
	}
      else if (to - fr == 2 || fr - to == 2)
        x |= FlagOO;
    }
  if (TO (x) != 0 && TO (x) == POSITION->DYN->ep
      && (pi == wEnumP || pi == bEnumP))
    x |= FlagEP;
  return x;
}

uint32 numeric_move (typePOS* POSITION, char* str)
{
  int x;
  x = FullMove (POSITION, (str[2] - 'a') + ((str[3] - '1') << 3) +
                ((str[0] - 'a') << 6) + ((str[1] - '1') << 9));
  if (str[4] == 'b')
    x |= FlagPromB;
  if (str[4] == 'n')
    x |= FlagPromN;
  if (str[4] == 'r')
    x |= FlagPromR;
  if (str[4] == 'q')
    x |= FlagPromQ;
  return x;
}

static void DoBad (char* x)
{
  SEND ("info string errore posizionale: %s\n", x);
}

#define BAD(x) { DoBad (x); return; }
#include "material_value.i"

#define TWEAK (0x74d3c012a8bf965e)
void InitBitboards (typePOS* POSITION)
{
  int i, b, pi;
  uint64 O;
  BOARD_IS_OK = FALSE;
  for (i = 0; i < 16; i++)
    POSITION->bitboard[i] = 0;
  POSITION->DYN->HASH = POSITION->DYN->PAWN_HASH = 0;
  POSITION->DYN->material = 0;
  POSITION->DYN->STATIC = 0;
  for (i = A1; i <= H8; i++)
    {
      if ((pi = POSITION->sq[i]))
	{
	  POSITION->DYN->STATIC += PST (pi, i);
	  POSITION->DYN->HASH ^= Zobrist (pi, i);
	  if (pi == wEnumP || pi == bEnumP)
	    POSITION->DYN->PAWN_HASH ^= Zobrist (pi, i);
	  POSITION->DYN->material += MATERIAL_VALUE[pi];
	  BitSet (i, POSITION->bitboard[POSITION->sq[i]]);
	}
    }
  wBitboardOcc = wBitboardK | wBitboardQ | wBitboardR |
    wBitboardB | wBitboardN | wBitboardP;
  bBitboardOcc = bBitboardK | bBitboardQ | bBitboardR |
    bBitboardB | bBitboardN | bBitboardP;
  POSITION->OccupiedBW = wBitboardOcc | bBitboardOcc;
#ifndef MAGIC_BITBOARDS
  POSITION->OccupiedL90 = POSITION->OccupiedL45 = POSITION->OccupiedR45 = 0;
#endif
  O = POSITION->OccupiedBW;
  if (POPCNT (wBitboardQ) > 1 || POPCNT (bBitboardQ) > 1
      || POPCNT (wBitboardR) > 2 || POPCNT (bBitboardR) > 2
      || POPCNT (wBitboardBL) > 1 || POPCNT (bBitboardBL) > 1
      || POPCNT (wBitboardN) > 2 || POPCNT (bBitboardN) > 2
      || POPCNT (wBitboardBD) > 1 || POPCNT (bBitboardBD) > 1)
    POSITION->DYN->material |= 0x80000000;
  if (POPCNT (wBitboardK) != 1)
    BAD ("re bianco != 1");
  if (POPCNT (bBitboardK) != 1)
    BAD ("re nero != 1");
  if (POPCNT (wBitboardQ) > 9)
    BAD ("donna bianca > 9");
  if (POPCNT (bBitboardQ) > 9)
    BAD ("donna nera > 9");
  if (POPCNT (wBitboardR) > 10)
    BAD ("torre bianco > 10");
  if (POPCNT (bBitboardR) > 10)
    BAD ("torre nero > 10");
  if (POPCNT (wBitboardBL) > 9)
    BAD ("chiaro bianco > 9");
  if (POPCNT (bBitboardBL) > 9)
    BAD ("chiaro nero > 9");
  if (POPCNT (wBitboardBD) > 9)
    BAD ("scuro bianco > 9");
  if (POPCNT (bBitboardBD) > 9)
    BAD ("scuro nero > 9");
  if (POPCNT (wBitboardBL | wBitboardBD) > 10)
    BAD ("alfiere bianco > 10");
  if (POPCNT (bBitboardBL | bBitboardBD) > 10)
    BAD ("alfiere nero > 10");
  if (POPCNT (wBitboardN) > 10)
    BAD ("cavallo bianco > 10");
  if (POPCNT (bBitboardN) > 10)
    BAD ("cavallo nero > 10");
  if (POPCNT (wBitboardP) > 8)
    BAD ("pedone bianco > 8");
  if (POPCNT (bBitboardP) > 8)
    BAD ("pedone nero > 8");
  if (POPCNT (wBitboardOcc) > 16)
    BAD ("pezzo bianco > 16");
  if (POPCNT (bBitboardOcc) > 16)
    BAD ("pezzo nero > 16");
  if ((wBitboardP | bBitboardP) & (RANK1 | RANK8))
    BAD ("pedone traversa prima o ottava");
#ifndef MAGIC_BITBOARDS
  while (O)
    {
      b = BSF (O);
      BitClear (b, O);
      BitSet (Left90[b], POSITION->OccupiedL90);
      BitSet (Left45[b], POSITION->OccupiedL45);
      BitSet (Right45[b], POSITION->OccupiedR45);
    }
#endif
  POSITION->wKsq = BSF (wBitboardK);
  POSITION->bKsq = BSF (bBitboardK);
  if (UCI_OPTION_CHESS_960)
    {
      if (WhiteOO && (POSITION->wKsq != CHESS_960_KING_FILE ||
		      !(wBitboardR & SqSet[CHESS_960_KR_FILE])))
	  BAD ("arrocco illegale");
      if (WhiteOOO && (POSITION->wKsq != CHESS_960_KING_FILE ||
		       !(wBitboardR & SqSet[CHESS_960_QR_FILE])))
	  BAD ("arrocco illegale");
      if (BlackOO && (POSITION->bKsq != (CHESS_960_KING_FILE + 070) ||
		      !(bBitboardR & SqSet[CHESS_960_KR_FILE + 070])))
	  BAD ("arrocco illegale");
      if (BlackOOO && (POSITION->bKsq != (CHESS_960_KING_FILE + 070) ||
		       !(bBitboardR & SqSet[CHESS_960_QR_FILE + 070])))
	  BAD ("arrocco illegale");
    }
  else
    {
      if ((WhiteOO && (POSITION->wKsq != E1 || !(wBitboardR & SqSet[H1])))
	  || (WhiteOOO && (POSITION->wKsq != E1 || !(wBitboardR & SqSet[A1])))
	  || (BlackOO && (POSITION->bKsq != E8 || !(bBitboardR & SqSet[H8])))
	  || (BlackOOO && (POSITION->bKsq != E8 || !(bBitboardR & SqSet[A8]))))
	BAD ("arrocco illegale");
    }
  POSITION->DYN->HASH ^= ZobristCastling[POSITION->DYN->oo];
  REV_HASH (POSITION);
  if (POSITION->DYN->ep)
    POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
  POSITION->DYN->PAWN_HASH ^=
    ZobristCastling[POSITION->DYN-> oo] ^ TWEAK ^
    Zobrist (wEnumK, POSITION->wKsq) ^ Zobrist(bEnumK, POSITION->bKsq);
  if (POSITION->wtm)
    POSITION->DYN->HASH ^= ZobristWTM;
  Mobility (POSITION);
  if (POSITION->wtm && POSITION->DYN->wAtt & bBitboardK)
    BAD ("bianco cattura re");
  if (!POSITION->wtm && POSITION->DYN->bAtt & wBitboardK)
    BAD ("nero catture re");
  if (POPCNT (POSITION->DYN->bKcheck) >= 3 || POPCNT (POSITION->DYN->wKcheck) >= 3)
    BAD ("InCheck from 3 pieces");
  BOARD_IS_OK = TRUE;
}

#ifdef WINDOWS
#include <time.h>
boolean TryInput ()
{
  static int init = 0, is_pipe; /* init is global? */
  static HANDLE stdin_h;
  DWORD val;

  if (SUPPRESS_INPUT)
    return FALSE;
  if (!SEARCH_IS_DONE && STALL_INPUT)
    return FALSE;
#if 0
  if (input_ptr != INPUT_BUFFER)
    return TRUE;
#endif
#if 0 /* purpose? */
  if (stdin->_cnt > 0)
    return 1;
#endif
  if (!init)
    {
      init = 1; /* TRUE? */
      stdin_h = GetStdHandle (STD_INPUT_HANDLE);
      is_pipe = !GetConsoleMode (stdin_h, &val);
      if (!is_pipe)
        {
          SetConsoleMode (stdin_h, val & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
          FlushConsoleInputBuffer (stdin_h);
        }
    }
  if (is_pipe)
    {
      if (!PeekNamedPipe (stdin_h, NULL, 0, NULL, &val, NULL))
        return 1;
      return val > 0; /* return val ? */
    }
  else
    {
      GetNumberOfConsoleInputEvents (stdin_h, &val);
      return val > 1; /* return (val <= 1) ? 0 : val ? */
    }
  return 0; /* dummy */
}


uint64 GetClock()
{
  return (GetTickCount () * 1000ULL);
}
#else /* not WINDOWS */
#include <unistd.h>
boolean TryInput ()
{
  int v;
  fd_set fd[1];
  struct timeval tv[1];

  if (SUPPRESS_INPUT)
    return FALSE;
  if (!SEARCH_IS_DONE && STALL_INPUT)
    return FALSE;
#if 0
  if (input_ptr != INPUT_BUFFER)
    return TRUE;
#endif
  FD_ZERO (fd);
  FD_SET (STDIN_FILENO, fd);
  tv->tv_sec = 0;
  tv->tv_usec = 0;
  v = select (STDIN_FILENO + 1, fd, NULL, NULL, tv);
  return (v > 0);
}

#include <sys/time.h>
uint64 GetClock ()
{
  uint64 x;
  struct timeval tv;
  gettimeofday (&tv, NULL);
  x = tv.tv_sec;
  x *= 1000000;
  x += tv.tv_usec;
  return x;
}
#endif /* ifdef WINDOWS */

/****************************************************************/

#include <stdarg.h>

static MUTEX_TYPE SEND_LOCK[1];
void InitSend () { LOCK_INIT (SEND_LOCK); }
void SEND (char* fmt, ...)
{
  va_list Value;
  LOCK (SEND_LOCK);
  va_start (Value, fmt);
  vfprintf (stdout, fmt, Value);
  va_end (Value);
  fflush (stdout);
  UNLOCK (SEND_LOCK);
}

void ERROR_END (char* fmt, ...)
{
  va_list Value;
  va_start (Value, fmt);
  va_end (Value);
  /*  stdout = stderr; // WINDOWS */
  fprintf (stdout, "*** ERRORE ***\n");
  vfprintf (stdout, fmt, Value);
  exit (1);
}

void FEN_ERROR (char* fmt, ...)
{
  va_list Value;
  va_start (Value, fmt);
  va_end (Value);
  /*  stdout = stderr; // WINDOWS */
  fprintf (stdout, "*** FEN ERR ***\n");
  vfprintf (stdout, fmt, Value);
  exit (1);
}

int PREVIOUS_DEPTH, PREVIOUS_FAST;
void NewGame (typePOS* POSITION, boolean full)
{
  int i;
  for (i = A1; i <= H8; i++)
    POSITION->sq[i] = 0;
  memset (POSITION->DYN_ROOT, 0, 256 * sizeof (typeDYNAMIC));
  POSITION->DYN = POSITION->DYN_ROOT + 1;
  POSITION->wtm = TRUE;
  POSITION->height = 0;
  POSITION->DYN->oo = 0x0f;
  POSITION->DYN->ep = 0;
  POSITION->DYN->reversible = 0;
  for (i = A2; i <= H2; i++)
    POSITION->sq[i] = wEnumP;
  for (i = A7; i <= H7; i++)
    POSITION->sq[i] = bEnumP;
  POSITION->sq[D1] = wEnumQ;
  POSITION->sq[D8] = bEnumQ;
  POSITION->sq[E1] = wEnumK;
  POSITION->sq[E8] = bEnumK;
  POSITION->sq[A1] = POSITION->sq[H1] = wEnumR;
  POSITION->sq[A8] = POSITION->sq[H8] = bEnumR;
  POSITION->sq[B1] = POSITION->sq[G1] = wEnumN;
  POSITION->sq[B8] = POSITION->sq[G8] = bEnumN;
  POSITION->sq[C1] = wEnumBD;
  POSITION->sq[F1] = wEnumBL;
  POSITION->sq[C8] = bEnumBL;
  POSITION->sq[F8] = bEnumBD;
  PREVIOUS_DEPTH = 1000;
  PREVIOUS_FAST = FALSE;
  NEW_GAME = TRUE;
  POSITION->StackHeight = 0;
  InitBitboards (POSITION);
  if (!full)
    return;
  PVHashClear ();
  HashClear ();
  EvalHashClear ();
  ResetHistory ();
  ResetPositionalGain ();
  PawnHashReset ();
#ifdef CON_ROBBO_BUILD
  TripleEnumerateReset ();
  TripleHashClear (); /* polite */
#endif
}

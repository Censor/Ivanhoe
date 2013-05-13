
static INLINE void MakeNull (typePOS* POSITION)
{
  TRACE (TRACE_MAKE_UNMAKE,
         printf ("M%cN %d\n", POSITION->wtm ? 'w' : 'b', HEIGHT(POSITION)));
  POSITION->nodes++;
  POSITION->DYN->SAVED_FLAGS = POSITION->DYN->flags;
  memcpy (POSITION->DYN + 1, POSITION->DYN, 64); /* safe with opt ? */
  POSITION->DYN++;
  POSITION->DYN->HASH ^= ZobristWTM;
  POSITION->wtm ^= 1;
  POSITION->height++;
  REV_HASH (POSITION);
  POSITION->DYN->reversible++;
  if (POSITION->DYN->ep)
    {
      POSITION->DYN->HASH ^= ZobristEP[POSITION->DYN->ep & 7];
      POSITION->DYN->ep = 0;
    }
  POSITION->DYN->Value = -((POSITION->DYN - 1)->Value + TempoValue);
  POSITION->DYN->PositionalValue = (POSITION->DYN - 1)->PositionalValue;
  POSITION->DYN->lazy = (POSITION->DYN - 1)->lazy;
  POSITION->DYN->flags &= ~3;
  POSITION->DYN->move = 0;
  POSITION->STACK[++(POSITION->StackHeight)] = POSITION->DYN->HASH;
  REV_HASH (POSITION);
  VALIDATE (POSITION, 0, MOVE_NONE);
}

static INLINE void UndoNull (typePOS* POSITION)
{
  TRACE (TRACE_MAKE_UNMAKE,
         printf ("U%cN %d\n", POSITION->wtm ? 'b' : 'w', HEIGHT(POSITION)));
  POSITION->DYN--;
  POSITION->StackHeight--;
  POSITION->height--;
  POSITION->wtm ^= 1;
  POSITION->DYN->flags = POSITION->DYN->SAVED_FLAGS;
  VALIDATE (POSITION, 1, MOVE_NONE);
}

#define NULL_REDUCTION (2 + 6) /* R = 3 */
#define VERIFY_REDUCTION (2)
#define SCORE_REDUCTION(x) (((uint32) (MIN ((x), 96))) / 32)
#define KING_DANGER_ADJUST(wK, bK) (0) /* wK plus bK now /4 */
//  (MAX (4, BSR (1 + (wK))) + MAX (4, BSR (1 + (bK))) - 8)

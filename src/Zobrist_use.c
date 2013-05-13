
#include "RobboLito.h"
#include <string.h>

#define MAX_AGE 256
#define MAX_DEPTH 256

void IncrementAge ()
{
  GLOBAL_AGE += 1;
  if (GLOBAL_AGE == MAX_AGE)
    GLOBAL_AGE = 0;
}

#define AGE_DEPTH_MIX(a, x, y) \
  ((ICI_HASH_MIX) ? \
   (3 * MIN (((uint8) ((a) - (x))), ((uint8) ((x) - (a)))) \
    + MAX_DEPTH - (y)) : \
   ((((a) - (x)) & (MAX_AGE - 1)) * MAX_DEPTH + (MAX_DEPTH - ((y) + 1))))

void HashLowerALLNew (uint64 Z, int move, int depth, int Value, int ht, int age)
{
  int DEPTH, i;
  TRANS_DECLARE ();
  int max = 0, w = 0, mix;
  YUSUF_DECLARE ();
  move &= 0x7fff;
  Trans = HASH_POINTER (Z);
  Value = VALUE_ADJUST_STORE (Value, ht);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (Z >> 32)) == 0 && STICKY_CONDITION (trans)
	  && (!trans->DepthLower || IsALL (trans))
	  && trans->DepthLower <= depth)
	{
	  trans->DepthLower = depth;
	  trans->move = move;
	  trans->ValueLower = Value;
	  trans->age = age;
	  trans->flags |= FLAG_LOWER | FLAG_ALL;
	  TRACE (TRACE_HASH, HASH_WRITE ("LA", trans));
	  HYATT_HASH_WRITE (trans, Trans);
	  return;
	}
      DEPTH = MAX (trans->DepthLower, trans->DepthUpper);
      mix = AGE_DEPTH_MIX (age, trans->age, DEPTH);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans = HASH_POINTER (Z) + w;
  trans->hash = (Z >> 32);
  trans->DepthUpper = 0;
  trans->ValueUpper = 0;
  trans->DepthLower = depth;
  trans->move = move;
  trans->ValueLower = Value;
  trans->age = age;
  trans->flags = FLAG_LOWER | FLAG_ALL;
  TRACE (TRACE_HASH, HASH_OVERWRITE ("LA", trans));
  HYATT_HASH_WRITE (trans, Trans);
  return;
}

void HashUpperCUTNew (uint64 Z, int depth, int Value, int ht, int age)
{
  int DEPTH, i;
  TRANS_DECLARE ();
  int max = 0, w = 0, mix;
  YUSUF_DECLARE ();
  Trans = HASH_POINTER (Z);
  Value = VALUE_ADJUST_STORE (Value, ht);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if (!(trans->hash ^ (Z >> 32)) && STICKY_CONDITION (trans)
	  && (!trans->DepthUpper || IsCUT (trans))
	  && trans->DepthUpper <= depth)
	{
	  trans->DepthUpper = depth;
	  trans->ValueUpper = Value;
	  trans->age = age;
	  trans->flags |= FLAG_UPPER | FLAG_CUT;
          TRACE (TRACE_HASH, HASH_WRITE ("UC", trans));
	  HYATT_HASH_WRITE (trans, Trans);
	  return;
	}
      DEPTH = MAX (trans->DepthLower, trans->DepthUpper);
      mix = AGE_DEPTH_MIX (age, trans->age, DEPTH);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans = HASH_POINTER (Z) + w;
  trans->hash = (Z >> 32);
  trans->DepthLower = 0;
  trans->move = 0;
  trans->ValueLower = 0;
  trans->DepthUpper = depth;
  trans->ValueUpper = Value;
  trans->age = age;
  trans->flags = FLAG_UPPER | FLAG_CUT;
  TRACE (TRACE_HASH, HASH_OVERWRITE ("UC", trans));
  HYATT_HASH_WRITE (trans, Trans);
  return;
}

void HashLowerNew (uint64 Z, int move, int depth, int Value, int FLAGS, int ht, int age)
{
  int DEPTH, i;
  TRANS_DECLARE ();
  int max = 0, w = 0, mix;
  YUSUF_DECLARE ();
  move &= 0x7fff;
  Trans = HASH_POINTER (Z);
  Value = VALUE_ADJUST_STORE (Value, ht);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if (!(trans->hash ^ (Z >> 32)) && !IsExact (trans)
	  && STICKY_CONDITION (trans) && trans->DepthLower <= depth)
	{
	  trans->DepthLower = depth;
	  trans->move = move;
	  trans->ValueLower = Value;
	  trans->age = age;
	  trans->flags &= ~(FLAG_ALL | LOWER_FROM_PV);
	  trans->flags |= FLAG_LOWER | FLAGS;
          TRACE (TRACE_HASH, HASH_WRITE ("L", trans));
	  HYATT_HASH_WRITE (trans, Trans);
	  return;
	}
      DEPTH = MAX (trans->DepthLower, trans->DepthUpper);
      mix = AGE_DEPTH_MIX (age, trans->age, DEPTH);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans = HASH_POINTER (Z) + w;
  trans->hash = (Z >> 32);
  trans->DepthUpper = 0;
  trans->ValueUpper = 0;
  trans->DepthLower = depth;
  trans->move = move;
  trans->ValueLower = Value;
  trans->age = age;
  trans->flags = FLAG_LOWER | FLAGS;
  TRACE (TRACE_HASH, HASH_OVERWRITE ("L", trans));
  HYATT_HASH_WRITE (trans, Trans);
  return;
}

void HashUpperNew (uint64 Z, int depth, int Value, int FLAGS, int ht, int age)
{
  int DEPTH, i;
  TRANS_DECLARE ();
  int max = 0, w = 0, mix;
  YUSUF_DECLARE ();
  Trans = HASH_POINTER (Z);
  Value = VALUE_ADJUST_STORE (Value, ht);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if (!(trans->hash ^ (Z >> 32)) && !IsExact (trans)
	  && STICKY_CONDITION (trans) && trans->DepthUpper <= depth)
	{
	  trans->DepthUpper = depth;
	  trans->ValueUpper = Value;
	  trans->age = age;
	  trans->flags &= ~(FLAG_CUT | UPPER_FROM_PV);
	  trans->flags |= FLAG_UPPER | FLAGS;
          TRACE (TRACE_HASH, HASH_WRITE ("U", trans));
	  HYATT_HASH_WRITE (trans, Trans);
	  return;
	}
      DEPTH = MAX (trans->DepthLower, trans->DepthUpper);
      mix = AGE_DEPTH_MIX (age, trans->age, DEPTH);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans = HASH_POINTER (Z) + w;
  trans->hash = (Z >> 32);
  trans->DepthLower = 0;
  trans->move = 0;
  trans->ValueLower = 0;
  trans->DepthUpper = depth;
  trans->ValueUpper = Value;
  trans->age = age;
  trans->flags = FLAG_UPPER | FLAGS;
  TRACE (TRACE_HASH, HASH_OVERWRITE ("U", trans));
  HYATT_HASH_WRITE (trans, Trans);
  return;
}

static void pv_zobrist (uint64 Z, int move, int depth, int Value, int age)
{
  int i, k = Z & PVHashMask;
  TRANS_PV_DECLARE ();
  int w = 0, max = 0, mix;
  YUSUF_PV_DECLARE ();
  for (i = 0; i < 4; i++)
    {
      Trans_pv = PVHashTable + (k + i);
      HYATT_HASH (Trans_pv, trans_pv);
      if (trans_pv->hash == Z)
	{
	  trans_pv->depth = depth; /* esta seguro que depth es mejor ? */
	  trans_pv->Value = Value;
	  trans_pv->move = move;
	  trans_pv->age = age;
	  HYATT_HASH_WRITE (trans_pv, Trans_pv);
	  return;
	}
      mix = AGE_DEPTH_MIX (age, trans_pv->age, trans_pv->depth);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans_pv = PVHashTable + (k + w);
  trans_pv->hash = Z;
  trans_pv->depth = depth;
  trans_pv->move = move;
  trans_pv->Value = Value;
  trans_pv->age = age;
  HYATT_HASH_WRITE (trans_pv, Trans_pv);
}

void HashExactNew (uint64 Z, int move, int depth, int Value, int FLAGS, int ht, int age)
{
  int DEPTH, i, j;
  TRANS_DECLARE ();
  int max = 0, w = 0, mix;
  YUSUF_DECLARE ();
  move &= 0x7fff;
  pv_zobrist (Z, move, depth, Value, age);
  Trans = HASH_POINTER (Z);
  Value = VALUE_ADJUST_STORE (Value, ht);
  for (i = 0; i < 4; i++, Trans++)
    {
      HYATT_HASH (Trans, trans);
      if ((trans->hash ^ (Z >> 32)) == 0 && STICKY_CONDITION (trans)
	  && MAX (trans->DepthUpper, trans->DepthLower) <= depth)
	{
	  trans->DepthUpper = trans->DepthLower = depth;
	  trans->move = move;
	  trans->ValueUpper = trans->ValueLower = Value;
	  trans->age = age;
	  trans->flags = FLAGS;
          TRACE (TRACE_HASH, HASH_WRITE ("E", trans));
	  HYATT_HASH_WRITE (trans, Trans);
	  for (j = i + 1; j < 4; j++)
	    {
	      Trans++;
	      HYATT_HASH (Trans, trans);
	      if ((trans->hash ^ (Z >> 32)) == 0
		  && MAX (trans->DepthUpper, trans->DepthLower) <= depth)
		{
		  memset (trans, 0, 16);
		  trans->age = age ^ (MAX_AGE / 2);
		  TRACE (TRACE_HASH, HASH_ZERO (trans));
		  HYATT_HASH_WRITE (trans, Trans);
		}
	    }
	  return;
	}
      DEPTH = MAX (trans->DepthLower, trans->DepthUpper);
      mix = AGE_DEPTH_MIX (age, trans->age, DEPTH);
      if (mix > max)
	{
	  max = mix;
	  w = i;
	}
    }
  Trans = HASH_POINTER (Z) + w;
  trans->hash = (Z >> 32);
  trans->DepthUpper = trans->DepthLower = depth;
  trans->move = move;
  trans->ValueUpper = trans->ValueLower = Value;
  trans->age = age;
  trans->flags = FLAGS;
  TRACE (TRACE_HASH, HASH_OVERWRITE ("E", trans));
  HYATT_HASH_WRITE (trans, Trans);
  return;
}

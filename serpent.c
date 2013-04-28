/*
 * Copyright (c) 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Douglas Voorhies    - Hilbert pixel sequence (Graphics Gems II)
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "defs.h"
#include "extern.h"

/**********************************************************************
 *    RAY TRACING - Serpentine - Version 8.4.1                        *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, May 1994               *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, May 1994               *
 **********************************************************************/

static int      p_x, p_y;
static int      pixel_index;
static char_ptr pixel_list;

void
serp_reset_pixel_list()
{
  pixel_index = 0;
  p_x = 0;
  p_y = 0;
}
void
serp_make_pixel_list(sx, sy)
  int             sx, sy;
{
  int             x, y, temp;

  ALLOCATE(pixel_list, char, 1 + (sx * sy) DIV 4, TRACE_TYPE);
  pixel_index = 0;
  for (y = 0; y < sy; POSINC(y))
  {
    if (ODD(y))
    {
      temp = 2; /* -X */
      for (x = sx - 2; x >= 0; POSDEC(x))
      {
        if (pixel_index MOD 4 == 0)
          pixel_list[pixel_index DIV 4] = (char) temp;
        else
          pixel_list[pixel_index DIV 4] |= (char)
            (temp SHL ((pixel_index MOD 4) * 2));
        POSINC(pixel_index);
      }
    } else
    {
      temp = 0; /* +X */
      for (x = 0; x < sx - 1; POSINC(x))
      {
        if (pixel_index MOD 4 == 0)
          pixel_list[pixel_index DIV 4] = (char) temp;
        else
          pixel_list[pixel_index DIV 4] |= (char)
            (temp SHL ((pixel_index MOD 4) * 2));
        POSINC(pixel_index);
      }
    }
    temp = 1; /* +Y */
    if (pixel_index MOD 4 == 0)
      pixel_list[pixel_index DIV 4] = (char) temp;
    else
      pixel_list[pixel_index DIV 4] |= (char)
        (temp SHL ((pixel_index MOD 4) * 2));
    POSINC(pixel_index);
  }
  serp_reset_pixel_list();
}
void
serp_get_next_pixel(x, y)
  int            *x, *y;
{
  REG int         temp;

  temp = pixel_index MOD 4;
  if (temp == 0)
    temp = pixel_list[pixel_index DIV 4] BIT_AND 3;
  else
    temp = (pixel_list[pixel_index DIV 4] SHR (temp * 2)) BIT_AND 3;
  POSINC(pixel_index);
  switch(temp)
  {
  case 0:
    POSINC(p_x);
    break;
  case 1:
    POSINC(p_y);
    break;
  case 2:
    POSDEC(p_x);
    break;
  case 3:
    POSDEC(p_y);
    break;
  }
  *x = p_x;
  *y = p_y;
}

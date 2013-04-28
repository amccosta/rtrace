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
 *    RAY TRACING - Hilbert - Version 8.4.1                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, January 1994           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, May 1994               *
 **********************************************************************/

static int      s_x, s_y, p_x, p_y;
static int      pixel_index;
static char_ptr pixel_list;

static void 
step(angle)
  int             angle;
{
  char            temp;

  while (angle > 270)
    angle -= 360;               /* Fold ANGLE to be 0, 90, 180, 270 */
  while (angle < 0)
    angle += 360;
  if (angle == 0)
    temp = 0;                   /* +X */
  else if (angle == 90)
    temp = 1;                   /* +Y */
  else if (angle == 180)
    temp = 2;                   /* -X */
  else
    temp = 3;                   /* -Y */
  if (pixel_index MOD 4 == 0)
    pixel_list[pixel_index DIV 4] = (char) temp;
  else
    pixel_list[pixel_index DIV 4] |= (char)
      (temp SHL ((pixel_index MOD 4) * 2));
  POSINC(pixel_index);
}

/* Recursive Hilbert-curve generation algorithm */
static void 
hilbert(orient, angle, level)
  int             orient, *angle, level;
{
  int             sw;

  if (level <= 0)
    return;
  POSDEC(level);
  *angle += orient * 90;
  hilbert(-orient, angle, level);
  step(*angle);
  *angle -= orient * 90;
  hilbert(orient, angle, level);
  step(*angle);
  hilbert(orient, angle, level);
  *angle -= orient * 90;
  step(*angle);
  hilbert(-orient, angle, level);
  *angle += orient * 90;
}
void
hilb_reset_pixel_list()
{
  pixel_index = 0;
  p_x = 0;
  p_y = 0;
}
void
hilb_make_pixel_list(sx, sy)
  int             sx, sy;
{
  int             temp, hilb_level, hilb_size, angle;

  s_x = sx;
  s_y = sy;
  temp = MAX(sx, sy);
  hilb_level = 0;
  hilb_size = 1;
  do {
    POSINC(hilb_level);
    hilb_size *= 2;
  } while (hilb_size < temp);
  ALLOCATE(pixel_list, char, 1 + (hilb_size * hilb_size) DIV 4, TRACE_TYPE);
  pixel_index = 0;
  angle = 0;
  hilbert(1, &angle, hilb_level);
  hilb_reset_pixel_list();
}
void
hilb_get_next_pixel(x, y)
  int            *x, *y;
{
  REG int         temp;

  do {
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
  } while ((p_x >= s_x) OR(p_y >= s_y));
  *x = p_x;
  *y = p_y;
}

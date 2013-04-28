/*
 * Copyright (c) 1993, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Gary Murphy         - original code
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
#include <grx.h>

#ifndef INDEX
#define INDEX(v)\
(ROUND(MAX(0.0, MIN((real) INDEX_MAX, (v) * (real) SUCC(INDEX_MAX)))))
#endif

#define RED (0.3)
#define GRN (0.51)
#define BLU (1.0 - GRN - RED)

#define SetColor(c, r, g, b) GrSetColor((c), (r) << 2, (g) << 2, (b) << 2)

#ifdef __STDC__
#define PARAMS(X) X
#else
#define PARAMS(X)
#endif

static void     pallete_init PARAMS((int));
static void     hsv_to_rgb PARAMS((real, real, real,
                                   unsigned *, unsigned *, unsigned *));
static void     rgb_to_hsv PARAMS((unsigned, unsigned, unsigned,
                                   real *, real *, real *));

static char     grey_mode = 0;
static int      image_height, image_width, offset_height, offset_width;

void
display_finished()
{
  while (!kbhit())
    ;
  if (!getkey())
    getkey();
}

void 
display_close()
{
  GrSetMode(GR_default_text);
}

void 
display_init(width, height)
  int             width, height;
{
  if (height <= 200)
    GrSetMode(GR_320_200_graphics);
  else if (height <= 480)
    GrSetMode(GR_width_height_graphics, 640, 480, 256);
  else
    GrSetMode(GR_default_graphics);
  if (GrNumColors() != 256)
  {
    display_close();
    runtime_abort("picture display only in 256 color mode");
  }

  image_height = height;
  image_width = width;

  pallete_init(verbose_mode == -2);

  if (image_height > GrSizeY())
    offset_height = 0;
  else
    offset_height = (GrSizeY() - image_height) / 2;

  if (image_width > GrSizeX())
    offset_width = 0;
  else
    offset_width = (GrSizeX() - image_width) / 2;

  if (image_height > GrSizeY())
    if (image_width > GrSizeX())
      GrBox(0, 0, GrMaxX(), GrMaxY(), 255);
    else
      GrBox(offset_width, 0, offset_width + image_width - 1, GrMaxY(), 255);
  else
    if (image_width > GrMaxX())
      GrBox(0, offset_height, GrMaxX(), offset_height + image_height - 1, 255);
    else
      GrBox(offset_width, offset_height, offset_width + image_width - 1,
            offset_height + image_height - 1, 255);
}

static void
check_interrupt_display()
{
  if (!kbhit())
    return;
  if (!getkey())
    getkey();
  display_close();
  runtime_abort("interrupted by user");
}

/* Fill VGA 256 color palette with colors! */
static void 
pallete_init(mode)
  int             mode;
{
  REG unsigned    m;
  unsigned        r, g, b;
  REG real        hue, sat, val;

  if (!mode)
  {
    grey_mode = 1;
    for (m = 0; m < 256; m++)
      GrSetColor(m, m, m, m);
    return;
  }

  GrSetColor(0, 0, 0, 0);
  GrSetColor(64, 63, 63, 63);
  GrSetColor(128, 31, 31, 31);
  GrSetColor(192, 48, 48, 48);
  for (m = 1; m < 64; m++)
  {
    sat = 0.5;          /* start with the saturation and intensity low */
    val = 0.5;
    hue = 360.0 * ((real) (m)) / 64.0;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    SetColor(m, r, g, b);

    sat = 1.0;          /* high saturation and half intensity (shades) */
    val = 0.50;
    hue = 360.0 * ((real) (m)) / 64.0;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    SetColor(m + 64, r, g, b);

    sat = 0.5;          /* half saturation and high intensity (pastels) */
    val = 1.0;

    hue = 360.0 * ((real) (m)) / 64.0;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    SetColor(m + 128, r, g, b);

    sat = 1.0;          /* normal full HSV set at full intensity */
    val = 1.0;

    hue = 360.0 * ((real) (m)) / 64.0;
    hsv_to_rgb(hue, sat, val, &r, &g, &b);
    SetColor(m + 192, r, g, b); /* set m + 192 */
  }
}

void 
display_plot(x, y, r, g, b)
  int             x, y;
  real            r, g, b;
{
  REG unsigned char color;
  real            h, s, v, fx, fy;
  static int      lasty = -1, lastline = -1, lastx = -1;
  unsigned        red, grn, blu;

  check_interrupt_display();

  if (!x)                       /* first pixel on this line? */
  {
    lastx = -1;                 /* reset cache, make sure we do the 1st one */
    lasty = lastline;           /* set last line do to prior line */
  }
  if (image_height > GrSizeY()) /* auto-scale Y */
  {
    fy = (real) y / (real) image_height * (real) GrSizeY();
    y = (int) fy;
    if (y <= lasty)
      return;
    lastline = y;
  }
  if (image_width > GrSizeX())  /* auto-scale X */
  {
    fx = (real) x / (real) image_width * (real) GrSizeX();
    x = (int) fx;
    if (x <= lastx)
      return;
    lastx = x;
  }
  
  if (grey_mode)
  {
    /* Translate RGB value to grey scale */
    color = (unsigned char) INDEX(r * RED + g * GRN + b * BLU);
    GrPlot(offset_width + x, offset_height + y, color);
    return;
  }

  red = (unsigned) INDEX(r);
  grn = (unsigned) INDEX(g);
  blu = (unsigned) INDEX(b);

  /* Translate RGB value to best of 256 pallete Colors (by HSV?) */
  rgb_to_hsv(red, grn, blu, &h, &s, &v);

  if (s < 0.20)                 /* black or white if no saturation of color */
  {
    if (v < 0.25)
      color = 0;                /* black */
    else if (v > 0.8)
      color = 64;               /* white */
    else if (v > 0.5)
      color = 192;              /* lite grey */
    else
      color = 128;              /* dark grey */
  } else
  {
    color = (unsigned char) (64.0 * ((real) (h)) / 360.0);

    if (!color)
      color = 1;                /* avoid black, white or grey */

    if (color > 63)
      color = 63;               /* avoid same */

    if (v > 0.50)
      color |= 0x80;            /* colors 128-255 for high intensity */

    if (s > 0.50)               /* more than half saturated? */
      color |= 0x40;            /* color range 64-128 or 192-255 */
  }

  GrPlot(offset_width + x, offset_height + y, color);
  return;
}

/* Conversion from Hue, Saturation, Value to Red, Green, and Blue and back */
/* From "Computer Graphics", Donald Hearn & M. Pauline Baker, p. 304 */

static void 
hsv_to_rgb(hue, s, v, r, g, b)
  real            hue, s, v;
  unsigned       *r, *g, *b;
{
  REG real        i, f, p1, p2, p3;
  REG real        xh;
  REG real        nr = 0.0, ng = 0.0, nb = 0.0;

  if (hue == 360.0)
    hue = 0.0;

  xh = hue / 60.0;
  i = floor(xh);
  f = xh - i;
  p1 = v * (1 - s);
  p2 = v * (1 - (s * f));
  p3 = v * (1 - (s * (1 - f)));

  switch ((int) i)
  {
  case 0:
    nr = v;
    ng = p3;
    nb = p1;
    break;
  case 1:
    nr = p2;
    ng = v;
    nb = p1;
    break;
  case 2:
    nr = p1;
    ng = v;
    nb = p3;
    break;
  case 3:
    nr = p1;
    ng = p2;
    nb = v;
    break;
  case 4:
    nr = p3;
    ng = p1;
    nb = v;
    break;
  case 5:
    nr = v;
    ng = p1;
    nb = p2;
    break;
  }

  *r = (unsigned) (nr * 63.0);  /* Normalize the values to 63 */
  *g = (unsigned) (ng * 63.0);
  *b = (unsigned) (nb * 63.0);

  return;
}

static void 
rgb_to_hsv(r, g, b, h, s, v)
  unsigned        r, g, b;
  real           *h, *s, *v;
{
  REG real        m, r1, g1, b1;
  REG real        nr, ng, nb;
  REG real        nh = 0.0, ns, nv;

  nr = (real) r / 255.0;
  ng = (real) g / 255.0;
  nb = (real) b / 255.0;

  nv = MAX(nr, MAX(ng, nb));
  m = MIN(nr, MIN(ng, nb));

  if (nv != 0.0)                /* if no value, it's black! */
    ns = (nv - m) / nv;
  else
    ns = 0.0;                   /* black = no colour saturation */

  if (ns == 0.0)                /* hue undefined if no saturation */
  {
    *h = 0.0;                   /* return black level (?) */
    *s = 0.0;
    *v = nv;
    return;
  }
  r1 = (nv - nr) / (nv - m);
  g1 = (nv - ng) / (nv - m);
  b1 = (nv - nb) / (nv - m);

  if (nv == nr)
  {
    if (m == ng)
      nh = 5.0 + b1;
    else
      nh = 1.0 - g1;
  }
  if (nv == ng)
  {
    if (m == nb)
      nh = 1.0 + r1;
    else
      nh = 3.0 - b1;
  }
  if (nv == nb)
  {
    if (m == nr)
      nh = 3.0 + g1;
    else
      nh = 5.0 - r1;
  }
  *h = nh * 60.0;               /* return h converted to degrees */
  *s = ns;
  *v = nv;
  return;
}

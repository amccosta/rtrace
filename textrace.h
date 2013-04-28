/*
 * Copyright (c) 1992, 1993 Pedro Borges, Paulo Almeida, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "nmalloc.h"
#include "pp.h"
#include "pp_ext.h"

/**********************************************************************
 *    RAY TRACING - Version 8.3.0                                     *
 *                                                                    *
 *    MADE BY: Pedro Borges, Paulo Almeida, INESC-Norte, June 1992    *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1993              *
 **********************************************************************/

#ifdef __sgi
#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#endif

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define new(t) (t *)alloc(sizeof(t))
#define newa(t,s) (t *)alloc((s) * sizeof(t))

#define BIG 1.0e8
#define TINY 1.0e-8
#define PIS2 (M_PI / 2.0)
#define REALCONV "%lf"
#define NULSP -10000
#define PSNAME '/'
#define BUFFERSIZE 100

typedef
struct
{
  xyz_struct      x_v, y_v, z_v;
}               xyz_axes;

typedef
struct
{
  real            x, y;
}               xy_struct, *xy_ptr;

typedef
struct
{
  xy_struct       p[3];
  char            t;
}               curve;

typedef
struct
{
  int             n;
  curve          *curve_arr;
}               ccurve;

typedef
struct
{
  real            bot, top, width;
  int             n;
  ccurve         *ccurve_arr;
}               prismdsc;

typedef
struct
{
  xyz_struct      position;
  xyz_axes       *orient_ptr;
  xyz_ptr         size_ptr;
  int             ccint, cint, enter;
  real            tint;
  xyz_struct      dirint;
  prismdsc       *prismdsc_ptr;
}               prism;

typedef
struct character
{
  struct character *next;
  char           *name;
  prismdsc       *prismdsc_ptr;
}               character;

typedef
struct font
{
  struct font    *next;
  char           *name;
  struct character *ch_ptr;
}               font;

typedef
struct code
{
  struct code    *next;
  int             code;
  char           *name;
}               code;

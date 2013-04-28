/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd           - overall, portability
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

/**********************************************************************
 *    RAY TRACING - Random generator - Version 8.4.1                  *
 *                                                                    *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, September 1994         *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1994         *
 **********************************************************************/

/* Random number generator - adapted from "Numerical Recipies in C" */
static int      PR_AAA;
static int      PR_BBB;
static int      PR_randx;

#define	PR_MULT		1103515245
#define	PR_ADD		12345
#define	PR_MASK		(0x7fffffff)
#define	PR_TWOTO31	2147483648.0

void
psrand(seed) 
  long            seed;
{
  PR_AAA = 1;
  PR_BBB = 0;

  PR_AAA = (PR_MULT * PR_AAA) & PR_MASK;
  PR_BBB = (PR_MULT * PR_BBB + PR_ADD) & PR_MASK;

  PR_randx = (PR_AAA * seed + PR_BBB) & PR_MASK;
}

static double
pranf()
/* Return a random double in [0, 1.0) */
{
  double          retvalue;

  retvalue = PR_randx / PR_TWOTO31;
  PR_randx = (PR_AAA * PR_randx + PR_BBB) & PR_MASK;
  return retvalue;
}

double
prand()
{
  static double   z, v[98], dum;
  static int      iff = 0;
  int             j;

  if (iff == 0)
  {
    iff = 1;
    for (j = 1; j <= 197; j++)
      dum = pranf();
    for (j = 1; j <= 97; j++)
      v[j] = pranf();
    z = pranf();
  }
  j = 1 + (int) (97.0 * z);
  z = v[j];
  v[j] = pranf();
  return z;
}

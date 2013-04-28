/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
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

/**********************************************************************
 *    RAY TRACING - Patch - Version 8.3.4                             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** Patch *****/
#define BLEND1(t) ((real) ((t) * (t) * (3.0 - 2.0 * (t))))
#define BLEND0(t) (1.0 - BLEND1(t))

#define DERIV_BLEND1(t) ((real) (6.0 * (t) * (1.0 - (t))))
#define DERIV_BLEND0(t) (-DERIV_BLEND1(t))

#define CUBIC(p, i, t, p0)\
do {\
  (p0).x = (p)->c[3][(i)].x + (t) * ((p)->c[2][(i)].x + (t) *\
           ((p)->c[1][(i)].x + (t) * (p)->c[0][(i)].x));\
  (p0).y = (p)->c[3][(i)].y + (t) * ((p)->c[2][(i)].y + (t) *\
           ((p)->c[1][(i)].y + (t) * (p)->c[0][(i)].y));\
  (p0).z = (p)->c[3][(i)].z + (t) * ((p)->c[2][(i)].z + (t) *\
           ((p)->c[1][(i)].z + (t) * (p)->c[0][(i)].z));\
} while (0)

#define DERIV_CUBIC(p, i, t, p0)\
do {\
  (p0).x = (p)->c[2][(i)].x + (t) * ((p)->c[1][(i)].x * 2.0 + (t) *\
           (p)->c[0][(i)].x * 3.0);\
  (p0).y = (p)->c[2][(i)].y + (t) * ((p)->c[1][(i)].y * 2.0 + (t) *\
           (p)->c[0][(i)].y * 3.0);\
  (p0).z = (p)->c[2][(i)].z + (t) * ((p)->c[1][(i)].z * 2.0 + (t) *\
           (p)->c[0][(i)].z * 3.0);\
} while (0)

#define MULTIPLY(p0, t, p1)\
do {\
  (p1).x = (p0).x * (t);\
  (p1).y = (p0).y * (t);\
  (p1).z = (p0).z * (t);\
} while (0)

#define SUBTRACT(p0, p1, p2, p3)\
do {\
  (p3).x = (p0).x - (p1).x - (p2).x;\
  (p3).y = (p0).y - (p1).y - (p2).y;\
  (p3).z = (p0).z - (p1).z - (p2).z;\
} while (0)

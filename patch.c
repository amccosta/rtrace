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
#include "defs.h"
#include "extern.h"

/**********************************************************************
 *    RAY TRACING - Patch (part1) - Version 8.3.4                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** Patch *****/
#include "patch.h"

static real     small_distance;
static xyz_struct k0, k1, k2, k3, k4, k5, k6, k7, p[4];
static patch_ptr patch;

static void
subpatch_enclose(p0, p1, p2, p3, pmin, pmax)
  xyz_ptr         p0, p1, p2, p3, pmin, pmax;
#define CORRECTION ((real) 0.25)/* 25% correction */
{
  xyz_struct      center;

  STRUCT_ASSIGN(*pmax, *p0);
  STRUCT_ASSIGN(*pmin, *p0);
  pmax->x = MAX(pmax->x, p1->x);
  pmax->y = MAX(pmax->y, p1->y);
  pmax->z = MAX(pmax->z, p1->z);
  pmin->x = MIN(pmin->x, p1->x);
  pmin->y = MIN(pmin->y, p1->y);
  pmin->z = MIN(pmin->z, p1->z);
  pmax->x = MAX(pmax->x, p2->x);
  pmax->y = MAX(pmax->y, p2->y);
  pmax->z = MAX(pmax->z, p2->z);
  pmin->x = MIN(pmin->x, p2->x);
  pmin->y = MIN(pmin->y, p2->y);
  pmin->z = MIN(pmin->z, p2->z);
  pmax->x = MAX(pmax->x, p3->x);
  pmax->y = MAX(pmax->y, p3->y);
  pmax->z = MAX(pmax->z, p3->z);
  pmin->x = MIN(pmin->x, p3->x);
  pmin->y = MIN(pmin->y, p3->y);
  pmin->z = MIN(pmin->z, p3->z);
  if (ABS(pmax->x - pmin->x) < threshold_distance)
    pmax->x += threshold_distance;
  if (ABS(pmax->y - pmin->y) < threshold_distance)
    pmax->y += threshold_distance;
  if (ABS(pmax->z - pmin->z) < threshold_distance)
    pmax->z += threshold_distance;
  center.x = (pmax->x + pmin->x) * 0.5;
  center.y = (pmax->y + pmin->y) * 0.5;
  center.z = (pmax->z + pmin->z) * 0.5;
  pmax->x += (pmax->x - center.x) * CORRECTION;
  pmax->y += (pmax->y - center.y) * CORRECTION;
  pmax->z += (pmax->z - center.z) * CORRECTION;
  pmin->x += (pmin->x - center.x) * CORRECTION;
  pmin->y += (pmin->y - center.y) * CORRECTION;
  pmin->z += (pmin->z - center.z) * CORRECTION;
#undef CORRECTION
}
static boolean
solve(a, b, c)
  array2          a;            /* It is modified */
  array1          b, c;
{
  REG int         row_pivot, i, j, k;
  REG real        row_max, col_max, col_ratio, temp0;
  array1          temp;
  int             pivot[3];

  for (i = 0; i <= 2; POSINC(i))
  {
    pivot[i] = i;
    row_max = 0.0;
    for (j = 0; j <= 2; POSINC(j))
      row_max = MAX(row_max, ABS(a[i][j]));
    if (ABS(row_max) < ROUNDOFF)
      return FALSE;
    temp[i] = row_max;
  }
  for (i = 0; i <= 1; POSINC(i))
  {
    col_max = ABS(a[i][i]) / temp[i];
    row_pivot = i;
    for (j = SUCC(i); j <= 2; POSINC(j))
    {
      col_ratio = ABS(a[j][i]) / temp[j];
      if (col_ratio > col_max)
      {
        col_max = col_ratio;
        row_pivot = j;
      }
    }
    if (ABS(col_max) < ROUNDOFF)
      return FALSE;
    if (row_pivot > i)
    {
      j = pivot[row_pivot];
      pivot[row_pivot] = pivot[i];
      pivot[i] = j;
      temp0 = temp[row_pivot];
      temp[row_pivot] = temp[i];
      temp[i] = temp0;
      for (j = 0; j <= 2; POSINC(j))
      {
        temp0 = a[row_pivot][j];
        a[row_pivot][j] = a[i][j];
        a[i][j] = temp0;
      }
    }
    for (j = SUCC(i); j <= 2; POSINC(j))
    {
      a[j][i] /= a[i][i];
      temp0 = a[j][i];
      for (k = SUCC(i); k <= 2; POSINC(k))
        a[j][k] -= temp0 * a[i][k];
    }
  }
  if (ABS(a[2][2]) < ROUNDOFF)
    return FALSE;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 2; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[2] /= a[2][2];
  for (i = 1; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 2; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  return TRUE;
}

#define ERROR_LIM ((real) 1.0e+5)
#define F_ERROR(f)\
((ABS((f)[0]) > ERROR_LIM) OR(ABS((f)[1]) > ERROR_LIM)\
OR(ABS((f)[2]) > ERROR_LIM) ?\
INFINITY : SQR((f)[0]) + SQR((f)[1]) + SQR((f)[2]))

static boolean
patch_damped_nr(u, v, position, vector)
  real            u, v;
  xyz_ptr         position, vector;
/***** Damped Newton-Raphson *****/
#define DELTA_U_MAX ROUNDOFF
#define DELTA_V_MAX DELTA_U_MAX

#define U_MAX ((real) 2.0)
#define U_MIN ((real) -1.0)
#define V_MAX U_MAX
#define V_MIN U_MIN

#define ERROR_MAX   ROUNDOFF
#define F_ERROR_MAX (SQR(ERROR_MAX) * 3.0)
#define F_ERROR_LIM (SQR(ERROR_LIM) * 3.0)

#define ITERATION_MAX         (24)
#define ITERATION_DAMPING_MAX (12)

{
  boolean         intersect;
  REG int         i, j;
  REG real        b0, b1, b2, b3, u0, v0, distance, distance0, f_error;
  REG real        damping, k;
  array1          f, f0, delta;
  array2          df;
  xyz_struct      temp;

  /* Initial values */
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b0, k5);
  MULTIPLY(p[2], b1, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  temp.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3 - position->x;
  temp.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3 - position->y;
  temp.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3 - position->z;
  distance = LENGTH(temp);
  f[0] = temp.x - distance * vector->x;
  f[1] = temp.y - distance * vector->y;
  f[2] = temp.z - distance * vector->z;
  i = 0;
  if (F_ERROR(f) > F_ERROR_LIM)
    return FALSE;
  intersect = FALSE;
  while ((i < ITERATION_MAX) AND NOT intersect)
  {
    POSINC(i);
    /* 1st partial derivative (u) */
    b0 = BLEND0(v);
    b1 = BLEND1(v);
    CUBIC(patch, 1, v, k1);
    CUBIC(patch, 3, v, k3);
    MULTIPLY(p[0], b0, k4);
    MULTIPLY(p[1], b0, k5);
    MULTIPLY(p[2], b1, k6);
    MULTIPLY(p[3], b1, k7);
    SUBTRACT(k1, k4, k6, k1);
    SUBTRACT(k3, k5, k7, k3);
    DERIV_CUBIC(patch, 0, u, k0);
    DERIV_CUBIC(patch, 2, u, k2);
    b2 = DERIV_BLEND0(u);
    b3 = DERIV_BLEND1(u);
    df[0][0] = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
    df[1][0] = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
    df[2][0] = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
    /* 2nd partial derivative (v) */
    b0 = BLEND0(u);
    b1 = BLEND1(u);
    CUBIC(patch, 0, u, k0);
    CUBIC(patch, 2, u, k2);
    MULTIPLY(p[0], b0, k4);
    MULTIPLY(p[1], b1, k5);
    MULTIPLY(p[2], b0, k6);
    MULTIPLY(p[3], b1, k7);
    SUBTRACT(k0, k4, k5, k0);
    SUBTRACT(k2, k6, k7, k2);
    DERIV_CUBIC(patch, 1, v, k1);
    DERIV_CUBIC(patch, 3, v, k3);
    b2 = DERIV_BLEND0(v);
    b3 = DERIV_BLEND1(v);
    df[0][1] = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
    df[1][1] = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
    df[2][1] = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
    df[0][2] = -vector->x;
    df[1][2] = -vector->y;
    df[2][2] = -vector->z;
    if (NOT solve(df, f, delta))
      return FALSE;
    /* Damping */
    u0 = u - delta[0];
    v0 = v - delta[1];
    if ((u0 < U_MIN) OR(u0 > U_MAX) OR(v0 < V_MIN) OR(v0 > V_MAX))
      return FALSE;
    distance0 = distance - delta[2];
    b0 = BLEND0(v0);
    b1 = BLEND1(v0);
    CUBIC(patch, 1, v0, k1);
    CUBIC(patch, 3, v0, k3);
    MULTIPLY(p[0], b0, k4);
    MULTIPLY(p[1], b0, k5);
    MULTIPLY(p[2], b1, k6);
    MULTIPLY(p[3], b1, k7);
    SUBTRACT(k1, k4, k6, k1);
    SUBTRACT(k3, k5, k7, k3);
    CUBIC(patch, 0, u0, k0);
    CUBIC(patch, 2, u0, k2);
    b2 = BLEND0(u0);
    b3 = BLEND1(u0);
    f[0] = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3 -
      position->x - distance0 * vector->x;
    f[1] = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3 -
      position->y - distance0 * vector->y;
    f[2] = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3 -
      position->z - distance0 * vector->z;
    damping = 1.0;
    f_error = F_ERROR(f);
    k = 1.0;
    j = 0;
    while (j < ITERATION_DAMPING_MAX)
    {
      POSINC(j);
      k = k * 0.5;
      u0 = u - delta[0] * k;
      v0 = v - delta[1] * k;
      distance0 = distance - delta[2] * k;
      b0 = BLEND0(v0);
      b1 = BLEND1(v0);
      CUBIC(patch, 1, v0, k1);
      CUBIC(patch, 3, v0, k3);
      MULTIPLY(p[0], b0, k4);
      MULTIPLY(p[1], b0, k5);
      MULTIPLY(p[2], b1, k6);
      MULTIPLY(p[3], b1, k7);
      SUBTRACT(k1, k4, k6, k1);
      SUBTRACT(k3, k5, k7, k3);
      CUBIC(patch, 0, u0, k0);
      CUBIC(patch, 2, u0, k2);
      b2 = BLEND0(u0);
      b3 = BLEND1(u0);
      f0[0] = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3 -
        position->x - distance0 * vector->x;
      f0[1] = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3 -
        position->y - distance0 * vector->y;
      f0[2] = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3 -
        position->z - distance0 * vector->z;
      if (F_ERROR(f0) < f_error)
      {
        damping = k;
        ARRAY_ASSIGN(f, f0, 1);
        f_error = F_ERROR(f);
      } else
        break;
    }
    if (f_error > F_ERROR_LIM)
      return FALSE;
    /* New values */
    u -= delta[0] * damping;
    v -= delta[1] * damping;
    distance -= delta[2] * damping;
    intersect = (boolean) ((f_error <= F_ERROR_MAX)
                           AND(ABS(delta[0]) <= DELTA_U_MAX)
                           AND(ABS(delta[1]) <= DELTA_V_MAX));
  }
  if (intersect
      AND(u >= -DELTA_U_MAX) AND(u <= 1.0 + DELTA_U_MAX)
      AND(v >= -DELTA_V_MAX) AND(v <= 1.0 + DELTA_V_MAX)
      AND(distance > threshold_distance) AND(distance < small_distance))
  {
    small_distance = distance;
    patch->u_hit = MAX(0.0, MIN(1.0, u));
    patch->v_hit = MAX(0.0, MIN(1.0, v));
  }
  return intersect;
}
#define BIG_DISTANCE(p0, p1) ((boolean) (\
SQR((p0).x - (p1).x) + SQR((p0).y - (p1).y) + SQR((p0).z - (p1).z) >=\
SQR(threshold_distance)))

static void
patch_bissect(p0, p2, p6, p8, u0, v0, delta, position, vector)
  xyz_ptr         p0, p2, p6, p8;
  real            u0, v0, delta;
  xyz_ptr         position, vector;
{
  REG real        u, v, b0, b1, b2, b3, distance, delta2;
  xyz_struct      p1, p3, p4, p5, p7, pmin, pmax, temp;

  delta2 = delta * 0.5;
  /* Points 1 and 7 */
  u = u0 + delta2;
  b0 = BLEND0(u);
  b1 = BLEND1(u);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b1, k5);
  MULTIPLY(p[2], b0, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k0, k4, k5, k0);
  SUBTRACT(k2, k6, k7, k2);
  /* Point 1 */
  v = v0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = v0 + delta;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = v0 + delta2;
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b0, k5);
  MULTIPLY(p[2], b1, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  /* Point 3 */
  u = u0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = u0 + delta2;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = u0 + delta;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  subpatch_enclose(p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p0, p4))
      patch_bissect(p0, &p1, &p3, &p4, u0, v0, delta2, position, vector);
    else
    {
      temp.x = (p0->x + p4.x) * 0.5 - position->x;
      temp.y = (p0->y + p4.y) * 0.5 - position->y;
      temp.z = (p0->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 0.5;
        patch->v_hit = v0 + delta2 * 0.5;
      }
    }
  subpatch_enclose(&p1, p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p2, p4))
      patch_bissect(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2,
                    position, vector);
    else
    {
      temp.x = (p2->x + p4.x) * 0.5 - position->x;
      temp.y = (p2->y + p4.y) * 0.5 - position->y;
      temp.z = (p2->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 1.5;
        patch->v_hit = v0 + delta2 * 0.5;
      }
    }
  subpatch_enclose(&p3, &p4, p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p6, p4))
      patch_bissect(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2,
                    position, vector);
    else
    {
      temp.x = (p6->x + p4.x) * 0.5 - position->x;
      temp.y = (p6->y + p4.y) * 0.5 - position->y;
      temp.z = (p6->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 0.5;
        patch->v_hit = v0 + delta2 * 1.5;
      }
    }
  subpatch_enclose(&p4, &p5, &p7, p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (BIG_DISTANCE(*p8, p4))
      patch_bissect(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                    position, vector);
    else
    {
      temp.x = (p8->x + p4.x) * 0.5 - position->x;
      temp.y = (p8->y + p4.y) * 0.5 - position->y;
      temp.z = (p8->z + p4.z) * 0.5 - position->z;
      distance = LENGTH(temp);
      if ((distance > threshold_distance) AND(distance < small_distance))
      {
        small_distance = distance;
        patch->u_hit = u0 + delta2 * 1.5;
        patch->v_hit = v0 + delta2 * 1.5;
      }
    }
}
static void
patch_divide(p0, p2, p6, p8, u0, v0, delta, level, position, vector)
  xyz_ptr         p0, p2, p6, p8;
  real            u0, v0, delta;
  int             level;
  xyz_ptr         position, vector;
{
#define LEVEL_MAX (8)
  real            u, v, distance;
  REG real        b0, b1, b2, b3, delta2;
  xyz_struct      p1, p3, p4, p5, p7, pmin, pmax;

  delta2 = delta * 0.5;
  /* Points 1 and 7 */
  u = u0 + delta2;
  b0 = BLEND0(u);
  b1 = BLEND1(u);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b1, k5);
  MULTIPLY(p[2], b0, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k0, k4, k5, k0);
  SUBTRACT(k2, k6, k7, k2);
  /* Point 1 */
  v = v0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = v0 + delta;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = v0 + delta2;
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b0, k5);
  MULTIPLY(p[2], b1, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  /* Point 3 */
  u = u0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = u0 + delta2;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = u0 + delta;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  subpatch_enclose(p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(p0, &p1, &p3, &p4, u0, v0, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 0.5, v0 + delta2 * 0.5,
                              position, vector))
        patch_bissect(p0, &p1, &p3, &p4, u0, v0, delta2, position, vector);
    }
  subpatch_enclose(&p1, p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 1.5, v0 + delta2 * 0.5,
                              position, vector))
        patch_bissect(&p1, p2, &p4, &p5, u0 + delta2, v0, delta2,
                      position, vector);
    }
  subpatch_enclose(&p3, &p4, p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2, SUCC(level),
                   position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 0.5, v0 + delta2 * 1.5,
                              position, vector))
        patch_bissect(&p3, &p4, p6, &p7, u0, v0 + delta2, delta2,
                      position, vector);
    }
  subpatch_enclose(&p4, &p5, &p7, p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    if (level <= LEVEL_MAX)
      patch_divide(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                   SUCC(level), position, vector);
    else
    {
      if (NOT patch_damped_nr(u0 + delta2 * 1.5, v0 + delta2 * 1.5,
                              position, vector))
        patch_bissect(&p4, &p5, &p7, p8, u0 + delta2, v0 + delta2, delta2,
                      position, vector);
    }
#undef LEVEL_MAX
}
real
patch_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        u, v, b0, b1, b2, b3, distance;
  xyz_struct      p0, p1, p2, p3, p4, p5, p6, p7, p8, pmin, pmax;

  REALINC(patch_tests);
  patch = (patch_ptr) object->data;
  STRUCT_ASSIGN(p[0], patch->p[3]->coords);
  STRUCT_ASSIGN(p[1], patch->p[4]->coords);
  STRUCT_ASSIGN(p[2], patch->p[7]->coords);
  STRUCT_ASSIGN(p[3], patch->p[8]->coords);
  STRUCT_ASSIGN(p0, p[0]);
  STRUCT_ASSIGN(p2, p[1]);
  STRUCT_ASSIGN(p6, p[2]);
  STRUCT_ASSIGN(p8, p[3]);
  /* Points 1 and 7 */
  u = 0.5;
  b0 = BLEND0(u);
  b1 = BLEND1(u);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b1, k5);
  MULTIPLY(p[2], b0, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k0, k4, k5, k0);
  SUBTRACT(k2, k6, k7, k2);
  /* Point 1 */
  v = 0.0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p1.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p1.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p1.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Point 7 */
  v = 1.0;
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  b2 = BLEND0(v);
  b3 = BLEND1(v);
  p7.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  p7.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  p7.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  /* Points 3, 4 and 5 */
  v = 0.5;
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(p[0], b0, k4);
  MULTIPLY(p[1], b0, k5);
  MULTIPLY(p[2], b1, k6);
  MULTIPLY(p[3], b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  /* Point 3 */
  u = 0.0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p3.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p3.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p3.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 4 */
  u = 0.5;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p4.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p4.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p4.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Point 5 */
  u = 1.0;
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  b2 = BLEND0(u);
  b3 = BLEND1(u);
  p5.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  p5.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  p5.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* Intersect */
  small_distance = INFINITY;
  subpatch_enclose(&p0, &p1, &p3, &p4, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p0, &p1, &p3, &p4, 0.0, 0.0, 0.5, 1, position, vector);
  subpatch_enclose(&p1, &p2, &p4, &p5, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p1, &p2, &p4, &p5, 0.5, 0.0, 0.5, 1, position, vector);
  subpatch_enclose(&p3, &p4, &p6, &p7, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p3, &p4, &p6, &p7, 0.0, 0.5, 0.5, 1, position, vector);
  subpatch_enclose(&p4, &p5, &p7, &p8, &pmin, &pmax);
  distance = bound_intersect(position, vector, &pmin, &pmax);
  if ((distance > 0.0) AND(distance < small_distance))
    patch_divide(&p4, &p5, &p7, &p8, 0.5, 0.5, 0.5, 1, position, vector);
  if (small_distance > INFINITY * 0.5)
    return 0.0;
  return small_distance;
}

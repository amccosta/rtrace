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
 *    RAY TRACING - Patch (part2) - Version 8.3.4                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** Patch *****/
#include "patch.h"

void
patch_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  REG real        u, v, b0, b1, b2, b3;
  xyz_struct      dfu, dfv, k0, k1, k2, k3, k4, k5, k6, k7;
  patch_ptr       patch;

  patch = (patch_ptr) object->data;
  u = patch->u_hit;
  v = patch->v_hit;
  /* 1st partial derivative (u) */
  b0 = BLEND0(v);
  b1 = BLEND1(v);
  CUBIC(patch, 1, v, k1);
  CUBIC(patch, 3, v, k3);
  MULTIPLY(patch->p[3]->coords, b0, k4);
  MULTIPLY(patch->p[4]->coords, b0, k5);
  MULTIPLY(patch->p[7]->coords, b1, k6);
  MULTIPLY(patch->p[8]->coords, b1, k7);
  SUBTRACT(k1, k4, k6, k1);
  SUBTRACT(k3, k5, k7, k3);
  DERIV_CUBIC(patch, 0, u, k0);
  DERIV_CUBIC(patch, 2, u, k2);
  b2 = DERIV_BLEND0(u);
  b3 = DERIV_BLEND1(u);
  dfu.x = k0.x * b0 + k1.x * b2 + k2.x * b1 + k3.x * b3;
  dfu.y = k0.y * b0 + k1.y * b2 + k2.y * b1 + k3.y * b3;
  dfu.z = k0.z * b0 + k1.z * b2 + k2.z * b1 + k3.z * b3;
  /* 2nd partial derivative (v) */
  b0 = BLEND0(u);
  b1 = BLEND1(u);
  CUBIC(patch, 0, u, k0);
  CUBIC(patch, 2, u, k2);
  MULTIPLY(patch->p[3]->coords, b0, k4);
  MULTIPLY(patch->p[4]->coords, b1, k5);
  MULTIPLY(patch->p[7]->coords, b0, k6);
  MULTIPLY(patch->p[8]->coords, b1, k7);
  SUBTRACT(k0, k4, k5, k0);
  SUBTRACT(k2, k6, k7, k2);
  DERIV_CUBIC(patch, 1, v, k1);
  DERIV_CUBIC(patch, 3, v, k3);
  b2 = DERIV_BLEND0(v);
  b3 = DERIV_BLEND1(v);
  dfv.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
  dfv.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
  dfv.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
  CROSS_PRODUCT(*normal, dfu, dfv);
  NORMALIZE(*normal);
}
static void
load_patch(patch, i, i0, i1, i2, i3)
  patch_ptr       patch;
  int             i, i0, i1, i2, i3;
{
  xyz_struct      p0, p1, p2, p3;

  STRUCT_ASSIGN(p0, patch->p[i0]->coords);
  STRUCT_ASSIGN(p1, patch->p[i1]->coords);
  STRUCT_ASSIGN(p2, patch->p[i2]->coords);
  STRUCT_ASSIGN(p3, patch->p[i3]->coords);
  patch->c[0][i].x = (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * 0.5;
  patch->c[1][i].x = (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * 0.5;
  patch->c[2][i].x = (-p0.x + p2.x) * 0.5;
  patch->c[3][i].x = p1.x;
  patch->c[0][i].y = (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * 0.5;
  patch->c[1][i].y = (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * 0.5;
  patch->c[2][i].y = (-p0.y + p2.y) * 0.5;
  patch->c[3][i].y = p1.y;
  patch->c[0][i].z = (-p0.z + 3.0 * p1.z - 3.0 * p2.z + p3.z) * 0.5;
  patch->c[1][i].z = (2.0 * p0.z - 5.0 * p1.z + 4.0 * p2.z - p3.z) * 0.5;
  patch->c[2][i].z = (-p0.z + p2.z) * 0.5;
  patch->c[3][i].z = p1.z;
}
void
patch_enclose(object)
  object_ptr      object;
{
#define INTERVALS  (4)
#define CORRECTION ((real) 0.15)/* 15% correction */

  REG int         i, j;
  REG real        u, v, b0, b1, b2, b3;
  xyz_struct      temp, f, k0, k1, k2, k3, k4, k5, k6, k7, center;
  patch_ptr       patch;

  patch = (patch_ptr) object->data;
  if (object->transf != NULL)
  {
    for (; patch->vertex != NULL;
         patch->vertex = (vertex_ptr) patch->vertex->next)
    {
      transform(object->inv_transf, &(patch->vertex->coords), &temp);
      STRUCT_ASSIGN(patch->vertex->coords, temp);
    }
    FREE(object->transf);
    object->transf = NULL;
  }
  load_patch(patch, 0, 2, 3, 4, 5);
  load_patch(patch, 1, 0, 3, 7, 10);
  load_patch(patch, 2, 6, 7, 8, 9);
  load_patch(patch, 3, 1, 4, 8, 11);
  STRUCT_ASSIGN(*(object->min), patch->p[3]->coords);
  STRUCT_ASSIGN(*(object->max), patch->p[3]->coords);
  for (i = 0; i <= INTERVALS; POSINC(i))
  {
    u = (real) i / (real) INTERVALS;
    b0 = BLEND0(u);
    b1 = BLEND1(u);
    CUBIC(patch, 0, u, k0);
    CUBIC(patch, 2, u, k2);
    MULTIPLY(patch->p[3]->coords, b0, k4);
    MULTIPLY(patch->p[4]->coords, b1, k5);
    MULTIPLY(patch->p[7]->coords, b0, k6);
    MULTIPLY(patch->p[8]->coords, b1, k7);
    SUBTRACT(k0, k4, k5, k0);
    SUBTRACT(k2, k6, k7, k2);
    if (i == 0)
      j = 1;
    else
      j = 0;
    do
    {
      v = (real) j / (real) INTERVALS;
      CUBIC(patch, 1, v, k1);
      CUBIC(patch, 3, v, k3);
      b2 = BLEND0(v);
      b3 = BLEND1(v);
      f.x = k0.x * b2 + k1.x * b0 + k2.x * b3 + k3.x * b1;
      f.y = k0.y * b2 + k1.y * b0 + k2.y * b3 + k3.y * b1;
      f.z = k0.z * b2 + k1.z * b0 + k2.z * b3 + k3.z * b1;
      object->max->x = MAX(object->max->x, f.x);
      object->max->y = MAX(object->max->y, f.y);
      object->max->z = MAX(object->max->z, f.z);
      object->min->x = MIN(object->min->x, f.x);
      object->min->y = MIN(object->min->y, f.y);
      object->min->z = MIN(object->min->z, f.z);
      POSINC(j);
    }
    while (j <= INTERVALS);
  }
  /* Adjust dimensions */
  if (ABS(object->max->x - object->min->x) < threshold_distance)
    object->max->x += threshold_distance;
  if (ABS(object->max->y - object->min->y) < threshold_distance)
    object->max->y += threshold_distance;
  if (ABS(object->max->z - object->min->z) < threshold_distance)
    object->max->z += threshold_distance;
  center.x = (object->max->x + object->min->x) * 0.5;
  center.y = (object->max->y + object->min->y) * 0.5;
  center.z = (object->max->z + object->min->z) * 0.5;
  object->max->x += (object->max->x - center.x) * CORRECTION;
  object->max->y += (object->max->y - center.y) * CORRECTION;
  object->max->z += (object->max->z - center.z) * CORRECTION;
  object->min->x += (object->min->x - center.x) * CORRECTION;
  object->min->y += (object->min->y - center.y) * CORRECTION;
  object->min->z += (object->min->z - center.z) * CORRECTION;
}

/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd           - portability
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
 *    RAY TRACING - Cone/Cylinder - Version 8.3.3                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, April 1992             *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Cone/Cylinder *****/
real
cone_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        slope, delta, k, k0, distance1, distance2, temp;
  xyz_struct      new_position, new_vector, v, p, d;
  cone_ptr        cone;

  if (object->transf != NULL)
  {
    transform(object->transf, position, &new_position);
    transform_vector(object->transf, position, vector, &new_position,
                     &new_vector);
    NORMALIZE(new_vector);
  } else
  {
    STRUCT_ASSIGN(new_position, *position);
    STRUCT_ASSIGN(new_vector, *vector);
  }
  REALINC(cone_tests);
  cone = (cone_ptr) object->data;
  slope = cone->slope;
  v.x = new_position.x - cone->base.x;
  v.y = new_position.y - cone->base.y;
  v.z = new_position.z - cone->base.z;
  p.x = DOT_PRODUCT(v, cone->u);
  p.y = DOT_PRODUCT(v, cone->v);
  p.z = DOT_PRODUCT(v, cone->w);
  d.x = DOT_PRODUCT(new_vector, cone->u);
  d.y = DOT_PRODUCT(new_vector, cone->v);
  d.z = DOT_PRODUCT(new_vector, cone->w);
  if (slope > -ROUNDOFF)
  {
    /* Cylinder */
    temp = cone->base_radius;
    k0 = 2.0 * (SQR(d.x) + SQR(d.y));
    k = 2.0 * (p.x * d.x + p.y * d.y);
  } else
  {
    /* Cone */
    temp = slope * p.z + cone->base_radius;
    k0 = 2.0 * (SQR(d.x) + SQR(d.y) - SQR(slope) * SQR(d.z));
    k = 2.0 * (p.x * d.x + p.y * d.y - slope * d.z * temp);
  }
  if (ABS(k0) < ROUNDOFF)
  {
    if (ABS(k) < ROUNDOFF)
      return 0.0;
    distance1 = (SQR(temp) - SQR(p.x) - SQR(p.y)) / k;
    if (distance1 <= threshold_distance)
      return 0.0;
  } else
  {
    delta = SQR(k) - 2.0 * k0 * (SQR(p.x) + SQR(p.y) - SQR(temp));
    if (delta < 0.0)
      return 0.0;
    delta = SQRT(delta);
    distance1 = (-k + delta) / k0;
    if (distance1 <= threshold_distance)
      return 0.0;
    distance2 = (-k - delta) / k0;
    if (distance2 > threshold_distance)
    {
      p.x = distance2 * new_vector.x + new_position.x;
      p.y = distance2 * new_vector.y + new_position.y;
      p.z = distance2 * new_vector.z + new_position.z;
      k = DOT_PRODUCT(cone->w, p);
      if ((k >= cone->min_d) AND(k <= cone->max_d))
      {
        if (object->transf != NULL)
        {
          STRUCT_ASSIGN(*(cone->position), p);
          return transform_distance(object->inv_transf, distance2,
                                    &new_position, &new_vector, position);
        } else
          return distance2;
      }
      p.x = distance1 * new_vector.x + new_position.x;
      p.y = distance1 * new_vector.y + new_position.y;
      p.z = distance1 * new_vector.z + new_position.z;
      k = DOT_PRODUCT(cone->w, p);
      if ((k >= cone->min_d) AND(k <= cone->max_d))
      {
        if (object->transf != NULL)
        {
          STRUCT_ASSIGN(*(cone->position), p);
          return transform_distance(object->inv_transf, distance1,
                                    &new_position, &new_vector, position);
        } else
          return distance1;
      }
      return 0.0;
    }
  }
  p.x = distance1 * new_vector.x + new_position.x;
  p.y = distance1 * new_vector.y + new_position.y;
  p.z = distance1 * new_vector.z + new_position.z;
  k = DOT_PRODUCT(cone->w, p);
  if ((k >= cone->min_d) AND(k <= cone->max_d))
  {
    if (object->transf != NULL)
    {
      STRUCT_ASSIGN(*(cone->position), p);
      return transform_distance(object->inv_transf, distance1,
                                &new_position, &new_vector, position);
    } else
      return distance1;
  }
  return 0.0;
}
void
cone_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  REG real        k;
  xyz_struct      old_position, new_normal;
  cone_ptr        cone;

  cone = (cone_ptr) object->data;
  if (object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(cone->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  k = -(DOT_PRODUCT(old_position, cone->w) + cone->base_d);
  normal->x = k * cone->w.x + old_position.x - cone->base.x;
  normal->y = k * cone->w.y + old_position.y - cone->base.y;
  normal->z = k * cone->w.z + old_position.z - cone->base.z;
  NORMALIZE(*normal);
  if (cone->slope > -ROUNDOFF)
  {
    if (object->transf != NULL)
    {
      transform_normal_vector(object->transf, cone->position, normal,
                              &new_normal);
      STRUCT_ASSIGN(*normal, new_normal);
      NORMALIZE(*normal);
    }
    return;
  }
  k = -cone->slope;
  normal->x = k * cone->w.x + normal->x;
  normal->y = k * cone->w.y + normal->y;
  normal->z = k * cone->w.z + normal->z;
  if (object->transf != NULL)
  {
    transform_normal_vector(object->transf, cone->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
  }
  NORMALIZE(*normal);
}
void
cone_enclose(object)
  object_ptr      object;
{
  REG real        k, base_delta, apex_delta;
  real            x, y, z, xs, ys, zs;
  xyz_struct      temp, vertex;
  cone_ptr        cone;

  cone = (cone_ptr) object->data;
  k = SQRT(1.0 - SQR(cone->w.x));
  apex_delta = k * cone->apex_radius + threshold_distance;
  base_delta = k * cone->base_radius + threshold_distance;
  object->min->x = MIN(cone->base.x - base_delta, cone->apex.x - apex_delta);
  object->max->x = MAX(cone->base.x + base_delta, cone->apex.x + apex_delta);
  k = SQRT(1.0 - SQR(cone->w.y));
  apex_delta = k * cone->apex_radius + threshold_distance;
  base_delta = k * cone->base_radius + threshold_distance;
  object->min->y = MIN(cone->base.y - base_delta, cone->apex.y - apex_delta);
  object->max->y = MAX(cone->base.y + base_delta, cone->apex.y + apex_delta);
  k = SQRT(1.0 - SQR(cone->w.z));
  apex_delta = k * cone->apex_radius + threshold_distance;
  base_delta = k * cone->base_radius + threshold_distance;
  object->min->z = MIN(cone->base.z - base_delta, cone->apex.z - apex_delta);
  object->max->z = MAX(cone->base.z + base_delta, cone->apex.z + apex_delta);
  if (object->transf != NULL)
  {
    ALLOCATE(cone->position, xyz_struct, 1, PARSE_TYPE);
    x = (object->min->x + object->max->x) * 0.5;
    y = (object->min->y + object->max->y) * 0.5;
    z = (object->min->z + object->max->z) * 0.5;
    xs = (object->max->x - object->min->x) * 0.5;
    ys = (object->max->y - object->min->y) * 0.5;
    zs = (object->max->z - object->min->z) * 0.5;
    temp.x = x - xs;
    temp.y = y - ys;
    temp.z = z - zs;
    transform(object->inv_transf, &temp, object->min);
    STRUCT_ASSIGN(*(object->max), *(object->min));
    temp.x = x - xs;
    temp.y = y - ys;
    temp.z = z + zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x - xs;
    temp.y = y + ys;
    temp.z = z - zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x - xs;
    temp.y = y + ys;
    temp.z = z + zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x + xs;
    temp.y = y - ys;
    temp.z = z - zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x + xs;
    temp.y = y - ys;
    temp.z = z + zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x + xs;
    temp.y = y + ys;
    temp.z = z - zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
    temp.x = x + xs;
    temp.y = y + ys;
    temp.z = z + zs;
    transform(object->inv_transf, &temp, &vertex);
    if (vertex.x < object->min->x)
      object->min->x = vertex.x;
    if (vertex.x > object->max->x)
      object->max->x = vertex.x;
    if (vertex.y < object->min->y)
      object->min->y = vertex.y;
    if (vertex.y > object->max->y)
      object->max->y = vertex.y;
    if (vertex.z < object->min->z)
      object->min->z = vertex.z;
    if (vertex.z > object->max->z)
      object->max->z = vertex.z;
  }
}

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
 *    RAY TRACING - Box - Version 8.3.3                               *
 *                                                                    *
 *    MADE BY    : Augusto Sousa, INESC-Norte, December 1989          *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, April 1990             *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Box (Parallelipiped) *****/
#define X_POSITIVE (1)
#define X_NEGATIVE (2)
#define Y_POSITIVE (3)
#define Y_NEGATIVE (4)
#define Z_POSITIVE (5)
#define Z_NEGATIVE (6)

real
box_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        distance1, distance2, temp;
  short int       small_hit, big_hit, small, big;
  real            small_distance, big_distance;
  xyz_struct      new_position, new_vector, low, high, center, size;
  box_ptr         box;

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
  REALINC(box_tests);
  box = (box_ptr) object->data;
  STRUCT_ASSIGN(center, box->center);
  STRUCT_ASSIGN(size, box->size);
  low.x = center.x - size.x;
  low.y = center.y - size.y;
  low.z = center.z - size.z;
  high.x = center.x + size.x;
  high.y = center.y + size.y;
  high.z = center.z + size.z;
  small_distance = -INFINITY;
  big_distance = INFINITY;
  if (ABS(new_vector.x) < ROUNDOFF)
  {
    if ((new_position.x <= low.x) OR(new_position.x >= high.x))
      return 0.0;
  } else
  {
    small_distance = (low.x - new_position.x) / new_vector.x;
    big_distance = (high.x - new_position.x) / new_vector.x;
    if (small_distance > big_distance)
    {
      temp = small_distance;
      small_distance = big_distance;
      big_distance = temp;
      small_hit = X_POSITIVE;
      big_hit = X_NEGATIVE;
    } else
    {
      small_hit = X_NEGATIVE;
      big_hit = X_POSITIVE;
    }
    if (big_distance < threshold_distance)
      return 0.0;
  }
  if (ABS(new_vector.y) < ROUNDOFF)
  {
    if ((new_position.y <= low.y) OR(new_position.y >= high.y))
      return 0.0;
  } else
  {
    distance1 = (low.y - new_position.y) / new_vector.y;
    distance2 = (high.y - new_position.y) / new_vector.y;
    if (distance1 > distance2)
    {
      temp = distance1;
      distance1 = distance2;
      distance2 = temp;
      small = Y_POSITIVE;
      big = Y_NEGATIVE;
    } else
    {
      small = Y_NEGATIVE;
      big = Y_POSITIVE;
    }
    if (distance1 > small_distance)
    {
      small_distance = distance1;
      small_hit = small;
    }
    if (distance2 < big_distance)
    {
      big_distance = distance2;
      big_hit = big;
    }
    if ((small_distance > big_distance) OR(big_distance < threshold_distance))
      return 0.0;
  }
  if (ABS(new_vector.z) < ROUNDOFF)
  {
    if ((new_position.z <= low.z) OR(new_position.z >= high.z))
      return 0.0;
  } else
  {
    distance1 = (low.z - new_position.z) / new_vector.z;
    distance2 = (high.z - new_position.z) / new_vector.z;
    if (distance1 > distance2)
    {
      temp = distance1;
      distance1 = distance2;
      distance2 = temp;
      small = Z_POSITIVE;
      big = Z_NEGATIVE;
    } else
    {
      small = Z_NEGATIVE;
      big = Z_POSITIVE;
    }
    if (distance1 > small_distance)
    {
      small_distance = distance1;
      small_hit = small;
    }
    if (distance2 < big_distance)
    {
      big_distance = distance2;
      big_hit = big;
    }
    if ((small_distance > big_distance) OR(big_distance < threshold_distance))
      return 0.0;
  }
  if (small_distance < threshold_distance)
  {
    box->side_hit = big_hit;
    if (object->transf != NULL)
    {
      box->position->x = new_position.x + big_distance * new_vector.x;
      box->position->y = new_position.y + big_distance * new_vector.y;
      box->position->z = new_position.z + big_distance * new_vector.z;
      return transform_distance(object->inv_transf, big_distance,
                                &new_position, &new_vector, position);
    } else
      return big_distance;
  }
  box->side_hit = small_hit;
  if (object->transf != NULL)
  {
    box->position->x = new_position.x + small_distance * new_vector.x;
    box->position->y = new_position.y + small_distance * new_vector.y;
    box->position->z = new_position.z + small_distance * new_vector.z;
    return transform_distance(object->inv_transf, small_distance,
                              &new_position, &new_vector, position);
  } else
    return small_distance;
}
void
box_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  xyz_struct      new_normal;
  box_ptr         box;

  box = (box_ptr) object->data;
  normal->x = 0.0;
  normal->y = 0.0;
  normal->z = 0.0;
  switch (box->side_hit)
  {
  case X_POSITIVE:
    normal->x = 1.0;
    break;
  case X_NEGATIVE:
    normal->x = -1.0;
    break;
  case Y_POSITIVE:
    normal->y = 1.0;
    break;
  case Y_NEGATIVE:
    normal->y = -1.0;
    break;
  case Z_POSITIVE:
    normal->z = 1.0;
    break;
  case Z_NEGATIVE:
    normal->z = -1.0;
    break;
  }
  if (object->transf != NULL)
  {
    transform_normal_vector(object->transf, box->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
}
void
box_enclose(object)
  object_ptr      object;
{
  REG real        x, y, z, xs, ys, zs;
  xyz_struct      temp, vertex;
  box_ptr         box;

  box = (box_ptr) object->data;
  x = box->center.x;
  y = box->center.y;
  z = box->center.z;
  xs = box->size.x + threshold_distance;
  ys = box->size.y + threshold_distance;
  zs = box->size.z + threshold_distance;
  if (object->transf != NULL)
  {
    ALLOCATE(box->position, xyz_struct, 1, PARSE_TYPE);
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
  } else
  {
    object->max->x = x + xs;
    object->max->y = y + ys;
    object->max->z = z + zs;
    object->min->x = x - xs;
    object->min->y = y - ys;
    object->min->z = z - zs;
  }
}

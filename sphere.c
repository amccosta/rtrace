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
 *    RAY TRACING - Sphere - Version 8.3.3                            *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Sphere *****/
real
sphere_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        k, delta, distance;
  xyz_struct      new_position, new_vector, p;
  sphere_ptr      sphere;

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
  REALINC(sphere_tests);
  sphere = (sphere_ptr) object->data;
  p.x = new_position.x - sphere->center.x;
  p.y = new_position.y - sphere->center.y;
  p.z = new_position.z - sphere->center.z;
  k = DOT_PRODUCT(p, new_vector);
  delta = SQR(k) - SQR(p.x) - SQR(p.y) - SQR(p.z) + sphere->radius2;
  if (delta < 0.0)
    return 0.0;
  delta = SQRT(delta);
  distance = -k - delta;
  if (distance > threshold_distance)
  {
    if (object->transf != NULL)
    {
      sphere->position->x = new_position.x + distance * new_vector.x;
      sphere->position->y = new_position.y + distance * new_vector.y;
      sphere->position->z = new_position.z + distance * new_vector.z;
      return transform_distance(object->inv_transf, distance,
                                &new_position, &new_vector, position);
    } else
      return distance;
  }
  distance = -k + delta;
  if (distance > threshold_distance)
  {
    if (object->transf != NULL)
    {
      sphere->position->x = new_position.x + distance * new_vector.x;
      sphere->position->y = new_position.y + distance * new_vector.y;
      sphere->position->z = new_position.z + distance * new_vector.z;
      return transform_distance(object->inv_transf, distance,
                                &new_position, &new_vector, position);
    } else
      return distance;
  }
  return 0.0;
}
void
sphere_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  xyz_struct      old_position, new_normal;
  sphere_ptr      sphere;

  sphere = (sphere_ptr) object->data;
  if (object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(sphere->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  normal->x = old_position.x - sphere->center.x;
  normal->y = old_position.y - sphere->center.y;
  normal->z = old_position.z - sphere->center.z;
  if (object->transf != NULL)
  {
    transform_normal_vector(object->transf, sphere->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  } else
  {
    normal->x *= sphere->radius;
    normal->y *= sphere->radius;
    normal->z *= sphere->radius;
  }
}
void
sphere_enclose(object)
  object_ptr      object;
{
  REG real        x, y, z, r;
  xyz_struct      temp, vertex;
  sphere_ptr      sphere;

  sphere = (sphere_ptr) object->data;
  r = sphere->radius + threshold_distance;
  sphere->radius = 1.0 / sphere->radius;
  x = sphere->center.x;
  y = sphere->center.y;
  z = sphere->center.z;
  if (object->transf != NULL)
  {
    ALLOCATE(sphere->position, xyz_struct, 1, PARSE_TYPE);
    temp.x = x - r;
    temp.y = y - r;
    temp.z = z - r;
    transform(object->inv_transf, &temp, object->min);
    STRUCT_ASSIGN(*(object->max), *(object->min));
    temp.x = x - r;
    temp.y = y - r;
    temp.z = z + r;
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
    temp.x = x - r;
    temp.y = y + r;
    temp.z = z - r;
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
    temp.x = x - r;
    temp.y = y + r;
    temp.z = z + r;
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
    temp.x = x + r;
    temp.y = y - r;
    temp.z = z - r;
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
    temp.x = x + r;
    temp.y = y - r;
    temp.z = z + r;
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
    temp.x = x + r;
    temp.y = y + r;
    temp.z = z - r;
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
    temp.x = x + r;
    temp.y = y + r;
    temp.z = z + r;
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
    object->max->x = x + r;
    object->max->y = y + r;
    object->max->z = z + r;
    object->min->x = x - r;
    object->min->y = y - r;
    object->min->z = z - r;
  }
}

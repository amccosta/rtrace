/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paulo Almeida       - TEXT3D primitive
 *  Pedro Borges        - TEXT3D primitive
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
#include "pp_ext.h"

/**********************************************************************
 *    RAY TRACING - Text - Version 8.3.3                              *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, April 1992             *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Text *****/
real
text_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        distance;
  xyz_struct      new_position, new_vector;
  text_ptr        text;

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
  REALINC(text_tests);
  text = (text_ptr) object->data;
  distance = intersect_pp_obj(&new_position, &new_vector, object->min,
                              object->max, text->data);
  if (distance > threshold_distance)
  {
    if (object->transf != NULL)
    {
      text->position->x = new_position.x + distance * new_vector.x;
      text->position->y = new_position.y + distance * new_vector.y;
      text->position->z = new_position.z + distance * new_vector.z;
      return transform_distance(object->inv_transf, distance,
                                &new_position, &new_vector, position);
    } else
      return distance;
  }
  return 0.0;
}
void
text_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  xyz_struct      old_position, new_normal;
  text_ptr        text;

  text = (text_ptr) object->data;
  if (object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(text->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  normal_pp_obj(&old_position, text->data, normal);
  if (object->transf != NULL)
  {
    transform_normal_vector(object->transf, text->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
  }
  NORMALIZE(*normal);
}
void
text_enclose(object)
  object_ptr      object;
{
  xyz_struct      max, min, temp, vertex;
  text_ptr        text;

  if (object->transf != NULL)
  {
    text = (text_ptr) object->data;
    STRUCT_ASSIGN(max, *(object->max));
    STRUCT_ASSIGN(min, *(object->min));
    ALLOCATE(text->position, xyz_struct, 1, PARSE_TYPE);
    temp.x = min.x;
    temp.y = min.y;
    temp.z = min.z;
    transform(object->inv_transf, &temp, object->min);
    STRUCT_ASSIGN(*(object->max), *(object->min));
    temp.x = min.x;
    temp.y = min.y;
    temp.z = max.z;
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
    temp.x = min.x;
    temp.y = max.y;
    temp.z = min.z;
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
    temp.x = min.x;
    temp.y = max.y;
    temp.z = max.z;
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
    temp.x = max.x;
    temp.y = min.y;
    temp.z = min.z;
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
    temp.x = max.x;
    temp.y = min.y;
    temp.z = max.z;
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
    temp.x = max.x;
    temp.y = max.y;
    temp.z = min.z;
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
    temp.x = max.x;
    temp.y = max.y;
    temp.z = max.z;
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

/***** Local stuff *****/
short int         pp_surface_id;
real              pp_refraction;

void_ptr *
create_pp_obj(min, max)
  xyz_ptr        *min, *max;
{
  text_ptr        text;

  POSINC(objects);
  ALLOCATE(object[objects], object_struct, 1, PARSE_TYPE);
  object[objects]->id = objects;
  object[objects]->surface_id = pp_surface_id;
  object[objects]->refraction = pp_refraction;
  ALLOCATE(object[objects]->min, xyz_struct, 1, PARSE_TYPE);
  ALLOCATE(object[objects]->max, xyz_struct, 1, PARSE_TYPE);
  object[objects]->transf = NULL;
  object[objects]->inv_transf = NULL;
  object[objects]->texture = NULL;
  object[objects]->texture_modify_normal = FALSE;
  object[objects]->object_type = TEXT_TYPE;
  ALLOCATE(text, text_struct, 1, PARSE_TYPE);
  object[objects]->data = (void_ptr) text;
  *min = object[objects]->min;
  *max = object[objects]->max;
  return &(text->data);
}

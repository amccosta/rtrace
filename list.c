/*
 * Copyright (c) 1992, 1993 Antonio Costa, INESC-Norte.
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
 *    RAY TRACING - List - Version 8.3.3                              *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, July 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Text *****/
real
list_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        distance, tmp_distance;
  xyz_struct      new_position, new_vector;
  list_ptr        list;
  list_node_ptr   node;
  object_ptr      tmp_object;
  int             octant;
  boolean         intersect;

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
  REALINC(list_tests);
  list = (list_ptr) object->data;
  distance = INFINITY;
  node = list->head;
  list->object = NULL;
  while (node != NULL)
  {
    tmp_object = node->object;
    FIND_OCTANT(octant, new_vector);
    intersect = octant_intersect(octant, &new_position, &new_vector,
                                 tmp_object->min, tmp_object->max);
    if (intersect AND CHECK_BOUNDS(tmp_object->object_type))
    {
      tmp_distance = bound_intersect(&new_position, &new_vector,
                                     tmp_object->min, tmp_object->max);
      if ((tmp_distance <= 0.0) OR(tmp_distance > distance))
        intersect = FALSE;
    }
    if (intersect)
    {
      OBJECT_INTERSECT(tmp_distance, &new_position, &new_vector, tmp_object);
      if ((tmp_distance > threshold_distance) AND(tmp_distance < distance))
      {
        distance = tmp_distance;
        list->object = tmp_object;
        if (list->surface_id < 0)
          object->surface_id = tmp_object->surface_id;
        if (list->refraction < 0.0)
          object->refraction = tmp_object->refraction;
      }
    }
    node = (list_node_ptr) node->next;
  }
  if (list->object == NULL)
    return 0.0;
  if (object->transf != NULL)
  {
    list->position->x = new_position.x + distance * new_vector.x;
    list->position->y = new_position.y + distance * new_vector.y;
    list->position->z = new_position.z + distance * new_vector.z;
    return transform_distance(object->inv_transf, distance,
                              &new_position, &new_vector, position);
  } else
    return distance;
}
void
list_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  xyz_struct      old_position, new_normal;
  list_ptr        list;

  list = (list_ptr) object->data;
  if (object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(list->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  list = (list_ptr) object->data;
  OBJECT_NORMAL(&old_position, list->object, normal);
  if (object->transf != NULL)
  {
    transform_normal_vector(object->transf, list->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
}
void
list_enclose(object)
  object_ptr      object;
{
  xyz_struct      max, min, temp, vertex;
  list_ptr        list;
  list_node_ptr   node;
  object_ptr      tmp_object;

  list = (list_ptr) object->data;
  list->surface_id = object->surface_id;
  list->refraction = object->refraction;
  node = list->head;
  tmp_object = node->object;
  OBJECT_ENCLOSE(tmp_object);
  validate_object(tmp_object, 0);
  STRUCT_ASSIGN(*(object->min), *(tmp_object->min));
  STRUCT_ASSIGN(*(object->max), *(tmp_object->max));
  node = (list_node_ptr) node->next;
  while (node != NULL)
  {
    tmp_object = node->object;
    OBJECT_ENCLOSE(tmp_object);
    validate_object(tmp_object, 0);
    object->min->x = MIN(object->min->x, tmp_object->min->x);
    object->min->y = MIN(object->min->y, tmp_object->min->y);
    object->min->z = MIN(object->min->z, tmp_object->min->z);
    object->max->x = MAX(object->max->x, tmp_object->max->x);
    object->max->y = MAX(object->max->y, tmp_object->max->y);
    object->max->z = MAX(object->max->z, tmp_object->max->z);
    node = (list_node_ptr) node->next;
  }
  if (object->transf != NULL)
  {
    STRUCT_ASSIGN(max, *(object->max));
    STRUCT_ASSIGN(min, *(object->min));
    ALLOCATE(list->position, xyz_struct, 1, PARSE_TYPE);
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

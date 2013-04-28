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
 *    RAY TRACING - Triangle - Version 8.3.3                          *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Triangle *****/
real
triangle_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  REG real        distance, k, k0;
  xyz_struct      p;
  triangle_ptr    triangle;

  REALINC(triangle_tests);
  triangle = (triangle_ptr) object->data;
  k = DOT_PRODUCT(*vector, triangle->transf[2]);
  if (ABS(k) >= ROUNDOFF)
  {
    p.x = triangle->coords[0].x - position->x;
    p.y = triangle->coords[0].y - position->y;
    p.z = triangle->coords[0].z - position->z;
    distance = DOT_PRODUCT(p, triangle->transf[2]) / k;
    if (distance > threshold_distance)
    {
      p.x = distance * vector->x + position->x - triangle->coords[0].x;
      p.y = distance * vector->y + position->y - triangle->coords[0].y;
      p.z = distance * vector->z + position->z - triangle->coords[0].z;
      k = DOT_PRODUCT(p, triangle->transf[0]);
      k0 = DOT_PRODUCT(p, triangle->transf[1]);
      if ((k >= 0.0) AND(k0 >= 0.0) AND(k + k0 <= 1.0))
      {
        triangle->u_hit = k;
        triangle->v_hit = k0;
        return distance;
      }
      return 0.0;
    }
    return 0.0;
  }
  return 0.0;
}
#define INTERP_RGB(attrib) \
surface[surfaces]->attrib.r =\
t * surface[triangle->surface[0]]->attrib.r +\
u * surface[triangle->surface[1]]->attrib.r +\
v * surface[triangle->surface[2]]->attrib.r;\
surface[surfaces]->attrib.g =\
t * surface[triangle->surface[0]]->attrib.g +\
u * surface[triangle->surface[1]]->attrib.g +\
v * surface[triangle->surface[2]]->attrib.g;\
surface[surfaces]->attrib.b =\
t * surface[triangle->surface[0]]->attrib.b +\
u * surface[triangle->surface[1]]->attrib.b +\
v * surface[triangle->surface[2]]->attrib.b

#define INTERP(attrib) \
surface[surfaces]->attrib =\
t * surface[triangle->surface[0]]->attrib +\
u * surface[triangle->surface[1]]->attrib +\
v * surface[triangle->surface[2]]->attrib

void
triangle_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  REG real        t, u, v;
  triangle_ptr    triangle;

  triangle = (triangle_ptr) object->data;
  u = triangle->u_hit;
  v = triangle->v_hit;
  t = 1.0 - u - v;
  normal->x = t * triangle->normal[0].x + u * triangle->normal[1].x +
    v * triangle->normal[2].x;
  normal->y = t * triangle->normal[0].y + u * triangle->normal[1].y +
    v * triangle->normal[2].y;
  normal->z = t * triangle->normal[0].z + u * triangle->normal[1].z +
    v * triangle->normal[2].z;
  NORMALIZE(*normal);
  if (triangle->surface != NULL)
  {
    INTERP_RGB(color);
    INTERP_RGB(diffuse);
    INTERP_RGB(specular);
    INTERP_RGB(transparent);
    INTERP_RGB(metal_factor);
    INTERP_RGB(diffuse_factor);
    INTERP(phong_factor);
  }
}
static void
load_triangle(triangle)
  triangle_ptr    triangle;
{
  int             i;
  REG real        k;
  xyz_struct      temp[3];

  NORMALIZE(triangle->normal[0]);
  NORMALIZE(triangle->normal[1]);
  NORMALIZE(triangle->normal[2]);
  temp[0].x = triangle->coords[1].x - triangle->coords[0].x;
  temp[0].y = triangle->coords[1].y - triangle->coords[0].y;
  temp[0].z = triangle->coords[1].z - triangle->coords[0].z;
  temp[1].x = triangle->coords[2].x - triangle->coords[0].x;
  temp[1].y = triangle->coords[2].y - triangle->coords[0].y;
  temp[1].z = triangle->coords[2].z - triangle->coords[0].z;
  CROSS_PRODUCT(temp[2], temp[0], temp[1]);
  NORMALIZE(temp[2]);
  k = 1.0 /
    (temp[0].x * temp[1].y * temp[2].z + temp[0].y * temp[1].z * temp[2].x +
     temp[0].z * temp[1].x * temp[2].y - temp[0].z * temp[1].y * temp[2].x -
     temp[0].x * temp[1].z * temp[2].y - temp[0].y * temp[1].x * temp[2].z);
  triangle->transf[0].x = (temp[1].y * temp[2].z - temp[1].z * temp[2].y);
  triangle->transf[1].x = -(temp[0].y * temp[2].z - temp[0].z * temp[2].y);
  triangle->transf[2].x = (temp[0].y * temp[1].z - temp[0].z * temp[1].y);
  triangle->transf[0].y = -(temp[1].x * temp[2].z - temp[1].z * temp[2].x);
  triangle->transf[1].y = (temp[0].x * temp[2].z - temp[0].z * temp[2].x);
  triangle->transf[2].y = -(temp[0].x * temp[1].z - temp[0].z * temp[1].x);
  triangle->transf[0].z = (temp[1].x * temp[2].y - temp[1].y * temp[2].x);
  triangle->transf[1].z = -(temp[0].x * temp[2].y - temp[0].y * temp[2].x);
  triangle->transf[2].z = (temp[0].x * temp[1].y - temp[0].y * temp[1].x);
  for (i = 0; i < 3; POSINC(i))
  {
    triangle->transf[i].x *= k;
    triangle->transf[i].y *= k;
    triangle->transf[i].z *= k;
  }
}
void
triangle_enclose(object)
  object_ptr      object;
{
  REG int         i;
  xyz_struct      temp1, temp2;
  triangle_ptr    triangle;

  triangle = (triangle_ptr) object->data;
  if (object->transf != NULL)
  {
    for (i = 0; i < 3; POSINC(i))
    {
      transform(object->inv_transf, &(triangle->coords[i]), &temp1);
      transform_vector(object->inv_transf, &(triangle->coords[i]),
                       &(triangle->normal[i]), &temp1, &temp2);
      STRUCT_ASSIGN(triangle->coords[i], temp1);
      STRUCT_ASSIGN(triangle->normal[i], temp2);
    }
    FREE(object->transf);
    object->transf = NULL;
  }
  load_triangle(triangle);
  STRUCT_ASSIGN(*(object->min), triangle->coords[0]);
  STRUCT_ASSIGN(*(object->max), triangle->coords[0]);
  for (i = 1; i < 3; POSINC(i))
  {
    object->max->x = MAX(object->max->x, triangle->coords[i].x);
    object->max->y = MAX(object->max->y, triangle->coords[i].y);
    object->max->z = MAX(object->max->z, triangle->coords[i].z);
    object->min->x = MIN(object->min->x, triangle->coords[i].x);
    object->min->y = MIN(object->min->y, triangle->coords[i].y);
    object->min->z = MIN(object->min->z, triangle->coords[i].z);
  }
  /* Adjust dimensions */
  object->max->x += threshold_distance;
  object->max->y += threshold_distance;
  object->max->z += threshold_distance;
  object->min->x -= threshold_distance;
  object->min->y -= threshold_distance;
  object->min->z -= threshold_distance;
  /* Adjust surface id */
  if (object->surface_id < 0)
    object->surface_id = surfaces;
}

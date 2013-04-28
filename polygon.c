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
 *    RAY TRACING - Polygon - Version 8.3.3                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

/***** Polygon *****/
real
polygon_intersect(position, vector, object)
  xyz_ptr         position, vector;
  object_ptr      object;
{
  boolean         qi, ri, qj, rj;
  REG int         cross, points, i, j, u, v;
  REG real        distance, k;
  real            temp[3];
  polygon_ptr     polygon;

  REALINC(polygon_tests);
  polygon = (polygon_ptr) object->data;
  k = DOT_PRODUCT(*vector, polygon->normal);
  if (ABS(k) >= ROUNDOFF)
  {
    distance = -(DOT_PRODUCT(*position, polygon->normal) + polygon->d) / k;
    if (distance > threshold_distance)
    {
      cross = 0;
      temp[0] = distance * vector->x + position->x;
      temp[1] = distance * vector->y + position->y;
      temp[2] = distance * vector->z + position->z;
      points = SUCC(polygon->points);
      u = polygon->u;
      v = polygon->v;
      for (i = 0; i <= polygon->points; POSINC(i))
      {
        j = SUCC(i) MOD points;
        qi = FALSE;
        ri = FALSE;
        qj = FALSE;
        rj = FALSE;
        if (polygon->coords[i][v] == polygon->coords[j][v])
          continue;
        if (polygon->coords[i][v] < temp[v])
          qi = TRUE;
        if (polygon->coords[j][v] < temp[v])
          qj = TRUE;
        if (qi == qj)
          continue;
        if (polygon->coords[i][u] < temp[u])
          ri = TRUE;
        if (polygon->coords[j][u] < temp[u])
          rj = TRUE;
        if (ri AND rj)
        {
          POSINC(cross);
          continue;
        }
        if ((NOT ri) AND(NOT rj))
          continue;
        k = (polygon->coords[j][v] - polygon->coords[i][v]) /
          (polygon->coords[j][u] - polygon->coords[i][u]);
        if ((temp[v] - polygon->coords[j][v] + k *
             (polygon->coords[j][u] - temp[u])) / k <= threshold_distance)
          POSINC(cross);
      }
      if (ODD(cross))
        return distance;
      return 0.0;
    }
    return 0.0;
  }
  return 0.0;
}
void
polygon_normal(position, object, normal)
  xyz_ptr         position;
  object_ptr      object;
  xyz_ptr         normal;
{
  polygon_ptr     polygon;

  polygon = (polygon_ptr) object->data;
  STRUCT_ASSIGN(*normal, polygon->normal);
}
static boolean
load_polygon(polygon)
  polygon_ptr     polygon;
{
  REG real        l;
  xyz_struct      p, p0;

  p.x = polygon->coords[1][0] - polygon->coords[0][0];
  p.y = polygon->coords[1][1] - polygon->coords[0][1];
  p.z = polygon->coords[1][2] - polygon->coords[0][2];
  p0.x = polygon->coords[2][0] - polygon->coords[0][0];
  p0.y = polygon->coords[2][1] - polygon->coords[0][1];
  p0.z = polygon->coords[2][2] - polygon->coords[0][2];
  CROSS_PRODUCT(polygon->normal, p, p0);
  l = LENGTH(polygon->normal);
  if (l < ROUNDOFF)
    return TRUE;
  polygon->normal.x /= l;
  polygon->normal.y /= l;
  polygon->normal.z /= l;
  STRUCT_ASSIGN(p, polygon->normal);
  if ((ABS(p.x) > ABS(p.y)) AND(ABS(p.x) > ABS(p.z)))
  {
    polygon->u = 1;
    polygon->v = 2;
  } else if ((ABS(p.y) > ABS(p.x)) AND(ABS(p.y) > ABS(p.z)))
  {
    polygon->u = 0;
    polygon->v = 2;
  } else
  {
    polygon->u = 0;
    polygon->v = 1;
  }
  p0.x = polygon->coords[0][0];
  p0.y = polygon->coords[0][1];
  p0.z = polygon->coords[0][2];
  polygon->d = -DOT_PRODUCT(p, p0);
  return FALSE;
}
void
polygon_enclose(object)
  object_ptr      object;
{
  REG int         i;
  xyz_struct      temp1, temp2;
  polygon_ptr     polygon;

  polygon = (polygon_ptr) object->data;
  if (object->transf != NULL)
  {
    for (i = 0; i <= polygon->points; POSINC(i))
    {
      temp1.x = polygon->coords[i][0];
      temp1.y = polygon->coords[i][1];
      temp1.z = polygon->coords[i][2];
      transform(object->inv_transf, &temp1, &temp2);
      polygon->coords[i][0] = temp2.x;
      polygon->coords[i][1] = temp2.y;
      polygon->coords[i][2] = temp2.z;
    }
    FREE(object->transf);
    object->transf = NULL;
  }
  if (load_polygon(polygon))
  {
    WRITE(results, "Object %d is a degenerate POLYGON\n", object->id);
    FLUSH(results);
    WRITE(results, "1st vertex COORDS: %g %g %g\n",
          polygon->coords[0][0],
          polygon->coords[0][1],
          polygon->coords[0][2]);
    FLUSH(results);
    WRITE(results, "2nd vertex COORDS: %g %g %g\n",
          polygon->coords[1][0],
          polygon->coords[1][1],
          polygon->coords[1][2]);
    FLUSH(results);
    runtime_abort("cannot process POLYGON");
  }
  object->min->x = INFINITY;
  object->min->y = INFINITY;
  object->min->z = INFINITY;
  object->max->x = -INFINITY;
  object->max->y = -INFINITY;
  object->max->z = -INFINITY;
  for (i = 0; i <= polygon->points; POSINC(i))
  {
    object->max->x = MAX(object->max->x, polygon->coords[i][0]);
    object->max->y = MAX(object->max->y, polygon->coords[i][1]);
    object->max->z = MAX(object->max->z, polygon->coords[i][2]);
    object->min->x = MIN(object->min->x, polygon->coords[i][0]);
    object->min->y = MIN(object->min->y, polygon->coords[i][1]);
    object->min->z = MIN(object->min->z, polygon->coords[i][2]);
  }
  /* Adjust dimensions */
  object->max->x += threshold_distance;
  object->max->y += threshold_distance;
  object->max->z += threshold_distance;
  object->min->x -= threshold_distance;
  object->min->y -= threshold_distance;
  object->min->z -= threshold_distance;
}

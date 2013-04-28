/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paulo Almeida       - improvements
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
 *    RAY TRACING - Intersect - Version 8.4.2                         *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1994          *
 **********************************************************************/

/***** Intersect objects *****/
#define CHECK_SMALLER(p1, p2, v1, v2)\
do {\
  if (((p1) < (p2)) OR((p1) * (v2) < (p2) * (v1)))\
    return FALSE;\
} while (0)
#define CHECK_BIGGER(p1, p2, v1, v2)\
do {\
  if (((p1) > (p2)) AND((p1) * (v2) > (p2) * (v1)))\
    return FALSE;\
} while (0)
boolean
octant_intersect(octant, position, vector, min, max)
  int             octant;
  REG xyz_ptr     position, vector, min, max;
{
  REG real        t1, t2;

  REALINC(octant_tests);
  switch (octant)
  {
  case 0:
    if ((min->x >= position->x) OR(min->y >= position->y)
        OR(min->z >= position->z))
      return FALSE;
    if ((vector->x <= vector->y) AND(vector->x <= vector->z))
    {
      t1 = -(min->x - position->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, -vector->x, -vector->y);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, -vector->x, -vector->z);
      t1 = -(max->x - position->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, -vector->x, -vector->y);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, -vector->x, -vector->z);
      return TRUE;
    } else
    if ((vector->y <= vector->x) AND(vector->y <= vector->z))
    {
      t1 = -(min->y - position->y);
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, -vector->y, -vector->x);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, -vector->y, -vector->z);
      t1 = -(max->y - position->y);
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, -vector->y, -vector->x);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, -vector->y, -vector->z);
      return TRUE;
    } else
    {
      t1 = -(min->z - position->z);
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, -vector->z, -vector->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, -vector->z, -vector->y);
      t1 = -(max->z - position->z);
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, -vector->z, -vector->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, -vector->z, -vector->y);
      return TRUE;
    }
  case 1:
    if ((max->x < position->x) OR(min->y >= position->y)
        OR(min->z >= position->z))
      return FALSE;
    if ((-vector->x <= vector->y) AND(-vector->x <= vector->z))
    {
      t1 = max->x - position->x;
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, vector->x, -vector->y);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, vector->x, -vector->z);
      t1 = min->x - position->x;
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, vector->x, -vector->y);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, vector->x, -vector->z);
      return TRUE;
    } else
    if ((vector->y <= -vector->x) AND(vector->y <= vector->z))
    {
      t1 = -(min->y - position->y);
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, -vector->y, vector->x);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, -vector->y, -vector->z);
      t1 = -(max->y - position->y);
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, -vector->y, vector->x);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, -vector->y, -vector->z);
      return TRUE;
    } else
    {
      t1 = -(min->z - position->z);
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, -vector->z, vector->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, -vector->z, -vector->y);
      t1 = -(max->z - position->z);
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, -vector->z, vector->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, -vector->z, -vector->y);
      return TRUE;
    }
  case 2:
    if ((min->x >= position->x) OR(max->y < position->y)
        OR(min->z >= position->z))
      return FALSE;
    if ((vector->x <= -vector->y) AND(vector->x <= vector->z))
    {
      t1 = -(min->x - position->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, -vector->x, vector->y);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, -vector->x, -vector->z);
      t1 = -(max->x - position->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, -vector->x, vector->y);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, -vector->x, -vector->z);
      return TRUE;
    } else
    if ((-vector->y <= vector->x) AND(-vector->y <= vector->z))
    {
      t1 = max->y - position->y;
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, vector->y, -vector->x);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, vector->y, -vector->z);
      t1 = min->y - position->y;
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, vector->y, -vector->x);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, vector->y, -vector->z);
      return TRUE;
    } else
    {
      t1 = -(min->z - position->z);
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, -vector->z, -vector->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, -vector->z, vector->y);
      t1 = -(max->z - position->z);
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, -vector->z, -vector->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, -vector->z, vector->y);
      return TRUE;
    }
  case 3:
    if ((max->x < position->x) OR(max->y < position->y)
        OR(min->z >= position->z))
      return FALSE;
    if ((vector->x >= vector->y) AND(vector->x >= -vector->z))
    {
      t1 = max->x - position->x;
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, vector->x, vector->y);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, vector->x, -vector->z);
      t1 = min->x - position->x;
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, vector->x, vector->y);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, vector->x, -vector->z);
      return TRUE;
    } else
    if ((vector->y >= vector->x) AND(vector->y >= -vector->z))
    {
      t1 = max->y - position->y;
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, vector->y, vector->x);
      t2 = -(max->z - position->z);
      CHECK_SMALLER(t1, t2, vector->y, -vector->z);
      t1 = min->y - position->y;
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, vector->y, vector->x);
      t2 = -(min->z - position->z);
      CHECK_BIGGER(t1, t2, vector->y, -vector->z);
      return TRUE;
    } else
    {
      t1 = -(min->z - position->z);
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, -vector->z, vector->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, -vector->z, vector->y);
      t1 = -(max->z - position->z);
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, -vector->z, vector->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, -vector->z, vector->y);
      return TRUE;
    }
  case 4:
    if ((min->x >= position->x) OR(min->y >= position->y)
        OR(max->z < position->z))
      return FALSE;
    if ((vector->x <= vector->y) AND(vector->x <= -vector->z))
    {
      t1 = -(min->x - position->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, -vector->x, -vector->y);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, -vector->x, vector->z);
      t1 = -(max->x - position->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, -vector->x, -vector->y);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, -vector->x, vector->z);
      return TRUE;
    } else
    if ((vector->y <= vector->x) AND(vector->y <= -vector->z))
    {
      t1 = -(min->y - position->y);
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, -vector->y, -vector->x);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, -vector->y, vector->z);
      t1 = -(max->y - position->y);
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, -vector->y, -vector->x);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, -vector->y, vector->z);
      return TRUE;
    } else
    {
      t1 = max->z - position->z;
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, vector->z, -vector->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, vector->z, -vector->y);
      t1 = min->z - position->z;
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, vector->z, -vector->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, vector->z, -vector->y);
      return TRUE;
    }
  case 5:
    if ((max->x < position->x) OR(min->y >= position->y)
        OR(max->z < position->z))
      return FALSE;
    if ((vector->x >= -vector->y) AND(vector->x >= vector->z))
    {
      t1 = max->x - position->x;
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, vector->x, -vector->y);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, vector->x, vector->z);
      t1 = min->x - position->x;
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, vector->x, -vector->y);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, vector->x, vector->z);
      return TRUE;
    } else
    if ((-vector->y >= vector->x) AND(-vector->y >= vector->z))
    {
      t1 = -(min->y - position->y);
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, -vector->y, vector->x);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, -vector->y, vector->z);
      t1 = -(max->y - position->y);
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, -vector->y, vector->x);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, -vector->y, vector->z);
      return TRUE;
    } else
    {
      t1 = max->z - position->z;
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, vector->z, vector->x);
      t2 = -(max->y - position->y);
      CHECK_SMALLER(t1, t2, vector->z, -vector->y);
      t1 = min->z - position->z;
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, vector->z, vector->x);
      t2 = -(min->y - position->y);
      CHECK_BIGGER(t1, t2, vector->z, -vector->y);
      return TRUE;
    }
  case 6:
    if ((min->x >= position->x) OR(max->y < position->y)
        OR(max->z < position->z))
      return FALSE;
    if ((-vector->x >= vector->y) AND(-vector->x >= vector->z))
    {
      t1 = -(min->x - position->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, -vector->x, vector->y);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, -vector->x, vector->z);
      t1 = -(max->x - position->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, -vector->x, vector->y);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, -vector->x, vector->z);
      return TRUE;
    } else
    if ((vector->y >= -vector->x) AND(vector->y >= vector->z))
    {
      t1 = max->y - position->y;
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, vector->y, -vector->x);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, vector->y, vector->z);
      t1 = min->y - position->y;
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, vector->y, -vector->x);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, vector->y, vector->z);
      return TRUE;
    } else
    {
      t1 = max->z - position->z;
      t2 = -(max->x - position->x);
      CHECK_SMALLER(t1, t2, vector->z, -vector->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, vector->z, vector->y);
      t1 = min->z - position->z;
      t2 = -(min->x - position->x);
      CHECK_BIGGER(t1, t2, vector->z, -vector->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, vector->z, vector->y);
      return TRUE;
    }
  case 7:
    if ((max->x < position->x) OR(max->y < position->y)
        OR(max->z < position->z))
      return FALSE;
    if ((vector->x >= vector->y) AND(vector->x >= vector->z))
    {
      t1 = max->x - position->x;
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, vector->x, vector->y);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, vector->x, vector->z);
      t1 = min->x - position->x;
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, vector->x, vector->y);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, vector->x, vector->z);
      return TRUE;
    } else
    if ((vector->y >= vector->x) AND(vector->y >= vector->z))
    {
      t1 = max->y - position->y;
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, vector->y, vector->x);
      t2 = min->z - position->z;
      CHECK_SMALLER(t1, t2, vector->y, vector->z);
      t1 = min->y - position->y;
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, vector->y, vector->x);
      t2 = max->z - position->z;
      CHECK_BIGGER(t1, t2, vector->y, vector->z);
      return TRUE;
    } else
    {
      t1 = max->z - position->z;
      t2 = min->x - position->x;
      CHECK_SMALLER(t1, t2, vector->z, vector->x);
      t2 = min->y - position->y;
      CHECK_SMALLER(t1, t2, vector->z, vector->y);
      t1 = min->z - position->z;
      t2 = max->x - position->x;
      CHECK_BIGGER(t1, t2, vector->z, vector->x);
      t2 = max->y - position->y;
      CHECK_BIGGER(t1, t2, vector->z, vector->y);
      return TRUE;
    }
  }
  return FALSE;
}
boolean
backface_check(position, vector, object)
  REG xyz_ptr     position, vector;
  object_ptr      object;
{
  polygon_ptr     polygon;
  triangle_ptr    triangle;

  REALINC(backface_tests);
  switch (object->object_type)
  {
  case POLYGON_TYPE:
    polygon = (polygon_ptr) (object->data);
    if (DOT_PRODUCT(*vector, polygon->normal) < 0.0)
      return FALSE;
    REALINC(backface_hits);
    return TRUE;
  case TRIANGLE_TYPE:
    triangle = (triangle_ptr) (object->data);
    if (DOT_PRODUCT(*vector, triangle->normal[0]) < 0.0)
      return FALSE;
    if (DOT_PRODUCT(*vector, triangle->normal[1]) < 0.0)
      return FALSE;
    if (DOT_PRODUCT(*vector, triangle->normal[2]) < 0.0)
      return FALSE;
    REALINC(backface_hits);
    return TRUE;
  default:
    return FALSE;
  }
}
real
bound_intersect(position, vector, min, max)
  REG xyz_ptr     position, vector, min, max;
{
  REG real        distance;
  xyz_struct      p, size;

  if ((position->x >= min->x)
      AND(position->x <= max->x)
      AND(position->y >= min->y)
      AND(position->y <= max->y)
      AND(position->z >= min->z)
      AND(position->z <= max->z))
    return ROUNDOFF;
  REALINC(bound_tests);
  p.x = position->x - (max->x + min->x) * 0.5;
  p.y = position->y - (max->y + min->y) * 0.5;
  p.z = position->z - (max->z + min->z) * 0.5;
  size.x = (max->x - min->x) * 0.5;
  size.y = (max->y - min->y) * 0.5;
  size.z = (max->z - min->z) * 0.5;
  if ((vector->x <= -ROUNDOFF) AND(position->x > max->x))
  {
    distance = (size.x - p.x) / vector->x;
    if ((ABS(p.y + distance * vector->y) <= size.y)
        AND(ABS(p.z + distance * vector->z) <= size.z))
      return distance;
  }
  else
  if ((vector->x >= ROUNDOFF) AND(position->x < min->x))
  {
    distance = (-size.x - p.x) / vector->x;
    if ((ABS(p.y + distance * vector->y) <= size.y)
        AND(ABS(p.z + distance * vector->z) <= size.z))
      return distance;
  }
  if ((vector->y <= -ROUNDOFF) AND(position->y > max->y))
  {
    distance = (size.y - p.y) / vector->y;
    if ((ABS(p.x + distance * vector->x) <= size.x)
        AND(ABS(p.z + distance * vector->z) <= size.z))
      return distance;
  }
  else
  if ((vector->y >= ROUNDOFF) AND(position->y < min->y))
  {
    distance = (-size.y - p.y) / vector->y;
    if ((ABS(p.x + distance * vector->x) <= size.x)
        AND(ABS(p.z + distance * vector->z) <= size.z))
      return distance;
  }
  if ((vector->z <= -ROUNDOFF) AND(position->z > max->z))
  {
    distance = (size.z - p.z) / vector->z;
    if ((ABS(p.x + distance * vector->x) <= size.x)
        AND(ABS(p.y + distance * vector->y) <= size.y))
      return distance;
  }
  else
  if ((vector->z >= ROUNDOFF) AND(position->z < min->z))
  {
    distance = (-size.z - p.z) / vector->z;
    if ((ABS(p.x + distance * vector->x) <= size.x)
        AND(ABS(p.y + distance * vector->y) <= size.y))
      return distance;
  }
  return 0.0;
}
void
cluster_intersect(octant, source_id, cached_id, position, vector, cluster,
                  max_distance, intersect_id, change)
  int             octant, source_id, cached_id;
  REG xyz_ptr     position, vector;
  cluster_ptr     cluster;
  real           *max_distance;
  int            *intersect_id;
  boolean         change;
{
  REG int         i;
  REG real        distance;
  object_ptr      object;

  if ((intersect_id != NULL) AND(*intersect_id < 0))
    return;
  for (i = 0; i < cluster->size; POSINC(i))
  {
    object = cluster->object[i];
    if (((object->id == source_id) OR(object->id == cached_id))
        AND CONVEX(object->object_type)
        AND(NOT COLOR_BIG(surface[object->surface_id]->transparent, ROUNDOFF)
            OR NOT SELF_INTERSECT(object->object_type, TRUE)))
      continue;
    if (object->max == NULL)
    {
      cluster_intersect(octant, source_id, cached_id, position, vector,
                        (cluster_ptr) object->data, max_distance,
                        intersect_id, change);
      continue;
    }
    if (NOT octant_intersect(octant, position, vector,
                             object->min, object->max))
      continue;
    if ((backface_mode != 0) AND POLYGONAL(object->object_type))
      if (backface_check(position, vector, object))
        continue;
    if (object->object_type == CLUSTER_TYPE)
    {
      distance = bound_intersect(position, vector, object->min, object->max);
      if ((distance > 0.0) AND(distance < *max_distance))
        cluster_intersect(octant, source_id, cached_id, position, vector,
                          (cluster_ptr) object->data, max_distance,
                          intersect_id, change);
      continue;
    }
    if (CHECK_BOUNDS(object->object_type))
    {
      distance = bound_intersect(position, vector, object->min, object->max);
      if ((distance <= 0.0) OR(distance >= *max_distance))
        continue;
      if ((light_mode != 0) OR change)
      {
        pqueue_insert(distance, object);
        continue;
      }
      /* Avoid the complex objects */
      switch (object->object_type)
      {
        case PATCH_TYPE:
        case TEXT_TYPE:
        case CSG_TYPE:
        case LIST_TYPE:
          pqueue_insert(distance, object);
          continue;
        default:
          break;
      }
    }
    OBJECT_INTERSECT(distance, position, vector, object);
    if ((distance <= 0.0) OR(distance >= *max_distance))
      continue;
    if (change)
    {
      *intersect_id = object->id;
      *max_distance = distance;
    } else
    {
      if ((light_mode == 0) AND(intersect_id != NULL))
      {
        *intersect_id = -(object->id);
        return;
      } else
        pqueue_insert(distance, object);
    }
  }
}
void
reset_ray_cache(level, node)
  REG int         level, node;
{
  REG int         next;

  ray_cache[node] = NO_OBJECTS;
  next = 1 SHL level;
  POSINC(level);
  if (level < RAY_CACHE_LEVEL_MAX)
  {
    node += next;
    if (ray_cache[node] != NO_OBJECTS)
      reset_ray_cache(level, node);
    node += next;
    if (ray_cache[node] != NO_OBJECTS)
      reset_ray_cache(level, node);
  }
}
#ifndef ISNAN
#ifdef __GNUC__
#define ISNAN(X) ((X) != (X))
#else
#define ISNAN(X) (0)
#endif
#endif

real
intersect_all(source_id, position, ray, color)
  int             source_id;
  xyz_ptr         position;
  ray_ptr         ray;
  rgb_ptr         color;
{
  REG boolean     intersect;
  REG int         octant, intersect_id;
  int             id;
  real            distance, small_distance;
  REG real        max_distance;
  xyz_struct      vector, hit, normal;
  object_ptr      intersect_object;

  STRUCT_ASSIGN(vector, ray->vector);
  FIND_OCTANT(octant, vector);
  PQUEUE_INITIALIZE;
  /* Check cached object */
  if ((shade_level < RAY_CACHE_LEVEL_MAX)
      AND(ray_cache[ray_node] != NO_OBJECTS))
  {
    intersect_object = object[ray_cache[ray_node]];
    intersect = octant_intersect(octant, position, &vector,
                                 intersect_object->min, intersect_object->max);
    if (intersect AND(backface_mode != 0)
        AND POLYGONAL(intersect_object->object_type))
      intersect = NOT backface_check(position, &vector, intersect_object);
    if (intersect AND CHECK_BOUNDS(intersect_object->object_type))
      intersect = (boolean)
        (bound_intersect(position, &vector, intersect_object->min,
                         intersect_object->max) > 0.0);
    if (intersect)
    {
      OBJECT_INTERSECT(max_distance, position, &vector, intersect_object);
      intersect = (boolean) (max_distance > 0.0);
    }
    if (intersect)
      REALINC(ray_cache_hits);
    else
    {
      REALINC(ray_cache_resets);
      max_distance = INFINITY;
      reset_ray_cache(shade_level, ray_node);
    }
    intersect_id = ray_cache[ray_node];
  } else
  {
    max_distance = INFINITY;
    intersect_id = NO_OBJECTS;
  }
  /* Start with root object */
  intersect = octant_intersect(octant, position, &vector,
                               ROOT_OBJECT->min, ROOT_OBJECT->max);
  if (intersect)
    intersect = (boolean)
      (bound_intersect(position, &vector, ROOT_OBJECT->min,
                       ROOT_OBJECT->max) > 0.0);
  if (intersect)
  {
    small_distance = max_distance;
    id = intersect_id;
    cluster_intersect(octant, source_id, intersect_id, position, &vector,
                      (cluster_ptr) ROOT_OBJECT->data, &small_distance,
                      &id, TRUE);
    if (id != intersect_id)
      intersect_id = id;
    while (PQUEUE_NOT_EMPTY)
    {
      pqueue_extract(&distance, &intersect_object);
      if (distance > small_distance)
        break;
      if (CHECK_BOUNDS(intersect_object->object_type))
        OBJECT_INTERSECT(distance, position, &vector, intersect_object);
      if ((distance > 0.0) AND(distance < small_distance))
      {
        REALINC(ray_hits);
        intersect_id = intersect_object->id;
        small_distance = distance;
      }
    }
  }
  if (shade_level < RAY_CACHE_LEVEL_MAX)
    ray_cache[ray_node] = intersect_id;
  if (intersect_id == NO_OBJECTS)
    return 0.0;
  if (ISNAN(small_distance))
    return 0.0;
  hit.x = position->x + small_distance * vector.x;
  hit.y = position->y + small_distance * vector.y;
  hit.z = position->z + small_distance * vector.z;
  intersect_object = object[intersect_id];
  OBJECT_NORMAL(&hit, intersect_object, &normal);
  shade(&hit, &normal, ray, intersect_object, color);
  ATTEN_COLOR(small_distance, TRUE, TRUE, TRUE, *color, *color);
  return small_distance;
}

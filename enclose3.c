/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Bernard Kwok        - hierarchical BV's
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
 *    RAY TRACING - Enclose 3 - Version 8.3.3                         *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, September 1993         *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

typedef
struct
{
  real            area;
  int             level;
  object_ptr      father;
} hier_struct;
typedef
hier_struct    *hier_ptr;

static hier_ptr *hier;

#define TYPE(object) ((object)->object_type)
#define CLUSTER_FULL(object) \
(((cluster_ptr) ((object)->data))->size == cluster_size)
#define BOUNDS_UNION(amax, amin, bmax, bmin, cmax, cmin) \
do {\
  (cmax).x = MAX((amax).x, (bmax).x);\
  (cmax).y = MAX((amax).y, (bmax).y);\
  (cmax).z = MAX((amax).z, (bmax).z);\
  (cmin).x = MIN((amin).x, (bmin).x);\
  (cmin).y = MIN((amin).y, (bmin).y);\
  (cmin).z = MIN((amin).z, (bmin).z);\
} while (0)

static real
cost(max, min)
  xyz_ptr         max, min;
{
  REG real        dx, dy, dz;

  dx = max->x - min->x;
  dy = max->y - min->y;
  dz = max->z - min->z;
  return (dx + dy) * dz + dx * dy;
}
static real
incremental_cost(object, max, min)
  object_ptr      object;
  xyz_ptr         max, min;
{
  real            old_area, new_cost;
  xyz_struct      new_max, new_min;

  if (TYPE(object) == CLUSTER_TYPE)
  {
    old_area = hier[object->id]->area;
    if (old_area < 0.0)
      return INFINITY;
  } else
    old_area = cost(object->max, object->min);
  BOUNDS_UNION(*(object->max), *(object->min), *max, *min, new_max, new_min);
  new_cost = cost(&new_max, &new_min) - old_area;
  if (new_cost < ROUNDOFF)
    return 0.0;
  else
    return new_cost;
}
static int
min_cost_child(object, max, min, min_incr)
  object_ptr      object;
  xyz_ptr         max, min;
  real           *min_incr;
{
  cluster_ptr     cluster;
  int             i, min_path;
  real            incr_cost;

  cluster = (cluster_ptr) object->data;
  *min_incr = incremental_cost(cluster->object[0], max, min);
  min_path = 0;
  for (i = 1; i < cluster->size; POSINC(i))
  {
    incr_cost = incremental_cost(cluster->object[i], max, min);
    if (incr_cost < *min_incr)
    {
      *min_incr = incr_cost;
      min_path = i;
    }
  }
  return min_path;
}
static object_ptr
create_cluster(max, min, father, level, inside)
  xyz_ptr         max, min;
  object_ptr      father;
  int             level;
  boolean         inside;
{
  int             i;
  cluster_ptr     cluster;

  ALLOCATE(cluster, cluster_struct, 1, PARSE_TYPE);
  ALLOCATE(cluster->object, object_ptr, cluster_size, PARSE_TYPE);
  cluster->size = 0;
  for (i = 0; i < cluster_size; POSINC(i))
    cluster->object[i] = NULL;
  POSINC(objects);
  if (objects >= objects_max)
    runtime_abort("too many OBJECTS and CLUSTERS");
  ALLOCATE(object[objects], object_struct, 1, PARSE_TYPE);
  object[objects]->id = objects;
  object[objects]->object_type = CLUSTER_TYPE;
  object[objects]->transf = NULL;
  object[objects]->inv_transf = NULL;
  object[objects]->texture = NULL;
  object[objects]->data = (void_ptr) cluster;
  if (inside)
    object[objects]->max = NULL;
  else
  {
    ALLOCATE(object[objects]->min, xyz_struct, 1, PARSE_TYPE);
    ALLOCATE(object[objects]->max, xyz_struct, 1, PARSE_TYPE);
    STRUCT_ASSIGN(*(object[objects]->max), *max);
    STRUCT_ASSIGN(*(object[objects]->min), *min);
  }
  ALLOCATE(hier[objects], hier_struct, 1, OTHER_TYPE);
  if (inside)
    hier[objects]->area = -1.0;
  else
    hier[objects]->area = cost(max, min);
  hier[objects]->father = father;
  hier[objects]->level = level;
  return object[objects];
}
static void
enlarge_ancestors(object, max, min)
  object_ptr      object;
  xyz_ptr         max, min;
{
  xyz_struct      new_max, new_min;

  if (object == NULL)
    return;
  if (hier[object->id]->area > 0.0)
  {
    BOUNDS_UNION(*max, *min, *(object->max), *(object->min), new_max, new_min);
    STRUCT_ASSIGN(*(object->max), new_max);
    STRUCT_ASSIGN(*(object->min), new_min);
    hier[object->id]->area = cost(&new_max, &new_min);
  }
  enlarge_ancestors(hier[object->id]->father, max, min);
}
static void
add_sibling(node_object, father_object, this_object, max, min, child_index,
	    inside)
  object_ptr      node_object, father_object, this_object;
  xyz_ptr         max, min;
  int             child_index;
  boolean         inside;
{
  xyz_struct      new_max, new_min;
  cluster_ptr     cluster;
  object_ptr      new_father;

  BOUNDS_UNION(*max, *min, *(node_object->max), *(node_object->min),
               new_max, new_min);
  if (father_object == NULL)
  {
    new_father = create_cluster(&new_max, &new_min, NULL, 0, inside);
    ROOT_OBJECT = new_father;
  } else
  {
    new_father = create_cluster(&new_max, &new_min, father_object,
                                hier[father_object->id]->level + 1, inside);
    ((cluster_ptr) (father_object->data))->object[child_index] = new_father;
  }
  cluster = (cluster_ptr) new_father->data;
  cluster->size = 2;
  cluster->object[0] = node_object;
  cluster->object[1] = this_object;
  if (father_object != NULL)
    enlarge_ancestors(father_object, max, min);
}
static void
add_child(node_object, this_object, max, min)
  object_ptr      node_object, this_object;
  xyz_ptr         max, min;
{
  xyz_struct      new_max, new_min;
  cluster_ptr     cluster;

  BOUNDS_UNION(*max, *min, *(node_object->max), *(node_object->min),
               new_max, new_min);
  STRUCT_ASSIGN(*(node_object->max), new_max);
  STRUCT_ASSIGN(*(node_object->min), new_min);
  cluster = (cluster_ptr) node_object->data;
  cluster->object[cluster->size] = this_object;
  POSINC(cluster->size);
  hier[node_object->id]->area = cost(&new_max, &new_min);
  if (hier[node_object->id]->father != NULL)
    enlarge_ancestors(hier[node_object->id]->father, max, min);
}
static object_ptr addition_node, father_node;
static int        child_index, min_child_index;
static real       min_insert_cost;
static boolean    inside;

static void
choose_node(node_object, this_object, max, min)
  object_ptr      node_object, this_object;
  xyz_ptr         max, min;
{
  real            child_min_incr;
  int             min_path;
  object_ptr      next_object;

  if (TYPE(node_object) != CLUSTER_TYPE)
  {
    child_min_incr = incremental_cost(node_object, max, min);
    if (child_min_incr < min_insert_cost)
    {
      min_child_index = child_index;
      min_insert_cost = child_min_incr;
      addition_node = node_object;
    }
    return;
  }
  min_path = min_cost_child(node_object, max, min, &child_min_incr);
  next_object = ((cluster_ptr) (node_object->data))->object[min_path];
  if ((child_min_incr <= min_insert_cost)
      AND((TYPE(next_object) != CLUSTER_TYPE)
      OR NOT CLUSTER_FULL(next_object)))
  {
    min_child_index = min_path;
    min_insert_cost = child_min_incr;
    addition_node = next_object;
    father_node = node_object;
    inside = (child_min_incr < ROUNDOFF);
  }
  child_index = min_path;
  if (TYPE(next_object) == CLUSTER_TYPE)
    choose_node(next_object, this_object, max, min);
}
#define STEP (5)

void
enclose_type_3()
{
  int             n, i, start, last;

  ALLOCATE(hier, hier_ptr, objects_max, OTHER_TYPE);
  last = objects;
  ROOT_OBJECT = object[1];
  for (n = 1; n <= STEP; POSINC(n))
  {
    if (n == 1)
      start = 1 + STEP;
    else
      start = n;
    for (i = start; i <= last; i += STEP)
    {
      min_insert_cost = INFINITY;
      child_index = -1;
      addition_node = NULL;
      father_node = NULL;
      inside = FALSE;
      choose_node(ROOT_OBJECT, object[i], object[i]->max, object[i]->min);
      if (TYPE(addition_node) == CLUSTER_TYPE)
        add_child(addition_node, object[i], object[i]->max, object[i]->min);
      else
        add_sibling(addition_node, father_node, object[i], object[i]->max,
                    object[i]->min, min_child_index, inside);
    }
  }
  for (i = last + 1; i <= objects; POSINC(i))
    FREE(hier[i]);
  FREE(hier);
}

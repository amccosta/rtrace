/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - CSG
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
 *    RAY TRACING - CSG - Version 8.3.4                               *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** CSG *****/
#define CSG_NODES_MAX (224)

typedef
struct
{
  unsigned short int depth;
  unsigned char   path[(CSG_NODES_MAX + 7) / 8];
} tree_struct;
typedef
tree_struct    *tree_ptr;

#define PATH_LEFT  (0)
#define PATH_RIGHT (1)

#define SAVE_TREE_PATH(side)\
do {\
  if ((side) == PATH_LEFT)\
    tree.path[tree.depth SHR 8] &= BIT_NOT(1 SHL (tree.depth BIT_AND 7));\
  if ((side) == PATH_RIGHT)\
    tree.path[tree.depth SHR 8] |= 1 SHL (tree.depth BIT_AND 7);\
} while (0)

typedef
struct
{
  object_ptr      object;
  real            distance;
  xyz_struct      vector;
  unsigned short int enter;
  tree_struct     tree;
} hit_node_struct;

#define ENTERING (2)

typedef
struct
{
  short int       nodes;
  hit_node_struct data[CSG_NODES_MAX];
} hit_struct;
typedef
hit_struct     *hit_ptr;

#define CSG_SET_ENTER(hit, flag) (hit)->data[0].enter = (flag) + 1

#ifdef _Windows
#pragma option -zEcsg1s -zFcsg1c -zHcsg1g
#define FAR far
#else
#define FAR
#endif

static tree_struct FAR tree;
static hit_struct FAR hit_global;

#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

static void
csg_copy_hit(hit_from, hit_to)
  hit_ptr         hit_from, hit_to;
{
  REG int         i;

  hit_to->nodes = hit_from->nodes;
  for (i = 0; i < hit_from->nodes; POSINC(i))
    STRUCT_ASSIGN(hit_to->data[i], hit_from->data[i]);
}
static unsigned int
csg_enter(position, vector, hit)
  xyz_ptr         position, vector;
  hit_ptr         hit;
{
  xyz_struct      tmp_position, normal;

  if (hit->data[0].enter != 0)
    return (hit->data[0].enter - 1);
  tmp_position.x = position->x + hit->data[0].distance * vector->x;
  tmp_position.y = position->y + hit->data[0].distance * vector->y;
  tmp_position.z = position->z + hit->data[0].distance * vector->z;
  if (hit->data[0].object->object_type == CSG_TYPE)
    csg_copy_hit(hit, &hit_global);
  OBJECT_NORMAL(&tmp_position, hit->data[0].object, &normal);
  if (DOT_PRODUCT(normal, *vector) < 0.0)
    return 1;
  return 0;
}
static void
csg_add_hit(hit, distance, vector, object, tree)
  hit_ptr         hit;
  real            distance;
  xyz_ptr         vector;
  object_ptr      object;
  tree_ptr        tree;
{
  if (hit->nodes == CSG_NODES_MAX)
    runtime_abort("too many CSG HIT NODES");
  hit->data[hit->nodes].distance = distance;
  STRUCT_ASSIGN(hit->data[hit->nodes].vector,  *vector);
  hit->data[hit->nodes].object = object;
  hit->data[hit->nodes].enter = 0;
  STRUCT_ASSIGN(hit->data[hit->nodes].tree, *tree);
  POSINC(hit->nodes);
}

static real
#ifdef __STDC__
object_intersect(object_ptr, xyz_ptr, xyz_ptr, hit_ptr, real);
#else
object_intersect();
#endif

static boolean
csg_union(position, vector, hit1, hit2, distance1, distance2,
          hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;
  hit_ptr         hit_tmp;

  while (TRUE)
  {
    if ((hit2->nodes == 0) OR(csg_enter(position, vector, hit2) != 0))
    {
      /* Hit 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
                                 position, vector, &hit3,
                                 distance2 + threshold_distance);
    if (distance3 < distance2 + threshold_distance)
    {
      /* Leaving 2nd */
      *hit = hit2;
      *distance = distance2;
      CSG_SET_ENTER(hit2, 0);
      return FALSE;
    }
    hit_tmp = hit1;
    hit1 = hit2;
    hit2 = hit_tmp;
    distance1 = distance2;
    csg_copy_hit(&hit3, hit2);
    distance2 = distance3;
  }
}
static boolean
csg_subtraction(position, vector, hit1, hit2, distance1, distance2,
                hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;

  while (TRUE)
  {
    if (distance1 < distance2)
    {
      if ((hit2->nodes == 0) OR(csg_enter(position, vector, hit2) != 0))
      {
        /* Hit 1st */
        *hit = hit1;
        *distance = distance1;
        CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
        return FALSE;
      }
      hit3.nodes = 0;
      distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
                                   position, vector, &hit3,
                                   distance2 + threshold_distance);
      if (distance3 < distance2 + threshold_distance)
        /* Miss */
        return TRUE;
      distance1 = distance3;
      csg_copy_hit(&hit3, hit1);
      continue;
    }
    if (hit1->nodes == 0)
      /* Miss */
      return TRUE;
    if (csg_enter(position, vector, hit1) == 0)
    {
      /* Hit 2nd inverted */
      *hit = hit2;
      *distance = distance2;
      CSG_SET_ENTER(hit2, NOT csg_enter(position, vector, hit2));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit2->data[hit2->nodes - 1].object,
                                 position, vector, &hit3,
                                 distance1 + threshold_distance);
    if (distance3 < distance1 + threshold_distance)
    {
      /* Entering 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, 1);
      return FALSE;
    }
    distance2 = distance3;
    csg_copy_hit(&hit3, hit2);
  }
}
static boolean
csg_intersection(position, vector, hit1, hit2, distance1, distance2,
                 hit, distance)
  xyz_ptr         position, vector;
  hit_ptr         hit1, hit2;
  real            distance1, distance2;
  hit_ptr        *hit;
  real           *distance;
{
  REG real        distance3;
  hit_struct      hit3;
  hit_ptr         hit_tmp;

  while (TRUE)
  {
    if (csg_enter(position, vector, hit2) == 0)
    {
      /* Hit 1st */
      *hit = hit1;
      *distance = distance1;
      CSG_SET_ENTER(hit1, csg_enter(position, vector, hit1));
      return FALSE;
    }
    hit3.nodes = 0;
    distance3 = object_intersect(hit1->data[hit1->nodes - 1].object,
                                 position, vector, &hit3,
                                 distance2 + threshold_distance);
    if (distance3 < distance2 + threshold_distance)
      /* Miss */
      return TRUE;
    hit_tmp = hit1;
    hit1 = hit2;
    hit2 = hit_tmp;
    distance1 = distance2;
    csg_copy_hit(&hit3, hit2);
    distance2 = distance3;
  }
}
static real
csg_sec_intersect(csg_object, position, vector, csg_hit, min_distance)
  object_ptr      csg_object;
  xyz_ptr         position, vector;
  hit_ptr         csg_hit;
  real            min_distance;
{
  real            distance;
  REG real        distance1, distance2;
  xyz_struct      new_position, new_vector;
  csg_ptr         csg;
  hit_ptr         hit1, hit2, hit;
  hit_struct      hit1_list, hit2_list;

  if (csg_object->transf != NULL)
  {
    transform(csg_object->transf, position, &new_position);
    transform_vector(csg_object->transf, position, vector, &new_position,
                     &new_vector);
    NORMALIZE(new_vector);
  } else
  {
    STRUCT_ASSIGN(new_position, *position);
    STRUCT_ASSIGN(new_vector, *vector);
  }
  REALINC(csg_tests);
  csg = (csg_ptr) csg_object->data;
  /* CSG intersection */
  hit1 = &hit1_list;
  hit2 = &hit2_list;
  hit1->nodes = 0;
  hit2->nodes = 0;
  POSINC(tree.depth);
  if (tree.depth >= CSG_NODES_MAX)
    runtime_abort("too big CSG TREE PATH");
  SAVE_TREE_PATH(PATH_LEFT);
  distance1 = object_intersect(object[csg->left], &new_position,
                               &new_vector, hit1, min_distance);
  if ((distance1 < min_distance) AND((csg->op == CSG_SUBTRACTION)
      OR(csg->op == CSG_INTERSECTION)))
  {
    POSDEC(tree.depth);
    return 0.0;
  }
  SAVE_TREE_PATH(PATH_RIGHT);
  distance2 = object_intersect(object[csg->right], &new_position,
                               &new_vector, hit2, min_distance);
  POSDEC(tree.depth);
  if ((distance2 < min_distance) AND((csg->op == CSG_INTERSECTION)
      OR((hit1->nodes == 0) AND(csg->op == CSG_UNION))))
    return 0.0;
  if (distance1 < min_distance)
    distance1 = INFINITY;
  if (distance2 < min_distance)
    distance2 = INFINITY;
  if ((distance1 > distance2) AND((csg->op == CSG_UNION)
      OR(csg->op == CSG_INTERSECTION)))
  {
    distance = distance2;
    distance2 = distance1;
    distance1 = distance;
    hit1 = &hit2_list;
    hit2 = &hit1_list;
  }
  switch(csg->op)
  {
  case CSG_UNION:
    if (csg_union(&new_position, &new_vector, hit1, hit2,
                  distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_SUBTRACTION:
    if (csg_subtraction(&new_position, &new_vector, hit1, hit2,
                        distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_INTERSECTION:
    if (csg_intersection(&new_position, &new_vector, hit1, hit2,
                         distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  }
  if (distance < min_distance)
    return 0.0;
  csg_copy_hit(hit, csg_hit);
  if (csg_object->transf != NULL)
  {
    csg->position->x = new_position.x + distance * new_vector.x;
    csg->position->y = new_position.y + distance * new_vector.y;
    csg->position->z = new_position.z + distance * new_vector.z;
    return transform_distance(csg_object->inv_transf, distance,
                              &new_position, &new_vector, position);
  } else
    return distance;
}
static real
object_intersect(intersect_object, position, vector, hit, min_distance)
  object_ptr      intersect_object;
  xyz_ptr         position, vector;
  hit_ptr         hit;
  real            min_distance;
{
  REG int         octant;
  REG real        distance;
  xyz_struct      tmp_position;

  tmp_position.x = position->x + min_distance * vector->x;
  tmp_position.y = position->y + min_distance * vector->y;
  tmp_position.z = position->z + min_distance * vector->z;
  FIND_OCTANT(octant, *vector);
  if (NOT octant_intersect(octant, &tmp_position, vector,
                           intersect_object->min, intersect_object->max))
    return 0.0;
  if (CHECK_BOUNDS(intersect_object->object_type))
    if (bound_intersect(&tmp_position, vector, intersect_object->min,
                        intersect_object->max) <= 0.0)
      return 0.0;
  /* Primitive objects */
  if (intersect_object->object_type != CSG_TYPE)
  {
    OBJECT_INTERSECT(distance, &tmp_position, vector, intersect_object);
    if (distance < threshold_distance)
      return 0.0;
    distance += min_distance;
    hit->nodes = 0;
    csg_add_hit(hit, distance, vector, intersect_object, &tree);
    return distance;
  }
  /* CSG objects */
  distance = csg_sec_intersect(intersect_object, position, vector, hit,
                               min_distance);
  if (distance < threshold_distance)
    return 0.0;
  csg_add_hit(hit, distance, vector, intersect_object, &tree);
  return distance;
}

/* Main CSG functions */
real
csg_intersect(position, vector, csg_object)
  xyz_ptr         position, vector;
  object_ptr      csg_object;
{
  real            distance;
  REG real        distance1, distance2;
  xyz_struct      new_position, new_vector;
  csg_ptr         csg;
  hit_ptr         hit1, hit2, hit;
  hit_struct      hit1_list, hit2_list;

  if (csg_object->transf != NULL)
  {
    transform(csg_object->transf, position, &new_position);
    transform_vector(csg_object->transf, position, vector, &new_position,
                     &new_vector);
    NORMALIZE(new_vector);
  } else
  {
    STRUCT_ASSIGN(new_position, *position);
    STRUCT_ASSIGN(new_vector, *vector);
  }
  REALINC(csg_tests);
  csg = (csg_ptr) csg_object->data;
  /* CSG intersection */
  hit1 = &hit1_list;
  hit2 = &hit2_list;
  hit1->nodes = 0;
  hit2->nodes = 0;
  tree.depth = 0;
  SAVE_TREE_PATH(PATH_LEFT);
  distance1 = object_intersect(object[csg->left], &new_position,
                               &new_vector, hit1, threshold_distance);
  if ((distance1 < threshold_distance) AND((csg->op == CSG_SUBTRACTION)
      OR(csg->op == CSG_INTERSECTION)))
    return 0.0;
  SAVE_TREE_PATH(PATH_RIGHT);
  distance2 = object_intersect(object[csg->right], &new_position,
                               &new_vector, hit2, threshold_distance);
  if ((distance2 < threshold_distance) AND((csg->op == CSG_INTERSECTION)
      OR((hit1->nodes == 0) AND(csg->op == CSG_UNION))))
    return 0.0;
  if (distance1 < threshold_distance)
    distance1 = INFINITY;
  if (distance2 < threshold_distance)
    distance2 = INFINITY;
  if ((distance1 > distance2) AND((csg->op == CSG_UNION)
      OR(csg->op == CSG_INTERSECTION)))
  {
    distance = distance2;
    distance2 = distance1;
    distance1 = distance;
    hit1 = &hit2_list;
    hit2 = &hit1_list;
  }
  switch(csg->op)
  {
  case CSG_UNION:
    if (csg_union(&new_position, &new_vector, hit1, hit2,
                  distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_SUBTRACTION:
    if (csg_subtraction(&new_position, &new_vector, hit1, hit2,
                        distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  case CSG_INTERSECTION:
    if (csg_intersection(&new_position, &new_vector, hit1, hit2,
                         distance1, distance2, &hit, &distance))
      return 0.0;
    break;
  }
  if (distance < threshold_distance)
    return 0.0;
  csg_copy_hit(hit, &hit_global);
  if (csg_object->transf != NULL)
  {
    csg->position->x = new_position.x + distance * new_vector.x;
    csg->position->y = new_position.y + distance * new_vector.y;
    csg->position->z = new_position.z + distance * new_vector.z;
    return transform_distance(csg_object->inv_transf, distance,
                              &new_position, &new_vector, position);
  } else
    return distance;
}
#define RIGHT_TREE_PATH(depth) \
(hit_global.data[0].tree.path[(depth) SHR 8] BIT_AND \
 (1 SHL ((depth) BIT_AND 7)))

void
csg_normal(position, csg_object, normal)
  xyz_ptr         position;
  object_ptr      csg_object;
  xyz_ptr         normal;
{
  REG int         i;
  REG real        k;
  xyz_struct      old_position, new_normal;
  object_ptr      new_csg_object;
  csg_ptr         csg;
  int             transfs;
  xyzw_ptr        transf[CSG_NODES_MAX];
  xyz_ptr         transf_position[CSG_NODES_MAX];

  new_csg_object = csg_object;
  csg = (csg_ptr) csg_object->data;
  if (csg_object->transf != NULL)
    STRUCT_ASSIGN(old_position, *(csg->position));
  else
    STRUCT_ASSIGN(old_position, *position);
  transfs = 0;
  csg_object->surface_id = csg->surface_id;
  csg_object->refraction = csg->refraction;
  csg_object->texture = csg->texture;
  /* CSG transformations */
  for (i = 0; i < (int) hit_global.data[0].tree.depth; POSINC(i))
  {
    if (RIGHT_TREE_PATH(i))
      new_csg_object = object[csg->right];
    else
      new_csg_object = object[csg->left];
    if (new_csg_object->object_type != CSG_TYPE)
      break;
    csg = (csg_ptr) (new_csg_object->data);
    if ((csg_object->surface_id < 0) AND(csg->surface_id >= 0))
      csg_object->surface_id = csg->surface_id;
    if ((csg_object->refraction < 0.0) AND(csg->refraction > 0.0))
      csg_object->refraction = csg->refraction;
    if ((csg_object->texture == NULL) AND(csg->texture != NULL))
      csg_object->texture = csg->texture;
    if (new_csg_object->transf != NULL)
    {
      STRUCT_ASSIGN(old_position, *(csg->position));
      transf[transfs] = new_csg_object->transf;
      transf_position[transfs] = csg->position;
      POSINC(transfs);
    }
  }
  if (csg_object->surface_id < 0)
    csg_object->surface_id = hit_global.data[0].object->surface_id;
  if (csg_object->refraction < 0.0)
    csg_object->refraction = hit_global.data[0].object->refraction;
  if (csg_object->texture == NULL)
    csg_object->texture = hit_global.data[0].object->texture;
  OBJECT_NORMAL(&old_position, hit_global.data[0].object, normal);
  /* CSG Normal inverse transformations */
  while (transfs > 0)
  {
    POSDEC(transfs);
    transform_normal_vector(transf[transfs], transf_position[transfs],
                            normal, &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
  if (csg_object->transf != NULL)
  {
    csg = (csg_ptr) csg_object->data;
    transform_normal_vector(csg_object->transf, csg->position, normal,
                            &new_normal);
    STRUCT_ASSIGN(*normal, new_normal);
    NORMALIZE(*normal);
  }
  k = DOT_PRODUCT(hit_global.data[0].vector, *normal);
  if ((hit_global.data[0].enter == ENTERING) == (k > 0.0))
  {
    /* Flip normal */
    normal->x = -normal->x;
    normal->y = -normal->y;
    normal->z = -normal->z;
  }
}
static void
csg_remove(rem_object, csg_removed, non_csg_removed)
  object_ptr      rem_object;
  int            *csg_removed, *non_csg_removed;
{
  csg_ptr         csg;
  texture_ptr     texture, old_texture;

  if (rem_object == NULL)
    return;
  FREE(rem_object->min);
  FREE(rem_object->max);
  if (rem_object->transf != NULL)
    FREE(rem_object->transf);
  if (rem_object->inv_transf != NULL)
    FREE(rem_object->inv_transf);
  texture = rem_object->texture;
  while (texture != NULL)
  {
    if (texture->transf != NULL)
      FREE(texture->transf);
    if (texture->data != NULL)
      FREE(texture->data);
    old_texture = texture;
    texture = texture->next;
    FREE(old_texture);
  }
  if (rem_object->object_type != CSG_TYPE)
  {
    FREE(rem_object->data);
    FREE(rem_object);
    POSINC(*non_csg_removed);
    return;
  }
  csg = (csg_ptr) rem_object->data;
  if (csg->position != NULL)
    FREE(csg->position);
  texture = csg->texture;
  while (texture != NULL)
  {
    if (texture->transf != NULL)
      FREE(texture->transf);
    if (texture->data != NULL)
      FREE(texture->data);
    old_texture = texture;
    texture = texture->next;
    FREE(old_texture);
  }
  csg_remove(object[csg->left], csg_removed, non_csg_removed);
  object[csg->left] = NULL;
  csg_remove(object[csg->right], csg_removed, non_csg_removed);
  object[csg->right] = NULL;
  FREE(rem_object->data);
  FREE(rem_object);
  POSINC(*csg_removed);
}
void
csg_enclose(csg_object, csg_removed, non_csg_removed)
  int             csg_object, *csg_removed, *non_csg_removed;
{
  xyz_struct      max, min, temp, vertex;
  csg_ptr         csg;
  object_ptr      parent, left, right;
  xyz_struct      tmp;

  parent = object[csg_object];
  csg = (csg_ptr) parent->data;
  left = object[csg->left];
  right = object[csg->right];
  switch (csg->op)
  {
  case CSG_UNION:
    if ((left == NULL) AND(right == NULL))
    {
      WRITE(results, "Warning: null CSG UNION (OBJECT %d)\n", csg_object);
      FLUSH(results);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    if (right == NULL)
    {
      WRITE(results, "Warning: dummy CSG UNION (null RIGHT OBJECT %d)\n",
            csg->right);
      FLUSH(results);
      object[csg_object] = left;
      object[csg->left] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    if (left == NULL)
    {
      WRITE(results, "Warning: dummy CSG UNION (null LEFT OBJECT %d)\n",
            csg->left);
      FLUSH(results);
      object[csg_object] = right;
      object[csg->right] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    parent->min->x = MIN(left->min->x, right->min->x);
    parent->min->y = MIN(left->min->y, right->min->y);
    parent->min->z = MIN(left->min->z, right->min->z);
    parent->max->x = MAX(left->max->x, right->max->x);
    parent->max->y = MAX(left->max->y, right->max->y);
    parent->max->z = MAX(left->max->z, right->max->z);
    break;
  case CSG_INTERSECTION:
    if ((left == NULL) OR(right == NULL)
        OR(left->max->x < right->min->x)
        OR(left->min->x > right->max->x)
        OR(left->max->y < right->min->y)
        OR(left->min->y > right->max->y)
        OR(left->max->z < right->min->z)
        OR(left->min->z > right->max->z))
    {
      WRITE(results, "Warning: null CSG INTERSECTION (OBJECT %d)\n",
            csg_object);
      FLUSH(results);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    parent->min->x = MAX(left->min->x, right->min->x);
    parent->min->y = MAX(left->min->y, right->min->y);
    parent->min->z = MAX(left->min->z, right->min->z);
    parent->max->x = MIN(left->max->x, right->max->x);
    parent->max->y = MIN(left->max->y, right->max->y);
    parent->max->z = MIN(left->max->z, right->max->z);
    break;
  case CSG_SUBTRACTION:
    if (left == NULL)
    {
      WRITE(results, "Warning: null CSG SUBTRACTION (OBJECT %d)\n",
            csg_object);
      FLUSH(results);
      csg_remove(parent, csg_removed, non_csg_removed);
      object[csg_object] = NULL;
      return;
    }
    if ((right == NULL)
        OR(left->max->x < right->min->x) OR(left->min->x > right->max->x)
        OR(left->max->y < right->min->y) OR(left->min->y > right->max->y)
        OR(left->max->z < right->min->z) OR(left->min->z > right->max->z))
    {
      WRITE(results, "Warning: dummy CSG SUBTRACTION (null RIGHT OBJECT %d)\n",
            csg->right);
      FLUSH(results);
      object[csg_object] = left;
      object[csg->left] = NULL;
      csg_remove(parent, csg_removed, non_csg_removed);
      return;
    }
    parent->min->x = left->min->x;
    parent->min->y = left->min->y;
    parent->min->z = left->min->z;
    parent->max->x = left->max->x;
    parent->max->y = left->max->y;
    parent->max->z = left->max->z;
    break;
  }
  csg->surface_id = parent->surface_id;
  csg->refraction = parent->refraction;
  csg->texture = parent->texture;
  if (parent->transf != NULL)
  {
    ALLOCATE(csg->position, xyz_struct, 1, PARSE_TYPE);
    STRUCT_ASSIGN(max, *(parent->max));
    STRUCT_ASSIGN(min, *(parent->min));
    temp.x = min.x;
    temp.y = min.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, parent->min);
    STRUCT_ASSIGN(*(parent->max), *(parent->min));
    temp.x = min.x;
    temp.y = min.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = min.x;
    temp.y = max.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = min.x;
    temp.y = max.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = min.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = min.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = max.y;
    temp.z = min.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
    temp.x = max.x;
    temp.y = max.y;
    temp.z = max.z;
    transform(parent->inv_transf, &temp, &vertex);
    if (vertex.x < parent->min->x)
      parent->min->x = vertex.x;
    if (vertex.x > parent->max->x)
      parent->max->x = vertex.x;
    if (vertex.y < parent->min->y)
      parent->min->y = vertex.y;
    if (vertex.y > parent->max->y)
      parent->max->y = vertex.y;
    if (vertex.z < parent->min->z)
      parent->min->z = vertex.z;
    if (vertex.z > parent->max->z)
      parent->max->z = vertex.z;
  }
}

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
 *    RAY TRACING - Enclose 2 - Version 8.3.3                         *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, November 1992          *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, October 1993           *
 **********************************************************************/

void
enclose_type_2()
{
  REG int         i, j;
  boolean        *done;
  cluster_ptr	  cluster;
  object_ptr      cluster_object;
  xyz_struct	  min, max;
  int             remaining, bound[6], list[6], count;

  ALLOCATE(done, boolean, objects + 1, OTHER_TYPE);
  for (i = 1; i <= objects; POSINC(i))
    done[i] = FALSE;
  i = objects;
  remaining = objects;
  ALLOCATE(ROOT_OBJECT, object_struct, 1, PARSE_TYPE);
  cluster_object = ROOT_OBJECT;
  while (TRUE)
  {
    min.x = X_MAX;
    min.y = Y_MAX;
    min.z = Z_MAX;
    max.x = X_MIN;
    max.y = Y_MIN;
    max.z = Z_MIN;
    for (j = 1; j <= i; POSINC(j))
    {
      if (done[j])
        continue;
      if (object[j]->min->x <= min.x)
      {
        min.x = object[j]->min->x;
        bound[0] = j;
      }
      if (object[j]->min->y <= min.y)
      {
        min.y = object[j]->min->y;
        bound[1] = j;
      }
      if (object[j]->min->z <= min.z)
      {
        min.z = object[j]->min->z;
        bound[2] = j;
      }
      if (object[j]->max->x >= max.x)
      {
        max.x = object[j]->max->x;
        bound[3] = j;
      }
      if (object[j]->max->y >= max.y)
      {
        max.y = object[j]->max->y;
        bound[4] = j;
      }
      if (object[j]->max->z >= max.z)
      {
        max.z = object[j]->max->z;
        bound[5] = j;
      }
    }
    count = 0;
    for (j = 0; j < 6; POSINC(j))
      if (NOT done[bound[j]])
      {
        done[bound[j]] = TRUE;
        list[count] = bound[j];
        POSINC(count);
      }
    remaining -= count;

    /* Create cluster object */
    cluster_object->object_type = CLUSTER_TYPE;
    cluster_object->transf = NULL;
    cluster_object->inv_transf = NULL;
    cluster_object->texture = NULL;
    ALLOCATE(cluster, cluster_struct, 1, PARSE_TYPE);
    cluster_object->data = (void_ptr) cluster;
    ALLOCATE(cluster_object->min, xyz_struct, 1, PARSE_TYPE);
    ALLOCATE(cluster_object->max, xyz_struct, 1, PARSE_TYPE);
    STRUCT_ASSIGN(*(cluster_object->min), min);
    STRUCT_ASSIGN(*(cluster_object->max), max);

    if (remaining == 0)
    {
      ALLOCATE(cluster->object, object_ptr, count, PARSE_TYPE);
      cluster->size = count;
      for (j = 0; j < count; POSINC(j))
        cluster->object[j] = object[list[j]];
      break;
    }
    if (remaining == 1)
    {
      ALLOCATE(cluster->object, object_ptr, count + 1, PARSE_TYPE);
      cluster->size = count + 1;
      for (j = 0; j < count; POSINC(j))
        cluster->object[j] = object[list[j]];
      for (j = 1; done[j]; POSINC(j))
        ;
      cluster->object[count] = object[j];
      break;
    }
    POSINC(objects);
    if (objects >= objects_max)
      runtime_abort("too many OBJECTS and CLUSTERS");
    ALLOCATE(cluster->object, object_ptr, count + 1, PARSE_TYPE);
    cluster->size = count + 1;
    for (j = 0; j < count; POSINC(j))
      cluster->object[j] = object[list[j]];
    ALLOCATE(object[objects], object_struct, 1, PARSE_TYPE);
    cluster_object = object[objects];
    cluster->object[count] = object[objects];
  }
  FREE(done);
}

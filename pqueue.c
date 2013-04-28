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
 *    RAY TRACING - Priority queue - Version 7.0                      *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, July 1990              *
 **********************************************************************/

/***** Priority Queue *****/
void
pqueue_insert(key, object)
  real            key;
  object_ptr      object;
{
  REG int         i, j;

  REALINC(pqueue_insertions);
  POSINC(pqueue_size);
  if (pqueue_size >= PQUEUE_SIZE_MAX)
    runtime_abort("exhausted PRIORITY QUEUE");
  pqueue[pqueue_size].key = key;
  pqueue[pqueue_size].object = object;
  i = pqueue_size;
  j = i DIV 2;
  while ((i > 1) AND(pqueue[i].key < pqueue[j].key))
  {
    STRUCT_ASSIGN(pqueue[0], pqueue[i]);
    STRUCT_ASSIGN(pqueue[i], pqueue[j]);
    STRUCT_ASSIGN(pqueue[j], pqueue[0]);
    i = j;
    j = i DIV 2;
  }
}
void
pqueue_extract(key, object)
  real           *key;
  object_ptr     *object;
{
  REG int         i, j;

  REALINC(pqueue_extractions);
  if (pqueue_size == 0)
    runtime_abort("empty PRIORITY QUEUE");
  *key = pqueue[1].key;
  *object = pqueue[1].object;
  pqueue[1] = pqueue[pqueue_size];
  POSDEC(pqueue_size);
  i = 1;
  while (i + i <= pqueue_size)
  {
    j = i + i;
    if (j != pqueue_size)
      if (pqueue[j].key > pqueue[SUCC(j)].key)
        POSINC(j);
    if (pqueue[i].key > pqueue[j].key)
    {
      STRUCT_ASSIGN(pqueue[0], pqueue[i]);
      STRUCT_ASSIGN(pqueue[i], pqueue[j]);
      STRUCT_ASSIGN(pqueue[j], pqueue[0]);
      i = j;
    } else
      break;
  }
}

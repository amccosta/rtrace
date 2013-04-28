/*
 * Copyright (c) 1992, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
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
#include "config.h"
#include "nmalloc.h"

/**********************************************************************
 *    RAY TRACING - Memory allocator - Version 8.0.0                  *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, September 1992         *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1992         *
 **********************************************************************/

typedef
struct
{
  void_ptr        node, next;
  char            type;
} node_struct;
typedef
node_struct      *node_struct_ptr;

static node_struct_ptr head_node = NULL;
static node_struct_ptr tail_node = NULL;

void
free_type(type)
  int             type;
{
  node_struct_ptr p, pt, po;
  boolean         first;

  first = TRUE;
  p = head_node;
  po = NULL;
  while (p != NULL)
  {
    if (p->type != (char) type)
    {
      p = (node_struct_ptr) p->next;
      if (first)
      {
        po = head_node;
        first = FALSE;
      } else
        po = (node_struct_ptr) po->next;
      continue;
    }
    free(p->node);
    pt = p;
    p = (node_struct_ptr) p->next;
    if (first)
      head_node = (void_ptr) p;
    else
      po->next = (void_ptr) p;
    if (pt == tail_node)
      tail_node = po;
    free(pt);
  }
}

static void
free_all()
{
  free_type(PARSE_TYPE);
  free_type(TRACE_TYPE);
  free_type(OTHER_TYPE);
  head_node = NULL;
  tail_node = NULL;
}

void
nexit(code)
  int             code;
{
  free_all();
  exit(code);
}

char_ptr
nmalloc(size, type)
  unsigned        size;
  int             type;
{
  char_ptr        node;
  node_struct_ptr pn;

  node = (char_ptr) malloc(size);
  if (node == NULL)
    return ((char_ptr) NULL);
  pn = (node_struct_ptr) malloc(sizeof(node_struct));
  pn->node = (void_ptr) node;
  pn->type = (char) type;
  pn->next = NULL;
  if (tail_node != NULL)
    tail_node->next = (void_ptr) pn;
  else
    head_node = pn;
  tail_node = pn;
  return node;
}

void
nfree(node)
  char_ptr        node;
{
  node_struct_ptr p, po;
  boolean         first;

  if (node == NULL)
    return;
  free(node);
  p = head_node;
  po = NULL;
  first = TRUE;
  while (p != NULL)
  {
    if (p->node == (void_ptr) node)
    {
      if (first)
        head_node = (node_struct_ptr) head_node->next;
      else
        po->next = (node_struct_ptr) p->next;
      if (p == tail_node)
        tail_node = po;
      free(p);
      return;
    }
    p = (node_struct_ptr) p->next;
    if (first)
    {
      first = FALSE;
      po = head_node;
    } else
      po = (node_struct_ptr) po->next;
  }
}

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
 *    RAY TRACING - Antialiasing - Version 8.3.4                      *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** Adaptive supersampling Antialiasing *****/
#define SAMPLING_DIVISIONS_MAX ((1 SHL SAMPLING_LEVEL_MAX) - 1)

#ifdef _Windows
#pragma option -zEalias1s -zFalias1c -zHalias1g
#define FAR far
#else
#define FAR
#endif

static FAR pixel_struct pixel[SAMPLING_DIVISIONS_MAX][SAMPLING_DIVISIONS_MAX];

#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

static boolean
big_color_change(weight, pixel0, pixel1, pixel2, pixel3, ids, id)
  int             weight;
  pixel_ptr       pixel0, pixel1, pixel2, pixel3;
  int            *ids;
  list_id         id;
{
  boolean         color_change;
  REG int         i, j;
  REG real        r, g, b;
  rgb_struct      color[4];

  if (weight < sampling_weight)
  {
    STRUCT_ASSIGN(color[0], pixel0->color);
    STRUCT_ASSIGN(color[1], pixel1->color);
    STRUCT_ASSIGN(color[2], pixel2->color);
    STRUCT_ASSIGN(color[3], pixel3->color);
    i = -1;
    do
    {
      POSINC(i);
      j = i;
      do
      {
        POSINC(j);
        if (contrast_mode)
        {
          r = (color[i].r - color[j].r) / (color[i].r + color[j].r + ROUNDOFF);
          g = (color[i].g - color[j].g) / (color[i].g + color[j].g + ROUNDOFF);
          b = (color[i].b - color[j].b) / (color[i].b + color[j].b + ROUNDOFF);
        } else
        {
          r = color[i].r - color[j].r;
          g = color[i].g - color[j].g;
          b = color[i].b - color[j].b;
        }
        color_change = (boolean) (SQR(r) + SQR(g) + SQR(b) >
                                  threshold_color);
      }
      while ((j != 3) AND NOT color_change);
    }
    while ((i != 2) AND NOT color_change);
    if (color_change)
    {
      *ids = 1;
      id[0] = pixel0->id;
      if (pixel1->id != pixel0->id)
        id[POSINC(*ids)] = pixel1->id;
      if ((pixel2->id != pixel1->id) AND(pixel2->id != pixel0->id))
        id[POSINC(*ids)] = pixel2->id;
      if ((pixel3->id != pixel2->id) AND(pixel3->id != pixel1->id)
          AND(pixel3->id != pixel0->id))
        id[POSINC(*ids)] = pixel3->id;
      return TRUE;
    }
    return FALSE;
  } else
    return FALSE;
}
static void
intersect_list(ray, ids, id, pixel)
  ray_ptr         ray;
  int             ids;
  list_id         id;
  pixel_ptr       pixel;
{
  REG int         i;
  REG real        small_distance, max_distance;
  real            distance;
  xyz_struct      hit, normal;
  object_ptr      intersect_object;

  REALINC(eye_rays);
  max_distance = INFINITY;
  pixel->id = NO_OBJECTS;
  PQUEUE_INITIALIZE;
  for (i = 0; i < ids; POSINC(i))
  {
    if (id[i] == NO_OBJECTS)
      continue;
    if (CHECK_BOUNDS(object[id[i]]->object_type))
    {
      distance = bound_intersect(&eye, &(ray->vector), object[id[i]]->min,
                                 object[id[i]]->max);
      if ((distance <= 0.0) OR(distance >= max_distance))
        continue;
      pqueue_insert(distance, object[id[i]]);
    } else
    {
      OBJECT_INTERSECT(distance, &eye, &(ray->vector), object[id[i]]);
      if ((distance <= 0.0) OR(distance >= max_distance))
        continue;
      max_distance = distance;
      pixel->id = object[id[i]]->id;
    }
  }
  small_distance = max_distance;
  while (PQUEUE_NOT_EMPTY)
  {
    pqueue_extract(&distance, &intersect_object);
    if (distance > small_distance)
      break;
    if (CHECK_BOUNDS(intersect_object->object_type))
      OBJECT_INTERSECT(distance, &eye, &(ray->vector), intersect_object);
    if ((distance > 0.0) AND(distance < small_distance))
    {
      REALINC(ray_hits);
      pixel->id = intersect_object->id;
      small_distance = distance;
    }
  }
  if ((intersect_mode == 1) AND(pixel->id == NO_OBJECTS))
  {
    REALINC(eye_rays);
    if (intersect_all(NO_OBJECTS, &eye, ray, &(pixel->color)) > 0.0)
    {
      pixel->id = ray_cache[0];
      return;
    }
  }
  if (pixel->id == NO_OBJECTS)
  {
    STRUCT_ASSIGN(pixel->color, back_color);
    return;
  }
  hit.x = eye.x + small_distance * ray->vector.x;
  hit.y = eye.y + small_distance * ray->vector.y;
  hit.z = eye.z + small_distance * ray->vector.z;
  intersect_object = object[pixel->id];
  OBJECT_NORMAL(&hit, intersect_object, &normal);
  shade(&hit, &normal, ray, intersect_object, &(pixel->color));
  ATTEN_COLOR(small_distance, TRUE, TRUE, TRUE, pixel->color, pixel->color);
}
#define GET_POINT(dx, dy, i, j)\
do {\
  (i) = ROUND((real) sampling_divisions * (dx));\
  (j) = ROUND((real) sampling_divisions * (dy));\
  if (((i) == 0) OR((i) == sampling_divisions)\
      OR((j) == 0) OR((j) == sampling_divisions))\
    (i) = -1;\
  else\
  {\
    POSDEC(i);\
    POSDEC(j);\
  }\
} while (0)
#define AVERAGE_PIXEL(weight)\
do {\
  REG real        w;\
\
  w = 1.0 / (real) (weight);\
  color->r += (pixel0->color.r + pixel2->color.r + pixel6->color.r +\
               pixel8->color.r) * w;\
  color->g += (pixel0->color.g + pixel2->color.g + pixel6->color.g +\
               pixel8->color.g) * w;\
  color->b += (pixel0->color.b + pixel2->color.b + pixel6->color.b +\
               pixel8->color.b) * w;\
  if (background_mode == 1)\
  {\
    if (pixel0->id == NO_OBJECTS)\
      pixel_mask += w;\
    if (pixel2->id == NO_OBJECTS)\
      pixel_mask += w;\
    if (pixel6->id == NO_OBJECTS)\
      pixel_mask += w;\
    if (pixel8->id == NO_OBJECTS)\
      pixel_mask += w;\
  }\
} while (0)

static real       pixel_mask;
static void
find_color_adaptive(xr, yr, x, y, delta, pixel0, pixel2, pixel6, pixel8,
                    weight, color)
  REG real        xr, yr, x, y, delta;
  pixel_ptr       pixel0, pixel2, pixel6, pixel8;
  int             weight;
  rgb_ptr         color;
{
  int             i, j, ids;
  REG real        x0, y0, delta0;
  list_id         id;
  pixel_struct    pixel1, pixel3, pixel4, pixel5, pixel7;
  ray_struct      ray0;

  if (big_color_change(weight, pixel0, pixel2, pixel6, pixel8, &ids, id))
  {
    REALINC(pixel_divisions);
    delta0 = delta * 0.5;
    x0 = x - delta0;
    y0 = y - delta0;
    ray0.level.r = 1.0;
    ray0.level.g = 1.0;
    ray0.level.b = 1.0;
    ray0.inside = FALSE;
    /* 1st quadrant */
    /* Point 1 */
    GET_POINT(xr - x0, yr - (y - delta), i, j);
    if (i < 0)                  /* Edges */
    {
      make_vector(&(ray0.vector), x0, y - delta, delta0);
      intersect_list(&ray0, ids, id, &pixel1);
    } else
    {
      if (pixel[i][j].id >= NO_OBJECTS) /* Done */
        STRUCT_ASSIGN(pixel1, pixel[i][j]);
      else                      /* Not done */
      {
        make_vector(&(ray0.vector), x0, y - delta, delta0);
        intersect_list(&ray0, ids, id, &pixel1);
        STRUCT_ASSIGN(pixel[i][j], pixel1);
      }
    }
    /* Point 3 */
    GET_POINT(xr - (x - delta), yr - y0, i, j);
    if (i < 0)                  /* Edges */
    {
      make_vector(&(ray0.vector), x - delta, y0, delta0);
      intersect_list(&ray0, ids, id, &pixel3);
    } else
    {
      if (pixel[i][j].id >= NO_OBJECTS) /* Done */
        STRUCT_ASSIGN(pixel3, pixel[i][j]);
      else                      /* Not done */
      {
        make_vector(&(ray0.vector), x - delta, y0, delta0);
        intersect_list(&ray0, ids, id, &pixel3);
        STRUCT_ASSIGN(pixel[i][j], pixel3);
      }
    }
    /* Point 4 */
    GET_POINT(xr - x0, yr - y0, i, j);
    if (i < 0)                  /* Edges */
    {
      make_vector(&(ray0.vector), x0, y0, delta0);
      intersect_list(&ray0, ids, id, &pixel4);
    } else
    {
      if (pixel[i][j].id >= NO_OBJECTS) /* Done */
        STRUCT_ASSIGN(pixel4, pixel[i][j]);
      else                      /* Not done */
      {
        make_vector(&(ray0.vector), x0, y0, delta0);
        intersect_list(&ray0, ids, id, &pixel4);
        STRUCT_ASSIGN(pixel[i][j], pixel4);
      }
    }
    find_color_adaptive(xr, yr, x0, y0, delta0,
                        pixel0, &pixel1, &pixel3, &pixel4,
                        weight * 4, color);
    /* 2nd quadrant */
    /* Point 5 */
    GET_POINT(xr - x, yr - y0, i, j);
    if (i < 0)                  /* Edges */
    {
      make_vector(&(ray0.vector), x, y0, delta0);
      intersect_list(&ray0, ids, id, &pixel5);
    } else
    {
      if (pixel[i][j].id >= NO_OBJECTS) /* Done */
        STRUCT_ASSIGN(pixel5, pixel[i][j]);
      else                      /* Not done */
      {
        make_vector(&(ray0.vector), x, y0, delta0);
        intersect_list(&ray0, ids, id, &pixel5);
        STRUCT_ASSIGN(pixel[i][j], pixel5);
      }
    }
    find_color_adaptive(xr, yr, x, y0, delta0,
                        &pixel1, pixel2, &pixel4, &pixel5,
                        weight * 4, color);
    /* 3rd quadrant */
    /* Point 7 */
    GET_POINT(xr - x0, yr - y, i, j);
    if (i < 0)                  /* Edges */
    {
      make_vector(&(ray0.vector), x0, y, delta0);
      intersect_list(&ray0, ids, id, &pixel7);
    } else
    {
      if (pixel[i][j].id >= NO_OBJECTS) /* Done */
        STRUCT_ASSIGN(pixel7, pixel[i][j]);
      else                      /* Not done */
      {
        make_vector(&(ray0.vector), x0, y, delta0);
        intersect_list(&ray0, ids, id, &pixel7);
        STRUCT_ASSIGN(pixel[i][j], pixel7);
      }
    }
    find_color_adaptive(xr, yr, x0, y, delta0,
                        &pixel3, &pixel4, pixel6, &pixel7,
                        weight * 4, color);
    /* 4th quadrant */
    find_color_adaptive(xr, yr, x, y, delta0,
                        &pixel4, &pixel5, &pixel7, pixel8,
                        weight * 4, color);
  } else
    AVERAGE_PIXEL(weight);
}
static void
find_color_fixed(x, y, ids, id, pixel0, pixel2, pixel6, pixel8, color)
  real            x, y;
  int             ids;
  list_id         id;
  pixel_ptr       pixel0, pixel2, pixel6, pixel8;
  rgb_ptr         color;
{
  REG int         weight, i, j;
  REG real        delta, delta0, w;
  pixel_struct    pixel_temp;
  ray_struct      ray0;

  weight = SQR(sampling_divisions) + 4;
  w = 1.0 / (real) weight;
  AVERAGE_PIXEL(weight);
  ray0.level.r = 1.0;
  ray0.level.g = 1.0;
  ray0.level.b = 1.0;
  ray0.inside = FALSE;
  delta = 1.0 / (real) sampling_divisions;
  delta0 = delta * 0.5;
  x -= delta0;
  y -= delta0;
  for (i = 0; i < sampling_divisions; POSINC(i))
    for (j = 0; j < sampling_divisions; POSINC(j))
    {
      REALINC(pixel_divisions);
      make_vector(&(ray0.vector), x - (real) i * delta,
                  y - (real) j * delta, delta0);
      if (antialiasing_mode == 1)
      {
        intersect_list(&ray0, ids, id, &pixel_temp);
        color->r += pixel_temp.color.r * w;
        color->g += pixel_temp.color.g * w;
        color->b += pixel_temp.color.b * w;
        if (background_mode == 1)
          if (pixel_temp.id == NO_OBJECTS)
            pixel_mask += w;
      } else
      {
        REALINC(eye_rays);
        if (intersect_all(NO_OBJECTS, &eye, &ray0, &(pixel_temp.color)) >
            0.0)
        {
          color->r += pixel_temp.color.r * w;
          color->g += pixel_temp.color.g * w;
          color->b += pixel_temp.color.b * w;
        } else
        {
          color->r += back_color.r * w;
          color->g += back_color.g * w;
          color->b += back_color.b * w;
          if (background_mode == 1)
            pixel_mask += w;
        }
      }
    }
}
static void
find_color(x, y, pixel0, pixel2, pixel6, pixel8, color)
  int             x, y;
  pixel_ptr       pixel0, pixel2, pixel6, pixel8;
  rgb_ptr         color;
{
  int             ids;
  list_id         id;

  if (sampling_levels == 0)
  {
    AVERAGE_PIXEL(4);
    return;
  }
  switch (antialiasing_mode)
  {
    case 0:
      find_color_adaptive((real) x, (real) y, (real) x, (real) y, 1.0,
                          pixel0, pixel2, pixel6, pixel8, 4, color);
      return;
    case 1:
      if (big_color_change(4, pixel0, pixel2, pixel6, pixel8, &ids, id))
        find_color_fixed((real) x, (real) y, ids, id,
                         pixel0, pixel2, pixel6, pixel8, color);
      else
        AVERAGE_PIXEL(4);
      return;
    case 2:
      find_color_fixed((real) x, (real) y, 0, id,
                       pixel0, pixel2, pixel6, pixel8, color);
      return;
  }
}
void
find_true_color(x, y, pixel0, pixel2, pixel6, pixel8, true_color)
  int             x, y;
  pixel_ptr       pixel0, pixel2, pixel6, pixel8;
  rgb_ptr         true_color;
{
  REG int         i, j;

  if (antialiasing_mode == 0)
    for (i = 0; i < PRED(sampling_divisions); POSINC(i))
      for (j = 0; j < PRED(sampling_divisions); POSINC(j))
        pixel[i][j].id = -1;
  true_color->r = 0.0;
  true_color->g = 0.0;
  true_color->b = 0.0;
  if (background_mode == 1)
    pixel_mask = 0.0;
  find_color(x, y, pixel0, pixel2, pixel6, pixel8, true_color);
  if (background_mode == 1)
    back_mask[x] = pixel_mask;
}

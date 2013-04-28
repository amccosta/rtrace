/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paul Strauss        - shading model
 *  Craig Kolb          - textures
 *  David Buck          - textures
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
 *    RAY TRACING - Lighting - Version 8.3.4                          *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

/***** Lighting *****/
void
vector_to_light(l, position, lighting)
  REG int         l;
  xyz_ptr         position, lighting;
{
  lighting->x = light[l].coords.x - position->x;
  lighting->y = light[l].coords.y - position->y;
  lighting->z = light[l].coords.z - position->z;
  light_distance = LENGTH(*lighting);
  NORMALIZE(*lighting);
}
real
light_intensity(l, lighting)
  REG int         l;
  xyz_ptr         lighting;
{
  real            k;
  dir_light_ptr   dir_light;

  if (light[l].light_type == DIRECT_LIGHT_TYPE)
  {
    dir_light = (dir_light_ptr) light[l].data;
    k = DOT_PRODUCT(dir_light->vector, *lighting);
    if (k > dir_light->cos_angle)
      return POWER((k - dir_light->cos_angle) * dir_light->t,
                   dir_light->factor);
    return 0.0;
  }
  return 1.0;
}
static void
point_light_brightness(source_id, l, position, lighting, brightness, noshadow)
  int             source_id, l;
  xyz_ptr         position, lighting;
  rgb_ptr         brightness;
  boolean         noshadow;
{
  boolean         intersect, shadow;
  REG int         cached_id, octant, surface_id;
  int             id;
  real            distance;
  object_ptr      intersect_object;

  if (noshadow)
  {
    ATTEN_COLOR(light_distance, light[l].attenuation[0],
                light[l].attenuation[1], light[l].attenuation[2],
                light[l].brightness, *brightness);
    return;
  }
  if (light_mode != 0)
    ATTEN_COLOR(light_distance, light[l].attenuation[0],
                light[l].attenuation[1], light[l].attenuation[2],
                light[l].brightness, *brightness);
  shadow = FALSE;
  FIND_OCTANT(octant, *lighting);
  if ((shade_level < LIGHT_CACHE_LEVEL_MAX)
      AND(light[l].cache_id[shade_level] != NO_OBJECTS))
  {
    id = light[l].cache_id[shade_level];
    cached_id = id;
    if ((id != source_id) OR NOT CONVEX(object[id]->object_type)
        OR(COLOR_BIG(surface[object[id]->surface_id]->transparent, ROUNDOFF)
        AND SELF_INTERSECT(object[id]->object_type, TRUE)))
    {
      intersect = octant_intersect(octant, position, lighting,
                                   object[id]->min, object[id]->max);
      if (intersect AND(backface_mode != 0)
          AND POLYGONAL(object[id]->object_type))
        intersect = NOT backface_check(position, lighting, object[id]);
      if (intersect AND CHECK_BOUNDS(object[id]->object_type))
      {
        distance = bound_intersect(position, lighting, object[id]->min,
                                   object[id]->max);
        intersect = (boolean)
          ((distance > 0.0) AND(distance < light_distance));
      }
      if (intersect)
      {
        REALINC(shadow_rays);
        OBJECT_INTERSECT(distance, position, lighting, object[id]);
        if ((distance > 0.0) AND(distance < light_distance))
        {
          REALINC(shadow_hits);
          REALINC(shadow_cache_hits);
          if (light_mode == 0)
            shadow = TRUE;
          else
          {
            surface_id = object[id]->surface_id;
            brightness->r *= surface[surface_id]->transparent.r;
            brightness->g *= surface[surface_id]->transparent.g;
            brightness->b *= surface[surface_id]->transparent.b;
            if (NOT COLOR_BIG(*brightness, threshold_level))
              shadow = TRUE;
          }
        } else
          light[l].cache_id[shade_level] = NO_OBJECTS;
      } else
        light[l].cache_id[shade_level] = NO_OBJECTS;
    } else
      light[l].cache_id[shade_level] = NO_OBJECTS;
  } else
    cached_id = NO_OBJECTS;
  if (NOT shadow)
  {
    PQUEUE_INITIALIZE;
    id = 0;
    cluster_intersect(octant, source_id, cached_id, position, lighting,
                      (cluster_ptr) ROOT_OBJECT->data, &light_distance,
                      &id, FALSE);
    if ((light_mode == 0) AND(id < 0))
    {
      shadow = TRUE;
      if (shade_level < LIGHT_CACHE_LEVEL_MAX)
        light[l].cache_id[shade_level] = -id;
    }
    while (NOT shadow AND PQUEUE_NOT_EMPTY)
    {
      REALINC(shadow_rays);
      pqueue_extract(&distance, &intersect_object);
      if (CHECK_BOUNDS(intersect_object->object_type))
        OBJECT_INTERSECT(distance, position, lighting, intersect_object);
      if ((distance > 0.0) AND(distance < light_distance))
      {
        REALINC(shadow_hits);
        if (light_mode == 0)
          shadow = TRUE;
        else
        {
          surface_id = intersect_object->surface_id;
          if ((surface_id < 0) OR(surface_id >= surfaces))
            runtime_abort("bug detected (invalid SURFACE ID) - sorry...");
          brightness->r *= surface[surface_id]->transparent.r;
          brightness->g *= surface[surface_id]->transparent.g;
          brightness->b *= surface[surface_id]->transparent.b;
          if (NOT COLOR_BIG(*brightness, threshold_level))
            shadow = TRUE;
        }
        if (shade_level < LIGHT_CACHE_LEVEL_MAX)
          light[l].cache_id[shade_level] = intersect_object->id;
      }
    }
  }
  if (shadow)
  {
    brightness->r = 0.0;
    brightness->g = 0.0;
    brightness->b = 0.0;
  } else
  if (light_mode == 0)
    ATTEN_COLOR(light_distance, light[l].attenuation[0],
                light[l].attenuation[1], light[l].attenuation[2],
                light[l].brightness, *brightness);
}
static void
extended_light_brightness(source_id, l, position, lighting, brightness,
                          noshadow)
  int             source_id, l;
  xyz_ptr         position, lighting;
  rgb_ptr         brightness;
  boolean         noshadow;
{
  REG int         i, j, samples;
  REG real        k, u, v, diameter;
  ext_light_ptr   ext_light;
  xyz_struct      temp, light_l, light_lu, light_u, light_v;
  rgb_struct      new_brightness;

  ext_light = (ext_light_ptr) light[l].data;
  diameter = ext_light->diameter;
  ATTEN_DISTANCE(diameter, k);
  samples = TRUNC(k * diameter / pixel_distance);
  if (samples <= 1)
  {
    point_light_brightness(source_id, l, position, lighting, brightness,
                           noshadow);
    return;
  }
  samples = MIN(samples, ext_light->samples);
  brightness->r = 0.0;
  brightness->g = 0.0;
  brightness->b = 0.0;
  k = 1.0 / (real) samples;
  if (ABS(lighting->y) > 1.0 - ROUNDOFF)
  {
    temp.x = 0.0;
    temp.y = 0.0;
    temp.z = 1.0;
  } else
  {
    temp.x = 0.0;
    temp.y = 1.0;
    temp.z = 0.0;
  }
  CROSS_PRODUCT(light_u, *lighting, temp);
  NORMALIZE(light_u);
  CROSS_PRODUCT(light_v, *lighting, light_u);
  light_l.x = light[l].coords.x - position->x;
  light_l.y = light[l].coords.y - position->y;
  light_l.z = light[l].coords.z - position->z;
  for (i = 0; i <= samples; POSINC(i))
  {
    u = (((real) i + JITTER) * k - 0.5) * diameter;
    light_lu.x = light_l.x + u * light_u.x;
    light_lu.y = light_l.y + u * light_u.y;
    light_lu.z = light_l.z + u * light_u.z;
    for (j = 0; j <= samples; POSINC(j))
    {
      if (((i == 0) OR(i == samples))
          AND((j == 0) OR(j == samples)))
        continue;
      v = (((real) j + JITTER) * k - 0.5) * diameter;
      temp.x = light_lu.x + v * light_v.x;
      temp.y = light_lu.y + v * light_v.y;
      temp.z = light_lu.z + v * light_v.z;
      NORMALIZE(temp);
      point_light_brightness(source_id, l, position, &temp, &new_brightness,
                             noshadow);
      brightness->r += new_brightness.r;
      brightness->g += new_brightness.g;
      brightness->b += new_brightness.b;
    }
  }
  k = 1.0 / (SQR((real) (samples + 1)) - 4.0);
  brightness->r *= k;
  brightness->g *= k;
  brightness->b *= k;
  if (NOT COLOR_BIG(*brightness, ROUNDOFF))
  {
    brightness->r = 0.0;
    brightness->g = 0.0;
    brightness->b = 0.0;
  }
}
static void
planar_light_brightness(source_id, l, position, lighting, brightness,
                        noshadow)
  int             source_id, l;
  xyz_ptr         position, lighting;
  rgb_ptr         brightness;
  boolean         noshadow;
{
  REG int         i, j, samples, samples1, samples2;
  REG real        k, k1, k2, u, v, size;
  planar_light_ptr planar_light;
  xyz_struct      temp, light_l, light_lu, light_u, light_v;
  rgb_struct      new_brightness;

  planar_light = (planar_light_ptr) light[l].data;
  size = planar_light->size;
  ATTEN_DISTANCE(size, k);
  samples = TRUNC(k * size / pixel_distance);
  if (samples <= 1)
  {
    point_light_brightness(source_id, l, position, lighting, brightness,
                           noshadow);
    return;
  }
  samples1 = MIN(samples, planar_light->samples1);
  samples2 = MIN(samples, planar_light->samples2);
  brightness->r = 0.0;
  brightness->g = 0.0;
  brightness->b = 0.0;
  light_l.x = light[l].coords.x - position->x;
  light_l.y = light[l].coords.y - position->y;
  light_l.z = light[l].coords.z - position->z;
  k1 = 1.0 / (real) samples1;
  STRUCT_ASSIGN(light_u, planar_light->vector1);
  if (samples2 > 0)
  {
    k2 = 1.0 / (real) samples2;
    STRUCT_ASSIGN(light_v, planar_light->vector2);
  }
  for (i = 0; i <= samples1; POSINC(i))
  {
    u = (real) i * k1 + JITTER * k1 - 0.5;
    light_lu.x = light_l.x + u * light_u.x;
    light_lu.y = light_l.y + u * light_u.y;
    light_lu.z = light_l.z + u * light_u.z;
    if (samples2 <= 0)
    {
      NORMALIZE(light_lu);
      point_light_brightness(source_id, l, position, &light_lu, &new_brightness,
                             noshadow);
      brightness->r += new_brightness.r;
      brightness->g += new_brightness.g;
      brightness->b += new_brightness.b;
      continue;
    }
    for (j = 0; j <= samples2; POSINC(j))
    {
      v = (real) j * k2 + JITTER * k2 - 0.5;
      temp.x = light_lu.x + v * light_v.x;
      temp.y = light_lu.y + v * light_v.y;
      temp.z = light_lu.z + v * light_v.z;
      NORMALIZE(temp);
      point_light_brightness(source_id, l, position, &temp, &new_brightness,
                             noshadow);
      brightness->r += new_brightness.r;
      brightness->g += new_brightness.g;
      brightness->b += new_brightness.b;
    }
  }
  if (samples2 <= 0)
    k = 1.0 / (real) (samples1 + 1);
  else
    k = 1.0 / ((real) (samples1 + 1) * (real) (samples2 + 1));
  brightness->r *= k;
  brightness->g *= k;
  brightness->b *= k;
  if (NOT COLOR_BIG(*brightness, ROUNDOFF))
  {
    brightness->r = 0.0;
    brightness->g = 0.0;
    brightness->b = 0.0;
  }
}
void
light_brightness(source_id, l, position, lighting, brightness, noshadow)
  int             source_id, l;
  xyz_ptr         position, lighting;
  rgb_ptr         brightness;
  boolean         noshadow;
{
  switch (light[l].light_type)
  {
  case POINT_LIGHT_TYPE:
  case DIRECT_LIGHT_TYPE:
    point_light_brightness(source_id, l, position, lighting, brightness,
                           noshadow);
    break;
  case EXTENDED_LIGHT_TYPE:
    extended_light_brightness(source_id, l, position, lighting, brightness,
                              noshadow);
    break;
  case PLANAR_LIGHT_TYPE:
    planar_light_brightness(source_id, l, position, lighting, brightness,
                            noshadow);
    break;
  }
}

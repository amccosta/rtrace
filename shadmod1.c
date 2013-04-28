/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
 *    RAY TRACING - Shade Model 1 - Version 8.3.0                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1993              *
 **********************************************************************/

/***** Shading Phong model *****/
#define LEFT  (1 SHL shade_level)
#define RIGHT (1 SHL (shade_level + 1))

static xyz_struct previous_normal, previous_reflected;
static rgb_struct previous_diffuse, previous_specular;
void
shade_phong(position, normal, ray, object, color)
  xyz_ptr         position, normal;     /* Normal may be modified */
  ray_ptr         ray;
  object_ptr      object;
  rgb_ptr         color;
{
  REG int         l;
  REG real        k, diffuse, specular;
  real            intensity, new_diffuse;
  boolean         refractive, opposite_light;
  rgb_struct      brightness, level;
  rgb_struct      surface_diffuse, surface_specular, surface_transparent;
  rgb_struct      new_color, distributed_color;
  xyz_struct      old_normal, old_reflected, lighting;
  ray_struct      reflected, refracted, distributed;
  surface_struct  new_surface;
  int             id, ambient_diffuse_rays, ambient_specular_rays;

  STRUCT_ASSIGN(new_surface, *(surface[object->surface_id]));
  k = -2.0 * DOT_PRODUCT(ray->vector, *normal);
  reflected.vector.x = k * normal->x + ray->vector.x;
  reflected.vector.y = k * normal->y + ray->vector.y;
  reflected.vector.z = k * normal->z + ray->vector.z;
  NORMALIZE(reflected.vector);
  if (normal_mode == 0)
  {
    if (k < 0.0)
    {
      normal->x = -(normal->x);
      normal->y = -(normal->y);
      normal->z = -(normal->z);
    }
  } else
  if ((k < 0.0) AND ray->inside)
  {
    normal->x = -(normal->x);
    normal->y = -(normal->y);
    normal->z = -(normal->z);
  }
  STRUCT_ASSIGN(old_normal, *normal);
  if ((texture_mode != 0) AND(object->texture != NULL))
  {
    if (normal_check_mode AND object->texture_modify_normal)
      STRUCT_ASSIGN(old_reflected, reflected.vector);
    surface_texture(position, normal, &new_surface, object->texture);
    if (normal_check_mode AND object->texture_modify_normal)
    {
      k = -2.0 * DOT_PRODUCT(ray->vector, *normal);
      reflected.vector.x = k * normal->x + ray->vector.x;
      reflected.vector.y = k * normal->y + ray->vector.y;
      reflected.vector.z = k * normal->z + ray->vector.z;
      NORMALIZE(reflected.vector);
      if (DOT_PRODUCT(reflected.vector, old_normal) < ROUNDOFF)
      {
        STRUCT_ASSIGN(*normal, old_normal);
        STRUCT_ASSIGN(reflected.vector, old_reflected);
      }
    }
  }
  if (new_surface.emitter)
  {
    STRUCT_ASSIGN(*color, new_surface.color);
    k = -DOT_PRODUCT(ray->vector, *normal);
    color->r *= k;
    color->g *= k;
    color->b *= k;
    return;
  }
  STRUCT_ASSIGN(surface_diffuse, new_surface.diffuse);
  STRUCT_ASSIGN(surface_specular, new_surface.specular);
  STRUCT_ASSIGN(surface_transparent, new_surface.transparent);
  new_surface.diffuse.r *= new_surface.color.r;
  new_surface.diffuse.g *= new_surface.color.g;
  new_surface.diffuse.b *= new_surface.color.b;
  new_surface.specular.r *= new_surface.color.r * (1.0 -
    new_surface.metal_factor.r) + new_surface.metal_factor.r;
  new_surface.specular.g *= new_surface.color.g * (1.0 -
    new_surface.metal_factor.g) + new_surface.metal_factor.g;
  new_surface.specular.b *= new_surface.color.b * (1.0 -
    new_surface.metal_factor.b) + new_surface.metal_factor.b;
  new_surface.transparent.r *= new_surface.color.r;
  new_surface.transparent.g *= new_surface.color.g;
  new_surface.transparent.b *= new_surface.color.b;
  color->r = 0.0;
  color->g = 0.0;
  color->b = 0.0;
  /* Refractions */
  refractive = FALSE;
  if (COLOR_BIG(surface_transparent, ROUNDOFF)
      AND(shade_level < last_shade_level))
  {
    level.r = ray->level.r * surface_transparent.r;
    level.g = ray->level.g * surface_transparent.g;
    level.b = ray->level.b * surface_transparent.b;
    if (COLOR_BIG(level, threshold_level))
    {
      STRUCT_ASSIGN(refracted.vector, ray->vector);
      if (refract(&refracted, normal, ray->inside, object->refraction))
      {
        refractive = TRUE;
        if (shade_level < RAY_CACHE_LEVEL_MAX)
          ray_node += RIGHT;
        POSINC(shade_level);
        if (shade_level > shade_level_max)
          shade_level_max = shade_level;
        STRUCT_ASSIGN(refracted.level, level);
        refracted.inside = NOT ray->inside;
        REALINC(refracted_rays);
        if (SELF_INTERSECT(object->object_type, refracted.inside))
          id = NO_OBJECTS;
        else
          id = object->id;
        if (intersect_all(id, position, &refracted, &new_color) > 0.0)
        {
          color->r += new_color.r * new_surface.transparent.r;
          color->g += new_color.g * new_surface.transparent.g;
          color->b += new_color.b * new_surface.transparent.b;
        } else
        {
          color->r += back_color.r * new_surface.transparent.r;
          color->g += back_color.g * new_surface.transparent.g;
          color->b += back_color.b * new_surface.transparent.b;
        }
        POSDEC(shade_level);
        if (shade_level < RAY_CACHE_LEVEL_MAX)
          ray_node -= RIGHT;
      } else
      {
        /* TIR */
        surface_specular.r += surface_transparent.r;
        surface_specular.g += surface_transparent.g;
        surface_specular.b += surface_transparent.b;
        new_surface.specular.r = surface_specular.r * (new_surface.color.r *
          (1.0 - new_surface.metal_factor.r) + new_surface.metal_factor.r);
        new_surface.specular.g = surface_specular.g * (new_surface.color.g *
          (1.0 - new_surface.metal_factor.g) + new_surface.metal_factor.g);
        new_surface.specular.b = surface_specular.b * (new_surface.color.b *
          (1.0 - new_surface.metal_factor.b) + new_surface.metal_factor.b);
      }
    }
  }
  /* Reflections */
  if (COLOR_BIG(surface_specular, ROUNDOFF)
      AND(shade_level < last_shade_level))
  {
    level.r = ray->level.r * surface_specular.r;
    level.g = ray->level.g * surface_specular.g;
    level.b = ray->level.b * surface_specular.b;
    if (COLOR_BIG(level, threshold_level))
    {
      if (shade_level < RAY_CACHE_LEVEL_MAX)
        ray_node += LEFT;
      POSINC(shade_level);
      if (shade_level > shade_level_max)
        shade_level_max = shade_level;
      STRUCT_ASSIGN(reflected.level, level);
      reflected.inside = ray->inside;
      REALINC(reflected_rays);
      if (SELF_INTERSECT(object->object_type, reflected.inside))
        id = NO_OBJECTS;
      else
        id = object->id;
      if (intersect_all(id, position, &reflected, &new_color) > 0.0)
      {
        color->r += new_color.r * new_surface.specular.r;
        color->g += new_color.g * new_surface.specular.g;
        color->b += new_color.b * new_surface.specular.b;
      } else
      {
        color->r += back_color.r * new_surface.specular.r;
        color->g += back_color.g * new_surface.specular.g;
        color->b += back_color.b * new_surface.specular.b;
      }
      POSDEC(shade_level);
      if (shade_level < RAY_CACHE_LEVEL_MAX)
        ray_node -= LEFT;
    }
  }
  /* Ambient */
  if ((shade_level >= last_ambient_level) OR(ambient_sample_rays == 0))
  {
    color->r += light_ambient.r * new_surface.diffuse.r;
    color->g += light_ambient.g * new_surface.diffuse.g;
    color->b += light_ambient.b * new_surface.diffuse.b;
  } else
  {
    diffuse = (surface_diffuse.r + surface_diffuse.g +
      surface_diffuse.b) / 3.0;
    specular = (surface_specular.r + surface_specular.g +
      surface_specular.b) / 3.0;
    if (diffuse + specular > ROUNDOFF)
    {
      if (ABS(diffuse + specular - 1.0) > ROUNDOFF)
      {
        k = diffuse + specular;
        diffuse /= k;
        specular /= k;
      }
      if ((shade_level == 0) AND(distributed_cache_mode != 0)
          AND(previous_repetitions <= distributed_cache_repetitions)
          AND(DOT_PRODUCT(old_normal, previous_normal) > threshold_vector)
          AND(DOT_PRODUCT(reflected.vector, previous_reflected) >
              threshold_vector))
      {
        POSINC(previous_repetitions);
        REALINC(distributed_cache_hits);
        color->r += previous_diffuse.r * diffuse * new_surface.diffuse.r;
        color->g += previous_diffuse.g * diffuse * new_surface.diffuse.g;
        color->b += previous_diffuse.b * diffuse * new_surface.diffuse.b;
        color->r += previous_specular.r * specular * new_surface.specular.r;
        color->g += previous_specular.g * specular * new_surface.specular.g;
        color->b += previous_specular.b * specular * new_surface.specular.b;
      } else
      {
        ambient_diffuse_rays = estimate_diffuse(ambient_sample_rays, diffuse);
        ambient_specular_rays = estimate_specular(ambient_sample_rays,
          specular, new_surface.phong_factor);
        if (shade_level < RAY_CACHE_LEVEL_MAX)
          ray_node += RIGHT;
        POSINC(shade_level);
        if (shade_level > shade_level_max)
          shade_level_max = shade_level;
        if (SELF_INTERSECT(object->object_type, distributed.inside))
          id = NO_OBJECTS;
        else
          id = object->id;
        distributed.inside = ray->inside;
        distributed.level.r = ray->level.r * surface_diffuse.r;
        distributed.level.g = ray->level.g * surface_diffuse.g;
        distributed.level.b = ray->level.b * surface_diffuse.b;
        distributed_color.r = 0.0;
        distributed_color.g = 0.0;
        distributed_color.b = 0.0;
        if (COLOR_BIG(surface_diffuse, ROUNDOFF)
            AND COLOR_BIG(distributed.level, ROUNDOFF)
            AND(ambient_diffuse_rays > 0))
        {
          /* Diffuse ambient */
          for (l = 0; l < ambient_diffuse_rays; POSINC(l))
          {
            make_diffuse_vector(&old_normal, &(distributed.vector));
            REALINC(ambient_rays);
            if (intersect_all(id, position, &distributed, &new_color) > 0.0)
            {
              distributed_color.r += new_color.r;
              distributed_color.g += new_color.g;
              distributed_color.b += new_color.b;
            } else
            {
              distributed_color.r += back_color.r;
              distributed_color.g += back_color.g;
              distributed_color.b += back_color.b;
            }
          }
          k = 1.0 / (real) ambient_diffuse_rays;
          distributed_color.r *= k;
          distributed_color.g *= k;
          distributed_color.b *= k;
          color->r += distributed_color.r * diffuse * new_surface.diffuse.r;
          color->g += distributed_color.g * diffuse * new_surface.diffuse.g;
          color->b += distributed_color.b * diffuse * new_surface.diffuse.b;
        }
        if ((shade_level == 1) AND(distributed_cache_mode != 0))
          STRUCT_ASSIGN(previous_diffuse, distributed_color);
        distributed.level.r = ray->level.r * surface_specular.r;
        distributed.level.g = ray->level.g * surface_specular.g;
        distributed.level.b = ray->level.b * surface_specular.b;
        distributed_color.r = 0.0;
        distributed_color.g = 0.0;
        distributed_color.b = 0.0;
        if (COLOR_BIG(surface_specular, ROUNDOFF)
            AND COLOR_BIG(distributed.level, ROUNDOFF)
            AND(ambient_specular_rays > 0)
            AND(new_surface.phong_factor > ROUNDOFF))
        {
          /* Specular ambient */
          for (l = 0; l < ambient_specular_rays; POSINC(l))
          {
            make_specular_vector(&(reflected.vector), &old_normal,
                                 new_surface.phong_factor,
                                 &(distributed.vector));
            REALINC(ambient_rays);
            if (intersect_all(id, position, &distributed, &new_color) > 0.0)
            {
              distributed_color.r += new_color.r;
              distributed_color.g += new_color.g;
              distributed_color.b += new_color.b;
            } else
            {
              distributed_color.r += back_color.r;
              distributed_color.g += back_color.g;
              distributed_color.b += back_color.b;
            }
          }
          k = 1.0 / (real) ambient_specular_rays;
          distributed_color.r *= k;
          distributed_color.g *= k;
          distributed_color.b *= k;
          color->r += distributed_color.r * specular * new_surface.specular.r;
          color->g += distributed_color.g * specular * new_surface.specular.g;
          color->b += distributed_color.b * specular * new_surface.specular.b;
        }
        POSDEC(shade_level);
        if (shade_level < RAY_CACHE_LEVEL_MAX)
          ray_node -= RIGHT;
        if ((shade_level == 0) AND(distributed_cache_mode != 0))
        {
          if (previous_repetitions > 0)
          {
            REALINC(distributed_cache_resets);
            previous_repetitions = 0;
          }
          STRUCT_ASSIGN(previous_normal, old_normal);
          STRUCT_ASSIGN(previous_reflected, reflected.vector);
          STRUCT_ASSIGN(previous_specular, distributed_color);
        }
      }
    }
  }
  new_surface.diffuse.r *= new_surface.diffuse_factor.r;
  new_surface.diffuse.g *= new_surface.diffuse_factor.g;
  new_surface.diffuse.b *= new_surface.diffuse_factor.b;
  /* Lights */
  for (l = 0; l < lights; POSINC(l))
  {
    vector_to_light(l, position, &lighting);
    intensity = -1.0;
    if ((texture_mode != 0) AND(object->texture != NULL)
        AND normal_check_mode AND object->texture_modify_normal)
    {
      k = DOT_PRODUCT(old_normal, lighting);
      diffuse = DOT_PRODUCT(*normal, lighting);
      if ((k < 0.0) == (diffuse > 0.0))
        diffuse = -diffuse;
    } else
      diffuse = DOT_PRODUCT(*normal, lighting);
    if (diffuse < 0.0)
    {
      opposite_light = TRUE;
      if (refractive AND(light_mode == 2))
        diffuse = -diffuse;
    } else
      opposite_light = FALSE;
    if ((diffuse > ROUNDOFF)
        AND COLOR_BIG(surface_diffuse, ROUNDOFF))
    {
      intensity = light_intensity(l, &lighting);
      diffuse *= intensity;
    } else
      diffuse = 0.0;
    if (COLOR_BIG(surface_specular, ROUNDOFF))
    {
      if (opposite_light)
        if (refractive AND(light_mode == 2))
          specular = DOT_PRODUCT(refracted.vector, lighting);
        else
          specular = 0.0;
      else
        specular = DOT_PRODUCT(reflected.vector, lighting);
      if (specular > ROUNDOFF)
      {
        if (intensity < 0.0)
          intensity = light_intensity(l, &lighting);
        if (intensity > ROUNDOFF)
          specular = POWER(specular, new_surface.phong_factor) * intensity;
        else
          specular = 0.0;
      }
    } else
      specular = 0.0;
    if ((diffuse < ROUNDOFF) AND(specular < ROUNDOFF))
    {
      if (shade_level < LIGHT_CACHE_LEVEL_MAX)
        light[l].cache_id[shade_level] = NO_OBJECTS;
      continue;
    }
    light_brightness(object->id, l, position, &lighting, &brightness,
                     new_surface.noshadow);
    if (brightness.r > ROUNDOFF)
    {
      new_diffuse = diffuse * new_surface.diffuse_factor.r;
      if (new_diffuse > ROUNDOFF)
        color->r += new_diffuse * brightness.r * new_surface.diffuse.r;
      if (specular > ROUNDOFF)
        color->r += specular * brightness.r * new_surface.specular.r;
    }
    if (brightness.g > ROUNDOFF)
    {
      new_diffuse = diffuse * new_surface.diffuse_factor.g;
      if (new_diffuse > ROUNDOFF)
        color->g += new_diffuse * brightness.g * new_surface.diffuse.g;
      if (specular > ROUNDOFF)
        color->g += specular * brightness.g * new_surface.specular.g;
    }
    if (brightness.b > ROUNDOFF)
    {
      new_diffuse = diffuse * new_surface.diffuse_factor.b;
      if (new_diffuse > ROUNDOFF)
        color->b += new_diffuse * brightness.b * new_surface.diffuse.b;
      if (specular > ROUNDOFF)
        color->b += specular * brightness.b * new_surface.specular.b;
    }
  }
}

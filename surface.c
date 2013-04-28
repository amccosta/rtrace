/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
 *    RAY TRACING - Surface - Version 8.3.0                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1991           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1993              *
 **********************************************************************/

/***** Surfaces *****/
#define NEAR_ONE (0.95)

void
get_surface_type1()
{
  real            value;
  rgb_struct      smooth, opaque;

  ALLOCATE(surface[surfaces], surface_struct, 1, PARSE_TYPE);

  get_valid(scene, &value, -1.0, 1.0, "SURFACE COLOR Red");
  surface[surfaces]->emitter = value < 0.0;
  surface[surfaces]->color.r = ABS(value);
  get_valid(scene, &value, -1.0, 1.0, "SURFACE COLOR Green");
  surface[surfaces]->noshadow = value < 0.0;
  surface[surfaces]->color.g = ABS(value);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE COLOR Blue");
  surface[surfaces]->color.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE DIFFUSION Red");
  surface[surfaces]->diffuse.r = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE DIFFUSION Green");
  surface[surfaces]->diffuse.g = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE DIFFUSION Blue");
  surface[surfaces]->diffuse.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE SPECULARITY Red");
  surface[surfaces]->specular.r = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE SPECULARITY Green");
  surface[surfaces]->specular.g = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE SPECULARITY Blue");
  surface[surfaces]->specular.b = MIN(value, NEAR_ONE);

  get_valid(scene, &value, 0.0, SPECULAR_FACTOR_MAX,
            "SURFACE SPECULARITY PHONG Factor");
  if (value < 3.0)
    value = 3.0;
  surface[surfaces]->phong_factor = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE METALNESS Factor");
  surface[surfaces]->metal_factor.r = value;
  surface[surfaces]->metal_factor.g = value;
  surface[surfaces]->metal_factor.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Red");
  surface[surfaces]->transparent.r = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Green");
  surface[surfaces]->transparent.g = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Blue");
  surface[surfaces]->transparent.b = MIN(value, NEAR_ONE);

  /* Compute remaining parameters */
  opaque.r = 1.0 - surface[surfaces]->transparent.r;
  opaque.g = 1.0 - surface[surfaces]->transparent.g;
  opaque.b = 1.0 - surface[surfaces]->transparent.b;

  smooth.r = 1.0 - 3.0 / surface[surfaces]->phong_factor;
  if (smooth.r < ROUNDOFF)
    smooth.r = 0.0;
  smooth.g = smooth.r;
  smooth.b = smooth.r;

  surface[surfaces]->diffuse_factor.r = 1.0 - smooth.r *
    surface[surfaces]->metal_factor.r;
  surface[surfaces]->diffuse_factor.g = 1.0 - smooth.g *
    surface[surfaces]->metal_factor.g;
  surface[surfaces]->diffuse_factor.b = 1.0 - smooth.b *
    surface[surfaces]->metal_factor.b;
}
#define PHONG_FACTOR (10000.0)

void
get_surface_type2()
{
  real            value;
  rgb_struct      smooth, opaque;

  ALLOCATE(surface[surfaces], surface_struct, 1, PARSE_TYPE);

  get_valid(scene, &value, -1.0, 1.0, "SURFACE COLOR Red");
  surface[surfaces]->emitter = value < 0.0;
  surface[surfaces]->color.r = ABS(value);
  get_valid(scene, &value, -1.0, 1.0, "SURFACE COLOR Green");
  surface[surfaces]->noshadow = value < 0.0;
  surface[surfaces]->color.g = ABS(value);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE COLOR Blue");
  surface[surfaces]->color.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE SMOOTHNESS Red");
  smooth.r = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE SMOOTHNESS Green");
  smooth.g = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE SMOOTHNESS Blue");
  smooth.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE METALNESS Red");
  surface[surfaces]->metal_factor.r = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE METALNESS Green");
  surface[surfaces]->metal_factor.g = value;
  get_valid(scene, &value, 0.0, 1.0, "SURFACE METALNESS Blue");
  surface[surfaces]->metal_factor.b = value;

  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Red");
  surface[surfaces]->transparent.r = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Green");
  surface[surfaces]->transparent.g = MIN(value, NEAR_ONE);
  get_valid(scene, &value, 0.0, 1.0, "SURFACE TRANSPARENCY Blue");
  surface[surfaces]->transparent.b = MIN(value, NEAR_ONE);

  /* Compute remaining parameters */
  opaque.r = 1.0 - surface[surfaces]->transparent.r;
  opaque.g = 1.0 - surface[surfaces]->transparent.g;
  opaque.b = 1.0 - surface[surfaces]->transparent.b;

  surface[surfaces]->diffuse.r = (1.0 - POWER(smooth.r, 3.0)) * opaque.r;
  surface[surfaces]->diffuse.g = (1.0 - POWER(smooth.g, 3.0)) * opaque.g;
  surface[surfaces]->diffuse.b = (1.0 - POWER(smooth.b, 3.0)) * opaque.b;

  surface[surfaces]->specular.r = opaque.r - surface[surfaces]->diffuse.r;
  surface[surfaces]->specular.g = opaque.g - surface[surfaces]->diffuse.g;
  surface[surfaces]->specular.b = opaque.b - surface[surfaces]->diffuse.b;

  surface[surfaces]->phong_factor = 9.0 / (1.0 / PHONG_FACTOR + 3.0 -
    smooth.r - smooth.g - smooth.b);

  surface[surfaces]->diffuse_factor.r = 1.0 - smooth.r *
    surface[surfaces]->metal_factor.r;
  surface[surfaces]->diffuse_factor.g = 1.0 - smooth.g *
    surface[surfaces]->metal_factor.g;
  surface[surfaces]->diffuse_factor.b = 1.0 - smooth.b *
    surface[surfaces]->metal_factor.b;
}

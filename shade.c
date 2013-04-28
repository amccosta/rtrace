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
 *    RAY TRACING - Shade - Version 8.3.0                             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1993              *
 **********************************************************************/

/***** Shading *****/
boolean
refract(ray, normal, inside, refraction)
  ray_ptr         ray;
  xyz_ptr         normal;
  boolean         inside;
  real            refraction;
{
  REG real        k0, k1, ratio;

  k0 = -DOT_PRODUCT(ray->vector, *normal);      /* Incidence angle cosine */
  if (ABS(k0) < ROUNDOFF)
    return FALSE;
  if (inside)
    ratio = refraction;         /* Leaving object */
  else
    ratio = 1.0 / refraction;   /* Entering object */
  k1 = 1.0 - SQR(ratio) * (1.0 - SQR(k0));
  if (k1 >= ROUNDOFF)
  {
    k1 = ratio * k0 - SQRT(k1);
    /* Refracted ray */
    ray->vector.x = ratio * ray->vector.x + k1 * normal->x;
    ray->vector.y = ratio * ray->vector.y + k1 * normal->y;
    ray->vector.z = ratio * ray->vector.z + k1 * normal->z;
    return TRUE;
  }
  return FALSE;
}
void
make_diffuse_vector(normal, result)
  xyz_ptr         normal, result;
{
  real            a1, sin_a1_sin_a2, cos_a1_sin_a2, sin_a2, cos_a2;
  xyz_struct      u, v;

  cos_a2 = SQRT(RANDOM);
  sin_a2 = SQRT(1.0 - SQR(cos_a2));
  a1 = 2 * PI * RANDOM;
  sin_a1_sin_a2 = SIN(a1) * sin_a2;
  cos_a1_sin_a2 = COS(a1) * sin_a2;
  v.x = 1.0;
  v.y = 0.0;
  v.z = 0.0;
  if (ABS(DOT_PRODUCT(*normal, v)) > 1.0 - ROUNDOFF)
  {
    v.y = 1.0;
    v.x = 0.0;
  }
  CROSS_PRODUCT(u, *normal, v);
  NORMALIZE(u);
  CROSS_PRODUCT(v, u, *normal);
  result->x = u.x * cos_a1_sin_a2 + v.x * sin_a1_sin_a2 + normal->x * cos_a2;
  result->y = u.y * cos_a1_sin_a2 + v.y * sin_a1_sin_a2 + normal->y * cos_a2;
  result->z = u.z * cos_a1_sin_a2 + v.z * sin_a1_sin_a2 + normal->z * cos_a2;
  NORMALIZE(*result);
}
void
make_specular_vector(reflected, normal, factor, result)
  xyz_ptr         reflected, normal, result;
  real            factor;
{
  real            a1, sin_a1_sin_a2, cos_a1_sin_a2, sin_a2, cos_a2;
  xyz_struct      u, v;

  do
  {
    cos_a2 = POWER(SQRT(RANDOM), factor + 1.0);
    sin_a2 = SQRT(1.0 - SQR(cos_a2));
    a1 = 2 * PI * RANDOM;
    sin_a1_sin_a2 = SIN(a1) * sin_a2;
    cos_a1_sin_a2 = COS(a1) * sin_a2;
    v.x = 1.0;
    v.y = 0.0;
    v.z = 0.0;
    if (ABS(DOT_PRODUCT(*reflected, v)) > 1.0 - ROUNDOFF)
    {
      v.y = 1.0;
      v.x = 0.0;
    }
    CROSS_PRODUCT(u, *reflected, v);
    NORMALIZE(u);
    CROSS_PRODUCT(v, u, *reflected);
    result->x = u.x * cos_a1_sin_a2 + v.x * sin_a1_sin_a2 +
      reflected->x * cos_a2;
    result->y = u.y * cos_a1_sin_a2 + v.y * sin_a1_sin_a2 +
      reflected->y * cos_a2;
    result->z = u.z * cos_a1_sin_a2 + v.z * sin_a1_sin_a2 +
      reflected->z * cos_a2;
    NORMALIZE(*result);
  } while (DOT_PRODUCT(*result, *normal) < ROUNDOFF);
}
int
estimate_diffuse(n, d)
  int             n;
  real            d;
{
   return ROUND((real) n * d);
}
int
estimate_specular(n, s, f)
  int             n;
  real            s, f;
{
#define S_THRESHOLD (0.5)
  if (s < S_THRESHOLD)
    return ROUND((real) n * s);
  else
    return ROUND((real) n / (1.0 + f) * (s + f / (1.0 - S_THRESHOLD) *
                                         (S_THRESHOLD - S_THRESHOLD * s))); 
#undef S_THRESHOLD
}
void
shade(position, normal, ray, object, color)
  xyz_ptr         position, normal;     /* Normal may be modified */
  ray_ptr         ray;
  object_ptr      object;
  rgb_ptr         color;
{
  switch (shade_mode)
  {
  case 0:
    shade_phong(position, normal, ray, object, color);
    break;
  case 1:
    shade_strauss(position, normal, ray, object, color);
    break;
  }
}

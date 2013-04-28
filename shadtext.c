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
 *  Reid Judd		- portability
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
 *    RAY TRACING - Shade texture - Version 8.1.0                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1991           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, December 1992          *
 **********************************************************************/

/***** Textures *****/
#define ADJUST(color)\
do {\
  (color).r = MIN(1.0, MAX(0.0, (color).r));\
  (color).g = MIN(1.0, MAX(0.0, (color).g));\
  (color).b = MIN(1.0, MAX(0.0, (color).b));\
} while (0)

#define SCALE(color, red, green, blue)\
do {\
  (color).r *= 2.0 * (red);\
  (color).g *= 2.0 * (green);\
  (color).b *= 2.0 * (blue);\
  ADJUST(color);\
} while (0)

static void
interpolate(t, colors, color)
  real            t;
  rgb_ptr         colors, color;
{
  REG int         i1, i2;
  REG real        t1, t2;
  if (t < 0.0)
  {
    STRUCT_ASSIGN(*color, colors[0]);
    return;
  }
  if (t >= 1.0)
  {
    STRUCT_ASSIGN(*color, colors[255]);
    return;
  }
  t1 = t * 256.0;
  i1 = (int) t1;
  if (i1 < 255)
    i2 = SUCC(i1);
  else
  {
    i1 = 255;
    i2 = 0;
  }
  t1 = FRAC(t1);
  t2 = 1.0 - t1;
  color->r = t1 * colors[i1].r + t2 * colors[i2].r;
  color->g = t1 * colors[i1].g + t2 * colors[i2].g;
  color->b = t1 * colors[i1].b + t2 * colors[i2].b;
}
static void
checker_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             x, y, z;
  checker_ptr     checker;

  checker = (checker_ptr) texture->data;
  x = position->x > 0.0 ? (int) position->x : (int) (1.0 - position->x);
  y = position->y > 0.0 ? (int) position->y : (int) (1.0 - position->y);
  z = position->z > 0.0 ? (int) position->z : (int) (1.0 - position->z);
  if (ODD(x + y + z))
    STRUCT_ASSIGN(*old_surface, *(surface[checker->surface_id]));
}
static void
blotch_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  blotch_ptr      blotch;

  blotch = (blotch_ptr) texture->data;
  t = black_noise(position);
  if (t > blotch->scale)
  {
    t = (t - blotch->scale) / (1.0 - blotch->scale);
    old_surface->color.r = old_surface->color.r * (1.0 - t) +
      surface[blotch->surface_id]->color.r * t;
    old_surface->color.g = old_surface->color.g * (1.0 - t) +
      surface[blotch->surface_id]->color.g * t;
    old_surface->color.b = old_surface->color.b * (1.0 - t) +
      surface[blotch->surface_id]->color.b * t;
    ADJUST(old_surface->color);
  }
}
static void
bump_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  xyz_struct      p;
  bump_ptr        bump;

  bump = (bump_ptr) texture->data;
  black_noise2(position, &p);
  normal->x += p.x * bump->scale;
  normal->y += p.y * bump->scale;
  normal->z += p.z * bump->scale;
  NORMALIZE(*normal);
}
static void
marble_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  rgb_struct      color;
  marble_ptr      marble;

  marble = (marble_ptr) texture->data;
  t = marble_noise(position);
  if (marble->color != NULL)
  {
    interpolate(t, marble->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
static void
fbm_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            t;
  rgb_struct      color;
  fbm_ptr         fbm;

  fbm = (fbm_ptr) texture->data;
  t = fbm_noise(position, fbm->omega, fbm->lambda, fbm->octaves);
  if (t < fbm->threshold)
    t = fbm->offset;
  else
    t = fbm->offset + fbm->scale * (t - fbm->threshold);
  if (fbm->color != NULL)
  {
    interpolate(t, fbm->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
static void
fbm_bump_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  xyz_struct      p;
  fbm_bump_ptr    fbm_bump;

  fbm_bump = (fbm_bump_ptr) texture->data;
  fbm_noise2(position, fbm_bump->omega, fbm_bump->lambda, fbm_bump->octaves,
             &p);
  normal->x += (p.x + fbm_bump->offset) * fbm_bump->scale;
  normal->y += (p.y + fbm_bump->offset) * fbm_bump->scale;
  normal->z += (p.z + fbm_bump->offset) * fbm_bump->scale;
  NORMALIZE(*normal);
}
static void
wood_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        r, g, b;
  REG real        brown_layer, green_layer;
  real            brown_noise, green_noise, green0_noise;
  real            chaos, t;
  wood_ptr        wood;

  wood = (wood_ptr) texture->data;
  chaos = chaos_noise(position, 7);
  t = SIN(SIN(8.0 * chaos + 7.0 * position->x + 3.0 * position->y));
  green_layer = ABS(t);
  brown_layer = green_layer;
  t = ABS(SIN(40.0 * chaos + 50.0 * position->z));
  brown_noise = 0.7 * t + 0.3;
  green_noise = 0.2 * t + 0.8;
  green0_noise = 0.15 * t + 0.85;
  g = POWER(ABS(brown_layer), 0.3);
  brown_layer = POWER(0.5 * (brown_layer + 1.0), 0.6) * brown_noise;
  green_layer = POWER(0.5 * (green_layer + 1.0), 0.6) * green_noise;
  r = ((wood->color.r + wood->color.b) * 0.5 * brown_layer +
       wood->color.g * green_layer) * g;
  b = ((wood->color.r + wood->color.b) * 0.25 * brown_layer +
       wood->color.g * green_layer) * g;
  g *= MAX(brown_layer, green_layer) * green0_noise;
  SCALE(old_surface->color, r, g, b);
}
static void
round_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  real            k1, k2;
  xyz_struct      u, v;
  round_ptr       round;

  round = (round_ptr) texture->data;
  if (round->scale < ROUNDOFF)
    return;
  v.x = 0.0;
  v.y = 1.0;
  v.z = 0.0;
  if (ABS(DOT_PRODUCT(*normal, v)) > 1.0 - ROUNDOFF)
  {
    v.x = 1.0;
    v.y = 0.0;
  }
  CROSS_PRODUCT(u, *normal, v);
  NORMALIZE(u);
  CROSS_PRODUCT(v, u, *normal);
  k1 = black_noise(position) * 2.0 - 1.0;
  position->x += k1;
  position->y += k1;
  position->z += k1;
  k2 = black_noise(position) * 2.0 - 1.0;
  k2 *= SQRT(1.0 - SQR(k1)) * round->scale;
  k1 *= round->scale;
  normal->x += k1 * u.x + k2 * v.x;
  normal->y += k1 * u.y + k2 * v.y;
  normal->z += k1 * u.z + k2 * v.z;
  NORMALIZE(*normal);
}
static void
bozo_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  xyz_struct      temp;
  rgb_struct      color;
  bozo_ptr        bozo;

  bozo = (bozo_ptr) texture->data;
  if (bozo->turbulence > ROUNDOFF)
  {
    turbulence2(position, &temp);
    position->x += temp.x * bozo->turbulence;
    position->y += temp.y * bozo->turbulence;
    position->z += temp.z * bozo->turbulence;
  }
  t = black_noise(position);
  if (bozo->color != NULL)
  {
    interpolate(t, bozo->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  if (t < 0.4)
  {
    SCALE(old_surface->color, 1.0, 1.0, 1.0);
    return;
  }
  if (t < 0.6)
  {
    SCALE(old_surface->color, 0.5, 1.0, 0.5);
    return;
  }
  if (t < 0.8)
  {
    SCALE(old_surface->color, 0.5, 0.5, 1.0);
    return;
  }
  SCALE(old_surface->color, 1.0, 0.5, 0.5);
}
static void
ripples_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        k, l, t;
  xyz_struct      temp;
  ripples_ptr     ripples;

  ripples = (ripples_ptr) texture->data;
  k = ripples->scale / (real) WAVES_MAX;
  for (i = 0; i < WAVES_MAX; POSINC(i))
  {
    temp.x = position->x - wave.source[i].x;
    temp.y = position->y - wave.source[i].y;
    temp.z = position->z - wave.source[i].z;
    l = LENGTH(temp);
    if (l < ROUNDOFF)
      l = 1.0;
    t = l * ripples->frequency + ripples->phase;
    t = cycloidal(t) * k / l;
    normal->x += temp.x * t;
    normal->y += temp.y * t;
    normal->z += temp.z * t;
  }
  NORMALIZE(*normal);
}
static void
waves_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        k, l, t;
  xyz_struct      temp;
  waves_ptr       waves;

  waves = (waves_ptr) texture->data;
  k = waves->scale / (real) WAVES_MAX;
  for (i = 0; i < WAVES_MAX; POSINC(i))
  {
    temp.x = position->x - wave.source[i].x;
    temp.y = position->y - wave.source[i].y;
    temp.z = position->z - wave.source[i].z;
    l = LENGTH(temp);
    if (l < ROUNDOFF)
      l = 1.0;
    t = l * waves->frequency * wave.frequency[i] + waves->phase;
    t = cycloidal(t) * k * wave.frequency[i] / l;
    normal->x += temp.x * t;
    normal->y += temp.y * t;
    normal->z += temp.z * t;
  }
  NORMALIZE(*normal);
}
static void
spotted_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  rgb_struct      color;
  spotted_ptr     spotted;

  spotted = (spotted_ptr) texture->data;
  t = black_noise(position);
  if (spotted->color != NULL)
  {
    interpolate(t, spotted->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
static void
dents_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  xyz_struct      temp;
  dents_ptr       dents;

  dents = (dents_ptr) texture->data;
  t = black_noise(position) * dents->scale;
  black_noise2(position, &temp);
  normal->x += temp.x * t;
  normal->y += temp.y * t;
  normal->z += temp.z * t;
  NORMALIZE(*normal);
}
static void
agate_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t, hue;
  rgb_struct      color;
  agate_ptr       agate;

  agate = (agate_ptr) texture->data;
  t = (cycloidal(1.3 * turbulence(position) + 1.1 * position->z) + 1.0) * 0.5;
  t = POWER(t, 0.77);
  if (agate->color != NULL)
  {
    interpolate(t, agate->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  hue = 1.0 - t;
  if (t < 0.5)
  {
    SCALE(old_surface->color, 1.0 - t / 10.0, 1.0 - t / 5.0, hue);
    return;
  }
  if (t < 0.6)
  {
    SCALE(old_surface->color, 0.9, 0.7, hue);
    return;
  }
  SCALE(old_surface->color, 0.6 + hue, 0.3 + hue, hue);
}
static void
wrinkles_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        scale;
  xyz_struct      r, p, value;
  wrinkles_ptr    wrinkles;

  wrinkles = (wrinkles_ptr) texture->data;
  black_noise2(position, &r);
  scale = 2.0;
  for (i = 1; i < 10; POSINC(i))
  {
    p.x = position->x * scale;
    p.y = position->y * scale;
    p.z = position->z * scale;
    black_noise2(&p, &value);
    r.x += ABS(value.x / scale);
    r.y += ABS(value.y / scale);
    r.z += ABS(value.z / scale);
    scale *= 2.0;
  }
  normal->x += r.x * wrinkles->scale;
  normal->y += r.y * wrinkles->scale;
  normal->z += r.z * wrinkles->scale;
  NORMALIZE(*normal);
}
static void
granite_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  int             i;
  REG real        t, scale;
  xyz_struct      p;
  rgb_struct      color;
  granite_ptr     granite;

  granite = (granite_ptr) texture->data;
  t = 0.0;
  scale = 2.0;
  for (i = 0; i < 6; POSINC(i))
  {
    p.x = position->x * scale * 4.0;
    p.y = position->y * scale * 4.0;
    p.z = position->z * scale * 4.0;
    t += black_noise(&p) / scale;
    scale *= 2.0;
  }
  if (granite->color != NULL)
  {
    interpolate(t, granite->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
static void
gradient_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t;
  xyz_struct      temp;
  rgb_struct      color;
  gradient_ptr    gradient;

  gradient = (gradient_ptr) texture->data;
  if (gradient->turbulence > ROUNDOFF)
  {
    turbulence2(position, &temp);
    position->x += temp.x * gradient->turbulence;
    position->y += temp.y * gradient->turbulence;
    position->z += temp.z * gradient->turbulence;
  }
  t = 0.0;
  if (ABS(gradient->direction.x) > ROUNDOFF)
    t += FRAC(gradient->direction.x * position->x);
  if (ABS(gradient->direction.y) > ROUNDOFF)
    t += FRAC(gradient->direction.y * position->y);
  if (ABS(gradient->direction.z) > ROUNDOFF)
    t += FRAC(gradient->direction.z * position->z);
  t = FRAC(t);

  if (gradient->color != NULL)
  {
    interpolate(t, gradient->color, &color);
    SCALE(old_surface->color, color.r, color.g, color.b);
    return;
  }
  SCALE(old_surface->color, t, t, t);
}
static void
image_map_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG unsigned char value;
  REG real        u, v, u0, v0, u1, v1, k;
  REG long        offset;
  xyz_struct      temp;
  image_map_ptr   image_map;

  image_map = (image_map_ptr) texture->data;
  if (image_map->turbulence > ROUNDOFF)
  {
    turbulence2(position, &temp);
    position->x += temp.x * image_map->turbulence;
    position->y += temp.y * image_map->turbulence;
    position->z += temp.z * image_map->turbulence;
  }
  switch (image_map->u)
  {
  case X_AXIS:
    if (image_map->once AND((position->x < 0.0) OR(position->x > 1.0)))
      return;
    u = (1.0 - FRAC(position->x)) * (real) (image_map->width - 1);
    break;
  case Y_AXIS:
    if (image_map->once AND((position->y < 0.0) OR(position->y > 1.0)))
      return;
    u = (1.0 - FRAC(position->y)) * (real) (image_map->width - 1);
    break;
  case Z_AXIS:
    if (image_map->once AND((position->z < 0.0) OR(position->z > 1.0)))
      return;
    u = (1.0 - FRAC(position->z)) * (real) (image_map->width - 1);
    break;
  }
  switch (image_map->v)
  {
  case X_AXIS:
    if (image_map->once AND((position->x < 0.0) OR(position->x > 1.0)))
      return;
    v = (1.0 - FRAC(position->x)) * (real) (image_map->height - 1);
    break;
  case Y_AXIS:
    if (image_map->once AND((position->y < 0.0) OR(position->y > 1.0)))
      return;
    v = (1.0 - FRAC(position->y)) * (real) (image_map->height - 1);
    break;
  case Z_AXIS:
    if (image_map->once AND((position->z < 0.0) OR(position->z > 1.0)))
      return;
    v = (1.0 - FRAC(position->z)) * (real) (image_map->height - 1);
    break;
  }
  if ((u < 0.0) OR(u >= (real) image_map->width)
      OR(v < 0.0) OR(v >= (real) image_map->height))
    runtime_abort("bad 2D MAPPING COORDINATES");
  u1 = FRAC(u);
  v1 = FRAC(v);
  u0 = 1.0 - u1;
  v0 = 1.0 - v1;
  offset = ((long) v * (long) (image_map->width) + (long) u) *
           3 + image_format_offset;
  SEEK(image_map->image, offset);
  if (IO_status != IO_OK)
    runtime_abort("unable to seek IMAGE");
  k = u0 * v0;
  READ_CHAR(image_map->image, value);
  old_surface->color.r = (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.g = (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.b = (real) (value) * k;
  k = u1 * v0;
  READ_CHAR(image_map->image, value);
  old_surface->color.r += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.g += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.b += (real) (value) * k;
  offset = (((long) v + 1) * (long) (image_map->width) + (long) u) *
           3 + image_format_offset;
  SEEK(image_map->image, offset);
  if (IO_status != IO_OK)
    runtime_abort("unable to seek IMAGE");
  k = u0 * v1;
  READ_CHAR(image_map->image, value);
  old_surface->color.r += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.g += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.b += (real) (value) * k;
  k = u1 * v1;
  READ_CHAR(image_map->image, value);
  old_surface->color.r += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.g += (real) (value) * k;
  READ_CHAR(image_map->image, value);
  old_surface->color.b += (real) (value) * k;
  k = 1.0 / 256.0;
  old_surface->color.r *= k;
  old_surface->color.g *= k;
  old_surface->color.b *= k;
}
static void
gloss_texture(position, normal, old_surface, texture)
  xyz_ptr         position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  REG real        t1, t2;
  gloss_ptr       gloss;

  gloss = (gloss_ptr) texture->data;
  t1 = black_noise(position) * gloss->scale;
  t2 = 1.0 - t1;
  old_surface->specular.r += old_surface->diffuse.r * t1;
  old_surface->specular.g += old_surface->diffuse.g * t1;
  old_surface->specular.b += old_surface->diffuse.b * t1;
  old_surface->diffuse.r *= t2;
  old_surface->diffuse.g *= t2;
  old_surface->diffuse.b *= t2;
  old_surface->phong_factor = 1.0 + 1000.0 * t1;
}
static void
bump3_texture(position, normal, old_position, texture)
  xyz_ptr         position, normal, old_position;
  texture_ptr     texture;
{
  xyz_struct      p, n, u, v, p1, p2, p3, p4;
  bump3_ptr       bump3;

  bump3 = (bump3_ptr) texture->data;
  if (bump3->scale < ROUNDOFF)
    return;
  v.x = 0.0;
  v.y = 1.0;
  v.z = 0.0;
  if (ABS(DOT_PRODUCT(*normal, v)) > 1.0 - ROUNDOFF)
  {
    v.x = 1.0;
    v.y = 0.0;
  }
  CROSS_PRODUCT(u, *normal, v);
  NORMALIZE(u);
  CROSS_PRODUCT(v, u, *normal);
  u.x *= bump3->size;
  u.y *= bump3->size;
  u.z *= bump3->size;
  v.x *= bump3->size;
  v.y *= bump3->size;
  v.z *= bump3->size;
  STRUCT_ASSIGN(p, *position);
  black_noise2(position, &n);
  position->x += n.x * bump3->scale;
  position->y += n.y * bump3->scale;
  position->z += n.z * bump3->scale;
  old_position->x += n.x * bump3->scale;
  old_position->y += n.y * bump3->scale;
  old_position->z += n.z * bump3->scale;
  p1.x = p.x + v.x;
  p1.y = p.y + v.y;
  p1.z = p.z + v.z;
  black_noise2(&p1, &n);
  p1.x += n.x * bump3->scale - position->x;
  p1.y += n.y * bump3->scale - position->y;
  p1.z += n.z * bump3->scale - position->z;
  p2.x = p.x + u.x;
  p2.y = p.y + u.y;
  p2.z = p.z + u.z;
  black_noise2(&p2, &n);
  p2.x += n.x * bump3->scale - position->x;
  p2.y += n.y * bump3->scale - position->y;
  p2.z += n.z * bump3->scale - position->z;
  p3.x = p.x - v.x;
  p3.y = p.y - v.y;
  p3.z = p.z - v.z;
  black_noise2(&p3, &n);
  p3.x += n.x * bump3->scale - position->x;
  p3.y += n.y * bump3->scale - position->y;
  p3.z += n.z * bump3->scale - position->z;
  p4.x = p.x - u.x;
  p4.y = p.y - u.y;
  p4.z = p.z - u.z;
  black_noise2(&p4, &n);
  p4.x += n.x * bump3->scale - position->x;
  p4.y += n.y * bump3->scale - position->y;
  p4.z += n.z * bump3->scale - position->z;
  CROSS_PRODUCT(*normal, p1, p2);
  CROSS_PRODUCT(n, p2, p3);
  normal->x += n.x;
  normal->y += n.y;
  normal->z += n.z;
  CROSS_PRODUCT(n, p3, p4);
  normal->x += n.x;
  normal->y += n.y;
  normal->z += n.z;
  CROSS_PRODUCT(n, p4, p1);
  normal->x += n.x;
  normal->y += n.y;
  normal->z += n.z;
  NORMALIZE(*normal);
}
/***** End of textures *****/
void
surface_texture(old_position, normal, old_surface, texture)
  xyz_ptr         old_position, normal;
  surface_ptr     old_surface;
  texture_ptr     texture;
{
  xyz_struct      position, temp;

  STRUCT_ASSIGN(position, *old_position);
  /* Apply texture(s) */
  for (; texture != NULL; texture = (texture_ptr) texture->next)
  {
    if (texture->transf != NULL)
    {
      /* Transform position */
      STRUCT_ASSIGN(temp, position);
      transform(texture->transf, &temp, &position);
    }
    switch (texture->type)
    {
    case NULL_TYPE:
      break;
    case CHECKER_TYPE:
      checker_texture(&position, normal, old_surface, texture);
      break;
    case BLOTCH_TYPE:
      blotch_texture(&position, normal, old_surface, texture);
      break;
    case BUMP_TYPE:
      bump_texture(&position, normal, old_surface, texture);
      break;
    case MARBLE_TYPE:
      marble_texture(&position, normal, old_surface, texture);
      break;
    case FBM_TYPE:
      fbm_texture(&position, normal, old_surface, texture);
      break;
    case FBM_BUMP_TYPE:
      fbm_bump_texture(&position, normal, old_surface, texture);
      break;
    case WOOD_TYPE:
      wood_texture(&position, normal, old_surface, texture);
      break;
    case ROUND_TYPE:
      round_texture(&position, normal, old_surface, texture);
      break;
    case BOZO_TYPE:
      bozo_texture(&position, normal, old_surface, texture);
      break;
    case RIPPLES_TYPE:
      ripples_texture(&position, normal, old_surface, texture);
      break;
    case WAVES_TYPE:
      waves_texture(&position, normal, old_surface, texture);
      break;
    case SPOTTED_TYPE:
      spotted_texture(&position, normal, old_surface, texture);
      break;
    case DENTS_TYPE:
      dents_texture(&position, normal, old_surface, texture);
      break;
    case AGATE_TYPE:
      agate_texture(&position, normal, old_surface, texture);
      break;
    case WRINKLES_TYPE:
      wrinkles_texture(&position, normal, old_surface, texture);
      break;
    case GRANITE_TYPE:
      granite_texture(&position, normal, old_surface, texture);
      break;
    case GRADIENT_TYPE:
      gradient_texture(&position, normal, old_surface, texture);
      break;
    case IMAGE_MAP_TYPE:
      image_map_texture(&position, normal, old_surface, texture);
      break;
    case GLOSS_TYPE:
      gloss_texture(&position, normal, old_surface, texture);
      break;
    case BUMP3_TYPE:
      bump3_texture(&position, normal, old_position, texture);
      break;
    }
  }
}

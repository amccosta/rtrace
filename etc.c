/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - stereo view mode
 *  Rodney Bogart       - depth of field (lens camera model)
 *  Reid Judd           - portability
 *  Douglas Voorhies    - Hilbert pixel sequence
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
 *    RAY TRACING - Etc - Version 8.4.1 (PVM3)                        *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1994         *
 **********************************************************************/

#ifdef _Windows
#pragma option -zEstrings -zFstringc -zHstringg
#define FAR far
#else
#define FAR
#endif
static char FAR str1[] = "Error: %s\n";
static char FAR str2[] = "TRANSFORMATION Matrix not invertible";
static char FAR str3[] = "EYE POINT equal to LOOK POINT";
static char FAR str4[] = "no UP VECTOR";
static char FAR str5[] = "bad UP VECTOR";
static char FAR str6[] = "EYE POINT equal to LOOK POINT in STEREO MODE";
static char FAR str7[] = "bad UP VECTOR in STEREO MODE";
static char FAR str8[] = "Resolution %dx%d\n";
static char FAR str9[] = "Total eye rays:          %-0.0f\n";
static char FAR str10[] = "Total shadow rays:       %-0.0f\n";
static char FAR str11[] = "Total reflected rays:    %-0.0f\n";
static char FAR str12[] = "Total refracted rays:    %-0.0f\n";
static char FAR str13[] = "Total ambient rays:      %-0.0f\n";
static char FAR str14[] = "Total shadow hits:       %-0.0f\n";
static char FAR str15[] = "Total shadow cache hits: %-0.0f\n";
static char FAR str16[] = "Total ray hits:          %-0.0f\n";
static char FAR str17[] = "Total ray cache resets:  %-0.0f\n";
static char FAR str18[] = "Total ray cache hits:    %-0.0f\n";
static char FAR str19[] = "Total amb. cache resets: %-0.0f\n";
static char FAR str20[] = "Total amb. cache hits:   %-0.0f\n";
static char FAR str21[] = "Total queue resets:      %-0.0f\n";
static char FAR str22[] = "Total queue insertions:  %-0.0f\n";
static char FAR str23[] = "Total queue extractions: %-0.0f\n";
static char FAR str24[] = "Total pixel divisions:   %-0.0f\n";
static char FAR str25[] = "Maximum shading level:   %d\n";
static char FAR str26[] = "%d Object(s), %d Light(s), %d Surface(s)\n";
static char FAR str27[] = "Total octant inclusion tests:      %-0.0f\n";
static char FAR str28[] = "Total BV intersection tests:       %-0.0f\n";
static char FAR str29[] = "Total backface removal tests:      %-0.0f\n";
static char FAR str30[] = "Total backface removal hits:       %-0.0f\n";
static char FAR str31[] = "Total sphere intersection tests:   %-0.0f\n";
static char FAR str32[] = "Total box intersection tests:      %-0.0f\n";
static char FAR str33[] = "Total patch intersection tests:    %-0.0f\n";
static char FAR str34[] = "Total cone intersection tests:     %-0.0f\n";
static char FAR str35[] = "Total polygon intersection tests:  %-0.0f\n";
static char FAR str36[] = "Total triangle intersection tests: %-0.0f\n";
static char FAR str37[] = "Total text intersection tests:     %-0.0f\n";
static char FAR str38[] = "Total CSG intersection tests:      %-0.0f\n";
static char FAR str39[] = "Total list intersection tests:     %-0.0f\n";
static char FAR str40[] = "Info: enclosing - %g second(s) CPU time\n";
static char FAR str41[] = " Line %d completed\n";
static char FAR str42[] = "Info: tracing - %g second(s) CPU time\n";
#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

/***** Main *****/
#ifndef PVM
#define PVM 0
#endif

#if PVM != 1
real
tang(x)
  real            x;
{
  if (ABS(COS(x)) < ROUNDOFF)
    return (COS(x) >= 0.0 ? INFINITY : -INFINITY);
  return (real) tan((double) x);
}
vertex_ptr
vertex_pointer(v, vertices, vertex_top, vertex_bottom)
  REG int         v;
  int            *vertices;
  vertex_ptr     *vertex_top, *vertex_bottom;
{
  REG int         i;
  REG vertex_ptr  vertex;

  if (v > *vertices)
  {
    if (*vertices == 0)
    {
      ALLOCATE(*vertex_top, vertex_struct, 1, PARSE_TYPE);
      *vertex_bottom = *vertex_top;
      *vertices = 1;
    } else
    {
      *vertex_bottom = *vertex_top;
      for (i = 2; i <= *vertices; POSINC(i))
        *vertex_bottom = (vertex_ptr) ((*vertex_bottom)->next);
    }
    for (i = SUCC(*vertices); i <= v; POSINC(i))
    {
      ALLOCATE(vertex, vertex_struct, 1, PARSE_TYPE);
      (*vertex_bottom)->next = (void_ptr) vertex;
      *vertex_bottom = vertex;
    }
    *vertices = v;
    return *vertex_bottom;
  }
  vertex = *vertex_top;
  for (i = 2; i <= v; POSINC(i))
    vertex = (vertex_ptr) (vertex->next);
  return vertex;
}
static real     tangent_x, tangent_y;
static real     opening_x, opening_y;
static xyz_struct eye_pin_hole, screen_unit_x, screen_unit_y;

void
make_vector(p, x, y, delta)
  xyz_ptr         p;
  real            x, y, delta;
{
  REG real        kx, ky;
  xyz_struct      aperture;

  if (jittering_mode == 1)
  {
    x += JITTER * delta;
    y += JITTER * delta;
  }
  kx = x * opening_x - tangent_x;
  ky = y * opening_y - tangent_y;
  p->x = kx * screen_x.x + ky * screen_y.x + gaze.x;
  p->y = kx * screen_x.y + ky * screen_y.y + gaze.y;
  p->z = kx * screen_x.z + ky * screen_y.z + gaze.z;
  NORMALIZE(*p);
  if (focal_aperture > ROUNDOFF)
  {
    UNIT_CIRCLE_POINT(kx, ky);
    kx *= focal_aperture;
    ky *= focal_aperture;
    aperture.x = kx * screen_unit_x.x + ky * screen_unit_y.x;
    aperture.y = kx * screen_unit_x.y + ky * screen_unit_y.y;
    aperture.z = kx * screen_unit_x.z + ky * screen_unit_y.z;
    eye.x = eye_pin_hole.x + aperture.x;
    eye.y = eye_pin_hole.y + aperture.y;
    eye.z = eye_pin_hole.z + aperture.z;
    p->x = focal_distance * p->x - aperture.x;
    p->y = focal_distance * p->y - aperture.y;
    p->z = focal_distance * p->z - aperture.z;
    NORMALIZE(*p);
  }
}
void
transform(t, p1, p2)
  xyzw_ptr       t;
  xyz_ptr        p1, p2;
{
  REG real       w;

  p2->x = p1->x * t[0].x + p1->y * t[0].y + p1->z * t[0].z + t[0].w;
  p2->y = p1->x * t[1].x + p1->y * t[1].y + p1->z * t[1].z + t[1].w;
  p2->z = p1->x * t[2].x + p1->y * t[2].y + p1->z * t[2].z + t[2].w;
  if (t[3].w == 0.0)
    return;
  w = p1->x * t[3].x + p1->y * t[3].y + p1->z * t[3].z + t[3].w;
  if (ABS(w) < ROUNDOFF)
    return;
  w = 1.0 / w;
  p2->x *= w;
  p2->y *= w;
  p2->z *= w;
}
void
transform_vector(t, p1, v1, p2, v2)
  xyzw_ptr       t;
  xyz_ptr        p1, v1, p2, v2;
{
  xyz_struct     temp;

  temp.x = p1->x + v1->x;
  temp.y = p1->y + v1->y;
  temp.z = p1->z + v1->z;
  transform(t, &temp, v2);
  v2->x -= p2->x;
  v2->y -= p2->y;
  v2->z -= p2->z;
}
void
transform_normal_vector(t, p1, v1, v2)
  xyzw_ptr       t;
  xyz_ptr        p1, v1, v2;
{
  /*
   * REG real       w;
   */
  xyz_struct     p2, temp;

  STRUCT_ASSIGN(temp, *p1);
  /* Transpost matrix */
  p2.x = temp.x * t[0].x + temp.y * t[1].x + temp.z * t[2].x + t[3].x;
  p2.y = temp.x * t[0].y + temp.y * t[1].y + temp.z * t[2].y + t[3].y;
  p2.z = temp.x * t[0].z + temp.y * t[1].z + temp.z * t[2].z + t[3].z;
  /*
   * w = temp.x * t[0].w + temp.y * t[1].w + temp.z * t[2].w + t[3].w;
   * if (ABS(w) > ROUNDOFF)
   * {
   *   w = 1.0 / w;
   *   p2.x *= w;
   *   p2.y *= w;
   *   p2.z *= w;
   * }
   */
  temp.x += v1->x;
  temp.y += v1->y;
  temp.z += v1->z;
  /* Transpost matrix */
  v2->x = temp.x * t[0].x + temp.y * t[1].x + temp.z * t[2].x;
  v2->y = temp.x * t[0].y + temp.y * t[1].y + temp.z * t[2].y;
  v2->z = temp.x * t[0].z + temp.y * t[1].z + temp.z * t[2].z;
  /*
   * w = temp.x * t[0].w + temp.y * t[1].w + temp.z * t[2].w + t[3].w;
   * if (ABS(w) > ROUNDOFF)
   * {
   *   w = 1.0 / w;
   *   v2->x *= w;
   *   v2->y *= w;
   *   v2->z *= w;
   * }
   */
  v2->x -= p2.x;
  v2->y -= p2.y;
  v2->z -= p2.z;
}
real
transform_distance(t, distance, p1, v1, p2)
  xyzw_ptr       t;
  real           distance;
  xyz_ptr        p1, v1, p2;
{
  xyz_struct     temp1, temp2;

  temp1.x = p1->x + v1->x * distance;
  temp1.y = p1->y + v1->y * distance;
  temp1.z = p1->z + v1->z * distance;
  transform(t, &temp1, &temp2);
  temp2.x -= p2->x;
  temp2.y -= p2->y;
  temp2.z -= p2->z;
  return LENGTH(temp2);
}
void
inverse_transform(t1, t2)
  xyzw_ptr        t1, t2;
{
  real            a[4][4];
  real            b[4], c[4], temp[4];
  REG int         row_pivot, i, j, k;
  REG real        row_max, col_max, col_ratio, temp0;
  int             pivot[4];

  a[0][0] = t1[0].x;
  a[0][1] = t1[0].y;
  a[0][2] = t1[0].z;
  a[0][3] = t1[0].w;
  a[1][0] = t1[1].x;
  a[1][1] = t1[1].y;
  a[1][2] = t1[1].z;
  a[1][3] = t1[1].w;
  a[2][0] = t1[2].x;
  a[2][1] = t1[2].y;
  a[2][2] = t1[2].z;
  a[2][3] = t1[2].w;
  a[3][0] = t1[3].x;
  a[3][1] = t1[3].y;
  a[3][2] = t1[3].z;
  a[3][3] = t1[3].w;
  /* Pivoting */
  for (i = 0; i <= 3; POSINC(i))
  {
    pivot[i] = i;
    row_max = 0.0;
    for (j = 0; j <= 3; POSINC(j))
      row_max = MAX(row_max, ABS(a[i][j]));
    if (ABS(row_max) < ROUNDOFF)
      runtime_abort(str2);
    temp[i] = row_max;
  }
  for (i = 0; i <= 2; POSINC(i))
  {
    col_max = ABS(a[i][i]) / temp[i];
    row_pivot = i;
    for (j = SUCC(i); j <= 3; POSINC(j))
    {
      col_ratio = ABS(a[j][i]) / temp[j];
      if (col_ratio > col_max)
      {
        col_max = col_ratio;
        row_pivot = j;
      }
    }
    if (ABS(col_max) < ROUNDOFF)
      runtime_abort(str2);
    if (row_pivot > i)
    {
      j = pivot[row_pivot];
      pivot[row_pivot] = pivot[i];
      pivot[i] = j;
      temp0 = temp[row_pivot];
      temp[row_pivot] = temp[i];
      temp[i] = temp0;
      for (j = 0; j <= 3; POSINC(j))
      {
        temp0 = a[row_pivot][j];
        a[row_pivot][j] = a[i][j];
        a[i][j] = temp0;
      }
    }
    for (j = SUCC(i); j <= 3; POSINC(j))
    {
      a[j][i] /= a[i][i];
      temp0 = a[j][i];
      for (k = SUCC(i); k <= 3; POSINC(k))
        a[j][k] -= temp0 * a[i][k];
    }
  }
  if (ABS(a[3][3]) < ROUNDOFF)
    runtime_abort(str2);
  /* Column 1 */
  b[0] = 1.0;
  b[1] = 0.0;
  b[2] = 0.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].x = c[0];
  t2[1].x = c[1];
  t2[2].x = c[2];
  t2[3].x = c[3];
  /* Column 2 */
  b[0] = 0.0;
  b[1] = 1.0;
  b[2] = 0.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].y = c[0];
  t2[1].y = c[1];
  t2[2].y = c[2];
  t2[3].y = c[3];
  /* Column 3 */
  b[0] = 0.0;
  b[1] = 0.0;
  b[2] = 1.0;
  b[3] = 0.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].z = c[0];
  t2[1].z = c[1];
  t2[2].z = c[2];
  t2[3].z = c[3];
  /* Column 4 */
  b[0] = 0.0;
  b[1] = 0.0;
  b[2] = 0.0;
  b[3] = 1.0;
  c[0] = b[pivot[0]];
  for (i = 1; i <= 3; POSINC(i))
  {
    temp0 = 0.0;
    for (j = 0; j <= PRED(i); POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = b[pivot[i]] - temp0;
  }
  c[3] /= a[3][3];
  for (i = 2; i >= 0; POSDEC(i))
  {
    temp0 = 0.0;
    for (j = SUCC(i); j <= 3; POSINC(j))
      temp0 += a[i][j] * c[j];
    c[i] = (c[i] - temp0) / a[i][i];
  }
  t2[0].w = c[0];
  t2[1].w = c[1];
  t2[2].w = c[2];
  t2[3].w = c[3];
}
void
normalize_transform(t)
  xyzw_ptr        t;
{
  REG real        k;

  if (t[3].w == 0.0)
    return;
  if (ABS(1.0 - t[3].w) < ROUNDOFF)
  {
    if ((ABS(t[3].x) < ROUNDOFF) AND(ABS(t[3].y) < ROUNDOFF)
        AND(ABS(t[3].z) < ROUNDOFF))
      t[3].w = 0.0;
    return;
  }
  k = 1.0 / t[3].w;
  t[0].x *= k;
  t[0].y *= k;
  t[0].z *= k;
  t[0].w *= k;
  t[1].x *= k;
  t[1].y *= k;
  t[1].z *= k;
  t[1].w *= k;
  t[2].x *= k;
  t[2].y *= k;
  t[2].z *= k;
  t[2].w *= k;
  t[3].x *= k;
  t[3].y *= k;
  t[3].z *= k;
  if ((ABS(t[3].x) < ROUNDOFF) AND(ABS(t[3].y) < ROUNDOFF)
      AND(ABS(t[3].z) < ROUNDOFF))
    t[3].w = 0.0;
}
void
make_view()
{
  REG real        kx, ky;
  xyz_struct      screen_xy;
  real            separation;

  gaze.x = look.x - eye.x;
  gaze.y = look.y - eye.y;
  gaze.z = look.z - eye.z;
  gaze_distance = LENGTH(gaze);
  if (gaze_distance < ROUNDOFF)
    runtime_abort(str3);
  NORMALIZE(gaze);
  if (LENGTH(up) < ROUNDOFF)
    runtime_abort(str4);
  NORMALIZE(up);
  if (ABS(DOT_PRODUCT(gaze, up)) > COS(ANGLE_MIN))
    runtime_abort(str5);
  CROSS_PRODUCT(screen_x, gaze, up);
  NORMALIZE(screen_x);
  CROSS_PRODUCT(screen_y, gaze, screen_x);
  if (view_mode != 0)
  {
    if (stereo_separation < -ROUNDOFF)
      separation = gaze_distance * -stereo_separation;
    else
      separation = stereo_separation;
    if (view_mode == STEREO_LEFT)
      kx = -0.5 * separation;
    if (view_mode == STEREO_RIGHT)
      kx = 0.5 * separation;
    eye.x += kx * screen_x.x;
    eye.y += kx * screen_x.y;
    eye.z += kx * screen_x.z;
    gaze.x = look.x - eye.x;
    gaze.y = look.y - eye.y;
    gaze.z = look.z - eye.z;
    gaze_distance = LENGTH(gaze);
    if (gaze_distance < ROUNDOFF)
      runtime_abort(str6);
    NORMALIZE(gaze);
    if (ABS(DOT_PRODUCT(gaze, up)) > COS(ANGLE_MIN))
      runtime_abort(str7);
    CROSS_PRODUCT(screen_x, gaze, up);
    NORMALIZE(screen_x);
    CROSS_PRODUCT(screen_y, gaze, screen_x);
  }
  if (focal_distance < ROUNDOFF)
    focal_distance = gaze_distance;
  else
    gaze_distance = focal_distance;
  tangent_x = tang(view_angle_x);
  tangent_y = tang(view_angle_y);
  opening_x = 2.0 / (real) screen_size_x * tangent_x;
  opening_y = 2.0 / (real) screen_size_y * tangent_y;
  kx = gaze_distance * opening_x;
  ky = gaze_distance * opening_y;
  screen_xy.x = kx * screen_x.x + ky * screen_y.x;
  screen_xy.y = kx * screen_x.y + ky * screen_y.y;
  screen_xy.z = kx * screen_x.z + ky * screen_y.z;
  pixel_distance = LENGTH(screen_xy);
  threshold_distance = pixel_distance * DISTANCE_FACTOR;
  if (focal_aperture > ROUNDOFF)
  {
    STRUCT_ASSIGN(eye_pin_hole, eye);
    kx *= (real) screen_size_x;
    ky *= (real) screen_size_y;
    screen_unit_x.x = kx * screen_x.x;
    screen_unit_x.y = kx * screen_x.y;
    screen_unit_x.z = kx * screen_x.z;
    screen_unit_y.x = ky * screen_y.x;
    screen_unit_y.y = ky * screen_y.y;
    screen_unit_y.z = ky * screen_y.z;
  }
}
#endif
void
init_globals()
{
  REG int         i;

  /* Rays */
  eye_rays = 0.0;
  shadow_rays = 0.0;
  reflected_rays = 0.0;
  refracted_rays = 0.0;
  ambient_rays = 0.0;
  shadow_hits = 0.0;
  shadow_cache_hits = 0.0;
  /* Intersection tests */
  octant_tests = 0.0;
  bound_tests = 0.0;
  sphere_tests = 0.0;
  box_tests = 0.0;
  patch_tests = 0.0;
  cone_tests = 0.0;
  polygon_tests = 0.0;
  triangle_tests = 0.0;
  text_tests = 0.0;
  csg_tests = 0.0;
  list_tests = 0.0;
  /* Others */
  shade_level = 0;
  shade_level_max = 0;
  ray_node = 0;
  ray_hits = 0.0;
  ray_cache_resets = 0.0;
  ray_cache_hits = 0.0;
  backface_tests = 0.0;
  backface_hits = 0.0;
  distributed_cache_resets = 0.0;
  distributed_cache_hits = 0.0;
  pqueue_resets = 0.0;
  pqueue_insertions = 0.0;
  pqueue_extractions = 0.0;
  pixel_divisions = 0.0;
#if PVM != 1
  for (i = 0; i < RAY_SIZE_MAX; POSINC(i))
    ray_cache[i] = NO_OBJECTS;
  RANDOM_START;
  if (texture_mode != 0)
    init_texture();
  previous_repetitions = MAXINT;
#endif
}
#if PVM == 1
void
#else
static void
#endif
trace_stats()
{
  WRITE(results, str8, screen_size_x, screen_size_y);
  FLUSH(results);
  WRITE(results, str9, eye_rays);
  FLUSH(results);
  WRITE(results, str10, shadow_rays);
  FLUSH(results);
  if (reflected_rays > 0)
  {
    WRITE(results, str11, reflected_rays);
    FLUSH(results);
  }
  if (refracted_rays > 0)
  {
    WRITE(results, str12, refracted_rays);
    FLUSH(results);
  }
  if (ambient_rays > 0)
  {
    WRITE(results, str13, ambient_rays);
    FLUSH(results);
  }
  WRITE(results, str14, shadow_hits);
  FLUSH(results);
  WRITE(results, str15, shadow_cache_hits);
  FLUSH(results);
  WRITE(results, str16, ray_hits);
  FLUSH(results);
  WRITE(results, str17, ray_cache_resets);
  FLUSH(results);
  WRITE(results, str18, ray_cache_hits);
  FLUSH(results);
  if (distributed_cache_mode != 0)
  {
    WRITE(results, str19, distributed_cache_resets);
    FLUSH(results);
    WRITE(results, str20, distributed_cache_hits);
    FLUSH(results);
  }
  WRITE(results, str21, pqueue_resets);
  FLUSH(results);
  WRITE(results, str22, pqueue_insertions);
  FLUSH(results);
  WRITE(results, str23, pqueue_extractions);
  FLUSH(results);
  WRITE(results, str24, pixel_divisions);
  FLUSH(results);
  WRITE(results, str25, shade_level_max);
  FLUSH(results);
  WRITE(results, str26, objects, lights, surfaces);
  FLUSH(results);
  WRITE(results, str27, octant_tests);
  FLUSH(results);
  if (bound_tests > 0.0)
  {
    WRITE(results, str28, bound_tests);
    FLUSH(results);
  }
  if (backface_tests > 0.0)
  {
    WRITE(results, str29, backface_tests);
    WRITE(results, str30, backface_hits);
    FLUSH(results);
  }
  if (sphere_tests > 0.0)
  {
    WRITE(results, str31, sphere_tests);
    FLUSH(results);
  }
  if (box_tests > 0.0)
  {
    WRITE(results, str32, box_tests);
    FLUSH(results);
  }
  if (patch_tests > 0.0)
  {
    WRITE(results, str33, patch_tests);
    FLUSH(results);
  }
  if (cone_tests > 0.0)
  {
    WRITE(results, str34, cone_tests);
    FLUSH(results);
  }
  if (polygon_tests > 0.0)
  {
    WRITE(results, str35, polygon_tests);
    FLUSH(results);
  }
  if (triangle_tests > 0.0)
  {
    WRITE(results, str36, triangle_tests);
    FLUSH(results);
  }
  if (text_tests > 0.0)
  {
    WRITE(results, str37, text_tests);
    FLUSH(results);
  }
  if (csg_tests > 0.0)
  {
    WRITE(results, str38, csg_tests);
    FLUSH(results);
  }
  if (list_tests > 0.0)
  {
    WRITE(results, str39, list_tests);
    FLUSH(results);
  }
}
#if PVM != 1
void
setup_scene()
{
  real            total_time;

  total_time = CPU_CLOCK;
  enclose_all();
  if (verbose_mode > 1)
  {
    WRITE(ERROR, str40, (CPU_CLOCK - total_time) / 1000.0);
    FLUSH(ERROR);
  }
}

#ifdef dos
#ifndef SUIT
#ifdef __GNUC__
#define GRX
extern void     display_plot();
#endif
#endif
#endif
#endif /* PVM */

#if PVM == 0
void
ray_trace()
{
  int             x, x_pitch, xi, y, yi, count;
  rgb_struct      color;
  ray_struct      ray;
  pixel_ptr       temp_image;
  real            start_time, total_time;

  x_pitch = screen_size_x + 1;
  count = x_pitch * (screen_size_y + 1);
  ALLOCATE(temp_image, pixel_struct, count, OTHER_TYPE);
  ray.level.r = 1.0;
  ray.level.g = 1.0;
  ray.level.b = 1.0;
  ray.inside = FALSE;
  x = 0;
  y = 0;
  xi = 0;
  yi = 0;
  if (verbose_mode > 2)
  {
    WRITE(ERROR, "Preliminary phase\n");
    FLUSH(ERROR);
  }
  total_time = CPU_CLOCK;
  while (count > 0)
  {
    POSDEC(count);
    INC_PIXEL_COUNTER;
    REALINC(eye_rays);
    make_vector(&(ray.vector), (real) x, (real) y, 1.0);
    if (intersect_all(NO_OBJECTS, &eye, &ray, &color) > 0.0)
      STRUCT_ASSIGN(temp_image[y * x_pitch + x].color, color);
    else
      STRUCT_ASSIGN(temp_image[y * x_pitch + x].color, back_color);
    temp_image[y * x_pitch + x].id = ray_cache[0];
    CHECK_EVENTS;
#ifdef GRX
    if ((verbose_mode < 0) AND(y > 0) AND(x > 0))
      display_plot(x - 1, y - 1, temp_image[y * x_pitch + x].color.r,
                   temp_image[y * x_pitch + x].color.g,
                   temp_image[y * x_pitch + x].color.b);
#endif
    if (verbose_mode > 2)
    {
      POSINC(xi);
      if (xi > x_pitch)
      {
        WRITE(ERROR, ".");
        FLUSH(ERROR);
        xi = 0;
        POSINC(yi);
        if (yi MOD 50 == 0)
        {
          WRITE(ERROR, "\n");
          FLUSH(ERROR);
        }
      }
    }
    if (count > 0)
      if (walk_mode == 0)
        serp_get_next_pixel(&x, &y);
      else
        hilb_get_next_pixel(&x, &y);
    else
      if (walk_mode == 0)
        serp_reset_pixel_list();
      else
        hilb_reset_pixel_list();
  }
  total_time = CPU_CLOCK - total_time;
  if (verbose_mode > 2)
  {
    WRITE(ERROR, "\nFinal phase\n");
    FLUSH(ERROR);
  }
  old_line = temp_image;
  new_line = old_line + x_pitch;
  for (y = 1; y <= screen_size_y; POSINC(y))
  {
    start_time = CPU_CLOCK;
    for (x = 1; x <= screen_size_x; POSINC(x))
    {
      find_true_color(x, y, &(old_line[x - 1]), &(old_line[x]),
                      &(new_line[x - 1]), &(new_line[x]), &(true_color[x]));
      CHECK_EVENTS;
    }
    total_time += CPU_CLOCK - start_time;
    line_picture();
    if (verbose_mode > 2)
    {
      WRITE(ERROR, ".");
      FLUSH(ERROR);
      if (y MOD 50 == 0)
      {
        WRITE(ERROR, str41, y);
        FLUSH(ERROR);
      }
    }
    old_line = new_line;
    new_line = old_line + x_pitch;
  }
  FREE(temp_image);
  if (verbose_mode > 2)
  {
    WRITE(ERROR, "\n");
    FLUSH(ERROR);
  }
  if (verbose_mode > 1)
    trace_stats();
  WRITE(ERROR, str42, total_time / 1000.0);
  FLUSH(ERROR);
}
#endif
#if PVM == 2

#define INDEX(v)\
(ROUND(MAX(0.0, MIN((real) INDEX_MAX, (v) * (real) SUCC(INDEX_MAX)))))

static void
send_line_picture()
{
  REG int         i, j;
  unsigned char  *buffer;

  ALLOCATE(buffer, unsigned char, 3 * screen_size_x, OTHER_TYPE);
  j = 0;
  for (i = 1; i <= screen_size_x; POSINC(i))
  {
    buffer[POSINC(j)] = (unsigned char) INDEX(true_color[i].r);
    buffer[POSINC(j)] = (unsigned char) INDEX(true_color[i].g);
    buffer[POSINC(j)] = (unsigned char) INDEX(true_color[i].b);
  }
  pvm_pkbyte(buffer, 3 * screen_size_x, 1);
  if (background_mode == 1)
  {
    j = 0;
    for (i = 1; i <= screen_size_x; POSINC(i))
      buffer[POSINC(j)] = (unsigned char) INDEX(back_mask[i]);
    pvm_pkbyte(buffer, screen_size_x, 1);
  }
  if (raw_mode == 1)
  {
    j = 0;
    for (i = 1; i <= screen_size_x; POSINC(i))
    {
      buffer[POSINC(j)] = (unsigned char) INDEX(new_line[i].color.r);
      buffer[POSINC(j)] = (unsigned char) INDEX(new_line[i].color.g);
      buffer[POSINC(j)] = (unsigned char) INDEX(new_line[i].color.b);
    }
    pvm_pkbyte(buffer, 3 * screen_size_x, 1);
  }
  FREE(buffer);
}
real
ray_trace_lines(first, size)
  int             first, size;
{
  int             x, x0, x1, x_count, y, step;
  rgb_struct      color;
  ray_struct      ray;
  pixel_ptr       temp_line;
  real            total_time;

  ray.level.r = 1.0;
  ray.level.g = 1.0;
  ray.level.b = 1.0;
  ray.inside = FALSE;
  step = -1;
  total_time = CPU_CLOCK;
  for (y = first; y <= first + size; POSINC(y))
  {
    if (step < 0)
    {
      step = 1;
      x = 0;
      x1 = 1;
      x0 = 0;
    } else
    {
      step = -1;
      x = screen_size_x;
      x1 = screen_size_x;
      x0 = PRED(screen_size_x);
    }
    for (x_count = 0; x_count <= screen_size_x; POSINC(x_count))
    {
      INC_PIXEL_COUNTER;
      REALINC(eye_rays);
      make_vector(&(ray.vector), (real) x, (real) y, 1.0);
      if (intersect_all(NO_OBJECTS, &eye, &ray, &color) > 0.0)
        STRUCT_ASSIGN(new_line[x].color, color);
      else
        STRUCT_ASSIGN(new_line[x].color, back_color);
      new_line[x].id = ray_cache[0];
      CHECK_EVENTS;
      if ((y > first) AND(x_count > 0))
      {
        find_true_color(x1, y, &(old_line[x0]), &(old_line[x1]),
                        &(new_line[x0]), &(new_line[x1]), &(true_color[x1]));
        x1 += step;
        x0 += step;
        CHECK_EVENTS;
      }
      x += step;
    }
    if (y > first)
    {
      send_line_picture();
      if (verbose_mode > 2)
      {
        WRITE(ERROR, ".");
        FLUSH(ERROR);
        if (y MOD 50 == 0)
        {
          WRITE(ERROR, str41, y);
          FLUSH(ERROR);
        }
      }
    }
    temp_line = old_line;
    old_line = new_line;
    new_line = temp_line;
  }
  if (verbose_mode > 2)
  {
    WRITE(ERROR, "\n");
    FLUSH(ERROR);
  }
  if (verbose_mode > 1)
    trace_stats();

  return ((CPU_CLOCK - total_time) / 1000.0);
}
#endif

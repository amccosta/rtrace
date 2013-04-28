/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - lights
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
 *    RAY TRACING - Light - Version 8.3.1                             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1991           *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1993         *
 **********************************************************************/

/***** Lights *****/
void
get_point_light()
{
  int             i;
  real            value;

  light[lights].light_type = POINT_LIGHT_TYPE;
  light[lights].data = NULL;
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT X");
  light[lights].coords.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT Y");
  light[lights].coords.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT Z");
  light[lights].coords.z = value;
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Red");
  light[lights].brightness.r = ABS(value);
  light[lights].attenuation[0] = (value >= 0.0);
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Green");
  light[lights].brightness.g = ABS(value);
  light[lights].attenuation[1] = (value >= 0.0);
  get_valid(scene, &value, -LIGHTING_FACTOR_MAX, LIGHTING_FACTOR_MAX,
            "LIGHT BRIGHTNESS Blue");
  light[lights].brightness.b = ABS(value);
  light[lights].attenuation[2] = (value >= 0.0);
  for (i = 0; i < LIGHT_CACHE_LEVEL_MAX; POSINC(i))
    light[lights].cache_id[i] = NO_OBJECTS;
}
void
get_dir_light()
{
  real            value;
  dir_light_ptr   dir_light;

  get_point_light();
  light[lights].light_type = DIRECT_LIGHT_TYPE;
  ALLOCATE(dir_light, dir_light_struct, 1, PARSE_TYPE);
  light[lights].data = (void_ptr) dir_light;
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT VECTOR X");
  dir_light->vector.x = -value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT VECTOR Y");
  dir_light->vector.y = -value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT VECTOR Z");
  dir_light->vector.z = -value;
  if (LENGTH(dir_light->vector) <= ROUNDOFF)
    runtime_abort("no LIGHT Vector");
  NORMALIZE(dir_light->vector);
  get_valid(scene, &value, 0.0, 180.0, "LIGHT ILLUMINATION Angle");
  dir_light->cos_angle = MIN(1.0 - ROUNDOFF,
                             COS(DEGREE_TO_RADIAN(value)));
  dir_light->t = 1.0 / (1.0 - dir_light->cos_angle);
  get_valid(scene, &value, 1.0, LIGHTING_FACTOR_MAX, "LIGHT LIGHTING Factor");
  dir_light->factor = 1.0 / value;
}
void
get_ext_light()
{
  real            value;
  ext_light_ptr   ext_light;

  get_point_light();
  light[lights].light_type = EXTENDED_LIGHT_TYPE;
  ALLOCATE(ext_light, ext_light_struct, 1, PARSE_TYPE);
  light[lights].data = (void_ptr) ext_light;
  get_valid(scene, &value, 0.0, X_MAX, "LIGHT Radius");
  ext_light->diameter = 2.0 * value;
  get_valid(scene, &value, 1.0, 256.0, "LIGHT Samples");
  ext_light->samples = ROUND(value) - 1;
}
void
get_planar_light()
{
  real            value, size1, size2;
  planar_light_ptr planar_light;

  get_point_light();
  light[lights].light_type = PLANAR_LIGHT_TYPE;
  ALLOCATE(planar_light, planar_light_struct, 1, PARSE_TYPE);
  light[lights].data = (void_ptr) planar_light;
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT VECTOR1 X");
  planar_light->vector1.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT VECTOR1 Y");
  planar_light->vector1.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT VECTOR1 Z");
  planar_light->vector1.z = value;
  if (LENGTH(planar_light->vector1) <= ROUNDOFF)
    runtime_abort("no LIGHT1 Vector");
  get_valid(scene, &value, X_MIN, X_MAX, "LIGHT VECTOR2 X");
  planar_light->vector2.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LIGHT VECTOR2 Y");
  planar_light->vector2.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LIGHT VECTOR2 Z");
  planar_light->vector2.z = value;
  get_valid(scene, &value, 1.0, 256.0, "LIGHT VECTOR1 Samples");
  planar_light->samples1 = ROUND(value) - 1;
  get_valid(scene, &value, 0.0, 256.0, "LIGHT VECTOR2 Samples");
  planar_light->samples2 = ROUND(value) - 1;
  size1 = LENGTH(planar_light->vector1);
  size2 = LENGTH(planar_light->vector2);
  planar_light->size = MAX(size1, size2);
}

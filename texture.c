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
 *    RAY TRACING - Scene (Textures) - Version 8.1.0                  *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, March 1990             *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, December 1992          *
 **********************************************************************/

static rgb_ptr
get_color_map(map_name)
  char_ptr        map_name;
{
  int             i;
  file_ptr        map;
  rgb_ptr         color_map;

  OPEN(map, map_name, READ_TEXT);
  if (IO_status != IO_OK)
  {
    WRITE(results, "Error: unable to open COLOR MAP (%s)\n", map_name);
    FLUSH(results);
    HALT;
  }
  ALLOCATE(color_map, rgb_struct, 256, PARSE_TYPE);
  for (i = 0; i < 256; POSINC(i))
  {
    get_valid(map, &(color_map[i].r), 0.0, 255.0, "COLOR MAP Red");
    get_valid(map, &(color_map[i].g), 0.0, 255.0, "COLOR MAP Green");
    get_valid(map, &(color_map[i].b), 0.0, 255.0, "COLOR MAP Blue");
    ADVANCE(map);
    color_map[i].r /= 255.0;
    color_map[i].g /= 255.0;
    color_map[i].b /= 255.0;
  }
  CLOSE(map);
  return color_map;
}
static void
get_image(image_name, image_map)
  char_ptr        image_name;
  image_map_ptr   image_map;
{
  REG unsigned char value;
  unsigned int    width, height;
  char            str[STRING_MAX];

  OPEN(image_map->image, image_name, READ_BINARY);
  if (IO_status != IO_OK)
  {
    WRITE(results, "Error: unable to open IMAGE (%s)\n", image_name);
    FLUSH(results);
    HALT;
  }
  switch (image_format)
  {
  case 0: /* PIC */
    READ_CHAR(image_map->image, value);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to read IMAGE HEADER (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
    width = value;
    READ_CHAR(image_map->image, value);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to read IMAGE HEADER (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
    width += 256 * value;
    READ_CHAR(image_map->image, value);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to read IMAGE HEADER (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
    height = value;
    READ_CHAR(image_map->image, value);
    if (IO_status != IO_OK)
    {
      WRITE(results, "Error: unable to read IMAGE HEADER (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
    height += 256 * value;
    break;
  case 1: /* PPM */
    READ_STRING(image_map->image, str);
    if ((IO_status != IO_OK) OR(strcmp(str, "P6") != 0))
    {
      WRITE(results, "Error: bad PPM IMAGE HEADER (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
    READ_STRING(image_map->image, str);
    width = atoi(str);
    READ_STRING(image_map->image, str);
    height = atoi(str);
    READ_STRING(image_map->image, str);
    if ((IO_status != IO_OK) OR(strcmp(str, "255") != 0))
    {
      WRITE(results, "Error: bad PPM IMAGE Depth (%s)\n", image_name);
      FLUSH(results);
      HALT;
    }
  }
  if (width < 2)
  {
    WRITE(results, "Error: bad IMAGE HEADER Width (%s)\n", image_name);
    FLUSH(results);
    HALT;
  }
  if (height < 2)
  {
    WRITE(results, "Error: bad IMAGE HEADER Height (%s)\n", image_name);
    FLUSH(results);
    HALT;
  }
  image_map->width = width;
  image_map->height = height;
  WRITE(results, "Image Map (%s) [%ux%u]\n", image_name, width, height);
  FLUSH(results);
}
static texture_ptr
get_object_texture(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  int             i, id;
  real            value;
  texture_ptr     previous_texture, texture;

  if (texture_mode == 1)
  {
    get_valid(scene, &value, 0.0, (real) scene_objects, "OBJECT ID");
    id = PRED(ROUND(value));
    if (id < 0)
      id = PRED(scene_objects);
  }
  else
  {
    get_valid(scene, &value, 1.0, (real) scene_objects, "OBJECT ID");
    id = PRED(ROUND(value));
  }
  ALLOCATE(texture, texture_struct, 1, PARSE_TYPE);
  texture->next = NULL;
  for (i = first_object[id]; i <= last_object[id]; POSINC(i))
  {
    if (object[i]->texture == NULL)
      object[i]->texture = texture;
    else
    {
      previous_texture = object[i]->texture;
      while (previous_texture->next != NULL)
        previous_texture = (texture_ptr) previous_texture->next;
      if (previous_texture != texture)
        previous_texture->next = (void_ptr) texture;
    }
  }
  return texture;
}
static void
get_transform(texture)
  texture_ptr     texture;
{
  boolean         null_transf;
  xyzw_ptr        inv_transf;

  ALLOCATE(texture->transf, xyzw_struct, 4, PARSE_TYPE);
  null_transf = TRUE;
  get_valid(scene, &(texture->transf[0].x), X_MIN, X_MAX, "TRANSFORM X0");
  null_transf = null_transf AND ABS(texture->transf[0].x) < ROUNDOFF;
  get_valid(scene, &(texture->transf[0].y), Y_MIN, Y_MAX, "TRANSFORM Y0");
  null_transf = null_transf AND ABS(texture->transf[0].y) < ROUNDOFF;
  get_valid(scene, &(texture->transf[0].z), Z_MIN, Z_MAX, "TRANSFORM Z0");
  null_transf = null_transf AND ABS(texture->transf[0].z) < ROUNDOFF;
  get_valid(scene, &(texture->transf[0].w), W_MIN, W_MAX, "TRANSFORM W0");
  null_transf = null_transf AND ABS(texture->transf[0].w) < ROUNDOFF;
  get_valid(scene, &(texture->transf[1].x), X_MIN, X_MAX, "TRANSFORM X1");
  null_transf = null_transf AND ABS(texture->transf[1].x) < ROUNDOFF;
  get_valid(scene, &(texture->transf[1].y), Y_MIN, Y_MAX, "TRANSFORM Y1");
  null_transf = null_transf AND ABS(texture->transf[1].y) < ROUNDOFF;
  get_valid(scene, &(texture->transf[1].z), Z_MIN, Z_MAX, "TRANSFORM Z1");
  null_transf = null_transf AND ABS(texture->transf[1].z) < ROUNDOFF;
  get_valid(scene, &(texture->transf[1].w), W_MIN, W_MAX, "TRANSFORM W1");
  null_transf = null_transf AND ABS(texture->transf[1].w) < ROUNDOFF;
  get_valid(scene, &(texture->transf[2].x), X_MIN, X_MAX, "TRANSFORM X2");
  null_transf = null_transf AND ABS(texture->transf[2].x) < ROUNDOFF;
  get_valid(scene, &(texture->transf[2].y), Y_MIN, Y_MAX, "TRANSFORM Y2");
  null_transf = null_transf AND ABS(texture->transf[2].y) < ROUNDOFF;
  get_valid(scene, &(texture->transf[2].z), Z_MIN, Z_MAX, "TRANSFORM Z2");
  null_transf = null_transf AND ABS(texture->transf[2].z) < ROUNDOFF;
  get_valid(scene, &(texture->transf[2].w), W_MIN, W_MAX, "TRANSFORM W2");
  null_transf = null_transf AND ABS(texture->transf[2].w) < ROUNDOFF;
  get_valid(scene, &(texture->transf[3].x), X_MIN, X_MAX, "TRANSFORM X3");
  null_transf = null_transf AND ABS(texture->transf[3].x) < ROUNDOFF;
  get_valid(scene, &(texture->transf[3].y), Y_MIN, Y_MAX, "TRANSFORM Y3");
  null_transf = null_transf AND ABS(texture->transf[3].y) < ROUNDOFF;
  get_valid(scene, &(texture->transf[3].z), Z_MIN, Z_MAX, "TRANSFORM Z3");
  null_transf = null_transf AND ABS(texture->transf[3].z) < ROUNDOFF;
  get_valid(scene, &(texture->transf[3].w), W_MIN, W_MAX, "TRANSFORM W3");
  null_transf = null_transf AND ABS(texture->transf[3].w) < ROUNDOFF;
  if (null_transf)
  {
    FREE(texture->transf);
    texture->transf = NULL;
  } else
  {
    ALLOCATE(inv_transf, xyzw_struct, 4, PARSE_TYPE);
    inverse_transform(texture->transf, inv_transf);
    FREE(texture->transf);
    texture->transf = inv_transf;
    normalize_transform(texture->transf);
  }
}
void
get_texture_null(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;

  texture = get_object_texture(scene_objects, first_object, last_object);
  get_transform(texture);
  texture->type = NULL_TYPE;
  ADVANCE(scene);
}
void
get_texture_checker(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  checker_ptr     checker;

  texture = get_object_texture(scene_objects, first_object, last_object);
  get_transform(texture);
  texture->type = CHECKER_TYPE;
  ALLOCATE(checker, checker_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) checker;
  get_valid(scene, &value, 1.0, (real) surfaces, "SURFACE ID");
  checker->surface_id = PRED(ROUND(value));
  ADVANCE(scene);
}
void
get_texture_blotch(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  blotch_ptr      blotch;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = BLOTCH_TYPE;
  ALLOCATE(blotch, blotch_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) blotch;
  get_valid(scene, &(blotch->scale), 0.0, 1.0, "BLOTCH SCALE");
  get_valid(scene, &value, 1.0, (real) surfaces, "SURFACE ID");
  blotch->surface_id = PRED(ROUND(value));
  ADVANCE(scene);
}
void
get_texture_bump(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  bump_ptr        bump;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = BUMP_TYPE;
  ALLOCATE(bump, bump_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) bump;
  get_valid(scene, &(bump->scale), 0.0, X_MAX, "BUMP SCALE");
  ADVANCE(scene);
}
void
get_texture_marble(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  marble_ptr      marble;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = MARBLE_TYPE;
  ALLOCATE(marble, marble_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) marble;
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    marble->color = NULL;
  else
    marble->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_fbm(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  fbm_ptr         fbm;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = FBM_TYPE;
  ALLOCATE(fbm, fbm_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) fbm;
  get_valid(scene, &(fbm->offset), 0.0, 1.0, "FBM OFFSET");
  get_valid(scene, &(fbm->scale), 0.0, X_MAX, "FBM SCALE");
  get_valid(scene, &value, 0.0, 1.0, "FBM OMEGA");
  get_valid(scene, &(fbm->lambda), 0.0, X_MAX, "FBM LAMBDA");
  fbm->omega = POWER(fbm->lambda, -0.5 - value);
  get_valid(scene, &(fbm->threshold), 0.0, 1.0, "FBM THRESHOLD");
  get_valid(scene, &value, 0.0, X_MAX, "FBM OCTAVES");
  fbm->octaves = ROUND(value);
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    fbm->color = NULL;
  else
    fbm->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_fbm_bump(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  fbm_bump_ptr    fbm_bump;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = FBM_BUMP_TYPE;
  ALLOCATE(fbm_bump, fbm_bump_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) fbm_bump;
  get_valid(scene, &(fbm_bump->offset), 0.0, 1.0, "FBM BUMP OFFSET");
  get_valid(scene, &(fbm_bump->scale), 0.0, X_MAX, "FBM BUMP SCALE");
  get_valid(scene, &value, 0.0, 1.0, "FBM BUMP OMEGA");
  get_valid(scene, &(fbm_bump->lambda), 0.0, X_MAX, "FBM BUMP LAMBDA");
  fbm_bump->omega = POWER(fbm_bump->lambda, -0.5 - value);
  get_valid(scene, &value, 0.0, X_MAX, "FBM BUMP OCTAVES");
  fbm_bump->octaves = ROUND(value);
  ADVANCE(scene);
}
void
get_texture_wood(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  wood_ptr        wood;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = WOOD_TYPE;
  ALLOCATE(wood, wood_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) wood;
  get_valid(scene, &(value), -1.0, 1.0, "WOOD COLOR Red");
  if (value < 0.0)
  {
    wood->color.r = 0.5;
    wood->color.g = 0.5;
    wood->color.b = 0.35;
  } else
  {
    wood->color.r = value;
    get_valid(scene, &(wood->color.g), 0.0, 1.0, "WOOD COLOR Green");
    get_valid(scene, &(wood->color.b), 0.0, 1.0, "WOOD COLOR Blue");
  }
  ADVANCE(scene);
}

void
get_texture_round(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  round_ptr       round;

  texture = get_object_texture(scene_objects, first_object, last_object);
  get_transform(texture);
  texture->type = ROUND_TYPE;
  ALLOCATE(round, round_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) round;
  get_valid(scene, &(round->scale), 0.0, 1.0, "ROUND SCALE");
  round->scale = 1.0 - round->scale;
  ADVANCE(scene);
}
void
get_texture_bozo(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  bozo_ptr        bozo;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = BOZO_TYPE;
  ALLOCATE(bozo, bozo_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) bozo;
  get_valid(scene, &(bozo->turbulence), 0.0, X_MAX, "BOZO TURBULENCE");
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    bozo->color = NULL;
  else
    bozo->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_ripples(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  ripples_ptr     ripples;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = RIPPLES_TYPE;
  ALLOCATE(ripples, ripples_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) ripples;
  get_valid(scene, &(ripples->frequency), 0.0, X_MAX, "RIPPLES FREQUENCY");
  get_valid(scene, &(ripples->phase), 0.0, X_MAX, "RIPPLES PHASE");
  get_valid(scene, &(ripples->scale), 0.0, X_MAX, "RIPPLES SCALE");
  ADVANCE(scene);
}
void
get_texture_waves(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  waves_ptr       waves;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = WAVES_TYPE;
  ALLOCATE(waves, waves_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) waves;
  get_valid(scene, &(waves->frequency), 0.0, X_MAX, "WAVES FREQUENCY");
  get_valid(scene, &(waves->phase), 0.0, X_MAX, "WAVES PHASE");
  get_valid(scene, &(waves->scale), 0.0, X_MAX, "WAVES SCALE");
  ADVANCE(scene);
}
void
get_texture_spotted(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  spotted_ptr     spotted;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = SPOTTED_TYPE;
  ALLOCATE(spotted, spotted_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) spotted;
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    spotted->color = NULL;
  else
    spotted->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_dents(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  dents_ptr       dents;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = DENTS_TYPE;
  ALLOCATE(dents, dents_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) dents;
  get_valid(scene, &(dents->scale), 0.0, X_MAX, "DENTS SCALE");
  ADVANCE(scene);
}
void
get_texture_agate(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  agate_ptr       agate;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = AGATE_TYPE;
  ALLOCATE(agate, agate_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) agate;
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    agate->color = NULL;
  else
    agate->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_wrinkles(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  wrinkles_ptr       wrinkles;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = WRINKLES_TYPE;
  ALLOCATE(wrinkles, wrinkles_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) wrinkles;
  get_valid(scene, &(wrinkles->scale), 0.0, X_MAX, "WRINKLES SCALE");
  ADVANCE(scene);
}
void
get_texture_granite(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  granite_ptr     granite;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = GRANITE_TYPE;
  ALLOCATE(granite, granite_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) granite;
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    granite->color = NULL;
  else
    granite->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_gradient(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  gradient_ptr    gradient;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = GRADIENT_TYPE;
  ALLOCATE(gradient, gradient_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) gradient;
  get_valid(scene, &(gradient->turbulence), 0.0, X_MAX,
            "GRADIENT TURBULENCE");
  get_valid(scene, &(gradient->direction.x), X_MIN, X_MAX,
            "GRADIENT DIRECTION X");
  get_valid(scene, &(gradient->direction.y), Y_MIN, Y_MAX,
            "GRADIENT DIRECTION Y");
  get_valid(scene, &(gradient->direction.z), Z_MIN, Z_MAX,
            "GRADIENT DIRECTION Z");
  if (LENGTH(gradient->direction) < ROUNDOFF)
    runtime_abort("no GRADIENT DIRECTION");
  READ_STRING(scene, name);
  if ((name[0] == '-') AND(name[1] == EOT))
    gradient->color = NULL;
  else
    gradient->color = get_color_map(name);
  ADVANCE(scene);
}
void
get_texture_image_map(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  real            value;
  texture_ptr     texture;
  image_map_ptr   image_map;
  char            name[STRING_MAX];

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = IMAGE_MAP_TYPE;
  ALLOCATE(image_map, image_map_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) image_map;
  get_valid(scene, &(image_map->turbulence), 0.0, X_MAX,
            "IMAGE MAP TURBULENCE");
  get_valid(scene, &value, 0.0, X_MAX, "IMAGE MAP Mode");
  image_map->once = (boolean) (value > ROUNDOFF);
  get_valid(scene, &value, X_MIN, X_MAX, "IMAGE MAP U Axis");
  switch (ROUND(value))
  {
  case X_AXIS:
    image_map->u = X_AXIS;
    break;
  case Y_AXIS:
    image_map->u = Y_AXIS;
    break;
  case Z_AXIS:
    image_map->u = Z_AXIS;
    break;
  default:
    runtime_abort("bad IMAGE MAP U Axis");
    break;
  }
  get_valid(scene, &value, X_MIN, X_MAX, "IMAGE MAP V Axis");
  switch (ROUND(value))
  {
  case X_AXIS:
    image_map->v = X_AXIS;
    break;
  case Y_AXIS:
    image_map->v = Y_AXIS;
    break;
  case Z_AXIS:
    image_map->v = Z_AXIS;
    break;
  default:
    runtime_abort("bad IMAGE MAP V Axis");
    break;
  }
  if (image_map->u == image_map->v)
    runtime_abort("same IMAGE MAP U and V Axis");
  READ_STRING(scene, name);
  get_image(name, image_map);
  ADVANCE(scene);
}
void
get_texture_gloss(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  gloss_ptr       gloss;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = GLOSS_TYPE;
  ALLOCATE(gloss, gloss_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) gloss;
  get_valid(scene, &(gloss->scale), 0.0, X_MAX, "GLOSS SCALE");
  ADVANCE(scene);
}
void
get_texture_bump3(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  texture_ptr     texture;
  bump3_ptr       bump3;

  texture = get_object_texture(scene_objects, first_object, last_object);;
  get_transform(texture);
  texture->type = BUMP3_TYPE;
  ALLOCATE(bump3, bump3_struct, 1, PARSE_TYPE);
  texture->data = (void_ptr) bump3;
  get_valid(scene, &(bump3->scale), 0.0, X_MAX, "BUMP3 SCALE");
  get_valid(scene, &(bump3->size), 0.0, X_MAX, "BUMP3 SIZE");
  ADVANCE(scene);
}

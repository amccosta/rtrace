/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
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
 *    RAY TRACING - Scene - Version 8.4.1                             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, May 1994               *
 **********************************************************************/

/***** Read scene *****/
static void
get_texture(file, scene_objects, first_object, last_object)
  file_ptr        file;
  int             scene_objects;
  int            *first_object, *last_object;
{
  real            value;

  get_valid(file, &value, 0.0, (real) TEXTURE_KINDS_MAX, "TEXTURE ID");
  switch (ROUND(value))
  {
  case NULL_TYPE:
    get_texture_null(scene_objects, first_object, last_object);
    break;
  case CHECKER_TYPE:
    get_texture_checker(scene_objects, first_object, last_object);
    break;
  case BLOTCH_TYPE:
    get_texture_blotch(scene_objects, first_object, last_object);
    break;
  case BUMP_TYPE:
    get_texture_bump(scene_objects, first_object, last_object);
    break;
  case MARBLE_TYPE:
    get_texture_marble(scene_objects, first_object, last_object);
    break;
  case FBM_TYPE:
    get_texture_fbm(scene_objects, first_object, last_object);
    break;
  case FBM_BUMP_TYPE:
    get_texture_fbm_bump(scene_objects, first_object, last_object);
    break;
  case WOOD_TYPE:
    get_texture_wood(scene_objects, first_object, last_object);
    break;
  case ROUND_TYPE:
    get_texture_round(scene_objects, first_object, last_object);
    break;
  case BOZO_TYPE:
    get_texture_bozo(scene_objects, first_object, last_object);
    break;
  case RIPPLES_TYPE:
    get_texture_ripples(scene_objects, first_object, last_object);
    break;
  case WAVES_TYPE:
    get_texture_waves(scene_objects, first_object, last_object);
    break;
  case SPOTTED_TYPE:
    get_texture_spotted(scene_objects, first_object, last_object);
    break;
  case DENTS_TYPE:
    get_texture_dents(scene_objects, first_object, last_object);
    break;
  case AGATE_TYPE:
    get_texture_agate(scene_objects, first_object, last_object);
    break;
  case WRINKLES_TYPE:
    get_texture_wrinkles(scene_objects, first_object, last_object);
    break;
  case GRANITE_TYPE:
    get_texture_granite(scene_objects, first_object, last_object);
    break;
  case GRADIENT_TYPE:
    get_texture_gradient(scene_objects, first_object, last_object);
    break;
  case IMAGE_MAP_TYPE:
    get_texture_image_map(scene_objects, first_object, last_object);
    break;
  case GLOSS_TYPE:
    get_texture_gloss(scene_objects, first_object, last_object);
    break;
  case BUMP3_TYPE:
    get_texture_bump3(scene_objects, first_object, last_object);
    break;
  }
}
typedef
struct
{
  char            node; /* Left/Right */
  csg_ptr         data;
  int             first, start;
} csg_scene_struct;
typedef
csg_scene_struct *csg_scene_ptr;

static csg_scene_ptr csg_scene;

#define LEFT_NODE  (0)
#define RIGHT_NODE (1)

typedef
struct
{
  int             first, start;
} list_scene_struct;

static list_scene_struct list_scene[LIST_LEVEL_MAX];

static int
csg_empty_node(start)
  int             start;
{
  csg_ptr         csg;

  csg = (csg_ptr) object[start]->data;
  if (object[csg->left]->object_type != CSG_TYPE)
    return start;
  if (object[csg->right]->object_type != CSG_TYPE)
    return start;
  if (RANDOM < 0.5)
    return csg_empty_node(csg->left);
  return csg_empty_node(csg->right);
}
static void
csg_convert(level)
  int             level;
{
  REG int         i, j, count, first, second;
  int             count2;
  csg_ptr         csg;

  count = 0;
  for (i = csg_scene[level].start; i <= objects; POSINC(i))
    if (object[i]->object_type == CSG_TYPE)
      POSDEC(count);
    else
      POSINC(count);
  if (count == 1)
    return;
  /* Insert CSG objects */
  for (i = 1; i < count; POSINC(i))
  {
    if (i == 1)
    {
      first = csg_scene[level].start;
      second = first;
    } else
    {
      first = csg_empty_node(csg_scene[level].start);
      POSINC(second);
    }
    if (object[second]->object_type == CSG_TYPE)
    {
      count2 = -1;
      while (count2 != 1)
      {
        POSINC(second);
        if (object[second]->object_type == CSG_TYPE)
          POSDEC(count2);
        else
          POSINC(count2);
      }
    }
    POSINC(second);
    if (objects == objects_max - 1)
      runtime_abort("too many OBJECTS");
    /* Shift objects */
    for (j = objects; j >= csg_scene[level].start; POSDEC(j))
    {
      if (object[j]->object_type == CSG_TYPE)
      {
        csg = (csg_ptr) object[j]->data;
        if (csg->left > first)
          POSINC(csg->left);
        if (csg->right > first)
          POSINC(csg->right);
      }
      if (j >= first)
      {
        POSINC(object[j]->id);
        object[j + 1] = object[j];
      }
    }
    POSINC(objects);
    /* Insert new CSG object */
    ALLOCATE(object[first], object_struct, 1, PARSE_TYPE);
    object[first]->id = first;
    object[first]->surface_id = object[csg_scene[level].first]->surface_id;
    object[first]->refraction = object[csg_scene[level].first]->refraction;
    ALLOCATE(object[first]->min, xyz_struct, 1, PARSE_TYPE);
    ALLOCATE(object[first]->max, xyz_struct, 1, PARSE_TYPE);
    object[first]->transf = NULL;
    object[first]->inv_transf = NULL;
    object[first]->texture = NULL;
    object[first]->texture_modify_normal = FALSE;
    object[first]->object_type = CSG_TYPE;
    ALLOCATE(csg, csg_struct, 1, PARSE_TYPE);
    object[first]->data = (void_ptr) csg;
    csg->op = CSG_UNION;
    csg->left = first + 1;
    csg->right = second + 1;
  }
}
static void
list_convert(level)
  int             level;
{
  REG int         i;
  list_ptr        list;
  list_node_ptr   head;

  list = (list_ptr) object[list_scene[level].first]->data;
  list->head = NULL;
  for (i = objects; i >= list_scene[level].start; POSDEC(i))
  {
    if (CLOSED(object[i]->object_type))
    {
      WRITE(results, "Warning: CLOSED OBJECT (%d) inside a LIST\n", i);
      FLUSH(results);
    }
    if (list->head == NULL)
    {
      ALLOCATE(list->head, list_node_struct, 1, PARSE_TYPE);
      list->head->next = NULL;
    } else
    {
      head = list->head;
      ALLOCATE(list->head, list_node_struct, 1, PARSE_TYPE);
      list->head->next = (void_ptr) head;
    }
    /* Move object */
    list->head->object = object[i];
    object[i] = NULL;
    POSDEC(objects);
  }
}
void
get_scene()
{
  real            value;
  int             id;
  int             scene_objects;
  int            *first_object, *last_object;
  short int       csg_level, list_level;
  real            total_time;

  ALLOCATE(first_object, int, objects_max, OTHER_TYPE);
  ALLOCATE(last_object, int, objects_max, OTHER_TYPE);
  ALLOCATE(csg_scene, csg_scene_struct, csg_level_max, OTHER_TYPE);
  total_time = CPU_CLOCK;
  /* View data */
  ADVANCE(scene);
  get_valid(scene, &value, X_MIN, X_MAX, "EYE POINT X");
  eye.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "EYE POINT Y");
  eye.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "EYE POINT Z");
  eye.z = value;
  ADVANCE(scene);
  get_valid(scene, &value, X_MIN, X_MAX, "LOOK POINT X");
  look.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "LOOK POINT Y");
  look.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "LOOK POINT Z");
  look.z = value;
  if (backface_mode == 2)
  {
    gaze.x = look.x - eye.x;
    gaze.y = look.y - eye.y;
    gaze.z = look.z - eye.z;
  }
  ADVANCE(scene);
  get_valid(scene, &value, X_MIN, X_MAX, "UP VECTOR X");
  up.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, "UP VECTOR Y");
  up.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, "UP VECTOR Z");
  up.z = value;
  ADVANCE(scene);
  get_valid(scene, &value, 0.5, 89.5, "HORIZONTAL VIEW Angle");   /* Degrees */
  view_angle_x = DEGREE_TO_RADIAN(value);
  get_valid(scene, &value, 0.5, 89.5, "VERTICAL VIEW Angle");     /* Degrees */
  view_angle_y = DEGREE_TO_RADIAN(value);
  ADVANCE(scene);
  /* Ambient/Background data */
  ADVANCE(scene);
  get_valid(scene, &value, 0.0, 1.0, "BACK COLOR Red");
  back_color.r = value;
  get_valid(scene, &value, 0.0, 1.0, "BACK COLOR Green");
  back_color.g = value;
  get_valid(scene, &value, 0.0, 1.0, "BACK COLOR Blue");
  back_color.b = value;
  ADVANCE(scene);
  get_valid(scene, &value, 0.0, 1.0, "LIGHT AMBIENT Red");
  light_ambient.r = value;
  get_valid(scene, &value, 0.0, 1.0, "LIGHT AMBIENT Green");
  light_ambient.g = value;
  get_valid(scene, &value, 0.0, 1.0, "LIGHT AMBIENT Blue");
  light_ambient.b = value;
  ADVANCE(scene);
  CHECK_EVENTS;
  /* Light data */
  ADVANCE(scene);
  lights = 0;
  while (NOT END_OF_LINE(scene))
  {
    if (lights >= lights_max)
      runtime_abort("too many LIGHTS");
    get_valid(scene, &value, 1.0, 255.0, "ID");
    if (ROUND(value) > LIGHT_KINDS_MAX)
      runtime_abort("invalid LIGHT ID");
    switch (ROUND(value))
    {
    case POINT_LIGHT_TYPE:
      get_point_light();
      break;
    case DIRECT_LIGHT_TYPE:
      get_dir_light();
      break;
    case EXTENDED_LIGHT_TYPE:
      get_ext_light();
      break;
    case PLANAR_LIGHT_TYPE:
      get_planar_light();
      break;
    }
    POSINC(lights);
    ADVANCE(scene);
  }
  /*****
  if (lights == 0)
    runtime_abort("no LIGHTS");
  *****/
  ADVANCE(scene);
  CHECK_EVENTS;
  /* Surface data */
  ADVANCE(scene);
  surfaces = 0;
  while (NOT END_OF_LINE(scene))
  {
    if (surfaces >= surfaces_max)
      runtime_abort("too many SURFACES");
    get_valid(scene, &value, 1.0, 255.0, "ID");
    if (ROUND(value) > SURFACE_KINDS_MAX)
      runtime_abort("invalid SURFACE ID");
    switch (ROUND(value))
    {
    case SURFACE_TYPE1:
      get_surface_type1();
      break;
    case SURFACE_TYPE2:
      get_surface_type2();
      break;
    }
    POSINC(surfaces);
    ADVANCE(scene);
  }
  if (surfaces == 0)
    runtime_abort("no SURFACES");
  /* Extra surface for TRIANGLE */
  ALLOCATE(surface[surfaces], surface_struct, 1, PARSE_TYPE);
  ADVANCE(scene);
  /* Objects */
  ADVANCE(scene);
  objects = 0;
  scene_objects = 0;
  csg_level = -1;
  list_level = -1;
  while (NOT END_OF_LINE(scene))
  {
    CHECK_EVENTS;
    get_valid(scene, &value, 1.0, 255.0, "ID");
    id = ROUND(value);
    /* Texture */
    if (id == TEXTURE_TYPE)
    {
      if (texture_mode == 1)
      {
        if (scene_objects == 0)
          runtime_abort("no OBJECTS before TEXTURE");
        get_texture(scene, scene_objects, first_object, last_object);
        continue;
      } else
      {
        ADVANCE(scene);
        continue;
      }
    }
    /* Transformation */
    if (id == TRANSF_TYPE)
    {
      if (scene_objects == 0)
        runtime_abort("no OBJECTS before TRANSFORMATION");
      get_object_transform(scene_objects, first_object, last_object);
      continue;
    }
    /* CSG */
    if (id == CSG_OP_TYPE)
    {
      get_valid(scene, &value, 0.0, 2.0, "CSG CODE");
      switch (ROUND(value))
      {
      case 0:   /* CSG BEGIN */
        POSINC(csg_level);
        if (csg_level == csg_level_max)
          runtime_abort("too many CSG LEVELS");
        POSINC(objects);
        if (objects >= objects_max)
          runtime_abort("too many OBJECTS");
        first_object[scene_objects] = objects;
        last_object[scene_objects] = objects;
        POSINC(scene_objects);
        get_csg();
        csg_scene[csg_level].node = LEFT_NODE;
        csg_scene[csg_level].data = (csg_ptr) object[objects]->data;
        csg_scene[csg_level].data->left = objects + 1;
        csg_scene[csg_level].start = objects + 1;
        csg_scene[csg_level].first = objects;
        break;
      case 1:   /* CSG NEXT */
        if (csg_level < 0)
          runtime_abort("no CSG tree");
        if (csg_scene[csg_level].node == RIGHT_NODE)
          runtime_abort("invalid CSG NODE (already has right subtree)");
        if (csg_scene[csg_level].data->left == objects + 1)
          runtime_abort("invalid CSG NODE (empty left subtree)");
        csg_convert(csg_level);
        csg_scene[csg_level].node = RIGHT_NODE;
        csg_scene[csg_level].data->right = objects + 1;
        csg_scene[csg_level].start = objects + 1;
        ADVANCE(scene);
        break;
      case 2:   /* CSG END */
        if (csg_level < 0)
          runtime_abort("no CSG TREE");
        if (csg_scene[csg_level].node == LEFT_NODE)
          runtime_abort("invalid CSG NODE (no right subtree)");
        if (csg_scene[csg_level].data->right == objects + 1)
          runtime_abort("invalid CSG NODE (empty right subtree)");
        csg_convert(csg_level);
        POSDEC(csg_level);
        ADVANCE(scene);
        break;
      }
      continue;
    }
    /* List */
    if (id == LIST_OP_TYPE)
    {
      get_valid(scene, &value, 0.0, 1.0, "LIST CODE");
      switch (ROUND(value))
      {
      case 0:   /* LIST BEGIN */
        POSINC(list_level);
        if (list_level == LIST_LEVEL_MAX)
          runtime_abort("too many LIST LEVELS");
        POSINC(objects);
        if (objects >= objects_max)
          runtime_abort("too many OBJECTS");
        first_object[scene_objects] = objects;
        last_object[scene_objects] = objects;
        POSINC(scene_objects);
        get_list();
        list_scene[list_level].start = objects + 1;
        list_scene[list_level].first = objects;
        break;
      case 1:   /* LIST END */
        if (list_level < 0)
          runtime_abort("no LIST");
        if (objects - list_scene[list_level].first <= 1)
          runtime_abort("invalid LIST (must have at least 2 OBJECTS)");
        list_convert(list_level);
        POSDEC(list_level);
        ADVANCE(scene);
        break;
      }
      continue;
    }
    /* Object */
    if (id > OBJECT_KINDS_MAX)
      runtime_abort("invalid OBJECT ID");
    POSINC(objects);
    if (objects >= objects_max)
      runtime_abort("too many OBJECTS");
    first_object[scene_objects] = objects;
    switch (id)
    {
    case SPHERE_TYPE:
      get_sphere();
      break;
    case BOX_TYPE:
      get_box();
      break;
    case PATCH_TYPE:
      get_patch();
      break;
    case CONE_TYPE:
      get_cone();
      break;
    case POLYGON_TYPE:
      get_polygon();
      break;
    case TRIANGLE_TYPE:
      get_triangle();
      break;
    case TEXT_TYPE:
      get_text();
      if ((sampling_levels > 0) AND(intersect_adjust_mode == 0))
      {
        intersect_adjust_mode = 1;
        if (verbose_mode > 0)
        {
          WRITE(results, "Inters. adjust mode: changed to %s\n", "ON");
          FLUSH(results);
        }
      }
      break;
    }
    last_object[scene_objects] = objects;
    POSINC(scene_objects);
  }
  if (objects == 0)
    runtime_abort("no OBJECTS");
  if (csg_level >= 0)
    runtime_abort("incomplete CSG OBJECT(S)");
  if (list_level >= 0)
    runtime_abort("incomplete LIST of OBJECT(S)");
  end_pp_get();
  if (texture_mode == 2)
  {
    ADVANCE(scene);
    /* Textures */
    ADVANCE(scene);
    while (NOT END_OF_LINE(scene))
    {
      CHECK_EVENTS;
      get_texture(scene, scene_objects, first_object, last_object);
    }
  }
  FREE(first_object);
  FREE(last_object);
  FREE(csg_scene);
  if (verbose_mode > 1)
  {
    WRITE(results, "%d Scene object(s) read, %d Total object(s) created\n",
          scene_objects, objects);
    FLUSH(results);
    WRITE(ERROR, "Info: scene ok\n");
    FLUSH(ERROR);
  }
  if (scene != INPUT)
    CLOSE(scene);
  if (verbose_mode > 1)
  {
    WRITE(ERROR, "Info: reading - %g second(s) CPU time\n",
          (CPU_CLOCK - total_time) / 1000.0);
    FLUSH(ERROR);
  }
  CHECK_EVENTS;
}

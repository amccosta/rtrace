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
#ifdef __TURBOC__
#define MAIN_MODULE
#endif
#ifndef lint
char            copyright[] =
"@(#) Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.\n\
All rights reserved.\n";
#endif
#include "defs.h"
#include "extern.h"

/**********************************************************************
 *    RAY TRACING - Version 8.4.2 (PVM3)                              *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1994          *
 **********************************************************************/

#ifndef PVM
#define PVM 0
#endif

static void
get_view(frame)
  int             frame;
{
  if (camera[frame].eye != NULL)
    STRUCT_ASSIGN(eye, *(camera[frame].eye));
  if (camera[frame].look != NULL)
    STRUCT_ASSIGN(look, *(camera[frame].look));
  if (camera[frame].up != NULL)
    STRUCT_ASSIGN(up, *(camera[frame].up));
  if (camera[frame].view_angle_x > 0.0)
    STRUCT_ASSIGN(view_angle_x, camera[frame].view_angle_x);
  if (camera[frame].view_angle_y > 0.0)
    STRUCT_ASSIGN(view_angle_y, camera[frame].view_angle_y);
}
static void
input_parameters(parameters, parameter)
  int            *parameters;
  char_ptr        parameter[];
{
  REG int         i;

  INPUT_PARAMETERS;
  POSDEC(*parameters);
  for (i = 1; i <= *parameters; POSINC(i))
    parameter[i - 1] = parameter[i];
}

#if PVM == 0
/***** Main *****/
#ifndef SUIT
void
main(parameters, parameter)
  int             parameters;
  char_ptr        parameter[];
{
  int             frame;
  real            total_time;

  input_parameters(&parameters, parameter);
  get_parameters(parameters, parameter);

  ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX, PARSE_TYPE);
  ALLOCATE(light, light_struct, lights_max, PARSE_TYPE);
  /* One extra surface for TRIANGLE */
  ALLOCATE(surface, surface_ptr, surfaces_max + 1, PARSE_TYPE);
  ALLOCATE(object, object_ptr, objects_max, PARSE_TYPE);
  get_scene();
  setup_scene();

  if (walk_mode == 0)
    serp_make_pixel_list(screen_size_x + 1, screen_size_y + 1);
  else
    hilb_make_pixel_list(screen_size_x + 1, screen_size_y + 1);
  ALLOCATE(true_color, rgb_struct, screen_size_x + 1, TRACE_TYPE);
  ALLOCATE(back_mask, real, screen_size_x + 1, TRACE_TYPE);

  if (movie_frames > 0)
  {
    total_time = CPU_CLOCK;
    for (frame = 0; frame < movie_frames; POSINC(frame))
    {
      WRITE(ERROR, "Info: FRAME %d\n", frame + 1);
      FLUSH(ERROR);
      if (results != ERROR)
      {
        WRITE(results, "\nFRAME %d\n", frame + 1);
        FLUSH(results);
      }
      get_view(frame);
      make_view();
      init_globals();
      init_picture();
      ray_trace();
    }
    WRITE(ERROR, "Info: total tracing - %g second(s) CPU time\n",
          (CPU_CLOCK - total_time) / 1000.0);
    FLUSH(ERROR);
  } else
  {
    make_view();
    init_globals();
    init_picture();
    ray_trace();
  }
  if (picture != OUTPUT)
    CLOSE(picture);
  if (background_mode == 1)
    CLOSE(background);
  if (raw_mode == 1)
    CLOSE(raw_picture);
  EXIT;
}
#else
/***** SUIT *****/
static boolean  change_anything, change_scene, change_params;
static boolean  busy;
static char     scene_name[STRING_MAX], picture_name[STRING_MAX];
static real     pixel_counter, max_pixel_counter;
jmp_buf         environment;

#define NULL_STRING(string) (strcmp(string, "") == 0)

#define CHECK_BUSY \
do {\
  if (busy)\
  {\
    SUIT_inform("Aborted!");\
    HALT;\
  }\
} while (0)

void
show(message)
  char_ptr      message;
{
  int           i;

  for (i = 0; message[i] != '\0'; POSINC(i))
    if (message[i] == '\n')
      message[i] = '\015';
  SUIT_setBoolean(message_window, READ_ONLY, FALSE);
  SUIT_sendToEditor(message_window, message);
  SUIT_setBoolean(message_window, READ_ONLY, TRUE);
}
void
inc_pixel_counter()
{
  REALINC(pixel_counter);
  SUIT_setDouble(SUIT_name("Counter"), CURRENT_VALUE,
                 pixel_counter / max_pixel_counter);
}
static void
open_scene()
{
  char            tmp[STRING_MAX];

  OPEN(scene, scene_name, READ_TEXT);
  if (IO_status != IO_OK)
  {
    sprintf(tmp, "Cannot open @bold(SFF) file: @italic(%s)", scene_name);
    SUIT_inform(tmp);
    scene = NULL;
    HALT;
  }
}
static void
reset()
{
  busy = FALSE;
  change_anything = FALSE;
  change_scene = FALSE;
  change_params = FALSE;
  SUIT_setBoolean(SUIT_name("Stop"), DISABLED, TRUE);
  SUIT_setDouble(SUIT_name("Counter"), CURRENT_VALUE, 0.0);
}
static void
old_main()
{
  int             frame;
  real            total_time;
  char            tmp[STRING_MAX];

  WRITE(ERROR, "\n[%s, %s]\n", scene_name, picture_name);
  FLUSH(ERROR);
  if (change_anything)
    options();
  if (change_params OR change_scene)
  {
    if (change_params)
    {
      free_type(PARSE_TYPE);
      ALLOCATE(light, light_struct, lights_max, PARSE_TYPE);
      /* One extra surface for TRIANGLE */
      ALLOCATE(surface, surface_ptr, surfaces_max + 1, PARSE_TYPE);
      ALLOCATE(object, object_ptr, objects_max, PARSE_TYPE);
      free_type(TRACE_TYPE);
      if (walk_mode == 0)
        serp_make_pixel_list(screen_size_x + 1, screen_size_y + 1);
      else
        hilb_make_pixel_list(screen_size_x + 1, screen_size_y + 1);
      ALLOCATE(true_color, rgb_struct, screen_size_x + 1, TRACE_TYPE);
      ALLOCATE(back_mask, real, screen_size_x + 1, TRACE_TYPE);
    }
    open_scene();
    get_scene();
    setup_scene();
  }

  if (movie_frames > 0)
  {
    total_time = CPU_CLOCK;
    for (frame = 0; frame < movie_frames; POSINC(frame))
    {
      WRITE(ERROR, "Info: FRAME %d\n", frame + 1);
      FLUSH(ERROR);
      if (results != ERROR)
      {
        WRITE(results, "\nFRAME %d\n", frame + 1);
        FLUSH(results);
      }
      get_view(frame);
      make_view();
      init_globals();
      init_picture();
      ray_trace();
      GP_beep();
      /*****
      sprintf(tmp, "Frame %d: ray tracing completed", frame);
      SUIT_inform(tmp);
      *****/
    }
    WRITE(ERROR, "Info: total tracing - %g second(s) CPU time\n",
          (CPU_CLOCK - total_time) / 1000.0);
    FLUSH(ERROR);
  } else
  {
    make_view();
    init_globals();
    init_picture();
    ray_trace();
    GP_beep();
    /*****
    SUIT_inform("Ray tracing completed");
    *****/
  }
}
static void
terminate()
{
  SUIT_setBoolean(SUIT_name("Go"), DISABLED, TRUE);
  if (scene != NULL)
    CLOSE(scene);
  if (picture != NULL)
    CLOSE(picture);
  picture_name[0] = NULL;
  if (background_mode == 1)
  {
    CLOSE(background);
    background_mode = 0;
    SUIT_setBoolean(SUIT_name("Back"), CURRENT_VALUE, FALSE);
  }
  if (raw_mode == 1)
  {
    CLOSE(raw_picture);
    raw_mode = 0;
    SUIT_setBoolean(SUIT_name("Raw"), CURRENT_VALUE, FALSE);
  }
}
#ifdef dos
static void
show_picture()
{
  REG int         width, height, value, h, w;
  char            str[8], *data;
  REG unsigned char r, g, b;

#define WIDTH_MAX 768
#define HEIGHT_MAX 600

  CLOSE(picture);
  OPEN(picture, picture_name, READ_BINARY);
  if (IO_status != IO_OK)
    return;
  switch(image_format)
  {
  case 0: /* PIC */
    READ_CHAR(picture, value);
    if (IO_status != IO_OK)
      return;
    width = value;
    READ_CHAR(picture, value);
    if (IO_status != IO_OK)
      return;
    width += 256 * value;
    READ_CHAR(picture, value);
    if (IO_status != IO_OK)
      return;
    height = value;
    READ_CHAR(picture, value);
    if (IO_status != IO_OK)
      return;
    height += 256 * value;
    break;
  case 1: /* PPM */
    READ_STRING(picture, str);
    if ((IO_status != IO_OK) OR(strcmp(str, "P6") != 0))
      return;
    READ_STRING(picture, str);
    width = atoi(str);
    READ_STRING(picture, str);
    height = atoi(str);
    READ_STRING(picture, str);
    if ((IO_status != IO_OK) OR(strcmp(str, "255") != 0))
      return;
    break;
  }
  if ((width < 16) OR(width > WIDTH_MAX))
    return;
  if ((height < 16) OR(height > HEIGHT_MAX))
    return;
  ALLOC(data, char, width * height, OTHER_TYPE);
  if (data == NULL)
    return;
  for (h = 0; h < height; POSINC(h))
    for (w = 0; w < width; POSINC(w))
    {
      READ_CHAR(picture, r);
      READ_CHAR(picture, g);
      READ_CHAR(picture, b);
      data[h * width + w] = (char) (0.3 * r + 0.5 * g + 0.2 * b);
    }
  CLOSE(picture);
  picture = NULL;
  SUIT_createImage("Picture", (WIDTH_MAX - width) / 2,
                   (HEIGHT_MAX - height) / 2, width, height, data);
  FREE(data);
#undef WIDTH_MAX
#undef HEIGHT_MAX
}
#else
static void
show_picture()
{
  char_ptr        env;
  char            command[STRING_MAX];

  switch(image_format)
  {
  case 0:
    env = getenv("RTRACE_SHOW_PIC");
    break;
  case 1:
    env = getenv("RTRACE_SHOW_PPM");
    break;
  }
  if (env != NULL)
  {
    sprintf(command, "%s %s", env, picture_name);
    system(command);
  }
}
#endif

static void
CB_clean(button)
  SUIT_object     button;
{
  /* free_all(); */
}
static void
CB_go(button)
  SUIT_object     button;
{
  if (SUIT_askYesNo("Ray trace?") == REPLY_YES)
  {
    busy = TRUE;
    SUIT_setBoolean(SUIT_name("Stop"), DISABLED, FALSE);
    SUIT_setBoolean(SUIT_name("Go"), DISABLED, TRUE);
    SUIT_setDouble(SUIT_name("Counter"), CURRENT_VALUE, 0.0);
    pixel_counter = 0.0;
    max_pixel_counter = (real) screen_size_x * screen_size_y;
    old_main();
    show_picture();
    terminate();
    reset();
  }
}
static void
CB_stop(button)
  SUIT_object     button;
{
  if (SUIT_askYesNo("Really stop?") == REPLY_YES)
    HALT;
}
static void
CB_scene_file(button)
  SUIT_object     button;
{
  char            tmp[STRING_MAX];
  char_ptr        s;

  do {
#ifdef __TURBOC__
    if (SUIT_getString("Open SFF file:", "new.sff", tmp,
                       STRING_MAX) == REPLY_CANCEL)
      return;
#else
    s = SUIT_askForFileName("", "Open SFF file:", "");
    if (s == NULL)
      tmp[0] = NULL;
    else
      strcpy(tmp, s);
#endif
    if (tmp[0] == NULL)
      return;
    if (scene != NULL)
      CLOSE(scene);
    OPEN(scene, tmp, READ_TEXT);
    if (IO_status != IO_OK)
    {
      scene = NULL;
      tmp[0] = NULL;
      SUIT_inform("Cannot open file to read");
    }
  } while (IO_status != IO_OK);
  change_scene = TRUE;
  change_anything = TRUE;
  strcpy(scene_name, tmp);
  CHECK_BUSY;
  if (NOT NULL_STRING(scene_name) AND NOT NULL_STRING(picture_name))
    SUIT_setBoolean(SUIT_name("Go"), DISABLED, FALSE);
}
static void
CB_picture_file(button)
  SUIT_object     button;
{
  char            tmp[STRING_MAX];

  do {
    if ((SUIT_getString("Open picture file:", "new.pic", tmp,
         STRING_MAX) == REPLY_CANCEL) OR(tmp[0] == NULL))
      return;
    if (picture != NULL)
      CLOSE(picture);
    OPEN(picture, tmp, WRITE_BINARY);
    if (IO_status != IO_OK)
    {
      picture = NULL;
      tmp[0] = NULL;
      SUIT_inform("Cannot open file to write");
    }
  } while (IO_status != IO_OK);
  strcpy(picture_name, tmp);
  CHECK_BUSY;
  if (NOT NULL_STRING(scene_name) AND NOT NULL_STRING(picture_name))
    SUIT_setBoolean(SUIT_name("Go"), DISABLED, FALSE);
}
static void
CB_sep_abs(button)
  SUIT_object     button;
{
  char            tmp[STRING_MAX];
  double          value;

  if (view_mode == 0)
    return;
  if ((SUIT_getString("Separation:", "0.1", tmp,
       STRING_MAX) == REPLY_CANCEL) OR(tmp[0] == NULL))
    return;
  if ((sscanf(tmp, "%lf", &value) != 1) OR(value <= 0.0))
  {
    tmp[0] = NULL;
    SUIT_inform("Invalid!");
    return;
  }
  stereo_separation = value;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_sep_pct(button)
  SUIT_object     button;
{
  char            tmp[STRING_MAX];
  double          value;

  if (view_mode == 0)
    return;
  if ((SUIT_getString("Separation (% of @bold(Gaze)):", "100", tmp,
       STRING_MAX) == REPLY_CANCEL) OR(tmp[0] == NULL))
    return;
  if ((sscanf(tmp, "%lf", &value) != 1) OR(value <= 0.0))
  {
    tmp[0] = NULL;
    SUIT_inform("Invalid!");
    return;
  }
  stereo_separation = -value / 100.0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_width(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 15.0)
      AND(ROUND(value) != screen_size_x))
  {
    screen_size_x = ROUND(value);
    change_params = TRUE;
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%d", (int) screen_size_x);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_height(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 15.0)
      AND(ROUND(value) != screen_size_y))
  {
    screen_size_y = ROUND(value);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%d", (int) screen_size_y);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_samples(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  if (last_ambient_level == 0)
  {
    SUIT_setText(box, CURRENT_VALUE, "0");
    SUIT_inform("Cannot accept while @bold(Distributed sampling levels) is 0");
    return;
  }
  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 0.0)
      AND(ROUND(value) != ambient_sample_rays))
  {
    ambient_sample_rays = ROUND(value);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%d", (int) ambient_sample_rays);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_cluster(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value >= CLUSTER_SIZE_MIN)
      AND(value <= CLUSTER_SIZE_MAX)
      AND(ROUND(value) != cluster_size))
  {
    cluster_size = ROUND(value);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%d", (int) cluster_size);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_thres_ambient(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 0.0)
      AND(value <= 1.0))
  {
    threshold_vector = value;
    distributed_cache_mode = (value > ROUNDOFF);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%g", threshold_vector);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_thres_aliasing(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 0.0)
      AND(value <= 1.0))
  {
    threshold_color = SQR(value) * 3.0;
    change_anything = TRUE;
    CHECK_BUSY;
    sprintf(tmp, "%g", value);
  } else
    sprintf(tmp, "%g", SQRT(threshold_color / 3.0));
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_thres_shading(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 0.0)
      AND(value <= 1.0))
  {
    threshold_level = value;
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%g", threshold_level);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_level_ambient(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > 0.0))
  {
    last_ambient_level = ROUND(value);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  SUIT_setBoolean(SUIT_name("Samples"), VISIBLE, last_ambient_level > 0);
  SUIT_setBoolean(SUIT_name("ThresAmbient"), VISIBLE, last_ambient_level > 0);
  sprintf(tmp, "%d", last_ambient_level);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_level_sampling(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value >= 0.0)
      AND(value <= SAMPLING_LEVEL_MAX))
  {
    sampling_levels = ROUND(value);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  SUIT_setBoolean(SUIT_name("ThresAliasing"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("Antialiasing"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("Intersect"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("Contrast"), DISABLED, sampling_levels == 0);
  sprintf(tmp, "%d", sampling_levels);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_level_shading(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value >= 1.0)
      AND(value <= 256.0))
  {
    last_shade_level = ROUND(value - 1.0);
    change_anything = TRUE;
    CHECK_BUSY;
  }
  SUIT_setBoolean(SUIT_name("ThresShading"), VISIBLE, last_shade_level > 0);
  sprintf(tmp, "%d", last_shade_level + 1);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_aperture(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value >= 0.0))
  {
    focal_aperture = value;
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%g", focal_aperture);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_distance(box)
  SUIT_object     box;
{
  double          value;
  char_ptr        s;
  char            tmp[STRING_MAX];

  s = SUIT_getText(box, CURRENT_VALUE);
  if ((sscanf(s, "%lf", &value) == 1)
      AND(value > ROUNDOFF))
  {
    focal_distance = value;
    change_anything = TRUE;
    CHECK_BUSY;
  }
  sprintf(tmp, "%g", focal_distance);
  SUIT_setText(box, CURRENT_VALUE, tmp);
}
static void
CB_back(onoff)
  SUIT_object     onoff;
{
  char            name[STRING_MAX];

  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
  {
    do {
      if ((SUIT_getString("Open background mask file:", "new.msk", name,
                         STRING_MAX) == REPLY_CANCEL) OR(name[0] == NULL))
      {
        SUIT_setBoolean(onoff, CURRENT_VALUE, FALSE);
        return;
      }
      if (background != NULL)
        CLOSE(background);
      OPEN(background, name, WRITE_BINARY);
      if (IO_status != IO_OK)
      {
        background = NULL;
        SUIT_inform("Cannot open file to write");
      }
    } while (IO_status != IO_OK);
    background_mode = 1;
  } else
    background_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_raw(onoff)
  SUIT_object     onoff;
{
  char            name[STRING_MAX];

  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
  {
    do {
      if ((SUIT_getString("Open raw picture file:", "raw.pic", name,
                         STRING_MAX) == REPLY_CANCEL) OR(name[0] == NULL))
      {
        SUIT_setBoolean(onoff, CURRENT_VALUE, FALSE);
        return;
      }
      if (raw_picture != NULL)
        CLOSE(raw_picture);
      OPEN(raw_picture, name, WRITE_BINARY);
      if (IO_status != IO_OK)
      {
        raw_picture = NULL;
        SUIT_inform("Cannot open file to write");
      }
    } while (IO_status != IO_OK);
    raw_mode = 1;
  } else
    raw_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_adjust(onoff)
  SUIT_object     onoff;
{
  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
    intersect_adjust_mode = 1;
  else
    intersect_adjust_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_jitter(onoff)
  SUIT_object     onoff;
{
  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
    jittering_mode = 1;
  else
    jittering_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_texture_normals(onoff)
  SUIT_object     onoff;
{
  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
    normal_check_mode = 1;
  else
    normal_check_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
static void
CB_contrast(onoff)
  SUIT_object     onoff;
{
  if (SUIT_getBoolean(onoff, CURRENT_VALUE))
    contrast_mode = 1;
  else
    contrast_mode = 0;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define ADAPTIVE_SUPERSAMPLING      "Adaptive supersampling"
#define SEMI_ADAPTIVE_SUPERSAMPLING "Semi-adaptive supersampling"
#define NORMAL_SUPERSAMPLING        "Normal supersampling"

static void
CB_aliasing(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, ADAPTIVE_SUPERSAMPLING))
    antialiasing_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, SEMI_ADAPTIVE_SUPERSAMPLING))
    antialiasing_mode = 1;
  else
  if (SUIT_stringsMatch(tmp, NORMAL_SUPERSAMPLING))
    antialiasing_mode = 2;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define NO_BACKFACE_REMOVAL      "No backface removal"
#define PARTIAL_BACKFACE_REMOVAL "Partial backface removal"
#define FULL_BACKFACE_REMOVAL    "Full backface removal"

static void
CB_backface(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, NO_BACKFACE_REMOVAL))
    backface_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, PARTIAL_BACKFACE_REMOVAL))
    backface_mode = 1;
  else
  if (SUIT_stringsMatch(tmp, FULL_BACKFACE_REMOVAL))
    backface_mode = 2;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define CORNER_OBJECTS "Check objects at pixel corners"
#define ALL_OBJECTS    "Check all objects"

static void
CB_intersect(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, CORNER_OBJECTS))
    intersect_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, ALL_OBJECTS))
    intersect_mode = 1;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define PHONG_SHADING   "Phong shading"
#define STRAUSS_SHADING "Strauss shading"

static void
CB_shading(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, PHONG_SHADING))
    shade_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, STRAUSS_SHADING))
    shade_mode = 1;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define CORRECT_ALWAYS "Correct always"
#define CORRECT_INSIDE "Correct only inside objects"

static void
CB_normals(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, CORRECT_INSIDE))
    normal_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, CORRECT_ALWAYS))
    normal_mode = 1;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define LIGHTING_NONE    "No transparent lighting"
#define LIGHTING_PARTIAL "Partial transparent lighting"
#define LIGHTING_FULL    "Full transparent lighting"

static void
CB_lighting(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, LIGHTING_NONE))
    light_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, LIGHTING_PARTIAL))
    light_mode = 1;
  else
  if (SUIT_stringsMatch(tmp, LIGHTING_FULL))
    light_mode = 2;
  change_anything = TRUE;
  CHECK_BUSY;
}
#define TEXTURES_NONE "No textures"
#define TEXTURES_NEW  "Textures"
#define TEXTURES_OLD  "Textures (old syntax)"

static void
CB_textures(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, TEXTURES_NONE))
    texture_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, TEXTURES_NEW))
    texture_mode = 1;
  else
  if (SUIT_stringsMatch(tmp, TEXTURES_OLD))
    texture_mode = 2;
  SUIT_setBoolean(SUIT_name("TextNormals"), DISABLED, texture_mode == 0);
  change_anything = TRUE;
  CHECK_BUSY;
}
#define VIEW_NORMAL "Normal view"
#define VIEW_LEFT   "Stereo left view"
#define VIEW_RIGHT  "Stereo right view"

static void
CB_view(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, VIEW_NORMAL))
    view_mode = 0;
  else
  if (SUIT_stringsMatch(tmp, VIEW_LEFT))
    view_mode = 1;
  else
  if (SUIT_stringsMatch(tmp, VIEW_RIGHT))
    view_mode = 2;
  SUIT_setBoolean(SUIT_name("Separation"), VISIBLE, view_mode != 0);
  change_anything = TRUE;
  CHECK_BUSY;
}
#define FORMAT_PIC "PIC format"
#define FORMAT_PPM "PPM format"

static void
CB_format(radio)
  SUIT_object     radio;
{
  char_ptr        tmp;

  tmp = SUIT_getEnumString(radio, CURRENT_VALUE);
  if (SUIT_stringsMatch(tmp, FORMAT_PIC))
    image_format = 0;
  else
  if (SUIT_stringsMatch(tmp, FORMAT_PPM))
    image_format = 1;
  change_anything = TRUE;
  CHECK_BUSY;
}

SUIT_object     message_window;

void
main(parameters, parameter)
  int             parameters;
  char_ptr        parameter[];
{
  char            tmp[STRING_MAX];

  ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX, OTHER_TYPE);
  /* Widgets */
#ifdef FINAL
  SUIT_initFromCode("RTrace");
#else
  SUIT_init("RTrace");
#endif
  /* SUIT_createDoneButton(CB_clean); */
  SUIT_createAbortButton(CB_clean);
  SUIT_createButton("Go", CB_go);
  SUIT_createButton("Stop", CB_stop);
  message_window = SUIT_createTextEditorWithScrollBar("Messages", NULL);
  (void) SUIT_createPullDownMenu("Files");
  (void) SUIT_addToMenu(SUIT_name("Files"), "SFF file", CB_scene_file);
  (void) SUIT_addToMenu(SUIT_name("Files"), "Picture file", CB_picture_file);
  (void) SUIT_createPullDownMenu("Separation");
  (void) SUIT_addToMenu(SUIT_name("Separation"), "Absolute", CB_sep_abs);
  (void) SUIT_addToMenu(SUIT_name("Separation"), "% of Gaze", CB_sep_pct);
  (void) SUIT_createTypeInBox("Width", CB_width);
  (void) SUIT_createTypeInBox("Height", CB_height);
  (void) SUIT_createTypeInBox("Samples", CB_samples);
  (void) SUIT_createTypeInBox("Cluster", CB_cluster);
  (void) SUIT_createTypeInBox("ThresAmbient", CB_thres_ambient);
  (void) SUIT_createTypeInBox("ThresAliasing", CB_thres_aliasing);
  (void) SUIT_createTypeInBox("ThresShading", CB_thres_shading);
  (void) SUIT_createTypeInBox("LevelAmbient", CB_level_ambient);
  (void) SUIT_createTypeInBox("LevelSampling", CB_level_sampling);
  (void) SUIT_createTypeInBox("LevelShading", CB_level_shading);
  (void) SUIT_createTypeInBox("Aperture", CB_aperture);
  (void) SUIT_createTypeInBox("Distance", CB_distance);
  (void) SUIT_createOnOffSwitch("Back", CB_back);
  (void) SUIT_createOnOffSwitch("Raw", CB_raw);
  (void) SUIT_createOnOffSwitch("Adjust", CB_adjust);
  (void) SUIT_createOnOffSwitch("Jitter", CB_jitter);
  (void) SUIT_createOnOffSwitch("TextNormals", CB_texture_normals);
  (void) SUIT_createOnOffSwitch("Contrast", CB_contrast);
  (void) SUIT_createBoundedValue("Counter", NULL);
  (void) SUIT_createRadioButtons("Antialiasing", CB_aliasing);
  SUIT_addButtonToRadioButtons(SUIT_name("Antialiasing"),
                               ADAPTIVE_SUPERSAMPLING);
  SUIT_addButtonToRadioButtons(SUIT_name("Antialiasing"),
                               SEMI_ADAPTIVE_SUPERSAMPLING);
  SUIT_addButtonToRadioButtons(SUIT_name("Antialiasing"),
                               NORMAL_SUPERSAMPLING);
  (void) SUIT_createRadioButtons("Backface", CB_backface);
  SUIT_addButtonToRadioButtons(SUIT_name("Backface"),
                               FULL_BACKFACE_REMOVAL);
  SUIT_addButtonToRadioButtons(SUIT_name("Backface"),
                               PARTIAL_BACKFACE_REMOVAL);
  SUIT_addButtonToRadioButtons(SUIT_name("Backface"),
                               NO_BACKFACE_REMOVAL);
  (void) SUIT_createRadioButtons("Intersect", CB_intersect);
  SUIT_addButtonToRadioButtons(SUIT_name("Intersect"), ALL_OBJECTS);
  SUIT_addButtonToRadioButtons(SUIT_name("Intersect"), CORNER_OBJECTS);
  (void) SUIT_createRadioButtons("Shading", CB_shading);
  SUIT_addButtonToRadioButtons(SUIT_name("Shading"), PHONG_SHADING);
  SUIT_addButtonToRadioButtons(SUIT_name("Shading"), STRAUSS_SHADING);
  (void) SUIT_createRadioButtons("Normals", CB_normals);
  SUIT_addButtonToRadioButtons(SUIT_name("Normals"), CORRECT_ALWAYS);
  SUIT_addButtonToRadioButtons(SUIT_name("Normals"), CORRECT_INSIDE);
  (void) SUIT_createRadioButtons("Lighting", CB_lighting);
  SUIT_addButtonToRadioButtons(SUIT_name("Lighting"), LIGHTING_NONE);
  SUIT_addButtonToRadioButtons(SUIT_name("Lighting"), LIGHTING_PARTIAL);
  SUIT_addButtonToRadioButtons(SUIT_name("Lighting"), LIGHTING_FULL);
  (void) SUIT_createRadioButtons("Textures", CB_textures);
  SUIT_addButtonToRadioButtons(SUIT_name("Textures"), TEXTURES_NONE);
  SUIT_addButtonToRadioButtons(SUIT_name("Textures"), TEXTURES_NEW);
  SUIT_addButtonToRadioButtons(SUIT_name("Textures"), TEXTURES_OLD);
  (void) SUIT_createRadioButtons("View", CB_view);
  SUIT_addButtonToRadioButtons(SUIT_name("View"), VIEW_NORMAL);
  SUIT_addButtonToRadioButtons(SUIT_name("View"), VIEW_LEFT);
  SUIT_addButtonToRadioButtons(SUIT_name("View"), VIEW_RIGHT);
  (void) SUIT_createRadioButtons("Format", CB_format);
  SUIT_addButtonToRadioButtons(SUIT_name("Format"), FORMAT_PIC);
  SUIT_addButtonToRadioButtons(SUIT_name("Format"), FORMAT_PPM);

  /* Properties */
  SUIT_beginDisplay();
  SUIT_setBoolean(SUIT_name("Go"), DISABLED, TRUE);
  SUIT_setBoolean(SUIT_name("Stop"), DISABLED, TRUE);
  SUIT_setBoolean(message_window, READ_ONLY, TRUE);
  SUIT_setText(message_window, CURRENT_VALUE, "");

  SUIT_makePropertyTemporary(message_window, CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Width"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Height"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Samples"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Cluster"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("ThresAmbient"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("ThresAliasing"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("ThresShading"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("LevelAmbient"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("LevelSampling"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("LevelShading"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Aperture"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Distance"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Back"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Raw"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Adjust"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Jitter"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("TextNormals"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Contrast"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Counter"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Antialiasing"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Backface"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Intersect"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Shading"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Normals"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Lighting"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Textures"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("View"), CURRENT_VALUE, OBJECT);
  SUIT_makePropertyTemporary(SUIT_name("Format"), CURRENT_VALUE, OBJECT);

  reset();
  input_parameters(&parameters, parameter);
  get_parameters(parameters, parameter);
  verbose_mode = 2;

  sprintf(tmp, "%d", (int) screen_size_x);
  SUIT_setText(SUIT_name("Width"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%d", (int) screen_size_y);
  SUIT_setText(SUIT_name("Height"), CURRENT_VALUE, tmp);
  if (last_ambient_level > 0)
  {
    sprintf(tmp, "%d", (int) ambient_sample_rays);
    SUIT_setText(SUIT_name("Samples"), CURRENT_VALUE, tmp);
  } else
    SUIT_setText(SUIT_name("Samples"), CURRENT_VALUE, "0");
  sprintf(tmp, "%d", (int) cluster_size);
  SUIT_setText(SUIT_name("Cluster"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%g", threshold_vector);
  SUIT_setText(SUIT_name("ThresAmbient"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%g", SQRT(threshold_color / 3.0));
  SUIT_setText(SUIT_name("ThresAliasing"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%g", threshold_level);
  SUIT_setText(SUIT_name("ThresShading"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%d", last_ambient_level);
  SUIT_setText(SUIT_name("LevelAmbient"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%d", sampling_levels);
  SUIT_setText(SUIT_name("LevelSampling"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%d", last_shade_level + 1);
  SUIT_setText(SUIT_name("LevelShading"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%g", focal_aperture);
  SUIT_setText(SUIT_name("Aperture"), CURRENT_VALUE, tmp);
  sprintf(tmp, "%g", focal_distance);
  SUIT_setText(SUIT_name("Distance"), CURRENT_VALUE, tmp);
  SUIT_setBoolean(SUIT_name("Back"), CURRENT_VALUE, background_mode != 0);
  SUIT_setBoolean(SUIT_name("Raw"), CURRENT_VALUE, raw_mode != 0);
  SUIT_setBoolean(SUIT_name("Adjust"), CURRENT_VALUE,
                  intersect_adjust_mode != 0);
  SUIT_setBoolean(SUIT_name("Jitter"), CURRENT_VALUE, jittering_mode != 0);
  SUIT_setBoolean(SUIT_name("TextNormals"), CURRENT_VALUE,
                  normal_check_mode != 0);
  SUIT_setBoolean(SUIT_name("Contrast"), CURRENT_VALUE, contrast_mode != 0);
  switch (antialiasing_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Antialiasing"),
                              ADAPTIVE_SUPERSAMPLING);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Antialiasing"),
                              SEMI_ADAPTIVE_SUPERSAMPLING);
    break;
  case 2:
    SUIT_pressThisRadioButton(SUIT_name("Antialiasing"),
                              NORMAL_SUPERSAMPLING);
    break;
  }
  switch (backface_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Backface"),
                              NO_BACKFACE_REMOVAL);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Backface"),
                              PARTIAL_BACKFACE_REMOVAL);
    break;
  case 2:
    SUIT_pressThisRadioButton(SUIT_name("Backface"),
                              FULL_BACKFACE_REMOVAL);
    break;
  }
  switch (intersect_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Intersect"), CORNER_OBJECTS);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Intersect"), ALL_OBJECTS);
    break;
  }
  switch (shade_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Shading"), PHONG_SHADING);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Shading"), STRAUSS_SHADING);
    break;
  }
  switch (normal_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Normals"), CORRECT_INSIDE);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Normals"), CORRECT_ALWAYS);
    break;
  }
  switch (light_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Lighting"), LIGHTING_NONE);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Lighting"), LIGHTING_PARTIAL);
    break;
  case 2:
    SUIT_pressThisRadioButton(SUIT_name("Lighting"), LIGHTING_FULL);
    break;
  }
  switch (texture_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Textures"), TEXTURES_NONE);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Textures"), TEXTURES_NEW);
    break;
  case 2:
    SUIT_pressThisRadioButton(SUIT_name("Textures"), TEXTURES_OLD);
    break;
  }
  switch (view_mode)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("View"), VIEW_NORMAL);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("View"), VIEW_LEFT);
    break;
  case 2:
    SUIT_pressThisRadioButton(SUIT_name("View"), VIEW_RIGHT);
    break;
  }
  switch (image_format)
  {
  case 0:
    SUIT_pressThisRadioButton(SUIT_name("Format"), FORMAT_PIC);
    break;
  case 1:
    SUIT_pressThisRadioButton(SUIT_name("Format"), FORMAT_PPM);
    break;
  }

  change_scene = TRUE;
  change_params = TRUE;
  if (scene != NULL)
  {
    CLOSE(scene);
    strncpy(scene_name, parameters[parameter - 2], STRING_MAX);
  } else
    strcpy(scene_name, "");
  if (picture != NULL)
    strncpy(picture_name, parameters[parameter - 1], STRING_MAX);
  else
    strcpy(picture_name, "");
  if (NULL_STRING(scene_name) OR NULL_STRING(picture_name))
    SUIT_setBoolean(SUIT_name("Go"), DISABLED, TRUE);
  else
    SUIT_setBoolean(SUIT_name("Go"), DISABLED, FALSE);
  SUIT_setBoolean(SUIT_name("Separation"), VISIBLE, view_mode != 0);
  SUIT_setBoolean(SUIT_name("TextNormals"), DISABLED, texture_mode == 0);
  SUIT_setBoolean(SUIT_name("Samples"), VISIBLE, last_ambient_level > 0);
  SUIT_setBoolean(SUIT_name("ThresAmbient"), VISIBLE, last_ambient_level > 0);
  SUIT_setBoolean(SUIT_name("ThresAliasing"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("ThresShading"), VISIBLE, last_shade_level > 0);
  SUIT_setBoolean(SUIT_name("Antialiasing"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("Intersect"), VISIBLE, sampling_levels > 0);
  SUIT_setBoolean(SUIT_name("Contrast"), DISABLED, sampling_levels == 0);

  if (setjmp(environment))
  {
    terminate();
    if (change_anything)
    {
      reset();
      change_anything = TRUE;
    } else
      reset();
  }
  while (TRUE)
    SUIT_checkAndProcessInput(INDEFINITE);
  EXIT;
}
#endif
#else /* PVM */
#include "pvm3.h"

#if PVM == 1
/***** Main *****/
typedef
struct
{
  int         task, free;
  int         first, size;
} server_struct;
typedef 
server_struct  *server_ptr;

int             lines_task;

static void
setup_picture()
{
  int             frames, x, y;

  frames = movie_frames;
  if (frames == 0)
    frames = 1;

  while (POSDEC(frames) > 0)
    for (y = 0; y < screen_size_y; POSINC(y))
      for (x = 0; x < screen_size_x; POSINC(x))
      {
	WRITE_CHAR(picture, 0);
	WRITE_CHAR(picture, 0);
	WRITE_CHAR(picture, 0);
	if (background_mode == 1)
	  WRITE_CHAR(background, 0);
	if (raw_mode == 1)
	{
	  WRITE_CHAR(raw_picture, 0);
	  WRITE_CHAR(raw_picture, 0);
	  WRITE_CHAR(raw_picture, 0);
	}
      }
}
static int      servers, *server_tid;
static real     enclosing_time, tracing_time, transmission_time;

static void
collect_stats()
{
  int           server;
  real          enclosing_time_rec, tracing_time_rec, transmission_time_rec;
  short int     shade_level_max_rec;
  int           objects_rec;
  int           lights_rec;
  int           surfaces_rec;
  real          eye_rays_rec, reflected_rays_rec, refracted_rays_rec,
                shadow_rays_rec;
  real          ambient_rays_rec;
  real          shadow_hits_rec, shadow_cache_hits_rec;
  real          octant_tests_rec, bound_tests_rec, sphere_tests_rec,
                box_tests_rec, patch_tests_rec, cone_tests_rec,
                polygon_tests_rec, triangle_tests_rec, text_tests_rec,
                csg_tests_rec, list_tests_rec,
                backface_tests_rec, backface_hits_rec;
  real          pqueue_resets_rec, pqueue_insertions_rec,
                pqueue_extractions_rec;
  real          ray_hits_rec, ray_cache_resets_rec, ray_cache_hits_rec;
  real          distributed_cache_resets_rec, distributed_cache_hits_rec;
  real          pixel_divisions_rec;

  init_globals();
  for (server = 0; server < servers; POSINC(server))
  {
    pvm_initsend(PvmDataDefault);
    pvm_send(server_tid[server], 3);
    if (pvm_recv(server_tid[server], 4) < 0)
    {
      WRITE(ERROR, "PVM warning: could not receive stats of server %d <%x>\n",
            server, server_tid[server]);
      FLUSH(ERROR);
      continue;
    }
    pvm_bufinfo(pvm_getrbuf(), (int *) 0, (int *) 0, (int *) 0);
    pvm_upkdouble(&enclosing_time_rec, 1, 1);
    pvm_upkdouble(&tracing_time_rec, 1, 1);
    pvm_upkdouble(&transmission_time_rec, 1, 1);
    pvm_upkint(&objects_rec, 1, 1);
    pvm_upkint(&lights_rec, 1, 1);
    pvm_upkint(&surfaces_rec, 1, 1);
    pvm_upkshort(&shade_level_max_rec, 1, 1);
    pvm_upkdouble(&eye_rays_rec, 1, 1);
    pvm_upkdouble(&shadow_rays_rec, 1, 1);
    pvm_upkdouble(&reflected_rays_rec, 1, 1);
    pvm_upkdouble(&refracted_rays_rec, 1, 1);
    pvm_upkdouble(&ambient_rays_rec, 1, 1);
    pvm_upkdouble(&shadow_hits_rec, 1, 1);
    pvm_upkdouble(&shadow_cache_hits_rec, 1, 1);
    pvm_upkdouble(&ray_hits_rec, 1, 1);
    pvm_upkdouble(&ray_cache_resets_rec, 1, 1);
    pvm_upkdouble(&ray_cache_hits_rec, 1, 1);
    pvm_upkdouble(&distributed_cache_resets_rec, 1, 1);
    pvm_upkdouble(&distributed_cache_hits_rec, 1, 1);
    pvm_upkdouble(&pqueue_resets_rec, 1, 1);
    pvm_upkdouble(&pqueue_insertions_rec, 1, 1);
    pvm_upkdouble(&pqueue_extractions_rec, 1, 1);
    pvm_upkdouble(&pixel_divisions_rec, 1, 1);
    pvm_upkdouble(&octant_tests_rec, 1, 1);
    pvm_upkdouble(&bound_tests_rec, 1, 1);
    pvm_upkdouble(&backface_tests_rec, 1, 1);
    pvm_upkdouble(&backface_hits_rec, 1, 1);
    pvm_upkdouble(&sphere_tests_rec, 1, 1);
    pvm_upkdouble(&box_tests_rec, 1, 1);
    pvm_upkdouble(&patch_tests_rec, 1, 1);
    pvm_upkdouble(&cone_tests_rec, 1, 1);
    pvm_upkdouble(&polygon_tests_rec, 1, 1);
    pvm_upkdouble(&triangle_tests_rec, 1, 1);
    pvm_upkdouble(&text_tests_rec, 1, 1);
    pvm_upkdouble(&csg_tests_rec, 1, 1);
    pvm_upkdouble(&list_tests_rec, 1, 1);
    enclosing_time += enclosing_time_rec;
    tracing_time += tracing_time_rec;
    transmission_time += transmission_time_rec;
    shade_level_max += shade_level_max_rec;
    eye_rays += eye_rays_rec;
    shadow_rays += shadow_rays_rec;
    reflected_rays += reflected_rays_rec;
    refracted_rays += refracted_rays_rec;
    ambient_rays += ambient_rays_rec;
    shadow_hits += shadow_hits_rec;
    shadow_cache_hits += shadow_cache_hits_rec;
    ray_hits += ray_hits_rec;
    ray_cache_resets += ray_cache_resets_rec;
    ray_cache_hits += ray_cache_hits_rec;
    distributed_cache_resets += distributed_cache_resets_rec;
    distributed_cache_hits += distributed_cache_hits_rec;
    pqueue_resets += pqueue_resets_rec;
    pqueue_insertions += pqueue_insertions_rec;
    pqueue_extractions += pqueue_extractions_rec;
    pixel_divisions += pixel_divisions_rec;
    octant_tests += octant_tests_rec;
    bound_tests += bound_tests_rec;
    backface_tests += backface_tests_rec;
    backface_hits += backface_hits_rec;
    sphere_tests += sphere_tests_rec;
    box_tests += box_tests_rec;
    patch_tests += patch_tests_rec;
    cone_tests += cone_tests_rec;
    polygon_tests += polygon_tests_rec;
    triangle_tests += triangle_tests_rec;
    text_tests += text_tests_rec;
    csg_tests += csg_tests_rec;
    list_tests += list_tests_rec;
    WRITE(ERROR, "PVM info: server %d tracing - %g second(s) CPU time\n",
          server, tracing_time_rec);
    FLUSH(ERROR);
    WRITE(ERROR, "PVM info: server %d transmission - %g second(s) time\n",
	  server, transmission_time_rec / 1000000.0);
    FLUSH(ERROR);
  }
  objects = objects_rec;
  lights = lights_rec;
  surfaces = surfaces_rec;
}
static void
ray_trace_all()
{
  int           server, ended_server;
  server_ptr    server_data;
  int           line_current, line_block;
  int           line_current_rec, line_block_rec;
  int           task, tasks, tasks_todo;
  int           tid;
  long          where;
  unsigned char *line_buffer;

  ALLOCATE(line_buffer, unsigned char, 3 * screen_size_x, OTHER_TYPE);

  ALLOCATE(server_data, server_struct, servers, OTHER_TYPE);
  for (server = 0; server < servers; POSINC(server))
    server_data[server].free = server - 1;
  POSDEC(server);

  tasks = (screen_size_y + lines_task - 1) / lines_task;

  if (verbose_mode > 3)
  {
    WRITE(ERROR, "PVM info: %d task(s)\n", tasks);
    FLUSH(ERROR);
  }

  task = tasks;
  tasks_todo = tasks;

  line_current = 0;
  line_block = lines_task;

  /* 1st group of tasks */
  while ((server >= 0) AND task)
  {
    server_data[server].task = PREDEC(task);

    pvm_initsend(PvmDataDefault);
    pvm_pkint(&line_current, 1, 1);
    pvm_pkint(&line_block, 1, 1);
    server_data[server].first = line_current;
    server_data[server].size = line_block;

    if (pvm_send(server_tid[server], 1))
    {
      WRITE(ERROR, "PVM error: cannot send task %d to server %d <%x>\n",
            task, server, server_tid[server]);
      FLUSH(ERROR);
      pvm_exit();
      HALT;
    }

    if (verbose_mode > 3)
    {
      WRITE(ERROR, "PVM info: sent task %d to server %d <%x>, line %d [%d]\n",
            task, server, server_tid[server], line_current, line_block);
      FLUSH(ERROR);
    }

    server = server_data[server].free;

    line_current += line_block;
    if (line_current + line_block - 1 >= screen_size_y)
      line_block = screen_size_y - line_current;
  }

  /* 2nd group of tasks and answers... */
  while (tasks_todo AND(pvm_recv(-1, 2) >= 0))
  {
    pvm_bufinfo(pvm_getrbuf(), (int *) 0, (int *) 0, &tid);
    for (ended_server = 0; ended_server < servers; POSINC(ended_server))
      if (server_tid[ended_server] == tid)
        break;

    if (ended_server >= servers)
    {
      WRITE(ERROR, "PVM warning: unknown server <%x>\n", tid);
      FLUSH(ERROR);
      continue;
    }

    POSDEC(tasks_todo);
    if (verbose_mode > 3)
    {
      WRITE(ERROR, "PVM info: finished task %d of server %d <%x>\n",
            server_data[ended_server].task, ended_server, tid);
      FLUSH(ERROR);
    }

    pvm_upkint(&line_current_rec, 1, 1);
    pvm_upkint(&line_block_rec, 1, 1);
    if ((line_current_rec != server_data[ended_server].first) OR
	(line_block_rec != server_data[ended_server].size))
    {
      WRITE(ERROR, "PVM warning: wrong data, line %d [%d] (%d [%d])\n",
	      line_current_rec, line_block_rec,
	      server_data[ended_server].first, server_data[ended_server].size);
      FLUSH(ERROR);
      continue;
    }

    if (verbose_mode > 2)
    {
      WRITE(ERROR, "lines %d to %d completed\n", line_current_rec,
	    line_current_rec + line_block_rec - 1);
      FLUSH(ERROR);
    }

    where = line_current_rec * screen_size_x;
    while (POSDEC(line_block_rec) > 0)
    {
      pvm_upkbyte(line_buffer, 3 * screen_size_x, 1);
      SEEK(picture, where * 3 + image_format_offset);
      fwrite(line_buffer, 3, screen_size_x, picture);
      FLUSH(picture);
      if (background_mode == 1)
      {
        pvm_upkbyte(line_buffer, screen_size_x, 1);
        SEEK(background, where + 4);
        fwrite(line_buffer, 1, screen_size_x, background);
        FLUSH(background);
      }
      if (raw_mode == 1)
      {
        pvm_upkbyte(line_buffer, 3 * screen_size_x, 1);
        SEEK(raw_picture, where * 3 + image_format_offset);
        fwrite(line_buffer, 3, screen_size_x, raw_picture);
        FLUSH(raw_picture);
      }
      where += screen_size_x;
    }

    server_data[ended_server].free = server;
    server = ended_server;

    pvm_initsend(PvmDataDefault);
    if (pvm_send(server_tid[server], 99))
    {
      WRITE(ERROR, "PVM error: cannot send acknowledge to server %d <%x>\n",
            server, server_tid[server]);
      FLUSH(ERROR);
      pvm_exit();
      HALT;
    }

    if (task)
    {
      server_data[server].task = PREDEC(task);

      pvm_initsend(PvmDataDefault);
      pvm_pkint(&line_current, 1, 1);
      pvm_pkint(&line_block, 1, 1);
      server_data[server].first = line_current;
      server_data[server].size = line_block;

      if (pvm_send(server_tid[server], 1))
      {
        WRITE(ERROR, "PVM error: cannot send task %d to server %d <%x>\n",
              task, server, server_tid[server]);
        FLUSH(ERROR);
        pvm_exit();
        HALT;
      }

      if (verbose_mode > 3)
      {
        WRITE(ERROR, "PVM info: sent task %d to server %d <%x>, line %d [%d]\n",
              task, server, server_tid[server], line_current, line_block);
        FLUSH(ERROR);
      }

      server = server_data[server].free;

      line_current += line_block;
      if (line_current + line_block - 1 >= screen_size_y)
        line_block = screen_size_y - line_current;
    }
  }

  if (tasks_todo > 0)
  {
    WRITE(ERROR, "PVM error: %d task(s) not done!\n", tasks_todo);
    FLUSH(ERROR);
  }

  tracing_time = 0;
  transmission_time = 0;
  collect_stats();

  FREE(server_data);
  FREE(line_buffer);
}

#ifndef RTRACE_SERVER
#define RTRACE_SERVER "rtrace_s"
#endif

extern void     trace_stats();

void
main(parameters, parameter)
  int             parameters;
  char_ptr        parameter[];
{
  int             i, frame;

  lines_task = 0;

  input_parameters(&parameters, parameter);
  get_parameters(parameters, parameter);

  if (pvm_mytid() < 0)
  {
    WRITE(ERROR, "PVM error: cannot start PVM\n");
    FLUSH(ERROR);
    HALT;
  }
  pvm_config(&servers, (int *) 0, (struct pvmhostinfo **) 0);

  ALLOCATE(server_tid, int, servers, PARSE_TYPE);
  i = 0;
  while (i < servers)
  {
    if (pvm_spawn(RTRACE_SERVER, parameter, PvmTaskDefault, "", 1,
                  &server_tid[i]) < 0)
    {
      WRITE(ERROR, "PVM error: cannot start server\n");
      FLUSH(ERROR);
      POSDEC(servers);
    } else
      POSINC(i);
  }
  if ((lines_task < 1) OR(lines_task > 1024))
    lines_task = 16;

  if (verbose_mode > 3)
  {
    WRITE(ERROR, "PVM info: %d server(s)\n", servers);
    FLUSH(ERROR);
    WRITE(ERROR, "PVM info: %d line(s) per task\n", lines_task);
    FLUSH(ERROR);
  }

  init_picture();
  setup_picture();

  if (movie_frames > 0)
  {
    for (frame = 0; frame < movie_frames; POSINC(frame))
    {
      WRITE(ERROR, "Info: FRAME %d\n", frame + 1);
      FLUSH(ERROR);
      if (results != ERROR)
      {
        WRITE(results, "\nFRAME %d\n", frame + 1);
        FLUSH(results);
      }
      ray_trace_all();
    }
  } else
    ray_trace_all();

  if (picture != OUTPUT)
    CLOSE(picture);
  if (background_mode == 1)
    CLOSE(background);
  if (raw_mode == 1)
    CLOSE(raw_picture);

  if (verbose_mode > 1)
    trace_stats();
  WRITE(ERROR, "Info: enclosing (average) - %g second(s) CPU time\n",
        enclosing_time / servers);
  FLUSH(ERROR);
  WRITE(ERROR, "Info: tracing - %g second(s) CPU time\n", tracing_time);
  FLUSH(ERROR);
  WRITE(ERROR, "Info: transmission - %g second(s) time\n",
	transmission_time / 1000000.0);
  FLUSH(ERROR);

  pvm_exit();
  EXIT;
}
#endif
#if PVM == 2
#include <sys/time.h>

extern real     ray_trace_lines();

void
main(parameters, parameter)
  int             parameters;
  char_ptr        parameter[];
{
  int             tid, master_tid, tag;
  int             first, size;
  real            enclosing_time, tracing_time, transmission_time;
  struct timeval  start_time, end_time;

  tid = pvm_mytid();
  if (tid < 0)
    HALT;
  master_tid = pvm_parent();

  POSDEC(parameters);
  input_parameters(&parameters, parameter);
  get_parameters(parameters, parameter);

  ALLOCATE(pqueue, pqueue_struct, PQUEUE_SIZE_MAX, PARSE_TYPE);
  ALLOCATE(light, light_struct, lights_max, PARSE_TYPE);
  /* One extra surface for TRIANGLE */
  ALLOCATE(surface, surface_ptr, surfaces_max + 1, PARSE_TYPE);
  ALLOCATE(object, object_ptr, objects_max, PARSE_TYPE);
  get_scene();
  enclosing_time = 0;
  setup_scene();
  enclosing_time = (CPU_CLOCK - enclosing_time) / 1000.0;

  ALLOCATE(true_color, rgb_struct, screen_size_x + 1, TRACE_TYPE);
  ALLOCATE(back_mask, real, screen_size_x + 1, TRACE_TYPE);
  ALLOCATE(new_line, pixel_struct, screen_size_x + 1, TRACE_TYPE);
  ALLOCATE(old_line, pixel_struct, screen_size_x + 1, TRACE_TYPE);

  make_view();
  init_globals();
  tracing_time = 0;
  transmission_time = 0;

  /* Wait... */
  while (pvm_recv(master_tid, -1) > 0)
  {
    pvm_bufinfo(pvm_getrbuf(), (int *) 0, &tag, (int *) 0);
    if (tag != 1)
      break;

    pvm_upkint(&first, 1, 1);
    pvm_upkint(&size, 1, 1);

    pvm_initsend(PvmDataDefault);
    pvm_pkint(&first, 1, 1);
    pvm_pkint(&size, 1, 1);

    tracing_time += ray_trace_lines(first, size);

    gettimeofday(&start_time, (struct timezone *) 0);
    pvm_send(master_tid, 2);
    pvm_recv(master_tid, 99);
    gettimeofday(&end_time, (struct timezone *) 0);
    transmission_time += (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
			 end_time.tv_usec - start_time.tv_usec;
  }
  if (tag == 3)
  {
    pvm_initsend(PvmDataDefault);
    pvm_pkdouble(&enclosing_time, 1, 1);
    pvm_pkdouble(&tracing_time, 1, 1);
    pvm_pkdouble(&transmission_time, 1, 1);
    pvm_pkint(&objects, 1, 1);
    pvm_pkint(&lights, 1, 1);
    pvm_pkint(&surfaces, 1, 1);
    pvm_pkshort(&shade_level_max, 1, 1);
    pvm_pkdouble(&eye_rays, 1, 1);
    pvm_pkdouble(&shadow_rays, 1, 1);
    pvm_pkdouble(&reflected_rays, 1, 1);
    pvm_pkdouble(&refracted_rays, 1, 1);
    pvm_pkdouble(&ambient_rays, 1, 1);
    pvm_pkdouble(&shadow_hits, 1, 1);
    pvm_pkdouble(&shadow_cache_hits, 1, 1);
    pvm_pkdouble(&ray_hits, 1, 1);
    pvm_pkdouble(&ray_cache_resets, 1, 1);
    pvm_pkdouble(&ray_cache_hits, 1, 1);
    pvm_pkdouble(&distributed_cache_resets, 1, 1);
    pvm_pkdouble(&distributed_cache_hits, 1, 1);
    pvm_pkdouble(&pqueue_resets, 1, 1);
    pvm_pkdouble(&pqueue_insertions, 1, 1);
    pvm_pkdouble(&pqueue_extractions, 1, 1);
    pvm_pkdouble(&pixel_divisions, 1, 1);
    pvm_pkdouble(&octant_tests, 1, 1);
    pvm_pkdouble(&bound_tests, 1, 1);
    pvm_pkdouble(&backface_tests, 1, 1);
    pvm_pkdouble(&backface_hits, 1, 1);
    pvm_pkdouble(&sphere_tests, 1, 1);
    pvm_pkdouble(&box_tests, 1, 1);
    pvm_pkdouble(&patch_tests, 1, 1);
    pvm_pkdouble(&cone_tests, 1, 1);
    pvm_pkdouble(&polygon_tests, 1, 1);
    pvm_pkdouble(&triangle_tests, 1, 1);
    pvm_pkdouble(&text_tests, 1, 1);
    pvm_pkdouble(&csg_tests, 1, 1);
    pvm_pkdouble(&list_tests, 1, 1);
    pvm_send(master_tid, 4);
  }
  pvm_exit();
  EXIT;
}
#endif
#endif /* PVM */

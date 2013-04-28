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
 *    RAY TRACING - Parameters - Version 8.4.2 (PVM3)                 *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1994          *
 **********************************************************************/

#ifndef PVM
#define PVM 0
#endif

#if PVM == 1
extern int      lines_task;
#endif
#if PVM == 2
#undef HALT
#define HALT \
{\
  pvm_exit();\
  nexit(1);\
}
#undef EXIT
#define EXIT \
{\
  pvm_exit();\
  nexit(0);\
}
#endif

#ifdef _Windows
#pragma option -zEstrings -zFstringc -zHstringg
#define FAR far
#else
#define FAR
#endif
static char FAR str1[] = "Error: %s\n";
static char FAR str2[] = "Version %s created on %s %s.\n";
static char FAR str3[] = "Usage: {PROGRAM}\n";
static char FAR str4[] = "  [w]Screen Width       -> 16... (256)\n";
static char FAR str5[] = "  [h]Screen Height      -> 16... (256)\n";
static char FAR str6[] = "  [p]Sampling Levels    -> 0..%d (0)\n";
static char FAR str7[] = "  [A]Aliasing Threshold -> 0..1 (0.05)\n";
static char FAR str8[] = "  [s]Shading Levels     -> 1..%d (8)\n";
static char FAR str9[] = "  [S]Shading Threshold  -> 0..1 (0.01) \n";
static char FAR str10[] = "  [d]Ambient Levels     -> 0..%d (0)\n";
static char FAR str11[] = "  [D]Ambient Samples    -> 2...\n";
static char FAR str12[] = "  [T]Ambient Threshold  -> 0..1 (0)\n";
static char FAR str13[] = "  [c]Cluster Size       -> %d..%d (4)\n";
static char FAR str14[] = "  [a]Antialiasing Mode  -> 0, 1, 2 (0)\n";
static char FAR str15[] = "  [b]Backface Mode      -> 0, 1, 2 (0)\n";
static char FAR str16[] = "  [i]Intersect Mode     -> 0, 1 (0)\n";
static char FAR str17[] = "  [I]Inter. Adjust Mode -> 0, 1 (0)\n";
static char FAR str18[] = "  [j]Jittering Mode     -> 0, 1 (0)\n";
static char FAR str19[] = "  [l]Lighting Mode      -> 0, 1, 2 (0)\n";
static char FAR str20[] = "  [m]Shading Mode       -> 0, 1 (1)\n";
static char FAR str21[] = "  [n]Normal Mode        -> 0, 1 (0)\n";
static char FAR str22[] = "  [t]Texture Mode       -> 0, 1, 2 (0)\n";
static char FAR str23[] = "  [v]View Mode          -> 0, 1, 2 (0)\n";
static char FAR str24[] = "  [z]Normal Check Mode  -> 0, 1 (0)\n";
static char FAR str25[] = "  [B]Mask Back File     -> FILE (only PIC format)\n";
static char FAR str26[] = "  [R]Raw Picture File   -> FILE\n";
static char FAR str27[] = "  [F]Focal Distance     -> 0..%0.0f (GAZE)\n";
static char FAR str28[] = "  [E]Stereo Separation  -> 0..%0.0f\n";
static char FAR str29[] = "  [P]Focal Aperture     -> 0..1\n";
static char FAR str30[] =
  "  [O]Output Format      -> 0 - PIC format, 1 - PPM (0)\n";
static char FAR str31[] = "  [V]Verbose Mode       -> 0, 1, 2, 3 (2)\n";
static char FAR str32[] = "  {-|INFILE} {-|OUTFILE} [>LOGFILE]\n";
static char FAR str33[] = "Image width        : %d\n";
static char FAR str34[] = "Image height       : %d\n";
static char FAR str35[] = "Aliasing threshold : %g\n";
static char FAR str36[] = "Shading threshold  : %g\n";
static char FAR str37[] = "Ambient threshold  : %g\n";
static char FAR str38[] = "Sampling levels    : %d\n";
static char FAR str39[] = "Shading levels     : %d\n";
static char FAR str40[] = "Ambient levels     : %d\n";
static char FAR str41[] = "Ambient samples    : %d\n";
static char FAR str42[] = "Cluster size       : variable up to %d\n";
static char FAR str43[] = "Cluster size       : %d\n";
static char FAR str44[] = "Antialiasing mode  : ";
static char FAR str45[] = "PIXEL CORNER AVERAGE\n";
static char FAR str46[] = "PIXEL ADAPTIVE SUPERSAMPLING\n";
static char FAR str47[] = "PIXEL SEMI-ADAPTIVE SUPERSAMPLING\n";
static char FAR str48[] = "PIXEL NORMAL SUPERSAMPLING\n";
static char FAR str49[] = "Backface mode      : ";
static char FAR str50[] = "PARTIAL REMOVAL\n";
static char FAR str51[] = "Backface mode      : ";
static char FAR str52[] = "FULL REMOVAL\n";
static char FAR str53[] = "Background mode    : %s\n";
static char FAR str54[] = "Intersect mode     : %s\n";
static char FAR str55[] = "Inters. adjust mode: %s\n";
static char FAR str56[] = "Jittering mode     : %s\n";
static char FAR str57[] = "Lighting mode      : ";
static char FAR str58[] = "NO TRANSLUCENT SHADOWS\n";
static char FAR str59[] = "PARTIAL TRANSLUCENT SHADOWS\n";
static char FAR str60[] = "FULL TRANSLUCENT SHADOWS\n";
static char FAR str61[] = "Normal mode        : %s\n";
static char FAR str62[] = "Normal check mode  : %s\n";
static char FAR str63[] = "NORMAL TEXTURE CORRECTION";
static char FAR str64[] = "NO NORMAL TEXTURE CORRECTION";
static char FAR str65[] = "Shading mode       : %s\n";
static char FAR str66[] = "Raw mode           : %s\n";
static char FAR str67[] = "Texture mode       : ";
static char FAR str68[] = "NO TEXTURES\n";
static char FAR str69[] = "TEXTURES INSIDE OBJECTS DESCRIPTION\n";
static char FAR str70[] = "TEXTURES AFTER OBJECTS DESCRIPTION\n";
static char FAR str71[] = "View mode          : ";
static char FAR str72[] = "NORMAL\n";
static char FAR str73[] = "LEFT EYE\n";
static char FAR str74[] = "RIGHT EYE\n";
static char FAR str75[] = "Focal aperture     : %g\n";
static char FAR str76[] = "Focal aperture     : %s\n";
static char FAR str77[] = "Focal distance     : %g\n";
static char FAR str78[] = "Focal distance     : %s\n";
static char FAR str79[] = "Stereo separation  : %g\n";
static char FAR str80[] = "Stereo separation  : %g%% of GAZE\n";
static char FAR str81[] = "Image format       : %s\n";
static char FAR str82[] = "Info: options ok\n";
static char FAR str83[] = "Error: PARAMETER(S) missing\n";
static char FAR str84[] = "Error: bad ANTIALIASING MODE\n";
static char FAR str85[] = "Error: bad BACKFACE MODE\n";
static char FAR str86[] = "Error: bad CLUSTER SIZE\n";
static char FAR str87[] = "Error: bad AMBIENT LEVELS\n";
static char FAR str88[] = "Error: bad SCREEN HEIGHT\n";
static char FAR str89[] = "Error: bad INTERSECT MODE\n";
static char FAR str90[] = "Error: bad JITTERING MODE\n";
static char FAR str91[] = "Error: bad LIGHTING MODE\n";
static char FAR str92[] = "Error: bad SHADING MODE\n";
static char FAR str93[] = "Error: bad NORMAL MODE\n";
static char FAR str94[] = "Error: bad SAMPLING LEVEL(S)\n";
static char FAR str95[] = "Error: bad SHADING LEVEL(S)\n";
static char FAR str96[] = "Error: bad TEXTURE MODE\n";
static char FAR str97[] = "Error: bad VIEW MODE\n";
static char FAR str98[] = "Error: bad SCREEN WIDTH\n";
static char FAR str99[] = "Error: bad NORMAL CHECK MODE\n";
static char FAR str100[] = "Error: bad ALIASING THRESHOLD\n";
static char FAR str101[] = "Error: BACKGROUND MODE already set\n";
static char FAR str102[] = "Error: unable to open BACKGROUND MASK FILE (%s)\n";
static char FAR str103[] = "Error: bad AMBIENT SAMPLES\n";
static char FAR str104[] = "Error: bad STEREO SEPARATION\n";
static char FAR str105[] = "Error: bad FOCAL DISTANCE\n";
static char FAR str106[] = "Error: bad INTERSECT ADJUST MODE\n";
static char FAR str107[] = "Error: bad IMAGE FORMAT\n";
static char FAR str108[] = "Error: bad FOCAL APERTURE\n";
static char FAR str109[] = "Error: RAW MODE already set\n";
static char FAR str110[] = "Error: unable to open RAW PICTURE FILE (%s)\n";
static char FAR str111[] = "Error: bad SHADING THRESHOLD\n";
static char FAR str112[] = "Error: bad AMBIENT THRESHOLD\n";
static char FAR str113[] = "Error: bad VERBOSE MODE\n";
static char FAR str114[] = "Error: bad CSG LEVEL MAX\n";
static char FAR str115[] = "Error: bad LIGHTS MAX\n";
static char FAR str116[] = "Error: bad OBJECTS MAX\n";
static char FAR str117[] = "Error: bad SURFACES MAX\n";
static char FAR str118[] = "Error: bad DIMENSION OPTION [+%c]\n";
static char FAR str119[] = "Error: bad number of MOVIE FRAMES\n";
static char FAR str120[] = "Error: MOVIE EYE FILE already set\n";
static char FAR str121[] = "Error: unable to open MOVIE EYE FILE (%s)\n";
static char FAR str122[] = "Error: MOVIE LOOK FILE already set\n";
static char FAR str123[] = "Error: unable to open MOVIE LOOK FILE (%s)\n";
static char FAR str124[] = "Error: MOVIE UP FILE already set\n";
static char FAR str125[] = "Error: unable to open MOVIE UP FILE (%s)\n";
static char FAR str126[] = "Error: MOVIE ANGLE FILE already set\n";
static char FAR str127[] = "Error: unable to open MOVIE ANGLE FILE (%s)\n";
static char FAR str128[] = "Error: bad MOVIE OPTION [M%c]\n";
static char FAR str129[] = "Error: bad OPTION [%c]\n";
static char FAR str130[] = "Error: cannot have STEREO SEPARATION\n";
static char FAR str131[] = "Error: no STEREO SEPARATION\n";
static char FAR str132[] = "Error: unable to open SCENE (%s)\n";
static char FAR str133[] = "cannot write correctly to OUTPUT";
static char FAR str134[] = "Error: unable to open PICTURE (%s)\n";
static char FAR str135[] = "too many MOVIE FRAMES (EYE)";
static char FAR str136[] = "CAMERA EYE POINT X";
static char FAR str137[] = "CAMERA EYE POINT Y";
static char FAR str138[] = "CAMERA EYE POINT Z";
static char FAR str139[] = "too many MOVIE FRAMES (LOOK)";
static char FAR str140[] = "CAMERA LOOK POINT X";
static char FAR str141[] = "CAMERA LOOK POINT Y";
static char FAR str142[] = "CAMERA LOOK POINT Z";
static char FAR str143[] = "too many MOVIE FRAMES (UP)";
static char FAR str144[] = "CAMERA UP VECTOR X";
static char FAR str145[] = "CAMERA UP VECTOR Y";
static char FAR str146[] = "CAMERA UP VECTOR Z";
static char FAR str147[] = "too many MOVIE FRAMES (ANGLE)";
static char FAR str148[] = "HORIZONTAL VIEW Angle";
static char FAR str149[] = "VERTICAL VIEW Angle";
static char FAR str150[] = "Walk mode          : %s\n";
static char FAR str151[] = "Error: bad WALK MODE\n";
#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

/***** Command line parameters *****/
#define THRESHOLD_COLOR(x) (3.0 * SQR(x))

#ifdef SUIT
#undef HALT
#define HALT nexit(1)
#define runtime_abort(message) \
{\
  WRITE(ERROR, str1, message);\
  FLUSH(ERROR);\
}
#endif

#if PVM == 1
static char       fullname[256];
static char_ptr
full_name(name)
  char_ptr        name;
{
  sprintf(fullname, "%s/%s", getenv("HOME"), name);
  return fullname;
}
#else
#define full_name(name) name
#endif

void
runtime_abort(s)
  char_ptr        s;
{
  WRITE(ERROR, str1, s);
  FLUSH(ERROR);
  HALT;
}
#if PVM != 1
void
get_valid(file, value, low, high, s)
  file_ptr        file;
  real           *value, low, high;
  char_ptr        s;
{
  double          data;

  READ_REAL(file, &data);
  if (IO_status != IO_OK)
  {
    WRITE(results, "Error: bad %s\n", s);
    FLUSH(results);
    HALT;
  }
  *value = (real) data;
  if (*value < low)
  {
    WRITE(results, "Error: too small %s\n", s);
    FLUSH(results);
    HALT;
  }
  if (*value > high)
  {
    WRITE(results, "Error: too big %s\n", s);
    FLUSH(results);
    HALT;
  }
}
#endif

void
defaults()
{
  screen_size_x = 0;
  screen_size_y = 0;
  threshold_level = 0.01;
  threshold_color = THRESHOLD_COLOR(0.05);
  last_shade_level = 7;
  sampling_levels = 0;
  sampling_divisions = 1 SHL sampling_levels;
  sampling_weight = 4 * SQR(sampling_divisions);
  cluster_size = 4;
  last_ambient_level = 0;
  ambient_sample_rays = 16;
  antialiasing_mode = 0;
  background_mode = 0;
  raw_mode = 0;
  light_mode = 0;
  shade_mode = 1;
  normal_mode = 0;
  normal_check_mode = 0;
  texture_mode = 0;
  view_mode = 0;
  intersect_mode = 0;
  intersect_adjust_mode = 0;
  jittering_mode = 0;
  distributed_cache_mode = 0;
  focal_aperture = 0.0;
  focal_distance = 0.0;
  stereo_separation = 0.0;
  image_format = 0;    /* PIC */
#ifdef ECHO
  verbose_mode = 3;
#else
  verbose_mode = 2;
#endif
  contrast_mode = 0;
  backface_mode = 0;
  walk_mode = 0;

  lights_max = 16;
  objects_max = 0;
  surfaces_max = 256;
  csg_level_max = 256;
  movie_frames = 0;
}
static void
usage()
{
#ifndef lint
  extern char     copyright[];

  WRITE(ERROR, "%s", &(copyright[5]));
  FLUSH(ERROR);
  WRITE(ERROR, str2, PROGRAM_VERSION, __DATE__, __TIME__);
  FLUSH(ERROR);
#endif
  WRITE(ERROR, str3);
  FLUSH(ERROR);
  WRITE(ERROR, "  [w]Screen Width       -> 16... (256)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [h]Screen Height      -> 16... (256)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [p]Sampling Levels    -> 0..%d (0)\n", SAMPLING_LEVEL_MAX);
  FLUSH(ERROR);
  WRITE(ERROR, "  [A]Aliasing Threshold -> 0..1 (0.05)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [s]Shading Levels     -> 1..%d (8)\n", last_shade_level);
  FLUSH(ERROR);
  WRITE(ERROR, "  [S]Shading Threshold  -> 0..1 (0.01) \n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [d]Ambient Levels     -> 0..%d (0)\n", last_shade_level);
  FLUSH(ERROR);
  WRITE(ERROR, "  [D]Ambient Samples    -> 2...\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [T]Ambient Threshold  -> 0..1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [c]Cluster Size       -> %d..%d (4)\n",
        CLUSTER_SIZE_MIN, CLUSTER_SIZE_MAX);
  FLUSH(ERROR);
  WRITE(ERROR, "  [a]Antialiasing Mode  -> 0, 1, 2 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [b]Backface Mode      -> 0, 1, 2 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [i]Intersect Mode     -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [I]Inter. Adjust Mode -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [j]Jittering Mode     -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [l]Lighting Mode      -> 0, 1, 2 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [m]Shading Mode       -> 0, 1 (1)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [n]Normal Mode        -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [t]Texture Mode       -> 0, 1, 2 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [v]View Mode          -> 0, 1, 2 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [x]Walk Mode          -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [z]Normal Check Mode  -> 0, 1 (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [B]Mask Back File     -> FILE (only PIC format)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [R]Raw Picture File   -> FILE\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [F]Focal Distance     -> 0..%0.0f (GAZE)\n", X_MAX);
  FLUSH(ERROR);
  WRITE(ERROR, "  [E]Stereo Separation  -> 0..%0.0f\n", X_MAX);
  FLUSH(ERROR);
  WRITE(ERROR, "  [P]Focal Aperture     -> 0..1\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [O]Output Format      -> 0 - PIC format, 1 - PPM (0)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  [V]Verbose Mode       -> 0, 1, 2, 3 (2)\n");
  FLUSH(ERROR);
  WRITE(ERROR, "  {-|INFILE} {-|OUTFILE} [>LOGFILE]\n");
  FLUSH(ERROR);
}
#ifdef SUIT
#define MIN_PARAMETERS (0)
#else
#define MIN_PARAMETERS (2)
#endif

#define GET_INT(first, number)\
do {\
  IO_status = (sscanf(&(parameter[option][first]), "%hd", &(number)) == 1) ?\
    IO_OK : IO_READ;\
} while (0)

#define GET_LONG(first, number)\
do {\
  IO_status = (sscanf(&(parameter[option][first]), "%d", &(number)) == 1) ?\
    IO_OK : IO_READ;\
} while (0)

#define GET_REAL(first, number)\
do {\
  IO_status = (sscanf(&(parameter[option][first]), "%lf", &(number)) == 1) ?\
    IO_OK : IO_READ;\
} while (0)

void
options()
{
  WRITE(results, str33, screen_size_x);
  FLUSH(results);
  WRITE(results, str34, screen_size_y);
  FLUSH(results);
  WRITE(results, str35, sampling_levels ?
        SQRT(threshold_color / 3.0) : 1.0);
  FLUSH(results);
  WRITE(results, str36, last_shade_level ?
        threshold_level : 1.0);
  FLUSH(results);
  WRITE(results, str37, last_ambient_level ?
        1.0 - threshold_vector : 1.0);
  FLUSH(results);
  WRITE(results, str38, sampling_levels);
  FLUSH(results);
  WRITE(results, str39, SUCC(last_shade_level));
  FLUSH(results);
  WRITE(results, str40, last_ambient_level);
  FLUSH(results);
  WRITE(results, str41, last_ambient_level ?
        ambient_sample_rays : 0);
  FLUSH(results);
  if (cluster_size < -1)
    WRITE(results, str42, -cluster_size);
  else if (cluster_size > 1)
    WRITE(results, str43, cluster_size);
  else
    WRITE(results, str43, 2);
  FLUSH(results);
  WRITE(results, str44);
  FLUSH(results);
  if (sampling_levels == 0)
  {
    WRITE(results, str45);
    FLUSH(results);
  }
  else
    switch (antialiasing_mode)
    {
    case 0:
      WRITE(results, str46);
      FLUSH(results);
      break;
    case 1:
      WRITE(results, str47);
      FLUSH(results);
      break;
    case 2:
      WRITE(results, str48);
      FLUSH(results);
      break;
    }
  switch (backface_mode)
  {
  case 0:
    break;
  case 1:
    WRITE(results, str49);
    FLUSH(results);
    WRITE(results, str50);
    FLUSH(results);
    break;
  case 2:
    WRITE(results, str49);
    FLUSH(results);
    WRITE(results, str52);
    FLUSH(results);
    break;
  }
  WRITE(results, str53, background_mode ? "ON" : "OFF");
  FLUSH(results);
  WRITE(results, str54,
        (intersect_mode OR(antialiasing_mode == 2)) ?
        "ALL OBJECTS" : "PIXEL CORNER OBJECTS");
  FLUSH(results);
  WRITE(results, str55, intersect_adjust_mode ? "ON" : "OFF");
  FLUSH(results);
  WRITE(results, str56, jittering_mode ? "ON" : "OFF");
  FLUSH(results);
  WRITE(results, str57);
  FLUSH(results);
  switch (light_mode)
  {
  case 0:
    WRITE(results, str58);
    FLUSH(results);
    break;
  case 1:
    WRITE(results, str59);
    FLUSH(results);
    break;
  case 2:
    WRITE(results, str60);
    FLUSH(results);
    break;
  }
  WRITE(results, str61, normal_mode ?
        "NORMAL CORRECTION" : "NO NORMAL CORRECTION");
  FLUSH(results);
  WRITE(results, str62, normal_check_mode ? str63 : str64);
  FLUSH(results);
  WRITE(results, str65, shade_mode ? "STRAUSS MODEL" : "PHONG MODEL");
  FLUSH(results);
  WRITE(results, str150, walk_mode ? "HILBERT" : "SERPENTINE");
  FLUSH(results);
  WRITE(results, str66, raw_mode ? "ON" : "OFF");
  FLUSH(results);
  WRITE(results, str67);
  FLUSH(results);
  switch (texture_mode)
  {
  case 0:
    WRITE(results, str68);
    FLUSH(results);
    break;
  case 1:
    WRITE(results, str69);
    FLUSH(results);
    break;
  case 2:
    WRITE(results, str70);
    FLUSH(results);
    break;
  }
  WRITE(results, str71);
  FLUSH(results);
  switch (view_mode)
  {
  case 0:
    WRITE(results, str72);
    FLUSH(results);
    break;
  case 1:
    WRITE(results, str73);
    FLUSH(results);
    break;
  case 2:
    WRITE(results, str74);
    FLUSH(results);
    break;
  }
  if (focal_aperture > ROUNDOFF)
  {
    WRITE(results, str75, focal_aperture);
    FLUSH(results);
  } else
  {
    WRITE(results, str76, "0 - PINHOLE CAMERA");
    FLUSH(results);
  }
  if (focal_distance > ROUNDOFF)
  {
    WRITE(results, str77, focal_distance);
    FLUSH(results);
  } else
  {
    WRITE(results, str78, "GAZE");
    FLUSH(results);
  }
  if (stereo_separation > ROUNDOFF)
  {
    WRITE(results, str79, stereo_separation);
    FLUSH(results);
  }
  if (stereo_separation < -ROUNDOFF)
  {
    WRITE(results, str80, -stereo_separation * 100.0);
    FLUSH(results);
  }
  WRITE(results, str81, image_format ? "PPM" : "PIC");
  FLUSH(results);
  WRITE(ERROR, str82);
  FLUSH(ERROR);
}
void
get_parameters(parameters, parameter)
  int             parameters;   /* Command parameter count */
  char_ptr        parameter[];
{
  int             option, size;
  file_ptr        movie_eye, movie_look, movie_up, movie_angle;

  defaults();
  movie_eye = NULL;
  movie_look = NULL;
  movie_up = NULL;
  movie_angle = NULL;
  if (parameters < MIN_PARAMETERS)
  {
    WRITE(ERROR, str83);
    FLUSH(ERROR);
    usage();
    EXIT;
  }
  for (option = 0; option < parameters - MIN_PARAMETERS; POSINC(option))
  {
    switch (parameter[option][0])
    {
      case 'a':
        GET_INT(1, antialiasing_mode);
        if ((IO_status != IO_OK)
            OR(antialiasing_mode < 0) OR(antialiasing_mode > 2))
        {
          WRITE(ERROR, str84);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'b':
        GET_INT(1, backface_mode);
        if ((IO_status != IO_OK)
            OR(backface_mode < 0) OR(backface_mode > 3))
        {
          WRITE(ERROR, str85);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'c':
        GET_INT(1, cluster_size);
        if ((IO_status != IO_OK)
            OR(cluster_size == -1)
            OR(cluster_size < -CLUSTER_SIZE_MAX)
            OR(cluster_size > CLUSTER_SIZE_MAX))
        {
          WRITE(ERROR, str86);
          FLUSH(ERROR);
          HALT;
        }
        if (cluster_size == 0)
          cluster_size = -4;
        break;

      case 'd':
        GET_INT(1, last_ambient_level);
        if ((IO_status != IO_OK)
            OR(last_ambient_level < 0)
            OR(last_ambient_level > last_shade_level))
        {
          WRITE(ERROR, str87);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'h':
        GET_LONG(1, screen_size_y);
        if ((IO_status != IO_OK) OR(screen_size_y < 16))
        {
          WRITE(ERROR, str88);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'i':
        GET_INT(1, intersect_mode);
        if ((IO_status != IO_OK)
            OR(intersect_mode < 0) OR(intersect_mode > 1))
        {
          WRITE(ERROR, str89);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'j':
        GET_INT(1, jittering_mode);
        if ((IO_status != IO_OK)
            OR(jittering_mode < 0) OR(jittering_mode > 1))
        {
          WRITE(ERROR, str90);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'l':
        GET_INT(1, light_mode);
        if ((IO_status != IO_OK)
            OR(light_mode < 0) OR(light_mode > 2))
        {
          WRITE(ERROR, str91);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'm':
        GET_INT(1, shade_mode);
        if ((IO_status != IO_OK)
            OR(shade_mode < 0) OR(shade_mode > 1))
        {
          WRITE(ERROR, str92);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'n':
        GET_INT(1, normal_mode);
        if ((IO_status != IO_OK)
            OR(normal_mode < 0) OR(normal_mode > 1))
        {
          WRITE(ERROR, str93);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'p':
        GET_INT(1, sampling_levels);
        if ((IO_status != IO_OK)
            OR(sampling_levels < 0) OR(sampling_levels > SAMPLING_LEVEL_MAX))
        {
          WRITE(ERROR, str94);
          FLUSH(ERROR);
          HALT;
        }
        sampling_divisions = 1 SHL sampling_levels;
        sampling_weight = 4 * SQR(sampling_divisions);
        break;

      case 's':
        GET_INT(1, last_shade_level);
        if ((IO_status != IO_OK) OR(last_shade_level <= 0))
        {
          WRITE(ERROR, str95);
          FLUSH(ERROR);
          HALT;
        }
        POSDEC(last_shade_level);
        break;

      case 't':
        GET_INT(1, texture_mode);
        if ((IO_status != IO_OK)
            OR(texture_mode < 0) OR(texture_mode > 2))
        {
          WRITE(ERROR, str96);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'v':
        GET_INT(1, view_mode);
        if ((IO_status != IO_OK)
            OR(view_mode < 0) OR(view_mode > 2))
        {
          WRITE(ERROR, str97);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'w':
        GET_LONG(1, screen_size_x);
        if ((IO_status != IO_OK) OR(screen_size_x < 16))
        {
          WRITE(ERROR, str98);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'x':
        GET_INT(1, walk_mode);
        if ((IO_status != IO_OK)
            OR(walk_mode < 0) OR(walk_mode > 1))
        {
          WRITE(ERROR, str151);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'z':
        GET_INT(1, normal_check_mode);
        if ((IO_status != IO_OK)
            OR(normal_check_mode < 0) OR(normal_check_mode > 1))
        {
          WRITE(ERROR, str99);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'A':
        GET_REAL(1, threshold_color);
        if ((IO_status != IO_OK) OR(threshold_color <= -1.0)
            OR(threshold_color >= 1.0))
        {
          WRITE(ERROR, str100);
          FLUSH(ERROR);
          HALT;
        }
        if (threshold_color < 0.0)
          contrast_mode = 1;
        threshold_color = THRESHOLD_COLOR(threshold_color);
        break;

      case 'B':
        if (background_mode == 1)
        {
          WRITE(ERROR, str101);
          FLUSH(ERROR);
          HALT;
        }
#if PVM != 2
        OPEN(background, &(parameter[option][1]), WRITE_BINARY);
        if (IO_status != IO_OK)
        {
          WRITE(ERROR, str102, &(parameter[option][1]));
          FLUSH(ERROR);
          HALT;
        }
#endif
        background_mode = 1;
        break;

      case 'D':
        GET_INT(1, ambient_sample_rays);
        if ((IO_status != IO_OK) OR(ambient_sample_rays < 2))
        {
          WRITE(ERROR, str103);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'E':
        GET_REAL(1, stereo_separation);
        if ((IO_status != IO_OK)
            OR(stereo_separation < -10.0) OR(stereo_separation > X_MAX))
        {
          WRITE(ERROR, str104);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'F':
        GET_REAL(1, focal_distance);
        if ((IO_status != IO_OK)
            OR(focal_distance < ROUNDOFF) OR(focal_distance > X_MAX))
        {
          WRITE(ERROR, str105);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'I':
        GET_INT(1, intersect_adjust_mode);
        if ((IO_status != IO_OK)
            OR(intersect_adjust_mode < 0) OR(intersect_adjust_mode > 1))
        {
          WRITE(ERROR, str106);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'O':
        GET_INT(1, image_format);
        if ((IO_status != IO_OK)
            OR(image_format < 0) OR(image_format > 1))
        {
          WRITE(ERROR, str107);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'P':
        GET_REAL(1, focal_aperture);
        if ((IO_status != IO_OK)
            OR(focal_aperture < ROUNDOFF) OR(focal_aperture > 1.0))
        {
          WRITE(ERROR, str108);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'R':
        if (raw_mode == 1)
        {
          WRITE(ERROR, str109);
          FLUSH(ERROR);
          HALT;
        }
#if PVM != 2
        OPEN(raw_picture, &(parameter[option][1]), WRITE_BINARY);
        if (IO_status != IO_OK)
        {
          WRITE(ERROR, str110, &(parameter[option][1]));
          FLUSH(ERROR);
          HALT;
        }
#endif
        raw_mode = 1;
        break;

      case 'S':
        GET_REAL(1, threshold_level);
        if ((IO_status != IO_OK) OR(threshold_level <= 0.0)
            OR(threshold_level >= 1.0))
        {
          WRITE(ERROR, str111);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case 'T':
        GET_REAL(1, threshold_vector);
        if ((IO_status != IO_OK) OR(threshold_vector < 0.0)
            OR(threshold_vector >= 1.0))
        {
          WRITE(ERROR, str112);
          FLUSH(ERROR);
          HALT;
        }
        distributed_cache_mode = (threshold_vector > ROUNDOFF);
        break;

      case 'V':
        GET_INT(1, verbose_mode);
        if ((IO_status != IO_OK)
#if PVM == 0
            OR(verbose_mode < -2) OR(verbose_mode > 3))
#else
            OR(verbose_mode < -2) OR(verbose_mode > 4))
#endif
        {
          WRITE(ERROR, str113);
          FLUSH(ERROR);
          HALT;
        }
        break;

      case '+':
        if (parameter[option][1] != EOT)
        {
          switch (parameter[option][1])
          {
            case 'C':
              GET_INT(2, csg_level_max);
              if ((IO_status != IO_OK) OR(csg_level_max < 1))
              {
                WRITE(ERROR, str114);
                FLUSH(ERROR);
                HALT;
              }
              break;

            case 'L':
              GET_LONG(2, lights_max);
              if ((IO_status != IO_OK) OR(lights_max < 1))
              {
                WRITE(ERROR, str115);
                FLUSH(ERROR);
                HALT;
              }
              break;

            case 'O':
              GET_LONG(2, objects_max);
              if ((IO_status != IO_OK) OR(objects_max < 1))
              {
                WRITE(ERROR, str116);
                FLUSH(ERROR);
                HALT;
              }
              break;

            case 'S':
              GET_LONG(2, surfaces_max);
              if ((IO_status != IO_OK) OR(surfaces_max < 1))
              {
                WRITE(ERROR, str117);
                FLUSH(ERROR);
                HALT;
              }
              break;

            default:
              WRITE(ERROR, "Error: bad DIMENSION OPTION [+%c]\n",
                    parameter[option][1]);
              FLUSH(ERROR);
              usage();
              HALT;
              break;
          }
        } else
	{
          WRITE(ERROR, "Error: no DIMENSION OPTION [+]\n");
          FLUSH(ERROR);
          usage();
          HALT;
	}
	break;

      case 'M':
        if (parameter[option][1] != EOT)
        {
          switch (parameter[option][1])
          {
            case '+':
              GET_INT(2, movie_frames);
              if ((IO_status != IO_OK) OR(movie_frames < 1))
              {
                WRITE(ERROR, str119);
                FLUSH(ERROR);
                HALT;
              }
              break;

            case 'e':
              if (movie_eye != NULL)
              {
                WRITE(ERROR, str120);
                FLUSH(ERROR);
                HALT;
              }
              OPEN(movie_eye, full_name(&(parameter[option][2])), READ_TEXT);
              if (IO_status != IO_OK)
              {
                WRITE(ERROR, str121, &(parameter[option][2]));
                FLUSH(ERROR);
                HALT;
              }
              if (movie_frames == 0)
                movie_frames = -1;
              break;

            case 'l':
              if (movie_look != NULL)
              {
                WRITE(ERROR, str122);
                FLUSH(ERROR);
                HALT;
              }
              OPEN(movie_look, full_name(&(parameter[option][2])), READ_TEXT);
              if (IO_status != IO_OK)
              {
                WRITE(ERROR, str123, &(parameter[option][2]));
                FLUSH(ERROR);
                HALT;
              }
              if (movie_frames == 0)
                movie_frames = -1;
              break;

            case 'u':
              if (movie_up != NULL)
              {
                WRITE(ERROR, str124);
                FLUSH(ERROR);
                HALT;
              }
              OPEN(movie_up, full_name(&(parameter[option][2])), READ_TEXT);
              if (IO_status != IO_OK)
              {
                WRITE(ERROR, str125, &(parameter[option][2]));
                FLUSH(ERROR);
                HALT;
              }
              if (movie_frames == 0)
                movie_frames = -1;
              break;

            case 'a':
              if (movie_angle != NULL)
              {
                WRITE(ERROR, str126);
                FLUSH(ERROR);
                HALT;
              }
              OPEN(movie_angle, full_name(&(parameter[option][2])), READ_TEXT);
              if (IO_status != IO_OK)
              {
                WRITE(ERROR, str127, &(parameter[option][2]));
                FLUSH(ERROR);
                HALT;
              }
              if (movie_frames == 0)
                movie_frames = -1;
              break;

            default:
              WRITE(ERROR, "Error: bad MOVIE OPTION [M%c]\n",
                    parameter[option][1]);
              FLUSH(ERROR);
              usage();
              HALT;
              break;
          }
        } else
	{
          WRITE(ERROR, "Error: no MOVIE OPTION [M]\n");
          FLUSH(ERROR);
          usage();
          HALT;
	}
	break;

#if PVM
      case '=':
#if PVM == 1
        if (parameter[option][1] != EOT)
        {
          switch (parameter[option][1])
          {
            case 'L':
              GET_LONG(2, lines_task);
              if ((IO_status != IO_OK) OR(lines_task < 1))
              {
                WRITE(ERROR, "Error: bad LINES per TASK\n");
                FLUSH(ERROR);
                HALT;
              }
              break;

            default:
              WRITE(ERROR, "Error: bad NET OPTION [=%c]\n",
                    parameter[option][1]);
              FLUSH(ERROR);
              usage();
              HALT;
              break;
          }
	} else
	{
          WRITE(ERROR, "Error: no NET OPTION [=]\n");
          FLUSH(ERROR);
          usage();
          HALT;
	}
#endif
	break;
#endif

      default:
        WRITE(ERROR, "Error: bad OPTION [%c]\n", parameter[option][0]);
        FLUSH(ERROR);
        usage();
        HALT;
        break;
    }
  }
  if ((light_mode != 0) AND(backface_mode != 0))
    backface_mode = 0;
  if ((screen_size_x == 0) AND(screen_size_y == 0))
  {
    screen_size_x = 256;
    screen_size_y = 256;
  } else
  if (screen_size_x == 0)
    screen_size_x = screen_size_y;
  else
  if (screen_size_y == 0)
    screen_size_y = screen_size_x;
  if ((view_mode == 0) AND(ABS(stereo_separation) > ROUNDOFF))
  {
    WRITE(ERROR, str130);
    FLUSH(ERROR);
    HALT;
  }
  if ((view_mode != 0) AND(ABS(stereo_separation) < ROUNDOFF))
  {
    WRITE(ERROR, str131);
    FLUSH(ERROR);
    HALT;
  }
  if (threshold_color > THRESHOLD_COLOR(0.9))
  {
    sampling_levels = 0;
    sampling_divisions = 1 SHL sampling_levels;
    sampling_weight = 4 * SQR(sampling_divisions);
    threshold_color = THRESHOLD_COLOR(1.0);
  }
  if (distributed_cache_mode != 0)
  {
    distributed_cache_repetitions = MAX(8, threshold_vector * screen_size_x);
    threshold_vector = 1.0 - threshold_vector;
  } else
    threshold_vector = 1.0;
  if (objects_max == 0)
    objects_max = 25000;
  else
  {
    if (cluster_size > 1)
      size = cluster_size;
    else
      size = 2;
    objects_max = (long) (objects_max * (size + 2.0) / size +
                          SUP(LOGN(objects_max, size))) - 1;
  }
#ifdef SUIT
  scene = NULL;
  picture = NULL;
#else
#if PVM == 0
  if ((parameter[option][0] == '-') AND(parameter[option][1] == EOT))
    scene = INPUT;
  else
#endif
  {
    OPEN(scene, full_name(parameter[option]), READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(ERROR, str132, parameter[option]);
      FLUSH(ERROR);
      HALT;
    }
  }
  POSINC(option);
#if PVM != 2
  if ((parameter[option][0] == '-') AND(parameter[option][1] == EOT))
  {
#ifdef dos
    runtime_abort(str133);
#endif
#ifdef THINK_C
    runtime_abort(str133);
#endif
#if PVM == 1
    runtime_abort(str133);
#endif
    picture = OUTPUT;
    results = ERROR;
  } else
  {
    OPEN(picture, parameter[option], WRITE_BINARY);
    if (IO_status != IO_OK)
    {
      WRITE(ERROR, str134, parameter[option]);
      FLUSH(ERROR);
      HALT;
    }
    results = OUTPUT;
  }
#else
  results = ERROR;
#endif
#endif /* SUIT */

  if (movie_frames != 0)
#if PVM != 1
  {
    int             i, movie_frames_max;

    if (backface_mode == 2)
      backface_mode = 1;
    if (movie_frames < 0)
      movie_frames = 50;
    movie_frames_max = movie_frames;
    ALLOCATE(camera, camera_struct, movie_frames, OTHER_TYPE);
    for (i = 0; i < movie_frames_max; POSINC(i))
    {
      camera[i].eye = NULL;
      camera[i].look = NULL;
      camera[i].up = NULL;
      camera[i].view_angle_x = -1.0;
      camera[i].view_angle_y = -1.0;
    }
    i = 0;
    movie_frames = 0;
    if (movie_eye != NULL)
    {
      while (NOT END_OF_LINE(movie_eye))
      {
        if (i == movie_frames_max)
          runtime_abort(str135);
        ALLOCATE(camera[i].eye, xyz_struct, 1, OTHER_TYPE);
        get_valid(movie_eye, &(camera[i].eye->x), X_MIN, X_MAX, str136);
        get_valid(movie_eye, &(camera[i].eye->y), Y_MIN, Y_MAX, str137);
        get_valid(movie_eye, &(camera[i].eye->z), Z_MIN, Z_MAX, str138);
        ADVANCE(movie_eye);
        POSINC(i);
      }
      CLOSE(movie_eye);
      movie_frames = i;
      i = 0;
    }
    if (movie_look != NULL)
    {
      while (NOT END_OF_LINE(movie_look))
      {
        if (i == movie_frames_max)
          runtime_abort(str139);
        ALLOCATE(camera[i].look, xyz_struct, 1, OTHER_TYPE);
        get_valid(movie_look, &(camera[i].look->x), X_MIN, X_MAX, str140);
        get_valid(movie_look, &(camera[i].look->y), Y_MIN, Y_MAX, str141);
        get_valid(movie_look, &(camera[i].look->z), Z_MIN, Z_MAX, str142);
        ADVANCE(movie_look);
        POSINC(i);
      }
      CLOSE(movie_look);
      if (i > movie_frames)
        movie_frames = i;
      i = 0;
    }
    if (movie_up != NULL)
    {
      while (NOT END_OF_LINE(movie_up))
      {
        if (i == movie_frames_max)
          runtime_abort(str143);
        ALLOCATE(camera[i].up, xyz_struct, 1, OTHER_TYPE);
        get_valid(movie_up, &(camera[i].up->x), X_MIN, X_MAX, str144);
        get_valid(movie_up, &(camera[i].up->y), Y_MIN, Y_MAX, str145);
        get_valid(movie_up, &(camera[i].up->z), Z_MIN, Z_MAX, str146);
        ADVANCE(movie_up);
        POSINC(i);
      }
      CLOSE(movie_up);
      if (i > movie_frames)
        movie_frames = i;
      i = 0;
    }
    if (movie_angle != NULL)
    {
      while (NOT END_OF_LINE(movie_angle))
      {
        if (i == movie_frames_max)
          runtime_abort(str147);
        get_valid(movie_angle, &(camera[i].view_angle_x), 0.5, 89.5, str148);
        camera[i].view_angle_x = DEGREE_TO_RADIAN(camera[i].view_angle_x);
        get_valid(movie_angle, &(camera[i].view_angle_y), 0.5, 89.5, str149);
        camera[i].view_angle_y = DEGREE_TO_RADIAN(camera[i].view_angle_y);
        ADVANCE(movie_angle);
        POSINC(i);
      }
      CLOSE(movie_angle);
      if (i > movie_frames)
        movie_frames = i;
      i = 0;
    }
  }
#else
  {
    if (movie_eye != NULL)
      CLOSE(movie_eye);
    if (movie_look != NULL)
      CLOSE(movie_look);
    if (movie_up != NULL)
      CLOSE(movie_up);
    if (movie_angle != NULL)
      CLOSE(movie_angle);
  }
#endif

#if PVM == 2
  verbose_mode = -99;
  return;
#else
  if (verbose_mode <= 0)
    return;

  options();
#endif
}

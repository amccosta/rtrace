/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd           - improvements
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
 *    RAY TRACING - Picture - Version 8.4.0                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, January 1994           *
 **********************************************************************/

#ifdef dos
#ifndef SUIT
#ifdef __GNUC__
#define GRX
#endif
#endif
#endif

#define INDEX(v)\
(ROUND(MAX(0.0, MIN((real) INDEX_MAX, (v) * (real) SUCC(INDEX_MAX)))))

#ifdef GRX
#include "dispgrx.c"
static int      current_line;
#endif

/***** Write picture *****/
void
init_picture()
{
  char            header[STRING_MAX];

#ifdef GRX
  if (verbose_mode < 0)
  {
    current_line = 0;
    display_init((int) screen_size_x, (int) screen_size_y);
  }
#endif

  switch (image_format)
  {
  case 0:       /* PIC */
    /* Screen Dimensions */
    WRITE_CHAR(picture, (unsigned char) (screen_size_x MOD 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_x DIV 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_y MOD 256));
    WRITE_CHAR(picture, (unsigned char) (screen_size_y DIV 256));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    if (raw_mode == 1)
    {
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_x MOD 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_x DIV 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_y MOD 256));
      WRITE_CHAR(raw_picture, (unsigned char) (screen_size_y DIV 256));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
    }
    image_format_offset = 4;
    break;
  case 1:       /* PPM */
    WRITE_STR(picture, "P6\n");
    WRITE_STR(picture, "%d\n", screen_size_x);
    WRITE_STR(picture, "%d\n", screen_size_y);
    WRITE_STR(picture, "255\n");
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE in PPM format");
    if (raw_mode == 1)
    {
      WRITE_STR(raw_picture, "P6\n");
      WRITE_STR(raw_picture, "%d\n", screen_size_x);
      WRITE_STR(raw_picture, "%d\n", screen_size_y);
      WRITE_STR(raw_picture, "255\n");
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE in PPM format");
    }
    sprintf(header, "P6\n%d\n%d\n255\n", screen_size_x, screen_size_y);
    image_format_offset = strlen(header);
    break;
  }
  if (background_mode == 1)
  {
    WRITE_CHAR(background, (unsigned char) (screen_size_x MOD 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_x DIV 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_y MOD 256));
    WRITE_CHAR(background, (unsigned char) (screen_size_y DIV 256));
    if (IO_status != IO_OK)
      runtime_abort("unable to write BACKGROUND MASK");
  }
}

void
line_picture()
{
  REG int         i;

  for (i = 1; i <= screen_size_x; POSINC(i))
  {
#ifdef GRX
    if (verbose_mode < 0)
      display_plot(i - 1, current_line, true_color[i].r, true_color[i].g,
                   true_color[i].b);
#endif

    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].r));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].g));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    WRITE_CHAR(picture, (unsigned char) INDEX(true_color[i].b));
    if (IO_status != IO_OK)
      runtime_abort("unable to write PICTURE");
    if (background_mode == 1)
    {
      WRITE_CHAR(background, (unsigned char) INDEX(back_mask[i]));
      if (IO_status != IO_OK)
        runtime_abort("unable to write BACKGROUND MASK");
    }
    if (raw_mode == 1)
    {
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.r));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.g));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
      WRITE_CHAR(raw_picture, (unsigned char) INDEX(new_line[i].color.b));
      if (IO_status != IO_OK)
        runtime_abort("unable to write RAW PICTURE");
    }
  }
  FLUSH(picture);
  if (background_mode == 1)
    FLUSH(background);
  if (raw_mode == 1)
    FLUSH(raw_picture);

#ifdef GRX
  if (verbose_mode < 0)
  {
    POSINC(current_line);
    if (current_line == screen_size_y)
    {
      display_finished();
      display_close();
    }
  }
#endif
}

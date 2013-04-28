/*
 * Copyright (c) 1992, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Reid Judd           - overall, portability
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

/**********************************************************************
 *    RAY TRACING - Configuration - Version 8.4.1                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, September 1992         *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1994         *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <console.h>
#ifdef TIMES
#undef TIMES
#endif
#ifndef TIME
#define TIME
#endif
#define ECHO

static char     Message[132];

#define ERROR  OUTPUT
#define OUTPUT Message

#define WRITE sprintf

#define FLUSH(file)\
do {\
  if ((file) == Message)\
    show(Message);\
  else\
    fflush(file);\
} while (0)

static void
show(message)
  char_ptr      message;
{
  /* ... */
}

#define CHECK_EVENTS check_events()

static void
check_events()
{
  /* ... */
}

#define INPUT_PARAMETERS\
do {\
  console_options.nrows = 35;\
  console_options.ncols = 55;\
  cshow(stdout);\
  parameters = ccommand(&parameter);\
} while (0)

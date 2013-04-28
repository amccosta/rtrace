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

#ifdef __STDC__
#include <stdlib.h>
#endif
#include <memory.h>
#ifdef TIME
#undef TIME
#endif
#ifndef TIMES
#define TIMES
#endif

#ifdef SUIT

static char     Message[255];

#define ERROR   OUTPUT
#define results OUTPUT
#define OUTPUT  Message

#define WRITE sprintf

extern void     show(char_ptr);
extern SUIT_object message_window;
extern void     inc_pixel_counter();
#include <setjmp.h>
extern jmp_buf  environment;

#define CHECK_EVENTS SUIT_checkAndProcessInput(0)

#define FLUSH(file)\
do {\
  if ((char_ptr) (file) == Message)\
  {\
    show(Message);\
    CHECK_EVENTS;\
  } else\
    fflush((file_ptr) file);\
} while (0)

#define INC_PIXEL_COUNTER inc_pixel_counter()

#define HALT longjmp(environment, 1)

#endif

/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
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
 *    RAY TRACING - Configuration - Version 8.4.2                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1994          *
 **********************************************************************/

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION "8.4.2"
#endif

#include <stdio.h>
#include <math.h>
#ifdef SUIT
#include "suit.h"
#endif

/***** Constants *****/
#ifndef NULL
#define NULL (0)
#endif
#ifdef TRUE
#undef TRUE
#endif
#define TRUE (1)
#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)
#ifndef MAXINT
#define MAXINT (~((unsigned) 1 << (sizeof(int) * 8 - 1)))
#endif

/***** Types *****/
#ifndef SUIT
typedef
int             boolean;
#endif

typedef
double          real;

typedef
char           *char_ptr;

typedef
FILE           *file_ptr;

/***** Boolean operators *****/
#define NOT !
#define AND &&
#define OR  ||

#define BIT_NOT ~
#define BIT_AND &
#define BIT_OR  |
#define BIT_XOR ^

/***** Integer operators *****/
#define DIV /
#define MOD %

#define SHL <<
#define SHR >>

/***** Others *****/
#define PREINC(x) (++(x))
#define POSINC(x) ((x)++)
#define PREDEC(x) (--(x))
#define POSDEC(x) ((x)--)

#define SUCC(x) ((x) + 1)
#define PRED(x) ((x) - 1)
#define ODD(x)  ((x) & 1)

#define TRUNC(x) ((long int) (x))
#define ROUND(x) ((long int) floor((double) (x) + 0.5))
#define SUP(x)   ((long int) ceil((double) (x)))
#define INF(x)   ((long int) floor((double) (x)))

#ifndef ABS
#define ABS(x)      ((x) > 0 ? (x) : -(x))
#endif
#define FRAC(x)     ((real) ((double) (x) - floor((double) (x))))
#define SIN(x)      ((real) sin((double) (x)))
#define COS(x)      ((real) cos((double) (x)))
#define SQR(x)      ((real) ((double) (x) * (double) (x)))
#define SQRT(x)     ((real) sqrt((double) (x)))
#define POWER(x, y) ((real) pow((double) (x), (double) (y)))
#define LOGN(x, y)  ((real) log((double) (x)) / log((double) (y)))
#define ARCCOS(x)   ((real) acos((double) (x)))

/***** Compiler stuffs *****/
#ifdef THINK_C
#include "mac.h"
#else

#ifdef vms
#include "vms.h"
#else

#ifdef _transputer
#include "transp.h"
#else

#ifdef dos
#include "dos.h"
#else

#ifdef hpux
#include "hpux.h"
#else

#include "general.h"

#endif
#endif
#endif
#endif
#endif

#ifndef EXIT
#define EXIT nexit(0)
#endif
#ifndef HALT
#define HALT nexit(1)
#endif

#ifndef __DATE__
#define __DATE__ "November 7 1994"
#endif
#ifndef __TIME__
#define __TIME__ "3:00:00"
#endif

/***** If there is no void type, define NOVOID *****/
#ifdef NOVOID
#define void char
#define void_ptr char_ptr
#else
typedef
void           *void_ptr;
#endif

/***** If there are no register variables, define NOREGISTERS *****/
#ifdef NOREGISTERS
#define REG
#undef NOREGISTERS
#else
#define REG register
#endif

/***** If there are function prototypes, define PROTOTYPES *****/
#ifndef PROTOTYPES
#ifdef __STDC__
#ifndef NOVOID
#define PROTOTYPES
#endif
#endif
#endif

#ifdef NOVOID
#undef NOVOID
#endif

/***** If there is a function that gives time, define TIME or TIMES *****/
#ifndef TIME
#ifndef TIMES
#define CPU_CLOCK ((real) 0)
#endif
#endif

/* Using time() function */
#ifdef TIME
#include <time.h>
/* CPU_CLOCK must return time in milliseconds */
#define CPU_CLOCK ((real) ((double) time(0) * 1000.0))
#undef TIME
#endif

/* Using times() function */
#ifdef TIMES
#ifdef vms                      /* VAX-VMS */
#include <time.h>
#define tms_utime proc_user_time
#define tms_stime proc_system_time
#define tms tbuffer_t
#else
#include <sys/types.h>
#include <sys/times.h>
#ifndef CLK_TCK
#ifdef ultrix                   /* ULTRIX */
#define CLK_TCK (60)
#else                           /* Others */
#define CLK_TCK (100)
#endif
#endif
#endif
/* CPU_CLOCK must return time in milliseconds */
#define CPU_CLOCK cpu_clock()
static real
cpu_clock()
{
#ifdef vms
  tms             cpu_time;
#else
  struct tms      cpu_time;
#endif
  (void) times(&cpu_time);
  return (real) ((double) cpu_time.tms_utime * 1000.0 / (double) CLK_TCK);
}
#undef TIMES
#endif

/***** If cannot assign structs to structs, define NOSTRUCTASSIGN *****/
#ifdef NOSTRUCTASSIGN
#define STRUCT_ASSIGN(d, s)\
(void) memcpy((char_ptr) &(d), (char_ptr) &(s), sizeof(d))
#undef NOSTRUCTASSIGN
#else
#define STRUCT_ASSIGN(d, s) (d) = (s)
#endif

#define ARRAY_ASSIGN(d, s, c)\
(void) memcpy((char_ptr) (d), (char_ptr) (s), (c) * sizeof(d))

/***** Input & Output *****/
#define IO_EOF   (-1)
#define IO_OK    (0)
#define IO_OPEN  (1)
#define IO_READ  (2)
#define IO_WRITE (3)
#define IO_SEEK  (4)

#define EOT     '\0'
#define NEWLINE '\n'
#define SPACE   ' '
#define TAB     '\t'

#ifndef INPUT
#define INPUT  stdin
#endif
#ifndef OUTPUT
#define OUTPUT stdout
#endif
#ifndef ERROR
#define ERROR  stderr
#endif

#define READ_BINARY  "r+b"
#define READ_TEXT    "r+t"
#define WRITE_BINARY "w+b"
#define WRITE_TEXT   "w+t"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

static int      IO_status;

#define STATUS(file, code)\
IO_status = feof(file) ? IO_EOF : (ferror(file) ? code : IO_OK)

#define OPEN(file, name, mode)\
do {\
  (file) = fopen(name, mode);\
  IO_status = (file) ? IO_OK : IO_OPEN;\
} while (0)

#define CLOSE(file)\
do {\
  IO_status = fclose(file) ? IO_EOF : IO_OK;\
  file = NULL;\
} while (0)

#ifndef FLUSH
#define FLUSH(file)\
IO_status = fflush(file) ? IO_EOF : IO_OK
#endif

#ifndef WRITE
#define WRITE (void) fprintf
#endif

#define WRITE_STR (void) fprintf

#define WRITE_CHAR(file, c)\
do {\
  (void) putc(c, file);\
  STATUS(file, IO_WRITE);\
} while (0)

static int
PEEK_CHAR(file)
  file_ptr        file;
{
  int             character;

  character = getc(file);
  STATUS(file, IO_READ);
  if (IO_status == IO_OK)
    return ungetc(character, file);
  if (IO_status == IO_EOF)
    return NEWLINE;
  return IO_READ;
}

#define END_OF_LINE(file)\
(PEEK_CHAR(file) == NEWLINE)

#define READ_LINE(file)\
while ((getc(file) != NEWLINE) AND((STATUS(file, IO_READ)) == IO_OK))

#define READ_CHAR(file, character)\
do {\
  (character) = (unsigned char) getc(file);\
  STATUS(file, IO_READ);\
} while (0)

#define READ_REAL(file, real)\
IO_status = fscanf(file, "%lf", real) ? IO_OK : IO_READ

#define READ_STRING(file, string)\
IO_status = fscanf(file, "%s", string) ? IO_OK : IO_READ

#define SEEK(file, offset)\
IO_status = fseek(file, offset, SEEK_SET) ? IO_SEEK : IO_OK

/***** Memory allocation *****/
#include "nmalloc.h"

#define ALLOC(p, s, c, t)\
(p) = (s *) nmalloc((unsigned int) ((c) * sizeof(s)), t)

#define FREE(p) nfree((char_ptr) (p))

/***** Etc *****/
#ifndef CHECK_EVENTS
#define CHECK_EVENTS
#endif

#ifndef INC_PIXEL_COUNTER
#define INC_PIXEL_COUNTER
#endif

#ifndef INPUT_PARAMETERS
#define INPUT_PARAMETERS
#endif

/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Craig Kolb          - noise code
 *  Robert Skinner      - noise functions
 *  David Buck          - noise functions
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
 *    RAY TRACING - Noise - Version 8.3.4                             *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, March 1990             *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

#define S_CURVE(a) ((a) * (a) * (3.0 - 2.0 * (a)))

#define HASH(a, b, c)\
hash_table[hash_table[hash_table[(a) BIT_AND 4095] BIT_XOR\
((b) BIT_AND 4095)] BIT_XOR\
((c) BIT_AND 4095)]

#define INCRSUM(m, s, x, y, z)\
((s) * (random_table[m] * 0.5\
+ random_table[m + 1] * (x)\
+ random_table[m + 2] * (y)\
+ random_table[m + 3] * (z)))

#define RANDOM_TABLE_SIZE (267)
#define SIN_TABLE_SIZE (1000)

#ifdef _Windows
#pragma option -zEnoise1s -zFnoise1c -zHnoise1g
#define FAR far
#else
#define FAR
#endif

static real FAR random_table[RANDOM_TABLE_SIZE];
static short int FAR *hash_table;
static real FAR sin_table[SIN_TABLE_SIZE];
static unsigned short int FAR crc_table[256] =
{
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

static void
init_hash_table()
{
  REG int         i, j;
  short int       temp;

  ALLOCATE(hash_table, short int, 4096, TRACE_TYPE);
  for (i = 0; i < 4096; POSINC(i))
    hash_table[i] = i;
  for (i = 4095; i > 0; POSDEC(i))
  {
    j = (int) (RANDOM * 4096.0);
    temp = hash_table[i];
    hash_table[i] = hash_table[j];
    hash_table[j] = temp;
  }
}
/*
 * ATTENTION: Note that passing a real to crc16 and interpreting it as
 * an array of chars means that machines with different floating-point
 * representation schemes will evaluate black_noise(point) differently.
 */
static unsigned int
crc16(byte, count)
  REG char_ptr    byte;
  REG int         count;
{
  REG unsigned int crc;

  crc = 0;
  while (POSDEC(count))
  {
    crc = (crc SHR 8) BIT_XOR
      crc_table[(unsigned char) (crc BIT_XOR *byte)];
    POSINC(byte);
  }
  return crc;
}
static unsigned int
multiply(p)
  xyz_ptr         p;
{
  p->x *= 0.12345;
  p->y *= 0.12345;
  p->z *= 0.12345;
  return crc16((char_ptr) p, sizeof(xyz_struct));
}
void
init_texture()
{
  REG int         i;
  xyz_struct      p;

  init_hash_table();
  for (i = 0; i < RANDOM_TABLE_SIZE; POSINC(i))
  {
    p.x = (real) i;
    p.y = (real) i;
    p.z = (real) i;
    random_table[i] = (real) multiply(&p) / 32768.0 - 1.0;
  }
  for (i = 0; i < SIN_TABLE_SIZE; POSINC(i))
    sin_table[i] = SIN((real) i / (real) SIN_TABLE_SIZE * 2.0 * PI);
  p.y = 0.0;
  p.z = 0.0;
  for (i = 0; i < WAVES_MAX; POSINC(i))
  {
    p.x = (real) i;
    black_noise2(&p, &(wave.source[i]));
    NORMALIZE(wave.source[i]);
    wave.frequency[i] = RANDOM;
  }
}
real
black_noise(p)
  xyz_ptr         p;
{
  REG long        ix, iy, iz, jx, jy, jz;
  real            x, y, z, fx, fy, fz;
  real            sx, sy, sz, tx, ty, tz;
  real            sum;
  short int       m;

  x = p->x + MAXINT DIV 2;
  y = p->y + MAXINT DIV 2;
  z = p->z + MAXINT DIV 2;
  ix = (long) x;
  iy = (long) y;
  iz = (long) z;
  jx = ix + 1;
  jy = iy + 1;
  jz = iz + 1;
  fx = FRAC(x);
  fy = FRAC(y);
  fz = FRAC(z);
  sx = S_CURVE(fx);
  sy = S_CURVE(fy);
  sz = S_CURVE(fz);
  tx = 1.0 - sx;
  ty = 1.0 - sy;
  tz = 1.0 - sz;

  m = HASH(ix, iy, iz) BIT_AND 255;
  sum = INCRSUM(m, tx * ty * tz, fx, fy, fz);
  m = HASH(jx, iy, iz) BIT_AND 255;
  sum += INCRSUM(m, sx * ty * tz, fx - 1.0, fy, fz);
  m = HASH(ix, jy, iz) BIT_AND 255;
  sum += INCRSUM(m, tx * sy * tz, fx, fy - 1.0, fz);
  m = HASH(jx, jy, iz) BIT_AND 255;
  sum += INCRSUM(m, sx * sy * tz, fx - 1.0, fy - 1.0, fz);
  m = HASH(ix, iy, jz) BIT_AND 255;
  sum += INCRSUM(m, tx * ty * sz, fx, fy, fz - 1.0);
  m = HASH(jx, iy, jz) BIT_AND 255;
  sum += INCRSUM(m, sx * ty * sz, fx - 1.0, fy, fz - 1.0);
  m = HASH(ix, jy, jz) BIT_AND 255;
  sum += INCRSUM(m, tx * sy * sz, fx, fy - 1.0, fz - 1.0);
  m = HASH(jx, jy, jz) BIT_AND 255;
  sum += INCRSUM(m, sx * sy * sz, fx - 1.0, fy - 1.0, fz - 1.0);
  sum = MAX(0.0, MIN(1.0, sum + 0.5));
  return sum;
}
void
black_noise2(p, r)
  xyz_ptr         p, r;
{
  REG long        ix, iy, iz, jx, jy, jz;
  real            x, y, z, fx, fy, fz;
  real            sx, sy, sz, tx, ty, tz;
  real            px, py, pz, s;
  short int       m;

  x = p->x + MAXINT DIV 2;
  y = p->y + MAXINT DIV 2;
  z = p->z + MAXINT DIV 2;
  ix = (long) x;
  iy = (long) y;
  iz = (long) z;
  jx = ix + 1;
  jy = iy + 1;
  jz = iz + 1;
  fx = FRAC(x);
  fy = FRAC(y);
  fz = FRAC(z);
  sx = S_CURVE(fx);
  sy = S_CURVE(fy);
  sz = S_CURVE(fz);
  tx = 1.0 - sx;
  ty = 1.0 - sy;
  tz = 1.0 - sz;

  m = HASH(ix, iy, iz) BIT_AND 255;
  px = fx;
  py = fy;
  pz = fz;
  s = tx * ty * tz;
  r->x = INCRSUM(m, s, px, py, pz);
  r->y = INCRSUM(m + 4, s, px, py, pz);
  r->z = INCRSUM(m + 8, s, px, py, pz);
  m = HASH(jx, iy, iz) BIT_AND 255;
  px = fx - 1.0;
  s = sx * ty * tz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(jx, jy, iz) BIT_AND 255;
  py = fy - 1.0;
  s = sx * sy * tz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(ix, jy, iz) BIT_AND 255;
  px = fx;
  s = tx * sy * tz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(ix, jy, jz) BIT_AND 255;
  pz = fz - 1.0;
  s = tx * sy * sz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(jx, jy, jz) BIT_AND 255;
  px = fx - 1.0;
  s = sx * sy * sz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(jx, iy, jz) BIT_AND 255;
  py = fy;
  s = sx * ty * sz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
  m = HASH(ix, iy, jz) BIT_AND 255;
  px = fx;
  s = tx * ty * sz;
  r->x += INCRSUM(m, s, px, py, pz);
  r->y += INCRSUM(m + 4, s, px, py, pz);
  r->z += INCRSUM(m + 8, s, px, py, pz);
}
real
fbm_noise(p, omega, lambda, octaves)
  REG xyz_ptr     p;
  real            omega, lambda;
  int             octaves;
{
  REG int         i;
  REG real        l, o, value;
  xyz_struct      temp;

  value = black_noise(p);
  l = lambda;
  o = omega;
  for (i = 2; i <= octaves; POSINC(i))
  {
    temp.x = l * p->x;
    temp.y = l * p->y;
    temp.z = l * p->z;
    value += o * black_noise(&temp);
    if (i < octaves)
    {
      l *= lambda;
      o *= omega;
    }
  }
  return value;
}
void
fbm_noise2(p, omega, lambda, octaves, r)
  REG xyz_ptr     p;
  real            omega, lambda;
  int             octaves;
  xyz_ptr         r;
{
  REG int         i;
  REG real        l, o;
  xyz_struct      temp, value;

  black_noise2(p, r);
  l = lambda;
  o = omega;
  for (i = 2; i <= octaves; POSINC(i))
  {
    temp.x = l * p->x;
    temp.y = l * p->y;
    temp.z = l * p->z;
    black_noise2(&temp, &value);
    r->x += o * value.x;
    r->y += o * value.y;
    r->z += o * value.z;
    if (i < octaves)
    {
      l *= lambda;
      o *= omega;
    }
  }
}
real
chaos_noise(p, octaves)
  xyz_ptr         p;
  int             octaves;
{
  return fbm_noise(p, 0.5, 2.0, octaves);
}
real
marble_noise(p)
  xyz_ptr         p;
{
  REG real        t;

  t = (SIN(8.0 * chaos_noise(p, 6) + 7.0 * p->z) + 1.0) * 0.5;
  return POWER(t, 0.77);
}
real
turbulence(p)
  xyz_ptr         p;
{
  return fbm_noise(p, 0.5, 2.0, 4);
}
void
turbulence2(p, r)
  xyz_ptr         p, r;
{
  fbm_noise2(p, 0.5, 2.0, 4, r);
}
real
cycloidal(value)
  real            value;
{
  REG real        t;

  t = sin_table[(unsigned int) (FRAC(ABS(value)) * (real) SIN_TABLE_SIZE)];
  return t;
}
real
triangle_wave(value)
  real            value;
{
  REG real        offset;

  if (value > 0.0)
    offset = FRAC(value);
  else
    offset = 1.0 - FRAC(ABS(value));
  if (offset > 0.5)
    return (2.0 * (1.0 - offset));
  return (2.0 * offset);
}

/*
 * Copyright (c) 1992, 1993 Pedro Borges, Paulo Almeida, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
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
#include "config.h"
#include "textrace.h"

/**********************************************************************
 *    RAY TRACING - Version 8.1.0                                     *
 *                                                                    *
 *    MADE BY: Pedro Borges, Paulo Almeida, INESC-Norte, June 1992    *
 *    MODIFIED BY: Pedro Borges, Paulo Almeida, September 1992        *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, December 1992          *
 **********************************************************************/

#ifndef __STDC__
#define ERR_MAX 16
#endif

extern void     runtime_abort();

static void
vcpy(u, v)
  xyz_ptr         u, v;
{
  u->x = v->x;
  u->y = v->y;
  u->z = v->z;
}

static void
vadd(u, v, w)
  xyz_ptr         u, v, w;
{
  u->x = v->x + w->x;
  u->y = v->y + w->y;
  u->z = v->z + w->z;
}

static void
vsub(u, v, w)
  xyz_ptr         u, v, w;
{
  u->x = v->x - w->x;
  u->y = v->y - w->y;
  u->z = v->z - w->z;
}

static void
vmul(u, v, r)
  xyz_ptr         u, v;
  real            r;
{
  u->x = v->x * r;
  u->y = v->y * r;
  u->z = v->z * r;
}

static void
vmuladd(u, v, r)
  xyz_ptr         u, v;
  real            r;
{
  u->x += v->x * r;
  u->y += v->y * r;
  u->z += v->z * r;
}

static void
vtransf(u, ax, v)
  xyz_ptr         u;
  xyz_axes       *ax;
  xyz_ptr         v;
{
  vmul(u, &ax->x_v, v->x);
  vmuladd(u, &ax->y_v, v->y);
  vmuladd(u, &ax->z_v, v->z);
}

static void
vminmax(min, max, v)
  xyz_ptr         min, max, v;
{
  if (v->x < min->x)
    min->x = v->x;
  else if (v->x > max->x)
    max->x = v->x;
  if (v->y < min->y)
    min->y = v->y;
  else if (v->y > max->y)
    max->y = v->y;
  if (v->z < min->z)
    min->z = v->z;
  else if (v->z > max->z)
    max->z = v->z;
}

real
vpint(u, v)
  xyz_ptr         u, v;
{
  return u->x * v->x + u->y * v->y + u->z * v->z;
}

real
vmod(v)
  xyz_ptr         v;
{
  return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

int
vnormalize(v)
  xyz_ptr         v;
{
  real            r;

  r = vmod(v);
  if (r < TINY)
    return 0;
  vmul(v, v, 1.0 / r);
  return 1;
}

real
vinvmat(x, y, z, ox, oy, oz)
  xyz_ptr         x, y, z, ox, oy, oz;
{
  real            det, idet;

  ox->x = y->y * z->z - y->z * z->y;
  oy->x = y->z * z->x - y->x * z->z;
  oz->x = y->x * z->y - y->y * z->x;
  det = x->x * ox->x + x->y * oy->x + x->z * oz->x;
  if (fabs(det) < TINY)
    return 0.0;
  ox->y = x->z * z->y - x->y * z->z;
  oy->y = x->x * z->z - x->z * z->x;
  oz->y = x->y * z->x - x->x * z->y;
  ox->z = x->y * y->z - x->z * y->y;
  oy->z = x->z * y->x - x->x * y->z;
  oz->z = x->x * y->y - x->y * y->x;
  idet = 1 / det;
  vmul(ox, ox, idet);
  vmul(oy, oy, idet);
  vmul(oz, oz, idet);
  return det;
}

static int
err_line(fp)
  FILE           *fp;
{
  int             pos, nl, i;

  nl = 1;
  pos = ftell(fp);
  rewind(fp);
  for (i = 0; i < pos - 1; i++)
    if (getc(fp) == '\n')
      nl++;
  return nl;
}

static void
prt_arr(s)
  char           *s[];
{
  char            message[255], tmp[255];

  strcpy(message, "");
  while (*s)
  {
    sprintf(tmp, "%s", *s++);
    strcat(message, tmp);
  }
  runtime_abort(message);
}

static void
errorfn(s, fp, fn)
  char           *s[];
  FILE           *fp;
  char           *fn;
{
  WRITE(ERROR, "Text error in %s file at line %d\n", fn, err_line(fp));
  FLUSH(ERROR);
  prt_arr(s);
  exit(1);
}

static void
error(s)
  char           *s[];
{
  WRITE(ERROR, "Text error\n");
  FLUSH(ERROR);
  prt_arr(s);
  exit(1);
}

void           *
alloc(t)
  unsigned        t;
{
  void           *p;
#ifdef __STDC__
  char           *err[] =
  {"Memory allocation error ", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Memory allocation error ";
  err[1] = 0;
#endif

  p = (void *) nmalloc(t, PARSE_TYPE);
  if (!p)
    error(err);
  return p;
}

static FILE    *
cfopen(file, mode)
  char           *file, *mode;
{
  FILE           *fp;
#ifdef __STDC__
  char           *err[] =
  {"Open error in file ", 0, 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Open error in file ";
  err[1] = 0;
  err[2] = 0;
#endif

  fp = fopen(file, mode);
  if (!fp)
  {
    err[1] = file;
    error(err);
  }
  return fp;
}

/***** Global variables *****/
static file_ptr fp;
static xyz_axes *orient_ptr = 0;
static xyz_ptr  size_ptr = 0;
static real     space = NULSP;
static font    *fonts_ptr = 0, *font_ptr = 0;
static code    *code_ptr = 0;
static char    *codename = 0;
static char     buffer[BUFFERSIZE];
static char     ifname[] = "input";

static void
skip_space(fp)
  FILE           *fp;
{
  int             c;

  do
  {
    c = getc(fp);
  } while (isspace(c));
  ungetc(c, fp);
}

static void
expect(s, fp, fname)
  char           *s;
  FILE           *fp;
  char           *fname;
{
  char            b[100];
  int             i;
#ifdef __STDC__
  char           *err[] =
  {"Expected '", 0, "', found '", 0, "'", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Expected '";
  err[1] = 0;
  err[2] = "', found '";
  err[3] = 0;
  err[4] = "'";
  err[5] = 0;
#endif

  i = 0;
  skip_space(fp);
  while (s[i])
  {
    if ((b[i] = getc(fp)) != s[i])
    {
      b[++i] = 0;
      err[1] = s;
      err[3] = b;
      errorfn(err, fp, fname);
    }
    i++;
  }
}

static char    *
get_ident(fp, fname)
  FILE           *fp;
  char           *fname;
{
  char           *b;
  int             c, i;
#ifdef __STDC__
  char           *err[] =
  {"Character name too long", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Character name too long";
  err[1] = 0;
#endif

  b = buffer;
  skip_space(fp);
  i = 0;
  while (c = getc(fp), !isspace(c) && c != EOF && i < BUFFERSIZE - 1)
    b[i++] = c;
  if (i == BUFFERSIZE - 1)
    errorfn(err, fp, fname);
  b[i] = 0;
  return b;
}

static          real
get_real(fp, fname)
  FILE           *fp;
  char           *fname;
{
  real            r;
#ifdef __STDC__
  char           *err[] =
  {"Real number expected", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Real number expected";
  err[1] = 0;
#endif

  if (fscanf(fp, REALCONV, &r) < 1)
    errorfn(err, fp, fname);
  return r;
}

static int
get_int(fp, fname)
  FILE           *fp;
  char           *fname;
{
  int             i;
#ifdef __STDC__
  char           *err[] =
  {"Integer expected", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Integer expected";
  err[1] = 0;
#endif

  if (fscanf(fp, "%d", &i) < 1)
    errorfn(err, fp, fname);
  return i;
}

static void
get_vector(vec_ptr)
  xyz_ptr         vec_ptr;
{
  vec_ptr->x = get_real(fp, ifname);
  vec_ptr->y = get_real(fp, ifname);
  vec_ptr->z = get_real(fp, ifname);
}

static char    *
get_name_from_code(c)
  int             c;
{
  code           *cp;
  char            a[10];
#ifdef __STDC__
  char           *err[] =
  {"Character code ", 0, " missing in encoding file ", 0, 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Character code ";
  err[1] = 0;
  err[2] = " missing in encoding file ";
  err[3] = 0;
  err[4] = 0;
#endif

  cp = code_ptr;
  while (cp && cp->code != c)
    cp = cp->next;
  if (!cp)
  {
    err[3] = codename;
    sprintf(a, "%d", c);
    err[1] = a;
    error(err);
  }
  return cp->name;
}

static char    *
get_name_from_file()
{
  char           *b;
  int             c, i;
#ifdef __STDC__
  char           *err1[] =
  {"End of file unexpected", 0};
  char           *err2[] =
  {"Character name too long", 0};
#else
  char           *err1[ERR_MAX];
  char           *err2[ERR_MAX];

  err1[0] = "End of file unexpected";
  err1[1] = 0;
  err2[0] = "Character name too long";
  err2[1] = 0;
#endif

  b = buffer;
  i = 0;
  while (c = getc(fp), c != PSNAME && c != EOF && i < BUFFERSIZE - 1)
    b[i++] = c;
  if (c == EOF)
    errorfn(err1, fp, ifname);
  if (i == BUFFERSIZE - 1)
    errorfn(err2, fp, ifname);
  b[i] = 0;
  return b;
}

static void
get_char_desc(pdp, fp, fname)
  prismdsc       *pdp;
  FILE           *fp;
  char           *fname;
{
  int             ncc, nc, cc, c;
  ccurve         *cca;
  curve          *ca;
  char           *type;
#ifdef __STDC__
  char           *err[] =
  {"Unknown curve type in font file", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Unknown curve type in font file";
  err[1] = 0;
#endif

  ncc = pdp->n = get_int(fp, fname);
  cca = pdp->ccurve_arr = newa(ccurve, ncc);
  for (cc = 0; cc < ncc; cc++)
  {
    nc = cca[cc].n = get_int(fp, fname);
    ca = cca[cc].curve_arr = newa(curve, nc + 1);
    for (c = 0; c < nc; c++)
    {
      type = get_ident(fp, fname);
      if (!strcmp(type, "l"))
      {
	ca[c].t = 1;
	ca[c].p[0].x = get_real(fp, fname);
	ca[c].p[0].y = get_real(fp, fname);
      } else if (!strcmp(type, "c"))
      {
	ca[c].t = 3;
	ca[c].p[0].x = get_real(fp, fname);
	ca[c].p[0].y = get_real(fp, fname);
	ca[c].p[1].x = get_real(fp, fname);
	ca[c].p[1].y = get_real(fp, fname);
	ca[c].p[2].x = get_real(fp, fname);
	ca[c].p[2].y = get_real(fp, fname);
      } else
	errorfn(err, fp, fname);
    }
    ca[nc].p[0].x = ca[0].p[0].x;
    ca[nc].p[0].y = ca[0].p[0].y;
  }
}

static void
get_char_font(chp)
  character      *chp;
{
  font           *fntp;
  FILE           *fp;
  char           *name;
  int             c;
#ifdef __STDC__
  char           *err1[] =
  {"Unexpected character in font file", 0};
  char           *err2[] =
  {"Missing character description of '", 0,
   "' in font file ", 0, 0};
#else
  char           *err1[ERR_MAX];
  char           *err2[ERR_MAX];

  err1[0] = "Unexpected character in font file";
  err1[1] = 0;
  err2[0] = "Missing character description of '";
  err2[1] = 0;
  err2[2] = "' in font file ";
  err2[3] = 0;
  err2[4] = 0;
#endif

  fntp = font_ptr;
  err2[1] = chp->name;
  err2[3] = fntp->name;
  fp = cfopen(fntp->name, "r");
  while (1)
  {
    skip_space(fp);
    c = getc(fp);
    if (c == EOF)
      error(err2);
    if (c != PSNAME)
      errorfn(err1, fp, fntp->name);
    name = get_ident(fp, fntp->name);
    if (strcmp(name, chp->name))
    {
      while (c = getc(fp), c != PSNAME && c != EOF)
	;
      if (c == EOF)
	error(err2);
      ungetc(c, fp);
    } else
    {
      get_char_desc(chp->prismdsc_ptr, fp, fntp->name);
      fclose(fp);
      break;
    }
  }
}

static prismdsc *
get_prismdsc_ptr(name)
  char           *name;
{
  character      *chp;

  chp = font_ptr->ch_ptr;
  while (chp && strcmp(name, chp->name))
    chp = chp->next;
  if (!chp)
  {
    chp = new(character);
    chp->next = font_ptr->ch_ptr;
    font_ptr->ch_ptr = chp;
    chp->name = alloc(strlen(name) + 1);
    strcpy(chp->name, name);
    chp->prismdsc_ptr = new(prismdsc);
    get_char_font(chp);
  }
  return chp->prismdsc_ptr;
}

static void
enclose(pdp, p1, p2, p3, p4)
  prismdsc       *pdp;
  xy_ptr          p1, p2, p3, p4;
{
  real            minx, miny, maxx, maxy;
  real            a, b, c;
  real            delta, t;

  minx = MIN(p1->x, p4->x);
  maxx = MAX(p1->x, p4->x);
  miny = MIN(p1->y, p4->y);
  maxy = MAX(p1->y, p4->y);

  c = 3.0 * (p2->x - p1->x);
  b = 3.0 * (p3->x - p2->x) - c;
  a = p4->x - p1->x - b - c;

  if (fabs(a) > TINY)		/* else is a quadratic, not a cubic... */
  {
    delta = 4.0 * b * b - 12.0 * a * c;

    if (delta >= 0)
    {
      delta = sqrt(delta);
      t = (-2.0 * b + delta) / (6.0 * a);
      if ((t < 1.0) && (t > 0))
      {
	minx = MIN(minx, a * t * t * t + b * t * t + c * t + p1->x);
	maxx = MAX(maxx, a * t * t * t + b * t * t + c * t + p1->x);
      }
      t = (-2.0 * b - delta) / (6.0 * a);
      if ((t < 1) && (t > 0))
      {
	minx = MIN(minx, a * t * t * t + b * t * t + c * t + p1->x);
	maxx = MAX(maxx, a * t * t * t + b * t * t + c * t + p1->x);
      }
    }
  } else
  {
    if (fabs(b) > TINY)		/* else is a line.... */
    {
      t = (-c / (2.0 * b));
      if ((t < 1.0) && (t > 0.0))
      {
	minx = MIN(minx, b * t * t + c * t + p1->x);
	maxx = MAX(maxx, b * t * t + c * t + p1->x);
      }
    }
  }

  c = 3.0 * (p2->y - p1->y);
  b = 3.0 * (p3->y - p2->y) - c;
  a = p4->y - p1->y - b - c;

  if (fabs(a) > TINY)		/* else is a quadratic, not a cubic... */
  {
    delta = 4.0 * b * b - 12.0 * a * c;

    if (delta >= 0.0)
    {
      delta = sqrt(delta);
      t = (-2.0 * b + delta) / (6.0 * a);
      if ((t < 1.0) && (t > 0.0))
      {
	miny = MIN(miny, a * t * t * t + b * t * t + c * t + p1->y);
	maxy = MAX(maxy, a * t * t * t + b * t * t + c * t + p1->y);
      }
      t = (-2.0 * b - delta) / (6.0 * a);
      if ((t < 1.0) && (t > 0.0))
      {
	miny = MIN(miny, a * t * t * t + b * t * t + c * t + p1->y);
	maxy = MAX(maxy, a * t * t * t + b * t * t + c * t + p1->y);
      }
    }
  } else
  {
    if (fabs(b) > TINY)		/* else is a line.... */
    {
      t = (-c / (2.0 * b));
      if ((t < 1.0) && (t > 0.0))
      {
	miny = MIN(miny, b * t * t + c * t + p1->y);
	maxy = MAX(maxy, b * t * t + c * t + p1->y);
      }
    }
  }

  if (maxx > pdp->width)
    pdp->width = maxx;
  if (maxy > pdp->top)
    pdp->top = maxy;
  else if (miny < pdp->bot)
    pdp->bot = miny;
}

static void
calc_size(pdp)
  prismdsc       *pdp;
{
  real            x0, y0;
  int             ncc, nc, cc, c;
  ccurve         *cca;
  curve          *ca;

  ncc = pdp->n;
  cca = pdp->ccurve_arr;

  pdp->top = pdp->bot = cca[0].curve_arr[0].p[0].y;
  pdp->width = cca[0].curve_arr[0].p[0].x;

  for (cc = 0; cc < ncc; cc++)
  {
    nc = cca[cc].n;
    ca = cca[cc].curve_arr;
    for (c = 0; c < nc; c++)
      if (ca[c].t == 1)
      {
	x0 = ca[c].p[0].x;
	y0 = ca[c].p[0].y;
	if (x0 > pdp->width)
	  pdp->width = x0;
	if (y0 > pdp->top)
	  pdp->top = y0;
	else if (y0 < pdp->bot)
	  pdp->bot = y0;
      } else
	enclose(pdp, &ca[c].p[0], &ca[c].p[1], &ca[c].p[2], &ca[c + 1].p[0]);
  }
}

static void
calc_englob(pp, minp, maxp)
  prism          *pp;
  xyz_ptr         minp, maxp;
{
  prismdsc       *pdp;
  xyz_struct      v[2], u, w;
  int             i, j, k;

  pdp = pp->prismdsc_ptr;
  calc_size(pdp);
  v[0].x = 0.0;
  v[1].x = pdp->width * pp->size_ptr->x;
  v[0].y = pdp->bot * pp->size_ptr->y;
  v[1].y = pdp->top * pp->size_ptr->y;
  v[0].z = 0.0;
  v[1].z = pp->size_ptr->z;
  for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++)
      for (k = 0; k < 2; k++)
      {
	u.x = v[i].x;
	u.y = v[j].y;
	u.z = v[k].z;
	vtransf(&w, pp->orient_ptr, &u);
	if (!i && !j && !k)
	{
	  vcpy(minp, &w);
	  vcpy(maxp, &w);
	} else
	  vminmax(minp, maxp, &w);
      }
  vadd(minp, minp, &pp->position);
  vadd(maxp, maxp, &pp->position);
}

static void
update_position(posp, width)
  xyz_ptr         posp;
  real            width;
{
  real            size;

  size = size_ptr->x;
  size *= width + space;
  vmuladd(posp, &orient_ptr->x_v, size);
}

static void
get_at_text()
{
  prism          *prism_ptr;
  void          **priv_p_p;
  xyz_struct      position;
  xyz_ptr         minp, maxp;
  char           *name;
  int             c;
  void_ptr 	 *create_pp_obj();

  get_vector(&position);
  expect("\"", fp, ifname);
  while (c = getc(fp), c != '\"')
  {
    if (c == ' ')
    {
      update_position(&position, 0.5);
      continue;
    }
    prism_ptr = new(prism);
    priv_p_p = (void **) create_pp_obj(&minp, &maxp);
    *priv_p_p = (void *) prism_ptr;
    if (c != PSNAME)
      name = get_name_from_code(c);
    else
      name = get_name_from_file();
    prism_ptr->prismdsc_ptr = get_prismdsc_ptr(name);
    prism_ptr->position = position;
    prism_ptr->orient_ptr = orient_ptr;
    prism_ptr->size_ptr = size_ptr;
    calc_englob(prism_ptr, minp, maxp);
    update_position(&position, prism_ptr->prismdsc_ptr->width);
  }
}

static void
add_font(name)
  char           *name;
{
  font           *fp;

  fp = fonts_ptr;
  while (fp && fp->name != name)
    fp = fp->next;
  if (!fp)
  {
    fp = new(font);
    fp->next = fonts_ptr;
    fonts_ptr = fp;
    fp->ch_ptr = 0;
    fp->name = alloc(strlen(name) + 1);
    strcpy(fp->name, name);
  }
  font_ptr = fp;
}

static void
get_encoding_file(fname)
  char           *fname;
{
  FILE           *fp;
  char           *cname;
  code           *cp;
  int             i;
#ifdef __STDC__
  char           *err1[] =
  {"Encoding file read error", 0};
  char           *err2[] =
  {"Unexpected character found in encoding file", 0};
#else
  char           *err1[ERR_MAX];
  char           *err2[ERR_MAX];

  err1[0] = "Encoding file read error";
  err1[1] = 0;
  err2[0] = "Unexpected character found in encoding file";
  err2[1] = 0;
#endif

  fp = cfopen(fname, "r");
  codename = alloc(strlen(fname) + 1);
  strcpy(codename, fname);
  while (fscanf(fp, "%d", &i) > 0)
  {
    expect("/", fp, fname);
    cname = fgets(buffer, BUFFERSIZE, fp);
    if (!cname)
      errorfn(err1, fp, codename);
    cp = new(code);
    cp->next = code_ptr;
    code_ptr = cp;
    cp->code = i;
    i = strlen(cname);
    cname[i - 1] = 0;
    cp->name = alloc(i);
    strcpy(cp->name, cname);
  }
  if (getc(fp) != EOF)
    errorfn(err2, fp, codename);
}

static void
change_encoding_vector(name)
  char           *name;
{
  code           *cp;

  if (codename && !strcmp(codename, name))
    return;
  if (codename)
  {
    nfree(codename);
    while (code_ptr)
    {
      cp = code_ptr;
      nfree(cp->name);
      code_ptr = code_ptr->next;
      nfree((char *) cp);
    }
  }
  get_encoding_file(name);
}

static char    *
get_com()
{
  int             nl, c;
  char           *p, *b;

  nl = 0;
  while (c = getc(fp), isspace(c))
    if (c == '\n')
      if (nl)
	return (char *) 0;
      else
	++nl;
  if (c == EOF)
    return (char *) 0;
  ungetc(c, fp);
  p = b = get_ident(fp, ifname);
  while (*p)
    *p = toupper(*p), p++;
  return b;
}

void
get_orient()
{
  xyz_axes       *matp;
#ifdef __STDC__
  char           *err[] =
  {"Null vector", 0};
#else
  char           *err[ERR_MAX];

  err[0] = "Null vector";
  err[1] = 0;
#endif

  orient_ptr = newa(xyz_axes, 2);
  get_vector(&orient_ptr->x_v);
  if (!vnormalize(&orient_ptr->x_v))
    errorfn(err, fp, ifname);
  get_vector(&orient_ptr->y_v);
  if (!vnormalize(&orient_ptr->y_v))
    errorfn(err, fp, ifname);
  get_vector(&orient_ptr->z_v);
  if (!vnormalize(&orient_ptr->z_v))
    errorfn(err, fp, ifname);
  matp = orient_ptr + 1;
  if (!vinvmat(&orient_ptr->x_v, &orient_ptr->y_v, &orient_ptr->z_v,
	       &matp->x_v, &matp->y_v, &matp->z_v))
    err[0] = "Linear dependent vectors", errorfn(err, fp, ifname);
}

void
get_pp_obj(fptr)
  file_ptr        fptr;
{
  char           *cp, *com;
#ifdef __STDC__
  char           *err1[] =
  {"Undefined ", 0, 0};
  char           *err2[] =
  {"Invalid command", 0};
#else
  char           *err1[ERR_MAX];
  char           *err2[ERR_MAX];

  err1[0] = "Undefined ";
  err1[1] = 0;
  err1[2] = 0;
  err2[0] = "Invalid command";
  err2[1] = 0;
#endif

  fp = fptr;
  while (com = get_com())
  {
    if (!strcmp(com, "ORIENTATION"))
    {
      get_orient();
    } else if (!strcmp(com, "SCALE"))
    {
      size_ptr = new(xyz_struct);
      get_vector(size_ptr);
    } else if (!strcmp(com, "SPACING"))
    {
      space = get_real(fp, ifname);
    } else if (!strcmp(com, "FONT"))
    {
      cp = get_ident(fp, ifname);
      add_font(cp);
    } else if (!strcmp(com, "ENCODING"))
    {
      cp = get_ident(fp, ifname);
      change_encoding_vector(cp);
    } else if (!strcmp(com, "AT"))
    {
      if (!orient_ptr)
	err1[1] = "ORIENTATION";
      else if (!size_ptr)
	err1[1] = "SCALE";
      else if (space == NULSP)
	err1[1] = "SPACING";
      else if (!font_ptr)
	err1[1] = "FONT";
      else if (!code_ptr)
	err1[1] = "ENCODING";
      if (err1[1])
	errorfn(err1, fp, ifname);
      get_at_text();
    } else
      errorfn(err2, fp, ifname);
  }
}

static void
free_tables()
{
  font           *fp, *fp1;
  character      *chp, *chp1;

  fp = fonts_ptr;
  while (fp)
  {
    nfree(fp->name);
    chp = fp->ch_ptr;
    while (chp)
    {
      nfree(chp->name);
      chp1 = chp;
      chp = chp->next;
      nfree((char *) chp1);
    }
    fp1 = fp;
    fp = fp->next;
    nfree((char *) fp1);
  }
}

void
end_pp_get()
{
  free_tables();
  orient_ptr = 0;
  size_ptr = 0;
  space = NULSP;
  fonts_ptr = 0;
  font_ptr = 0;
  code_ptr = 0;
  codename = 0;
}

static int
intersect_line(arr, pos, dir, p0, p1, lim)
  real          **arr;
  xy_ptr          pos, dir, p0, p1;
  real            lim;
{
  real            a, b, c, d, e, f, t, det;
  static real     dist[2];

  a = dir->x;
  c = dir->y;
  b = p0->x - p1->x;
  d = p0->y - p1->y;
  det = a * d - b * c;
  if (fabs(det) < TINY)
    return 0;
  e = p0->x - pos->x;
  f = p0->y - pos->y;
  t = (a * f - c * e) / det;
  if (t < 0.0 || t >= 1.0)
    return 0;
  dist[0] = (e * d - f * b) / det;
  dist[1] = t;
  *arr = dist;
  return 1;
}

static int
intersect_cubic(arr, pos, dir, p0, p1, p2, p3, lim)
  real          **arr;
  xy_ptr          pos, dir, p0, p1, p2, p3;
  real            lim;
{
  static real     x[6];
  real            ax, ay, bx, by, cx, cy;
  real            a, b, c, d, p, q, delta;
  real            t1, t2, t3;
  real            rx, ry, vx, vy;
  real            sqrd, m, tt;
  int             numint, i;

  rx = pos->x;
  ry = pos->y;
  vx = dir->x;
  vy = dir->y;
  cx = 3.0 * (p1->x - p0->x);
  cy = 3.0 * (p1->y - p0->y);
  bx = 3.0 * (p2->x - p1->x) - cx;
  by = 3.0 * (p2->y - p1->y) - cy;
  ax = p3->x - p0->x - cx - bx;
  ay = p3->y - p0->y - cy - by;
  a = (vy * ax - vx * ay) * 3.0;
  b = vy * bx - vx * by;
  c = vy * cx - vx * cy;
  d = vy * (p0->x - rx) - vx * (p0->y - ry);
  if (fabs(a) < TINY)
    if (fabs(b) < TINY)
      if (fabs(c) < TINY)
	numint = 0;
      else
      {
	x[1] = t1 = -d / c;
	if (t1 >= 0.0 && t1 < 1.0)
	  numint = 1;
	else
	  numint = 0;
      }
    else
    {
      real            b1;

      b1 = c / 2.0 / b;
      delta = b1 * b1 - d / b;
      if (delta > 0.0)
      {
	numint = 0;
	for (i = -1; i < 2; i += 2)
	{
	  t1 = -b1 + i * sqrt(delta);
	  if (t1 >= 0.0 && t1 < 1.0)
	  {
	    x[2 * numint + 1] = t1;
	    ++numint;
	  }
	}
      } else
	numint = 0;
    }
  else
  {
    b /= a;
    c /= a;
    d /= a;
    if (b > 0.0 && c > 0.0 && d > 0.0)
      return 0.0;
    if ((d > 0) == (1.0 + 3 * (b + c + d) > 0))
      if ((c > 0.0) != (1 + 2 * b + c > 0.0))
      {
	if ((d > 0.0) == (c > 0.0))
	  return 0.0;
      } else if (b < -1.0 || b > 0.0)
	return 0.0;
    p = 3 * (c - b * b);
    q = b * (2 * b * b - 3 * c) + 3 * d;
    delta = q * q / 4 + p * p * p / 27;
    if (delta >= TINY)
    {
      real            b1, b2;

      sqrd = sqrt(delta);
      b1 = -q / 2.0 + sqrd;
      b2 = -q / 2.0 - sqrd;
      t1 = (b1 > 0.0 ? 1.0 : -1.0) * pow(fabs(b1), (double) 1.0 / 3.0) +
	(b2 > 0.0 ? 1.0 : -1.0) * pow(fabs(b2), (double) 1.0 / 3.0) - b;
      if (t1 >= 0.0 && t1 < 1.0)
      {
	x[1] = t1;
	numint = 1;
      } else
	numint = 0;
    } else
    {
      real            arg;

      m = sqrt(fabs(-p) / 3.0);
      arg = -q / (2 * m * m * m);
      if (arg < -1.0)
	arg = -1.0;
      else if (arg > 1.0)
	arg = 1.0;
      tt = acos(arg);
      numint = 0;
      for (i = 0; i < 3; i++)
      {
	t1 = 2 * m * cos((tt + 2 * M_PI * i) / 3.0) - b;
	if (t1 >= 0.0 && t1 < 1.0)
	{
	  x[2 * numint + 1] = t1;
	  ++numint;
	}
      }
    }
  }
  for (i = 0; i < numint; i++)
  {
    t1 = x[2 * i + 1];
    t2 = t1 * t1;
    t3 = t2 * t1;
    x[2 * i] = fabs(vx) > fabs(vy) ?
      (ax * t3 + bx * t2 + cx * t1 + p0->x - rx) / vx :
      (ay * t3 + by * t2 + cy * t1 + p0->y - ry) / vy;
  }
  *arr = x;
  return numint;
}

real
intersect_pp_obj(point_ptr, dir_ptr, bmin_ptr, bmax_ptr, desc_ptr)
  xyz_ptr         point_ptr, dir_ptr, bmin_ptr, bmax_ptr;
  void_ptr        desc_ptr;
{
  xyz_struct      point, pos, dir;
  prism          *pp;
  prismdsc       *pdp;
  int             ncc, nc, cc, c;
  ccurve         *cca;
  curve          *ca;
  real            dist, inf, sup;
  real            tintm0;
  int             nintlt0, nintltinf, nintwingt0, icur;
  int             ccim0, cim0;
  int             top_down, i;
  real           *arr;
  real            tmp;

  nintlt0 = nintltinf = nintwingt0 = 0;
  pp = (prism *) desc_ptr;
  pdp = pp->prismdsc_ptr;
  vsub(&point, point_ptr, &pp->position);
  vtransf(&pos, pp->orient_ptr + 1, &point);
  vtransf(&dir, pp->orient_ptr + 1, dir_ptr);
  pos.x /= pp->size_ptr->x;
  pos.y /= pp->size_ptr->y;
  dir.x /= pp->size_ptr->x;
  dir.y /= pp->size_ptr->y;
  if (fabs(dir.z) < TINY)
  {
    if (pos.z <= 0.0 || pos.z >= pp->size_ptr->z)
      return 0.0;
    else
      inf = -BIG, sup = BIG;
  } else
  {
    inf = -pos.z / dir.z;
    sup = (pp->size_ptr->z - pos.z) / dir.z;
    if (inf > sup)
      tmp = inf, inf = sup, sup = tmp;
  }
  ncc = pdp->n;
  cca = pdp->ccurve_arr;
  for (cc = 0; cc < ncc; cc++)
  {
    nc = cca[cc].n;
    ca = cca[cc].curve_arr;
    for (c = 0; c < nc; c++)
    {
      if (ca[c].t == 1)
	icur = intersect_line(&arr, (xy_ptr) & pos, (xy_ptr) & dir,
			      &ca[c].p[0], &ca[c + 1].p[0], sup);
      else
	icur = intersect_cubic(&arr, (xy_ptr) & pos, (xy_ptr) & dir,
			       &ca[c].p[0], &ca[c].p[1], &ca[c].p[2],
			       &ca[c + 1].p[0], sup);
      for (i = 0; i < icur; i++)
      {
	dist = arr[2 * i];
	if (dist < 0.0)
	  ++nintlt0;
	if (dist < inf)
	  ++nintltinf;
	else if (dist > 0.0 && dist < sup)
	{
	  ++nintwingt0;
	  sup = dist;
	  tintm0 = arr[2 * i + 1];
	  ccim0 = cc;
	  cim0 = c;
	}
      }
    }
  }
  if (sup < 0.0)
    return 0.0;
  if (!(nintlt0 % 2) || pos.z < 0.0 || pos.z > pp->size_ptr->z)
  {
    pp->enter = 1;
    if (inf > 0.0)
    {
      if (nintltinf % 2)
	top_down = 1, dist = inf;
      else if (nintwingt0)
	top_down = 0, dist = sup;
      else
	return 0.0;
    } else
    {
      if (nintwingt0)
	top_down = 0, dist = sup;
      else
	return 0.0;
    }
  } else
  {
    pp->enter = 0;
    dist = sup;
    if (nintwingt0)
      top_down = 0;
    else
      top_down = 1;
  }
  if (top_down)
    pp->ccint = -1;
  else
  {
    pp->ccint = ccim0;
    pp->cint = cim0;
    pp->tint = tintm0;
  }
  pp->dirint = dir;
  return dist;
}

void
normal_pp_obj(pos_ptr, desc_ptr, normal_ptr)
  xyz_ptr         pos_ptr;
  void_ptr        desc_ptr;
  xyz_ptr         normal_ptr;
{
  real            x0, y0, x1, y1, x2, y2, x3, y3;
  real            ax, ay, bx, by, cx, cy;
  real            t, xl, yl;
  prism          *pp;
  prismdsc       *pdp;
  int             cc, c;
  ccurve         *cca;
  curve          *ca;
  xyz_struct      n;


  pp = (prism *) desc_ptr;
  pdp = pp->prismdsc_ptr;
  cc = pp->ccint;
  c = pp->cint;
  cca = pdp->ccurve_arr;
  ca = cca[cc].curve_arr;
  if (pp->ccint < 0)
  {
    n.x = n.y = 0.0;
    n.z = 1.0;
  } else if (ca[c].t == 1)
  {
    n.x = ca[c + 1].p[0].y - ca[c].p[0].y;
    n.y = ca[c].p[0].x - ca[c + 1].p[0].x;
    n.z = 0.0;
    n.x /= pp->size_ptr->x;
    n.y /= pp->size_ptr->y;
  } else
  {
    x0 = ca[c].p[0].x;
    y0 = ca[c].p[0].y;
    x1 = ca[c].p[1].x;
    y1 = ca[c].p[1].y;
    x2 = ca[c].p[2].x;
    y2 = ca[c].p[2].y;
    x3 = ca[c + 1].p[0].x;
    y3 = ca[c + 1].p[0].y;
    cx = 3 * (x1 - x0);
    cy = 3 * (y1 - y0);
    bx = 3 * (x2 - x1) - cx;
    by = 3 * (y2 - y1) - cy;
    ax = x3 - x0 - cx - bx;
    ay = y3 - y0 - cy - by;
    t = pp->tint;
    xl = 3 * ax * t * t + 2 * bx * t + cx;
    yl = 3 * ay * t * t + 2 * by * t + cy;
    n.x = yl;
    n.y = -xl;
    n.z = 0.0;
    n.x /= pp->size_ptr->x;
    n.y /= pp->size_ptr->y;
  }
  if ((vpint(&pp->dirint, &n) > 0.0) ^ !pp->enter)
    n.x = -n.x, n.y = -n.y, n.z = -n.z;
  vtransf(normal_ptr, pp->orient_ptr, &n);
}

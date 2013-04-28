/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
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
#include "pp_ext.h"

/**********************************************************************
 *    RAY TRACING - Scene (Objects) - Version 8.3.4                   *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1993          *
 **********************************************************************/

#ifdef _Windows
#pragma option -zEstrings -zFstringc -zHstringg
#define FAR far
#else
#define FAR
#endif
static char FAR str1[] = "OBJECT (SPHERE) SURFACE ID";
static char FAR str2[] = "OBJECT (SPHERE) REFRACTION Factor";
static char FAR str3[] = "OBJECT (SPHERE) CENTER X";
static char FAR str4[] = "OBJECT (SPHERE) CENTER Y";
static char FAR str5[] = "OBJECT (SPHERE) CENTER Z";
static char FAR str6[] = "OBJECT (SPHERE) Radius";
static char FAR str7[] = "OBJECT (BOX) SURFACE ID";
static char FAR str8[] = "OBJECT (BOX) REFRACTION Factor";
static char FAR str9[] = "OBJECT (BOX) CENTER X";
static char FAR str10[] = "OBJECT (BOX) CENTER Y";
static char FAR str11[] = "OBJECT (BOX) CENTER Z";
static char FAR str12[] = "OBJECT (BOX) SIZE X";
static char FAR str13[] = "OBJECT (BOX) SIZE Y";
static char FAR str14[] = "OBJECT (BOX) SIZE Z";
static char FAR str15[] = "OBJECT (PATCH) SURFACE ID";
static char FAR str16[] = "OBJECT (PATCH) REFRACTION Factor";
static char FAR str17[] = "OBJECT (PATCH) ORIGIN X";
static char FAR str18[] = "OBJECT (PATCH) ORIGIN Y";
static char FAR str19[] = "OBJECT (PATCH) ORIGIN Z";
static char FAR str20[] = "OBJECT (PATCH) SCALE X";
static char FAR str21[] = "OBJECT (PATCH) SCALE Y";
static char FAR str22[] = "OBJECT (PATCH) SCALE Z";
static char FAR str23[] = "Error: unable to open PATCH (%s)\n";
static char FAR str24[] = "OBJECT (PATCH) VERTEX ID";
static char FAR str25[] = "too many OBJECTS";
static char FAR str26[] = "no OBJECT (PATCH) VERTEX Coordinates";
static char FAR str27[] = "too many OBJECT (PATCH) VERTEX Coordinates";
static char FAR str28[] = "OBJECT (PATCH) VERTEX X";
static char FAR str29[] = "OBJECT (PATCH) VERTEX Y";
static char FAR str30[] = "OBJECT (PATCH) VERTEX Z";
static char FAR str31[] = "not enough OBJECT (PATCH) VERTEX Coordinates";
static char FAR str32[] = "OBJECT (CONE) SURFACE ID";
static char FAR str33[] = "OBJECT (CONE) REFRACTION Factor";
static char FAR str34[] = "OBJECT (CONE) BASE X";
static char FAR str35[] = "OBJECT (CONE) BASE Y";
static char FAR str36[] = "OBJECT (CONE) BASE Z";
static char FAR str37[] = "OBJECT (CONE) BASE Radius";
static char FAR str38[] = "OBJECT (CONE) APEX X";
static char FAR str39[] = "OBJECT (CONE) APEX Y";
static char FAR str40[] = "OBJECT (CONE) APEX Z";
static char FAR str41[] = "OBJECT (CONE) APEX Radius";
static char FAR str42[] = "bad OBJECT (CONE) APEX Radius";
static char FAR str43[] = "bad OBJECT (CONE) APEX and BASE";
static char FAR str44[] = "OBJECT (POLYGON) SURFACE ID";
static char FAR str45[] = "OBJECT (POLYGON) REFRACTION Factor";
static char FAR str46[] = "OBJECT (POLYGON) ORIGIN X";
static char FAR str47[] = "OBJECT (POLYGON) ORIGIN Y";
static char FAR str48[] = "OBJECT (POLYGON) ORIGIN Z";
static char FAR str49[] = "OBJECT (POLYGON) SCALE X";
static char FAR str50[] = "OBJECT (POLYGON) SCALE Y";
static char FAR str51[] = "OBJECT (POLYGON) SCALE Z";
static char FAR str52[] = "Error: unable to open POLYGON (%s)\n";
static char FAR str53[] = "OBJECT (POLYGON) VERTICES Polygon";
static char FAR str54[] = "OBJECT (POLYGON) VERTEX ID";
static char FAR str55[] = "too many OBJECTS";
static char FAR str56[] = "no OBJECT (POLYGON) VERTEX Coordinates";
static char FAR str57[] = "too many OBJECT (POLYGON) VERTEX Coordinates";
static char FAR str58[] = "OBJECT (POLYGON) VERTEX X";
static char FAR str59[] = "OBJECT (POLYGON) VERTEX Y";
static char FAR str60[] = "OBJECT (POLYGON) VERTEX Z";
static char FAR str61[] = "not enough OBJECT (POLYGON) VERTEX Coordinates";
static char FAR str62[] = "OBJECT (TRIANGLE) SURFACE ID";
static char FAR str63[] = "OBJECT (TRIANGLE) REFRACTION Factor";
static char FAR str64[] = "OBJECT (TRIANGLE) ORIGIN X";
static char FAR str65[] = "OBJECT (TRIANGLE) ORIGIN Y";
static char FAR str66[] = "OBJECT (TRIANGLE) ORIGIN Z";
static char FAR str67[] = "OBJECT (TRIANGLE) SCALE X";
static char FAR str68[] = "OBJECT (TRIANGLE) SCALE Y";
static char FAR str69[] = "OBJECT (TRIANGLE) SCALE Z";
static char FAR str70[] = "Error: unable to open TRIANGLE (%s)\n";
static char FAR str71[] = "OBJECT (TRIANGLE) VERTEX Coord X";
static char FAR str72[] = "OBJECT (TRIANGLE) VERTEX Coord Y";
static char FAR str73[] = "OBJECT (TRIANGLE) VERTEX Coord Z";
static char FAR str74[] = "OBJECT (TRIANGLE) VERTEX Normal X";
static char FAR str75[] = "OBJECT (TRIANGLE) VERTEX Normal Y";
static char FAR str76[] = "OBJECT (TRIANGLE) VERTEX Normal Z";
static char FAR str77[] = "no OBJECT (TRIANGLE) VERTEX Normal";
static char FAR str78[] = "OBJECT (TRIANGLE) VERTEX 1 SURFACE ID";
static char FAR str79[] = "OBJECT (TRIANGLE) VERTEX 2 SURFACE ID";
static char FAR str80[] = "OBJECT (TRIANGLE) VERTEX 3 SURFACE ID";
static char FAR str81[] = "too many OBJECTS";
static char FAR str82[] = "OBJECT (TEXT) SURFACE ID";
static char FAR str83[] = "OBJECT (TEXT) REFRACTION Factor";
static char FAR str84[] = "Error: unable to open TEXT (%s)\n";
static char FAR str85[] = "OBJECT (CSG) SURFACE ID";
static char FAR str86[] = "OBJECT (CSG) REFRACTION Factor";
static char FAR str87[] = "OBJECT (CSG) OPERATION";
static char FAR str88[] = "OBJECT (LIST) SURFACE ID";
static char FAR str89[] = "OBJECT (LIST) REFRACTION Factor";
static char FAR str90[] = "OBJECT ID";
static char FAR str91[] = "TRANSFORM X0";
static char FAR str92[] = "TRANSFORM Y0";
static char FAR str93[] = "TRANSFORM Z0";
static char FAR str94[] = "TRANSFORM W0";
static char FAR str95[] = "TRANSFORM X1";
static char FAR str96[] = "TRANSFORM Y1";
static char FAR str97[] = "TRANSFORM Z1";
static char FAR str98[] = "TRANSFORM W1";
static char FAR str99[] = "TRANSFORM X2";
static char FAR str100[] = "TRANSFORM Y2";
static char FAR str101[] = "TRANSFORM Z2";
static char FAR str102[] = "TRANSFORM W2";
static char FAR str103[] = "TRANSFORM X3";
static char FAR str104[] = "TRANSFORM Y3";
static char FAR str105[] = "TRANSFORM Z3";
static char FAR str106[] = "TRANSFORM W3";
#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

#define CREATE_OBJECT(type, object_data)\
do {\
  ALLOCATE(object[objects], object_struct, 1, PARSE_TYPE);\
  object[objects]->id = objects;\
  object[objects]->surface_id = surface_id;\
  object[objects]->refraction = refraction;\
  ALLOCATE(object[objects]->min, xyz_struct, 1, PARSE_TYPE);\
  ALLOCATE(object[objects]->max, xyz_struct, 1, PARSE_TYPE);\
  object[objects]->transf = NULL;\
  object[objects]->inv_transf = NULL;\
  object[objects]->texture = NULL;\
  object[objects]->texture_modify_normal = FALSE;\
  object[objects]->object_type = type;\
  object[objects]->data = (void_ptr) object_data;\
} while (0)

static boolean
polygon_backface_check(polygon)
  polygon_ptr     polygon;
{
  xyz_struct      p, p0, normal;

  REALINC(backface_tests);
  p.x = polygon->coords[1][0] - polygon->coords[0][0];
  p.y = polygon->coords[1][1] - polygon->coords[0][1];
  p.z = polygon->coords[1][2] - polygon->coords[0][2];
  p0.x = polygon->coords[2][0] - polygon->coords[0][0];
  p0.y = polygon->coords[2][1] - polygon->coords[0][1];
  p0.z = polygon->coords[2][2] - polygon->coords[0][2];
  CROSS_PRODUCT(normal, p, p0);
  if (DOT_PRODUCT(gaze, normal) < 0.0)
    return FALSE;
  REALINC(backface_hits);
  return TRUE;
}
static boolean
triangle_backface_check(triangle)
  triangle_ptr    triangle;
{
  REALINC(backface_tests);
  if (DOT_PRODUCT(gaze, triangle->normal[0]) < 0.0)
    return FALSE;
  if (DOT_PRODUCT(gaze, triangle->normal[1]) < 0.0)
    return FALSE;
  if (DOT_PRODUCT(gaze, triangle->normal[2]) < 0.0)
    return FALSE;
  REALINC(backface_hits);
  return TRUE;
}
void
get_sphere()
{
  int             surface_id;
  real            value, refraction, radius;
  xyz_struct      center;
  sphere_ptr      sphere;

  get_valid(scene, &value, 1.0, (real) surfaces, str1);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str2);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str3);
  center.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str4);
  center.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str5);
  center.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, str6);
  radius = value;
  ADVANCE(scene);
  /* Create Sphere */
  ALLOCATE(sphere, sphere_struct, 1, PARSE_TYPE);
  STRUCT_ASSIGN(sphere->center, center);
  sphere->radius = radius;
  sphere->radius2 = SQR(radius);
  CREATE_OBJECT(SPHERE_TYPE, sphere);
}
void
get_box()
{
  int             surface_id;
  real            value, refraction;
  xyz_struct      center, size;
  box_ptr         box;

  get_valid(scene, &value, 1.0, (real) surfaces, str7);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str8);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str9);
  center.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str10);
  center.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str11);
  center.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, str12);
  size.x = value;
  get_valid(scene, &value, ROUNDOFF, Y_MAX * 0.5, str13);
  size.y = value;
  get_valid(scene, &value, ROUNDOFF, Z_MAX * 0.5, str14);
  size.z = value;
  ADVANCE(scene);
  /* Create Box */
  ALLOCATE(box, box_struct, 1, PARSE_TYPE);
  STRUCT_ASSIGN(box->center, center);
  STRUCT_ASSIGN(box->size, size);
  CREATE_OBJECT(BOX_TYPE, box);
}
void
get_patch()
{
  int             surface_id, i, vertices;
  real            value, refraction;
  xyz_struct      origin, scale;
  patch_ptr       patch;
  vertex_ptr      vertex_top, vertex_bottom, vertex;
  char            name[STRING_MAX];
  file_ptr        patch_file;

  get_valid(scene, &value, 1.0, (real) surfaces, str15);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str16);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str17);
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str18);
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str19);
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str20);
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str21);
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str22);
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    patch_file = scene;
  else
  {
    OPEN(patch_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, str23, name);
      FLUSH(results);
      HALT;
    }
  }
  vertices = 0;
  vertex_top = NULL;
  while (NOT END_OF_LINE(patch_file))
  {
    ALLOCATE(patch, patch_struct, 1, PARSE_TYPE);
    for (i = 0; i < 12; POSINC(i))
    {
      get_valid(patch_file, &value, 1.0, INFINITY, str24);
      patch->p[i] = vertex_pointer(ROUND(value), &vertices,
                                   &vertex_top, &vertex_bottom);
    }
    ADVANCE(patch_file);
    if (objects >= objects_max)
      runtime_abort(str25);
    /* Create Patch */
    CREATE_OBJECT(PATCH_TYPE, patch);
    POSINC(objects);
  }
  ADVANCE(patch_file);
  POSDEC(objects);
  patch->vertex = vertex_top;
  vertex = vertex_top;
  i = 0;
  while (NOT END_OF_LINE(patch_file))
  {
    if (vertex == NULL)
      runtime_abort(str26);
    POSINC(i);
    if (i > vertices)
      runtime_abort(str27);
    get_valid(patch_file, &value, X_MIN, X_MAX, str28);
    vertex->coords.x = value * scale.x + origin.x;
    get_valid(patch_file, &value, Y_MIN, Y_MAX, str29);
    vertex->coords.y = value * scale.y + origin.y;
    get_valid(patch_file, &value, Z_MIN, Z_MAX, str30);
    vertex->coords.z = value * scale.z + origin.z;
    vertex = (vertex_ptr) (vertex->next);
    ADVANCE(patch_file);
  }
  if (i < vertices)
    runtime_abort(str31);
  if (patch_file != scene)
    CLOSE(patch_file);
  else
    ADVANCE(scene);
}
void
get_cone()
{
  int             surface_id;
  real            value, refraction, base_radius, apex_radius, k;
  xyz_struct      base, apex, temp;
  cone_ptr        cone;

  get_valid(scene, &value, 1.0, (real) surfaces, str32);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str33);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str34);
  base.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str35);
  base.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str36);
  base.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, str37);
  base_radius = value;
  get_valid(scene, &value, X_MIN, X_MAX, str38);
  apex.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str39);
  apex.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str40);
  apex.z = value;
  get_valid(scene, &value, ROUNDOFF, X_MAX * 0.5, str41);
  apex_radius = value;
  if (apex_radius > base_radius)
    runtime_abort(str42);
  ADVANCE(scene);
  /* Create Cone / Cylinder */
  ALLOCATE(cone, cone_struct, 1, PARSE_TYPE);
  STRUCT_ASSIGN(cone->base, base);
  STRUCT_ASSIGN(cone->apex, apex);
  cone->base_radius = base_radius;
  cone->apex_radius = apex_radius;
  cone->w.x = apex.x - base.x;
  cone->w.y = apex.y - base.y;
  cone->w.z = apex.z - base.z;
  cone->height = LENGTH(cone->w);
  if (cone->height < ROUNDOFF)
    runtime_abort(str43);
  NORMALIZE(cone->w);
  cone->slope = (apex_radius - base_radius) / cone->height;
  cone->base_d = -DOT_PRODUCT(base, cone->w);
  if (ABS(cone->w.z) > 1.0 - ROUNDOFF)
  {
    temp.x = 0.0;
    temp.y = 1.0;
    temp.z = 0.0;
  } else
  {
    temp.x = 0.0;
    temp.y = 0.0;
    temp.z = 1.0;
  }
  CROSS_PRODUCT(cone->u, cone->w, temp);
  NORMALIZE(cone->u);
  CROSS_PRODUCT(cone->v, cone->u, cone->w);
  cone->min_d = DOT_PRODUCT(cone->w, base);
  cone->max_d = DOT_PRODUCT(cone->w, apex);
  if (cone->max_d < cone->min_d)
  {
    k = cone->max_d;
    cone->max_d = cone->min_d;
    cone->min_d = k;
  }
  CREATE_OBJECT(CONE_TYPE, cone);
}
typedef
struct
{
  vertex_ptr     *point;
  void_ptr        next;
} data_struct;
typedef
data_struct    *data_ptr;

void
get_polygon()
{
  int             surface_id, i, j, first, vertices;
  real            value, refraction;
  xyz_struct      origin, scale;
  polygon_ptr     polygon;
  vertex_ptr      vertex_top, vertex_bottom, vertex;
  data_ptr        data_top, data_bottom, data;
  char            name[STRING_MAX];
  file_ptr        poly_file;

  get_valid(scene, &value, 1.0, (real) surfaces, str44);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str45);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str46);
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str47);
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str48);
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str49);
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str50);
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str51);
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    poly_file = scene;
  else
  {
    OPEN(poly_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, str52, name);
      FLUSH(results);
      HALT;
    }
  }
  first = objects;
  data_top = NULL;
  vertices = 0;
  vertex_top = NULL;
  while (NOT END_OF_LINE(poly_file))
  {
    ALLOCATE(polygon, polygon_struct, 1, PARSE_TYPE);
    ALLOCATE(data, data_struct, 1, PARSE_TYPE);
    if (data_top == NULL)
      data_top = data;
    else
      data_bottom->next = (void_ptr) data;
    data_bottom = data;
    get_valid(poly_file, &value, 3.0, (real) MAXINT, str53);
    polygon->points = PRED(ROUND(value));
    ALLOCATE(data->point, vertex_ptr, SUCC(polygon->points), PARSE_TYPE);
    ALLOCATE(polygon->coords, array1, SUCC(polygon->points), PARSE_TYPE);
    for (i = 0; i <= polygon->points; POSINC(i))
    {
      get_valid(poly_file, &value, 1.0, INFINITY, str54);
      data->point[i] = vertex_pointer(ROUND(value), &vertices,
                                      &vertex_top, &vertex_bottom);
    }
    data->next = NULL;
    ADVANCE(poly_file);
    if (objects >= objects_max)
      runtime_abort(str25);
    /* Create Polygon */
    CREATE_OBJECT(POLYGON_TYPE, polygon);
    POSINC(objects);
  }
  ADVANCE(poly_file);
  POSDEC(objects);
  vertex = vertex_top;
  i = 0;
  while (NOT END_OF_LINE(poly_file))
  {
    if (vertex == NULL)
      runtime_abort(str56);
    POSINC(i);
    if (i > vertices)
      runtime_abort(str57);
    get_valid(poly_file, &value, X_MIN, X_MAX, str58);
    vertex->coords.x = value * scale.x + origin.x;
    get_valid(poly_file, &value, Y_MIN, Y_MAX, str59);
    vertex->coords.y = value * scale.y + origin.y;
    get_valid(poly_file, &value, Z_MIN, Z_MAX, str60);
    vertex->coords.z = value * scale.z + origin.z;
    vertex = (vertex_ptr) (vertex->next);
    ADVANCE(poly_file);
  }
  if (i < vertices)
    runtime_abort(str61);
  vertex_bottom->next = NULL;
  data = data_top;
  for (i = first; i <= objects; POSINC(i))
  {
    polygon = (polygon_ptr) object[i]->data;
    for (j = 0; j <= polygon->points; POSINC(j))
    {
      polygon->coords[j][0] = data->point[j]->coords.x;
      polygon->coords[j][1] = data->point[j]->coords.y;
      polygon->coords[j][2] = data->point[j]->coords.z;
    }
    if ((backface_mode == 2) AND polygon_backface_check(polygon))
    {
      FREE(polygon);
      FREE(object[i]->min);
      FREE(object[i]->max);
      FREE(object[i]);
      object[i] = object[objects];
      POSDEC(objects);
    }
    data_top = data;
    data = (data_ptr) (data->next);
    FREE(data_top);
  }
  while (vertex_top != NULL)
  {
    vertex = vertex_top;
    vertex_top = (vertex_ptr) (vertex_top->next);
    FREE(vertex);
  }
  if (poly_file != scene)
    CLOSE(poly_file);
  else
    ADVANCE(scene);
}
void
get_triangle()
{
  int             surface_id, i;
  real            value, refraction;
  xyz_struct      origin, scale;
  triangle_ptr    triangle;
  char            name[STRING_MAX];
  file_ptr        tri_file;

  get_valid(scene, &value, 0.0, (real) surfaces, str62);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str63);
  refraction = value;
  get_valid(scene, &value, X_MIN, X_MAX, str64);
  origin.x = value;
  get_valid(scene, &value, Y_MIN, Y_MAX, str65);
  origin.y = value;
  get_valid(scene, &value, Z_MIN, Z_MAX, str66);
  origin.z = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str67);
  scale.x = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str68);
  scale.y = value;
  get_valid(scene, &value, SCALE_MIN, SCALE_MAX, str69);
  scale.z = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    tri_file = scene;
  else
  {
    OPEN(tri_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, str70, name);
      FLUSH(results);
      HALT;
    }
  }
  while (NOT END_OF_LINE(tri_file))
  {
    ALLOCATE(triangle, triangle_struct, 1, PARSE_TYPE);
    for (i = 0; i < 3; POSINC(i))
    {
      get_valid(tri_file, &value, X_MIN, X_MAX, str71);
      triangle->coords[i].x = value * scale.x + origin.x;
      get_valid(tri_file, &value, Y_MIN, Y_MAX, str72);
      triangle->coords[i].y = value * scale.y + origin.y;
      get_valid(tri_file, &value, Z_MIN, Z_MAX, str73);
      triangle->coords[i].z = value * scale.z + origin.z;
      get_valid(tri_file, &value, X_MIN, X_MAX, str74);
      triangle->normal[i].x = value;
      get_valid(tri_file, &value, Y_MIN, Y_MAX, str75);
      triangle->normal[i].y = value;
      get_valid(tri_file, &value, Z_MIN, Z_MAX, str76);
      triangle->normal[i].z = value;
      if (LENGTH(triangle->normal[i]) < ROUNDOFF)
        runtime_abort(str77);
    }
    if (surface_id < 0)
    {
      ALLOCATE(triangle->surface, short int, 3, PARSE_TYPE);
      get_valid(tri_file, &value, 1.0, (real) surfaces, str78);
      triangle->surface[0] = PRED(ROUND(value));
      get_valid(tri_file, &value, 1.0, (real) surfaces, str79);
      triangle->surface[1] = PRED(ROUND(value));
      get_valid(tri_file, &value, 1.0, (real) surfaces, str80);
      triangle->surface[2] = PRED(ROUND(value));
    } else
      triangle->surface = NULL;
    ADVANCE(tri_file);
    if (objects >= objects_max)
      runtime_abort(str25);
    if ((backface_mode == 2) AND triangle_backface_check(triangle))
    {
      FREE(triangle);
      continue;
    }
    /* Create Triangle */
    CREATE_OBJECT(TRIANGLE_TYPE, triangle);
    POSINC(objects);
  }
  POSDEC(objects);
  if (tri_file != scene)
    CLOSE(tri_file);
  else
    ADVANCE(scene);
}
void
get_text()
{
  real            value;
  char            name[STRING_MAX];
  file_ptr        text_file;

  get_valid(scene, &value, 1.0, (real) surfaces, str82);
  pp_surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 1.0, REFRACTION_FACTOR_MAX, str83);
  pp_refraction = value;
  READ_STRING(scene, name);
  ADVANCE(scene);
  if ((name[0] == '-') AND(name[1] == EOT))
    text_file = scene;
  else
  {
    OPEN(text_file, name, READ_TEXT);
    if (IO_status != IO_OK)
    {
      WRITE(results, str84, name);
      FLUSH(results);
      HALT;
    }
  }
  /* Create Text */
  POSDEC(objects);
  get_pp_obj(text_file);
  if (text_file != scene)
    CLOSE(text_file);
}
void
get_csg()
{
  int             surface_id;
  real            value, refraction;
  csg_ptr         csg;
  int             op;

  get_valid(scene, &value, 0.0, (real) surfaces, str85);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 0.0, REFRACTION_FACTOR_MAX, str86);
  if (value < 1.0 - ROUNDOFF)
    refraction = -1.0;
  else
    refraction = value;
  get_valid(scene, &value, (real) CSG_UNION, (real) CSG_INTERSECTION, str87);
  op = ROUND(value);
  ADVANCE(scene);
  /* Create CSG */
  ALLOCATE(csg, csg_struct, 1, PARSE_TYPE);
  switch (op)
  {
  case CSG_UNION:
    csg->op = CSG_UNION;
    break;
  case CSG_SUBTRACTION:
    csg->op = CSG_SUBTRACTION;
    break;
  case CSG_INTERSECTION:
    csg->op = CSG_INTERSECTION;
    break;
  }
  CREATE_OBJECT(CSG_TYPE, csg);
}
void
get_list()
{
  int             surface_id;
  real            value, refraction;
  list_ptr        list;
  int             op;

  get_valid(scene, &value, 0.0, (real) surfaces, str88);
  surface_id = PRED(ROUND(value));
  get_valid(scene, &value, 0.0, REFRACTION_FACTOR_MAX, str89);
  if (value < 1.0 - ROUNDOFF)
    refraction = -1.0;
  else
    refraction = value;
  ADVANCE(scene);
  /* Create List */
  ALLOCATE(list, list_struct, 1, PARSE_TYPE);
  CREATE_OBJECT(LIST_TYPE, list);
}

#define MULTIPLY(result, first, second, index, coord)\
do {\
  result[index].coord = first[index].x * second[0].coord +\
                        first[index].y * second[1].coord +\
                        first[index].z * second[2].coord +\
                        first[index].w * second[3].coord;\
} while (0)

void
get_object_transform(scene_objects, first_object, last_object)
  int             scene_objects, *first_object, *last_object;
{
  boolean         null_transf;
  int             i, id;
  real            value;
  boolean         first_transf;
  xyzw_ptr        transf, new_transf;

  get_valid(scene, &value, 0.0, (real) scene_objects, str90);
  id = PRED(ROUND(value));
  if (id < 0)
    id = PRED(scene_objects);
  ALLOCATE(transf, xyzw_struct, 4, PARSE_TYPE);
  null_transf = TRUE;
  get_valid(scene, &(transf[0].x), X_MIN, X_MAX, str91);
  null_transf = null_transf AND ABS(transf[0].x) < ROUNDOFF;
  get_valid(scene, &(transf[0].y), Y_MIN, Y_MAX, str92);
  null_transf = null_transf AND ABS(transf[0].y) < ROUNDOFF;
  get_valid(scene, &(transf[0].z), Z_MIN, Z_MAX, str93);
  null_transf = null_transf AND ABS(transf[0].z) < ROUNDOFF;
  get_valid(scene, &(transf[0].w), W_MIN, W_MAX, str94);
  null_transf = null_transf AND ABS(transf[0].w) < ROUNDOFF;
  get_valid(scene, &(transf[1].x), X_MIN, X_MAX, str95);
  null_transf = null_transf AND ABS(transf[1].x) < ROUNDOFF;
  get_valid(scene, &(transf[1].y), Y_MIN, Y_MAX, str96);
  null_transf = null_transf AND ABS(transf[1].y) < ROUNDOFF;
  get_valid(scene, &(transf[1].z), Z_MIN, Z_MAX, str97);
  null_transf = null_transf AND ABS(transf[1].z) < ROUNDOFF;
  get_valid(scene, &(transf[1].w), W_MIN, W_MAX, str98);
  null_transf = null_transf AND ABS(transf[1].w) < ROUNDOFF;
  get_valid(scene, &(transf[2].x), X_MIN, X_MAX, str99);
  null_transf = null_transf AND ABS(transf[2].x) < ROUNDOFF;
  get_valid(scene, &(transf[2].y), Y_MIN, Y_MAX, str100);
  null_transf = null_transf AND ABS(transf[2].y) < ROUNDOFF;
  get_valid(scene, &(transf[2].z), Z_MIN, Z_MAX, str101);
  null_transf = null_transf AND ABS(transf[2].z) < ROUNDOFF;
  get_valid(scene, &(transf[2].w), W_MIN, W_MAX, str102);
  null_transf = null_transf AND ABS(transf[2].w) < ROUNDOFF;
  get_valid(scene, &(transf[3].x), X_MIN, X_MAX, str103);
  null_transf = null_transf AND ABS(transf[3].x) < ROUNDOFF;
  get_valid(scene, &(transf[3].y), Y_MIN, Y_MAX, str104);
  null_transf = null_transf AND ABS(transf[3].y) < ROUNDOFF;
  get_valid(scene, &(transf[3].z), Z_MIN, Z_MAX, str105);
  null_transf = null_transf AND ABS(transf[3].z) < ROUNDOFF;
  get_valid(scene, &(transf[3].w), W_MIN, W_MAX, str106);
  null_transf = null_transf AND ABS(transf[3].w) < ROUNDOFF;
  ADVANCE(scene);
  if (null_transf)
  {
    FREE(transf);
    return;
  }
  first_transf = FALSE;
  for (i = first_object[id]; i <= last_object[id]; POSINC(i))
  {
    if (object[i]->transf == NULL)
    {
      object[i]->transf = transf;
      first_transf = TRUE;
    } else
    {
      ALLOCATE(new_transf, xyzw_struct, 4, PARSE_TYPE);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 0, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 1, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 2, w);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, x);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, y);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, z);
      MULTIPLY(new_transf, transf, object[i]->transf, 3, w);
      if (i == last_object[id])
        FREE(object[i]->transf);
      object[i]->transf = new_transf;
    }
  }
  if (NOT first_transf)
    FREE(transf);
}

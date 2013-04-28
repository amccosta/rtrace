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
#include "config.h"

/**********************************************************************
 *    RAY TRACING - Defs and Typedefs - Version 8.4.2                 *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, November 1994          *
 **********************************************************************/

#define STRING_MAX (255)

#define ROUNDOFF ((real) 1.0e-12)
#define INFINITY ((real) 1.0e+50)

#ifndef PI
#ifdef M_PI
#define PI ((real) M_PI)
#else
#define PI ((real) 3.1415926535897932)
#endif
#endif

/* Minimum angle between Gaze and Up (5 degrees) */
#define ANGLE_MIN ((real) 5.0 * PI / 180.0)

/***** Ray Tracing constants *****/
#define INDEX_MAX (255)

#define X_MAX ((real) 100000.0)
#define X_MIN (-X_MAX)
#define Y_MAX (X_MAX)
#define Y_MIN (X_MIN)
#define Z_MAX (X_MAX)
#define Z_MIN (X_MIN)
#define W_MAX (X_MAX)
#define W_MIN (X_MIN)

#define LIGHTING_FACTOR_MAX   ((real) 300.0)
#define SPECULAR_FACTOR_MAX   ((real) 300.0)
#define REFRACTION_FACTOR_MAX ((real) 300.0)
#define DISTANCE_FACTOR       ((real) 1.0e-4)

#define CSG_LEVEL_MAX  (256)
#define LIST_LEVEL_MAX (1)

#define LIGHT_CACHE_LEVEL_MAX (12)
#define RAY_CACHE_LEVEL_MAX   (8)

#define RAY_SIZE_MAX ((1 SHL RAY_CACHE_LEVEL_MAX) - 1)

#define NO_OBJECTS (0)

#define SCALE_MIN ((real) 0.0)
#define SCALE_MAX ((real) MAXINT)

#define CLUSTER_SIZE_MIN (0)    /* Cluster size minimum */
#define CLUSTER_SIZE_MAX (16)   /* Cluster size maximum */

#define PQUEUE_SIZE_MAX (256)

#define SAMPLING_LEVEL_MAX (3)  /* Pixel supersampling level maximum */

#define X_AXIS (1)
#define Y_AXIS (2)
#define Z_AXIS (3)

#define STEREO_LEFT  (1)
#define STEREO_RIGHT (2)

#define ROOT_OBJECT object[0]

/***** Typedefs *****/
typedef
real            array1[3];

typedef
real            array2[3][3];

/***** Ray Tracing types *****/
typedef
struct
{
  real            x, y, z;
} xyz_struct;
typedef
xyz_struct     *xyz_ptr;

typedef
struct
{
  real            x, y, z, w;
} xyzw_struct;
typedef
xyzw_struct    *xyzw_ptr;

typedef
struct
{
  real            r, g, b;
} rgb_struct;
typedef
rgb_struct     *rgb_ptr;

typedef
struct
{
  int             id;
  rgb_struct      color;
} pixel_struct;
typedef
pixel_struct   *pixel_ptr;

typedef
struct
{
  xyz_struct      vector;
  rgb_struct      level;
  boolean         inside;
} ray_struct;
typedef
ray_struct     *ray_ptr;

typedef
struct
{
  rgb_struct      color, diffuse, specular, transparent;
  real            phong_factor;
  rgb_struct      diffuse_factor, metal_factor;
  boolean         emitter, noshadow;
} surface_struct;
typedef
surface_struct *surface_ptr;

#define SURFACE_TYPE1     (1)   /* Original format: C Kd Ks P M Kt */
#define SURFACE_TYPE2     (2)   /* Paul Strauss format: C S M Kt   */

#define SURFACE_KINDS_MAX (2)

/***** Textures *****/
#define NULL_TYPE      (0)
#define CHECKER_TYPE   (1)      /* Checkerboard   */
#define BLOTCH_TYPE    (2)      /* Color blotches */
#define BUMP_TYPE      (3)      /* Bump mapping   */
#define MARBLE_TYPE    (4)      /* Marble texture */
#define FBM_TYPE       (5)      /* fBm texture    */
#define FBM_BUMP_TYPE  (6)      /* fBm bump map   */
#define WOOD_TYPE      (7)      /* Wood           */
#define ROUND_TYPE     (8)      /* Round          */
#define BOZO_TYPE      (9)      /* Bozo           */
#define RIPPLES_TYPE   (10)     /* Ripples        */
#define WAVES_TYPE     (11)     /* Sea waves      */
#define SPOTTED_TYPE   (12)     /* Spotts         */
#define DENTS_TYPE     (13)     /* Dents          */
#define AGATE_TYPE     (14)     /* Agate          */
#define WRINKLES_TYPE  (15)     /* Wrinkles       */
#define GRANITE_TYPE   (16)     /* Granite        */
#define GRADIENT_TYPE  (17)     /* Color gradient */
#define IMAGE_MAP_TYPE (18)     /* Image mapping  */
#define GLOSS_TYPE     (19)     /* Gloss          */
#define BUMP3_TYPE     (20)     /* Bump 3d        */

#define TEXTURE_KINDS_MAX (20)

#define WAVES_MAX (10)
typedef
struct
{
  xyz_struct      source[WAVES_MAX];
  real            frequency[WAVES_MAX];
} wave_struct;

typedef
struct
{
  short int       surface_id;
} checker_struct;
typedef
checker_struct *checker_ptr;

typedef
struct
{
  short int       surface_id;
  real            scale;
} blotch_struct;
typedef
blotch_struct  *blotch_ptr;

typedef
struct
{
  real            scale;
} bump_struct;
typedef
bump_struct    *bump_ptr;

typedef
struct
{
  rgb_ptr         color;
} marble_struct;
typedef
marble_struct  *marble_ptr;

typedef
struct
{
  real            offset, scale, omega, lambda, threshold;
  int             octaves;
  rgb_ptr         color;
} fbm_struct;
typedef
fbm_struct     *fbm_ptr;

typedef
struct
{
  real            offset, scale, omega, lambda;
  int             octaves;
} fbm_bump_struct;
typedef
fbm_bump_struct *fbm_bump_ptr;

typedef
struct
{
  rgb_struct      color;
} wood_struct;
typedef
wood_struct    *wood_ptr;

typedef
struct
{
  real            scale;
} round_struct;
typedef
round_struct   *round_ptr;

typedef
struct
{
  real            turbulence;
  rgb_ptr         color;
} bozo_struct;
typedef
bozo_struct    *bozo_ptr;

typedef
struct
{
  real            frequency, phase, scale;
} ripples_struct;
typedef
ripples_struct *ripples_ptr;

typedef
struct
{
  real            frequency, phase, scale;
} waves_struct;
typedef
waves_struct   *waves_ptr;

typedef
struct
{
  rgb_ptr         color;
} spotted_struct;
typedef
spotted_struct *spotted_ptr;

typedef
struct
{
  real            scale;
} dents_struct;
typedef
dents_struct   *dents_ptr;

typedef
struct
{
  rgb_ptr         color;
} agate_struct;
typedef
agate_struct   *agate_ptr;

typedef
struct
{
  real            scale;
} wrinkles_struct;
typedef
wrinkles_struct *wrinkles_ptr;

typedef
struct
{
  rgb_ptr         color;
} granite_struct;
typedef
granite_struct *granite_ptr;

typedef
struct
{
  real            turbulence;
  xyz_struct      direction;
  rgb_ptr         color;
} gradient_struct;
typedef
gradient_struct *gradient_ptr;

typedef
struct
{
  real            turbulence;
  boolean         once;
  unsigned short int width, height;
  short int       u, v;
  file_ptr        image;
} image_map_struct;
typedef
image_map_struct *image_map_ptr;

typedef
struct
{
  real            scale;
} gloss_struct;
typedef
gloss_struct   *gloss_ptr;

typedef
struct
{
  real            scale;
  real            size;
} bump3_struct;
typedef
bump3_struct   *bump3_ptr;

typedef
struct
{
  short int       type;
  xyzw_ptr        transf;
  void_ptr        next;
  void_ptr        data;
} texture_struct;
typedef
texture_struct *texture_ptr;

typedef
struct
{
  xyz_struct      center;
  real            radius, radius2;
  xyz_ptr         position;
} sphere_struct;
typedef
sphere_struct  *sphere_ptr;

typedef
struct
{
  xyz_struct      center, size;
  short int       side_hit;
  xyz_ptr         position;
} box_struct;
typedef
box_struct     *box_ptr;

typedef
struct
{
  xyz_struct      coords;
  void_ptr        next;
} vertex_struct;
typedef
vertex_struct  *vertex_ptr;

typedef
struct
{
  vertex_ptr      vertex;
  vertex_ptr      p[12];
  xyz_struct      c[4][4];
  real            u_hit, v_hit;
} patch_struct;
typedef
patch_struct   *patch_ptr;

typedef
struct
{
  xyz_struct      base, apex, u, v, w;
  real            base_radius, base_d, apex_radius;
  real            height, slope, min_d, max_d;
  xyz_ptr         position;
} cone_struct;
typedef
cone_struct    *cone_ptr;

typedef
struct
{
  short int       points;
  array1         *coords;
  xyz_struct      normal;
  real            d;
  short int       u, v;
} polygon_struct;
typedef
polygon_struct *polygon_ptr;

typedef
struct
{
  xyz_struct      coords[3], normal[3], transf[3];
  real            u_hit, v_hit;
  short int      *surface;
} triangle_struct;
typedef
triangle_struct *triangle_ptr;

typedef
struct
{
  xyz_ptr         position;
  void_ptr        data;
} text_struct;
typedef
text_struct    *text_ptr;

typedef
struct
{
  unsigned char   op;
  xyz_ptr         position;
  short int       surface_id;
  real            refraction;
  texture_ptr     texture;
  int             left, right;
} csg_struct;
typedef
csg_struct     *csg_ptr;

/***** Objects *****/
#define CLUSTER_TYPE  (0)       /* Cluster            */
#define SPHERE_TYPE   (1)       /* Spheres            */
#define BOX_TYPE      (2)       /* Axis aligned Boxes */
#define PATCH_TYPE    (3)       /* Bicubic Patches    */
#define CONE_TYPE     (4)       /* Cones/Cylinders    */
#define POLYGON_TYPE  (5)       /* Polygons           */
#define TRIANGLE_TYPE (6)       /* Triangular Patches */
#define TEXT_TYPE     (7)       /* Text PP Primitive  */
#define CSG_TYPE      (8)       /* CSG Tree           */
#define LIST_TYPE     (9)       /* List of Objects    */

#define OBJECT_KINDS_MAX (9)

#define TEXTURE_TYPE  (64)      /* Texture        */
#define TRANSF_TYPE   (65)      /* Transformation */
#define CSG_OP_TYPE   (66)      /* CSG            */
#define LIST_OP_TYPE  (67)      /* List           */

#define CSG_UNION        (0)
#define CSG_SUBTRACTION  (1)
#define CSG_INTERSECTION (2)

#define LIST_BEGIN (0)
#define LIST_END   (1)

typedef
struct
{
  int             id;                    /* Object identifier  */
  short int       surface_id;            /* Surface identifier */
  real            refraction;            /* Refraction index   */
  xyz_ptr         min, max;              /* Bounding volume    */
  xyzw_ptr        transf;                /* Transformation     */
  xyzw_ptr        inv_transf;            /* Inverse transf.    */
  texture_ptr     texture;               /* List of textures   */
  boolean         texture_modify_normal;
  unsigned char   object_type;
  void_ptr        data;
} object_struct;
typedef
object_struct  *object_ptr;

typedef
struct
{
  short int       size;
  object_ptr     *object;
} cluster_struct;
typedef
cluster_struct *cluster_ptr;

typedef 
struct
{
  object_ptr      object;
  void_ptr        next;
} list_node_struct;
typedef
list_node_struct *list_node_ptr;

typedef
struct
{
  xyz_ptr         position;
  short int       surface_id;
  real            refraction;
  object_ptr      object;
  list_node_ptr   head;
} list_struct;
typedef
list_struct    *list_ptr;

/***** Lights *****/
#define POINT_LIGHT_TYPE    (1) /* Point light            */
#define DIRECT_LIGHT_TYPE   (2) /* Directional/Spot light */
#define EXTENDED_LIGHT_TYPE (3) /* Extended light         */
#define PLANAR_LIGHT_TYPE   (4) /* Planar light           */

#define LIGHT_KINDS_MAX (4)

typedef
struct
{
  xyz_struct      coords;
  rgb_struct      brightness;
  boolean         attenuation[3];
  int             cache_id[LIGHT_CACHE_LEVEL_MAX];
  short int       light_type;
  void_ptr        data;
} light_struct;
typedef
light_struct   *light_ptr;

typedef
struct
{
  xyz_struct      vector;
  real            cos_angle, t, factor;
} dir_light_struct;
typedef
dir_light_struct *dir_light_ptr;

typedef
struct
{
  real            diameter;
  short int       samples;
} ext_light_struct;
typedef
ext_light_struct *ext_light_ptr;

typedef
struct
{
  xyz_struct      vector1, vector2;
  short int       samples1, samples2;
  real            size;
} planar_light_struct;
typedef
planar_light_struct *planar_light_ptr;

/***** Others *****/
typedef
struct
{
  real            key;
  object_ptr      object;
} pqueue_struct;

typedef
int             list_id[4];

typedef
struct
{
  xyz_ptr         eye, look, up;
  real            view_angle_x, view_angle_y;
} camera_struct;
typedef
camera_struct  *camera_ptr;

/***** General Macros *****/
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define REALINC(x) ((x) += 1.0)

#define DEGREE_TO_RADIAN(angle) ((real) (angle) * PI / 180.0)

#define PQUEUE_INITIALIZE \
do {\
  pqueue_size = 0;\
  REALINC(pqueue_resets);\
} while (0)
#define PQUEUE_NOT_EMPTY ((boolean) (pqueue_size > 0))

#define DOT_PRODUCT(p0, p1)\
((p0).x * (p1).x + (p0).y * (p1).y + (p0).z * (p1).z)

#define LENGTH(p) (SQRT(DOT_PRODUCT(p, p)))
#define NORMALIZE(p)\
do {\
  REG real        d_TEMP;\
\
  d_TEMP = LENGTH(p);\
  if (d_TEMP < ROUNDOFF)\
    runtime_abort("cannot normalize a NULL VECTOR");\
  d_TEMP = 1.0 / d_TEMP;\
  (p).x *= d_TEMP;\
  (p).y *= d_TEMP;\
  (p).z *= d_TEMP;\
} while (0)

#define CROSS_PRODUCT(p, p0, p1)\
do {\
  (p).x = (p0).y * (p1).z - (p0).z * (p1).y;\
  (p).y = (p0).z * (p1).x - (p0).x * (p1).z;\
  (p).z = (p0).x * (p1).y - (p0).y * (p1).x;\
} while (0)

#define CHECK_BOUNDS(type)\
(((type) != SPHERE_TYPE)\
AND((type) != BOX_TYPE)\
AND((type) != CONE_TYPE)\
AND((type) != TRIANGLE_TYPE))

#define OBJECT_INTERSECT(distance, position, vector, object)\
do {\
  xyz_struct      pos_TEMP;\
\
  if (intersect_adjust_mode)\
  {\
    pos_TEMP.x = (position)->x + threshold_distance * (vector)->x;\
    pos_TEMP.y = (position)->y + threshold_distance * (vector)->y;\
    pos_TEMP.z = (position)->z + threshold_distance * (vector)->z;\
  } else\
    STRUCT_ASSIGN(pos_TEMP, *(position));\
  switch ((object)->object_type)\
  {\
  case SPHERE_TYPE:\
    distance = sphere_intersect(&pos_TEMP, vector, object);\
    break;\
  case BOX_TYPE:\
    distance = box_intersect(&pos_TEMP, vector, object);\
    break;\
  case PATCH_TYPE:\
    distance = patch_intersect(&pos_TEMP, vector, object);\
    break;\
  case CONE_TYPE:\
    distance = cone_intersect(&pos_TEMP, vector, object);\
    break;\
  case POLYGON_TYPE:\
    distance = polygon_intersect(&pos_TEMP, vector, object);\
    break;\
  case TRIANGLE_TYPE:\
    distance = triangle_intersect(&pos_TEMP, vector, object);\
    break;\
  case TEXT_TYPE:\
    distance = text_intersect(&pos_TEMP, vector, object);\
    break;\
  case CSG_TYPE:\
    distance = csg_intersect(&pos_TEMP, vector, object);\
    break;\
  case LIST_TYPE:\
    distance = list_intersect(&pos_TEMP, vector, object);\
    break;\
  }\
  if (intersect_adjust_mode AND(distance != 0.0))\
    distance += threshold_distance;\
} while (0)

#define OBJECT_NORMAL(position, object, normal)\
switch ((object)->object_type)\
{\
case SPHERE_TYPE:\
  sphere_normal(position, object, normal);\
  break;\
case BOX_TYPE:\
  box_normal(position, object, normal);\
  break;\
case PATCH_TYPE:\
  patch_normal(position, object, normal);\
  break;\
case CONE_TYPE:\
  cone_normal(position, object, normal);\
  break;\
case POLYGON_TYPE:\
  polygon_normal(position, object, normal);\
  break;\
case TRIANGLE_TYPE:\
  triangle_normal(position, object, normal);\
  break;\
case TEXT_TYPE:\
  text_normal(position, object, normal);\
  break;\
case CSG_TYPE:\
  csg_normal(position, object, normal);\
  break;\
case LIST_TYPE:\
  list_normal(position, object, normal);\
  break;\
}

#define ATTEN_DISTANCE(distance, factor)\
factor = gaze_distance / (gaze_distance + (distance) * 0.25)

#define ATTEN_COLOR(distance, atten_r, atten_g, atten_b, old, new)\
do {\
  REG real        k_TEMP;\
\
  ATTEN_DISTANCE(distance, k_TEMP);\
  (new).r = atten_r ? k_TEMP * (old).r : (old).r;\
  (new).g = atten_g ? k_TEMP * (old).g : (old).g;\
  (new).b = atten_b ? k_TEMP * (old).b : (old).b;\
} while (0)

#define FIND_OCTANT(octant, vector)\
do {\
  (octant) = (vector).x >= 0.0 ? 1 : 0;\
  if ((vector).y >= 0.0)\
    (octant) += 2;\
  if ((vector).z >= 0.0)\
    (octant) += 4;\
} while (0)

#define RANDOM_START psrand(1L)
#define RANDOM prand()

#define JITTER (RANDOM - 0.5)

#define ALLOCATE(p, s, c, t)\
do {\
  ALLOC(p, s, c, t);\
  if (NOT(p))\
    {\
    WRITE(results, "Error: cannot allocate MEMORY\n");\
    HALT;\
    }\
} while (0)

#define ADVANCE(file)\
do {\
  READ_LINE(file);\
  while (PEEK_CHAR(file) == '#')\
    READ_LINE(file);\
  if (IO_status == IO_READ)\
  {\
    WRITE(results, "Error: bad INPUT FORMAT\n");\
    HALT;\
  }\
} while (0)

#define MODIFY_NORMAL(type)\
(((type) == BUMP_TYPE)\
 OR((type) == FBM_BUMP_TYPE)\
 OR((type) == ROUND_TYPE)\
 OR((type) == RIPPLES_TYPE)\
 OR((type) == WAVES_TYPE)\
 OR((type) == DENTS_TYPE)\
 OR((type) == WRINKLES_TYPE)\
 OR((type) == BUMP3_TYPE))

#define CLOSED(type)\
(((type) == SPHERE_TYPE)\
 OR((type) == BOX_TYPE)\
 OR((type) == TEXT_TYPE)\
 OR((type) == CSG_TYPE))

#define CONVEX(type)\
(((type) != PATCH_TYPE)\
 AND((type) != TEXT_TYPE)\
 AND((type) != CSG_TYPE)\
 AND((type) != LIST_TYPE))

#define SELF_INTERSECT(type, inside)\
((inside) ?\
(((type) == SPHERE_TYPE)\
 OR((type) == BOX_TYPE)\
 OR((type) == CONE_TYPE)\
 OR((type) == PATCH_TYPE)\
 OR((type) == TEXT_TYPE)\
 OR((type) == CSG_TYPE)\
 OR((type) == LIST_TYPE)) :\
(((type) == PATCH_TYPE)\
 OR((type) == TEXT_TYPE)\
 OR((type) == CSG_TYPE)\
 OR((type) == LIST_TYPE)))

#define POLYGONAL(type)\
(((type) == POLYGON_TYPE)\
 OR((type) == TRIANGLE_TYPE))

#define UNIT_CIRCLE_POINT(x, y)\
do {\
  x = 2.0 * RANDOM - 1.0;\
  y = SQRT(1.0 - SQR(x)) * (2.0 * RANDOM - 1.0);\
} while (0)

#define COLOR_BIG(color, value)\
(((color).r > (value)) OR((color).g > (value)) OR((color).b > (value)))

#define OBJECT_ENCLOSE(object)\
do {\
  if (object->transf != NULL)\
  {\
    object->inv_transf = object->transf;\
    ALLOCATE(object->transf, xyzw_struct, 4, PARSE_TYPE);\
    inverse_transform(object->inv_transf, object->transf);\
  }\
  switch ((object)->object_type)\
  {\
  case SPHERE_TYPE:\
    sphere_enclose(object);\
    break;\
  case BOX_TYPE:\
    box_enclose(object);\
    break;\
  case PATCH_TYPE:\
    patch_enclose(object);\
    break;\
  case CONE_TYPE:\
    cone_enclose(object);\
    break;\
  case POLYGON_TYPE:\
    polygon_enclose(object);\
    break;\
  case TRIANGLE_TYPE:\
    triangle_enclose(object);\
    break;\
  case TEXT_TYPE:\
    text_enclose(object);\
    break;\
  case CSG_TYPE:\
    break;\
  case LIST_TYPE:\
    list_enclose(object);\
    break;\
  }\
} while (0)

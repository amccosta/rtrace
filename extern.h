/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
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

/**********************************************************************
 *    RAY TRACING - Externs - Version 8.4.1                           *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, September 1994         *
 **********************************************************************/

#ifdef _Windows
#define FAR far
#else
#define FAR
#endif

/***** External declarations *****/
extern short int FAR antialiasing_mode;
extern short int FAR view_mode;
extern short int FAR texture_mode;
extern short int FAR light_mode;
extern short int FAR last_shade_level;
extern short int FAR background_mode;
extern short int FAR sampling_levels;
extern short int FAR sampling_divisions;
extern short int FAR sampling_weight;
extern short int FAR cluster_size;
extern short int FAR ambient_sample_rays;
extern short int FAR last_ambient_level;
extern short int FAR distributed_cache_mode;
extern short int FAR distributed_cache_repetitions;
extern short int FAR intersect_mode;
extern short int FAR intersect_adjust_mode;
extern short int FAR jittering_mode;
extern short int FAR raw_mode;
extern short int FAR normal_mode;
extern short int FAR normal_check_mode;
extern short int FAR shade_mode;
extern short int FAR image_format;
extern short int FAR image_format_offset;
extern short int FAR verbose_mode;
extern short int FAR contrast_mode;
extern short int FAR backface_mode;
extern short int FAR walk_mode;
extern short int FAR shade_level, shade_level_max;
extern short int FAR pqueue_size;
extern short int FAR ray_node;
extern short int FAR csg_level_max;
extern short int FAR movie_frames;

extern int FAR  previous_repetitions;

extern int FAR  objects;
extern int FAR  lights;
extern int FAR  surfaces;
extern int FAR  screen_size_x, screen_size_y;
extern int FAR  lights_max;
extern int FAR  surfaces_max;
extern int FAR  objects_max;

extern real FAR gaze_distance, light_distance;
extern real FAR pixel_distance, threshold_distance;
extern real FAR view_angle_x, view_angle_y;
extern real FAR threshold_level, threshold_color, threshold_vector;
extern real FAR focal_aperture, focal_distance;
extern real FAR stereo_separation;

extern real FAR eye_rays, reflected_rays, refracted_rays, shadow_rays;
extern real FAR ambient_rays;
extern real FAR shadow_hits, shadow_cache_hits;
extern real FAR octant_tests, bound_tests, sphere_tests, box_tests,
                patch_tests, cone_tests, polygon_tests,
                triangle_tests, text_tests, csg_tests, list_tests,
                backface_tests, backface_hits;
extern real FAR pqueue_resets, pqueue_insertions, pqueue_extractions;
extern real FAR ray_hits, ray_cache_resets, ray_cache_hits;
extern real FAR distributed_cache_resets, distributed_cache_hits;
extern real FAR pixel_divisions;

extern
xyz_struct FAR  eye, look, up, gaze, screen_x, screen_y;

extern
int FAR         ray_cache[RAY_SIZE_MAX];

extern
rgb_struct FAR  back_color, light_ambient;
extern
rgb_ptr FAR     true_color;

extern
object_ptr FAR *object;

extern
surface_ptr FAR *surface;

extern
light_ptr FAR   light;

extern
pqueue_struct FAR *pqueue;

extern
pixel_ptr FAR   new_line, old_line;

extern real FAR *back_mask;

extern
wave_struct FAR wave;

extern
file_ptr FAR    scene, picture, background, raw_picture;
#ifndef SUIT
extern
file_ptr FAR    results;
#endif

extern
camera_ptr FAR  camera;

#ifdef PROTOTYPES
#define P(args) args
#else
#define P(args) ()
#endif

extern
vertex_ptr      vertex_pointer P((int, int *, vertex_ptr *, vertex_ptr *));

extern void     runtime_abort P((char_ptr));
extern void     make_vector P((xyz_ptr, real, real, real));
extern void     pqueue_extract P((real *, object_ptr *));
extern void     pqueue_insert P((real, object_ptr));
extern void     box_enclose P((object_ptr));
extern void     sphere_enclose P((object_ptr));
extern void     patch_enclose P((object_ptr));
extern void     cone_enclose P((object_ptr));
extern void     polygon_enclose P((object_ptr));
extern void     triangle_enclose P((object_ptr));
extern void     text_enclose P((object_ptr));
extern void     csg_enclose P((int, int *, int *));
extern void     list_enclose P((object_ptr));
extern void     enclose_type_2 P((void));
extern void     enclose_type_3 P((void));
extern void     enclose_all P((void));
extern void     box_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     sphere_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     patch_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     cone_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     polygon_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     triangle_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     text_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     csg_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     list_normal P((xyz_ptr, object_ptr, xyz_ptr));
extern void     vector_to_light P((int, xyz_ptr, xyz_ptr));
extern void     light_brightness P((int, int, xyz_ptr, xyz_ptr, rgb_ptr, int));
extern void     make_diffuse_vector P((xyz_ptr, xyz_ptr));
extern void     make_specular_vector P((xyz_ptr, xyz_ptr, real, xyz_ptr));
extern void     shade P((xyz_ptr, xyz_ptr, ray_ptr, object_ptr, rgb_ptr));
extern void     shade_phong P((xyz_ptr, xyz_ptr, ray_ptr, object_ptr,
                            rgb_ptr));
extern void     shade_strauss P((xyz_ptr, xyz_ptr, ray_ptr, object_ptr,
                              rgb_ptr));
extern void     surface_texture P((xyz_ptr, xyz_ptr, surface_ptr,
                                texture_ptr));
extern void     init_picture P((void));
extern void     line_picture P((void));
extern void     find_true_color P((int, int,
                                pixel_ptr, pixel_ptr, pixel_ptr,pixel_ptr,
                                rgb_ptr));
extern void     get_valid P((file_ptr, real *, real, real, char_ptr));
extern void     get_point_light P((void));
extern void     get_dir_light P((void));
extern void     get_ext_light P((void));
extern void     get_planar_light P((void));
extern void     get_surface_type1 P((void));
extern void     get_surface_type2 P((void));
extern void     get_sphere P((void));
extern void     get_box P((void));
extern void     get_patch P((void));
extern void     get_cone P((void));
extern void     get_polygon P((void));
extern void     get_triangle P((void));
extern void     get_text P((void));
extern void     get_csg P((void));
extern void     get_list P((void));
extern void     get_object_transform P((int, int *, int *));
extern void     get_texture_null P((int, int *, int *));
extern void     get_texture_checker P((int, int *, int *));
extern void     get_texture_blotch P((int, int *, int *));
extern void     get_texture_bump P((int, int *, int *));
extern void     get_texture_marble P((int, int *, int *));
extern void     get_texture_fbm P((int, int *, int *));
extern void     get_texture_fbm_bump P((int, int *, int *));
extern void     get_texture_wood P((int, int *, int *));
extern void     get_texture_round P((int, int *, int *));
extern void     get_texture_bozo P((int, int *, int *));
extern void     get_texture_ripples P((int, int *, int *));
extern void     get_texture_waves P((int, int *, int *));
extern void     get_texture_spotted P((int, int *, int *));
extern void     get_texture_dents P((int, int *, int *));
extern void     get_texture_agate P((int, int *, int *));
extern void     get_texture_wrinkles P((int, int *, int *));
extern void     get_texture_granite P((int, int *, int *));
extern void     get_texture_gradient P((int, int *, int *));
extern void     get_texture_image_map P((int, int *, int *));
extern void     get_texture_gloss P((int, int *, int *));
extern void     get_texture_bump3 P((int, int *, int *));
extern void     get_parameters P((int, char_ptr[]));
extern void     get_scene P((void));
extern void     cluster_intersect P((int, int, int, xyz_ptr, xyz_ptr,
                                  cluster_ptr, real *, int *, int));
extern void     init_texture P((void));
extern void     black_noise2 P((xyz_ptr, xyz_ptr));
extern void     fbm_noise2 P((xyz_ptr, real, real, int, xyz_ptr));
extern void     turbulence2 P((xyz_ptr, xyz_ptr));
extern void     transform P((xyzw_ptr, xyz_ptr, xyz_ptr));
extern void     transform_vector P((xyzw_ptr, xyz_ptr, xyz_ptr, xyz_ptr,
                                 xyz_ptr));
extern void     transform_normal_vector P((xyzw_ptr, xyz_ptr, xyz_ptr,
                                        xyz_ptr));
extern void     inverse_transform P((xyzw_ptr, xyzw_ptr));
extern void     normalize_transform P((xyzw_ptr));
extern void     validate_object P((object_ptr, int));
extern void     setup_scene P((void));
extern void     make_view P((void));
extern void     init_globals P((void));
extern void     ray_trace P((void));
extern void     defaults P((void));
extern void     options P((void));
extern void     serp_make_pixel_list P((int, int));
extern void     hilb_make_pixel_list P((int, int));
extern void     serp_reset_pixel_list P((void));
extern void     hilb_reset_pixel_list P((void));
extern void     serp_get_next_pixel P((int *, int *));
extern void     hilb_get_next_pixel P((int *, int *));
extern void     psrand P((long));

extern boolean  octant_intersect P((int, xyz_ptr, xyz_ptr, xyz_ptr,
                                 xyz_ptr));
extern boolean  backface_check P((xyz_ptr, xyz_ptr, object_ptr));
extern boolean  refract P((ray_ptr, xyz_ptr, int, real));

extern int      estimate_diffuse P((int, real));
extern int      estimate_specular P((int, real, real));

extern real     tang P((real));
extern real     bound_intersect P((xyz_ptr, xyz_ptr, xyz_ptr, xyz_ptr));
extern real     box_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     sphere_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     patch_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     cone_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     polygon_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     triangle_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     text_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     csg_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     list_intersect P((xyz_ptr, xyz_ptr, object_ptr));
extern real     intersect_all P((int, xyz_ptr, ray_ptr, rgb_ptr));
extern real     chaos_noise P((xyz_ptr, int));
extern real     black_noise P((xyz_ptr));
extern real     fbm_noise P((xyz_ptr, real, real, int));
extern real     marble_noise P((xyz_ptr));
extern real     turbulence P((xyz_ptr));
extern real     cycloidal P((real));
extern real     triangle_wave P((real));
extern real     transform_distance P((xyzw_ptr, real, xyz_ptr, xyz_ptr,
                                   xyz_ptr));
extern real     light_intensity P((int, xyz_ptr));

extern double   prand P((void));
#undef P

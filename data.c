/*
 * Copyright (c) 1988, 1994 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paul Strauss        - shading model
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

/**********************************************************************
 *    RAY TRACING - Data - Version 8.4.0                              *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, January 1994           *
 **********************************************************************/

#ifdef _Windows
#pragma option -zEdata1s -zFdata1c -zHdata1g
#define FAR far
#else
#define FAR
#endif

/***** Global Variables *****/
short int FAR   antialiasing_mode;
short int FAR   view_mode;
short int FAR   texture_mode;
short int FAR   light_mode;
short int FAR   last_shade_level;
short int FAR   background_mode;
short int FAR   sampling_levels;
short int FAR   sampling_divisions;
short int FAR   sampling_weight;
short int FAR   cluster_size;
short int FAR   ambient_sample_rays;
short int FAR   last_ambient_level;
short int FAR   distributed_cache_mode;
short int FAR   distributed_cache_repetitions;
short int FAR   intersect_mode;
short int FAR   intersect_adjust_mode;
short int FAR   jittering_mode;
short int FAR   raw_mode;
short int FAR   normal_mode;
short int FAR   normal_check_mode;
short int FAR   shade_mode;
short int FAR   image_format;
short int FAR   image_format_offset;
short int FAR   verbose_mode;
short int FAR   contrast_mode;
short int FAR   backface_mode;
short int FAR   walk_mode;
short int FAR   shade_level, shade_level_max;
short int FAR   pqueue_size;
short int FAR   ray_node;
short int FAR   csg_level_max;
short int FAR   movie_frames;

int FAR         previous_repetitions;

int FAR         objects;
int FAR         lights;
int FAR         surfaces;
int FAR         screen_size_x, screen_size_y;   /* Screen dimensions */
int FAR         lights_max;
int FAR         surfaces_max;
int FAR         objects_max;

real FAR        gaze_distance, light_distance;
real FAR        pixel_distance, threshold_distance;
real FAR        view_angle_x, view_angle_y;     /* View field angles */
real FAR        threshold_level, threshold_color, threshold_vector;
real FAR        focal_aperture, focal_distance;
real FAR        stereo_separation;

real FAR        eye_rays, reflected_rays, refracted_rays, shadow_rays;
real FAR        ambient_rays;
real FAR        shadow_hits, shadow_cache_hits;
real FAR        octant_tests, bound_tests, sphere_tests, box_tests,
                patch_tests, cone_tests, polygon_tests,
                triangle_tests, text_tests, csg_tests, list_tests,
                backface_tests, backface_hits;
real FAR        pqueue_resets, pqueue_insertions, pqueue_extractions;
real FAR        ray_hits, ray_cache_resets, ray_cache_hits;
real FAR        distributed_cache_resets, distributed_cache_hits;
real FAR        pixel_divisions;

rgb_struct FAR  back_color,     /* Background color */
                light_ambient;  /* Ambient lighting */

xyz_struct FAR  eye, look, up, gaze, screen_x, screen_y;

int FAR         ray_cache[RAY_SIZE_MAX];

object_ptr FAR *object;

surface_ptr FAR *surface;

light_ptr FAR   light;

pqueue_struct FAR  *pqueue;

pixel_ptr FAR   new_line, old_line;

rgb_ptr FAR     true_color;

real FAR       *back_mask;

wave_struct FAR wave;

file_ptr FAR    scene, picture, background, raw_picture;
#ifndef SUIT
file_ptr FAR    results;
#endif

camera_ptr FAR  camera;

#ifdef _Windows
#pragma option -zE* -zF* -zH*
#endif

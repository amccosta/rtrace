/*
 * Copyright (c) 1988, 1993 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * Code, ideas or suggestions were taken from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *  Paulo Almeida       - TEXT3D primitive
 *  Pedro Borges        - TEXT3D primitive
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
 *    RAY TRACING - Version 8.3.0                                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, June 1992              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, June 1993              *
 **********************************************************************/

/***** Interface with PP code *****/
#ifdef __STDC__
extern void       get_pp_obj(file_ptr);
extern real       intersect_pp_obj(xyz_ptr, xyz_ptr, xyz_ptr, xyz_ptr,
                                   void_ptr);
extern void       normal_pp_obj(xyz_ptr, void_ptr, xyz_ptr);
extern void       end_pp_get(void);
#else
extern void       get_pp_obj();
extern real       intersect_pp_obj();
extern void       normal_pp_obj();
extern void       end_pp_get();
#endif

/***** Local stuff *****/
extern short int  pp_surface_id;
extern real       pp_refraction;

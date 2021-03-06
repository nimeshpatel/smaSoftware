/*

-Header_File SpiceZim.h ( CSPICE interface macros )

-Abstract

   Define interface macros to be called in place of CSPICE 
   user-interface-level functions.  These macros are generally used
   to compensate for compiler deficiencies.
   
-Copyright

   Copyright (1999), California Institute of Technology.
   U.S. Government sponsorship acknowledged.

-Required_Reading

   None.

-Literature_References

   None.

-Particulars

   This header file defines interface macros to be called in place of 
   CSPICE user-interface-level functions.  Currently, the sole purpose
   of these macros is to implement automatic type casting under some 
   environments that generate compile-time warnings without the casts.
   The typical case that causes a problem is a function argument list
   containing an input formal argument of type
   
      const double [3][3]
      
   Under some compilers, a non-const actual argument supplied in a call
   to such a function will generate a spurious warning due to the 
   "mismatched" type.  These macros generate type casts that will
   make such compilers happy.
   
   Examples of compilers that generate warnings of this type are
   
      gcc version 2.2.2, hosted on NeXT workstations running 
      NeXTStep 3.3
      
      Sun C compiler, version 4.2, running under Solaris.
            
      
-Author_and_Institution

   N.J. Bachman       (JPL)
   E.D. Wright        (JPL)

-Version

   -CSPICE Version 4.0.0, 22-MAR-2000 (NJB) 

       Added macros for
       
          spkw12_c
          spkw13_c
       
   -CSPICE Version 3.0.0, 27-AUG-1999 (NJB) (EDW)

       Fixed cut & paste error in macro nvp2pl_c.
       
       Added macros for
       
          axisar_c
          cgv2el_c
          dafps_c
          dafus_c
          diags2_c
          dvdot_c
          dvhat_c
          edlimb_c
          ekacli_c
          ekacld_c
          ekacli_c
          eul2xf_c
          el2cgv_c
          getelm_c
          inedpl_c
          isrot_c
          mequ_c
          npedln_c
          nplnpt_c
          rav2xf_c
          raxisa_c
          saelgv_c
          spk14a_c
          spkapo_c
          spkapp_c
          spkw02_c
          spkw03_c
          spkw05_c
          spkw08_c
          spkw09_c
          spkw10_c
          spkw15_c
          spkw17_c
          sumai_c
          trace_c
          vadd_g
          vhatg_c
          vlcomg_c
          vminug_c
          vrel_c
          vrelg_c
          vsepg_c
          vtmv_c
          vtmvg_c
          vupack_c
          vzerog_c
          xf2eul_c
          xf2rav_c
       
   -CSPICE Version 2.0.0, 07-MAR-1999 (NJB)  

       Added macros for
       
          inrypl_c
          nvc2pl_c
          nvp2pl_c
          pl2nvc_c
          pl2nvp_c
          pl2psv_c
          psv2pl_c
          vprjp_c
          vprjpi_c

   -CSPICE Version 1.0.0, 24-JAN-1999 (NJB) (EDW)


-Index_Entries

   interface macros for CSPICE functions

*/


/*
Include Files:
*/


#ifndef  HAVE_SPICEDEFS_H
#include "SpiceZdf.h"
#endif

#ifndef HAVE_SPICEIFMACROS_H
#define HAVE_SPICEIFMACROS_H


/*
Macros used to abbreviate type casts:
*/
   
   #define  CONST_BOOL         ( ConstSpiceBoolean   *      )
   #define  CONST_ELLIPSE      ( ConstSpiceEllipse   *      )
   #define  CONST_IVEC         ( ConstSpiceInt       *      )
   #define  CONST_MAT          ( ConstSpiceDouble   (*) [3] )
   #define  CONST_MAT2         ( ConstSpiceDouble   (*) [2] )
   #define  CONST_MAT6         ( ConstSpiceDouble   (*) [6] )
   #define  CONST_PLANE        ( ConstSpicePlane     *      )
   #define  CONST_VEC          ( ConstSpiceDouble    *      )
   #define  CONST_VOID         ( const void          *      )

/*
Macros that substitute for function calls:
*/

   #define  axisar_c( axis, angle, r )                                 \
                                                                       \
        (   axisar_c( CONST_VEC(axis), (angle), (r) )   )


   #define  cgv2el_c( center, vec1, vec2, ellipse )                    \
                                                                       \
        (   cgv2el_c( CONST_VEC(center), CONST_VEC(vec1),              \
                      CONST_VEC(vec2),   (ellipse)        )   )
                      
           
   #define  dafps_c( nd, ni, dc, ic, sum )                             \
                                                                       \
        (   dafps_c ( (nd), (ni), CONST_VEC(dc), CONST_IVEC(ic),       \
                      (sum)                                     )   )
                      
                      
   #define  dafus_c( sum, nd, ni, dc, ic )                             \
                                                                       \
        (   dafus_c ( CONST_VEC(sum), (nd), (ni), (dc), (ic) )   )
                      
                      
   #define  det_c( m1 )                                                \
                                                                       \
        (   det_c ( CONST_MAT(m1) )   )


   #define  diags2_c( symmat, diag, rotate )                           \
                                                                       \
        (   diags2_c ( CONST_MAT2(symmat), (diag), (rotate) )   )                       
                                                

   #define  dvdot_c( s1, s2 )                                         \
                                                                      \
           ( dvdot_c ( CONST_VEC(s1), CONST_VEC(s2) )   )
   
   
   #define  dvhat_c( v1, v2 )                                         \
                                                                      \
           ( dvhat_c ( CONST_VEC(v1), (v2) )   )


   #define  edlimb_c( a, b, c, viewpt, limb )                          \
                                                                       \
        (   edlimb_c( (a), (b), (c), CONST_VEC(viewpt), (limb) )   )


   #define  ekaclc_c( handle, segno,  column, vallen, cvals, entszs,   \
                      nlflgs, rcptrs, wkindx                         ) \
                                                                       \
        (   ekaclc_c( (handle), (segno),  (column),  (vallen),         \
                      (cvals),            CONST_IVEC(entszs),          \
                      CONST_BOOL(nlflgs), CONST_IVEC(rcptrs),          \
                      (wkindx)                                      )  ) 


   #define  ekacld_c( handle, segno,  column, dvals, entszs, nlflgs,   \
                      rcptrs, wkindx                                 ) \
                                                                       \
        (   ekacld_c( (handle),           (segno),           (column), \
                      CONST_VEC(dvals),   CONST_IVEC(entszs),          \
                      CONST_BOOL(nlflgs), CONST_IVEC(rcptrs),          \
                      (wkindx)                                      )  ) 


   #define  ekacli_c( handle, segno,  column, ivals, entszs, nlflgs,   \
                      rcptrs, wkindx                                 ) \
                                                                       \
        (   ekacli_c( (handle),           (segno),           (column), \
                      CONST_IVEC(ivals),  CONST_IVEC(entszs),          \
                      CONST_BOOL(nlflgs), CONST_IVEC(rcptrs),          \
                      (wkindx)                                      )  ) 


   #define  el2cgv_c( ellipse, center, smajor, sminor )                \
                                                                       \
        (   el2cgv_c( CONST_ELLIPSE(ellipse), (center),                \
                      (smajor),               (sminor)  )   )


   #define  eul2xf_c( eulang, axisa, axisb, axisc, xform )             \
                                                                       \
        (   eul2xf_c ( CONST_VEC(eulang), (axisa), (axisb), (axisc),   \
                       (xform)                                     )  )
   
   
   #define  getelm_c( frstyr, lineln, lines, epoch, elems )            \
                                                                       \
        (   getelm_c ( (frstyr), (lineln), CONST_VOID(lines),          \
                       (epoch),  (elems)                      )   )
   
   
   #define  inedpl_c( a, b, c, plane, ellipse, found )                 \
                                                                       \
        (   inedpl_c ( (a),                (b),         (c),           \
                       CONST_PLANE(plane), (ellipse),   (found) )   )


   #define  inrypl_c( vertex, dir, plane, nxpts, xpt )                 \
                                                                       \
        (   inrypl_c ( CONST_VEC(vertex),   CONST_VEC(dir),            \
                       CONST_PLANE(plane),  (nxpts),        (xpt) )   )
   

   #define  invert_c( m1, m2 )                                         \
                                                                       \
        (   invert_c ( CONST_MAT(m1), (m2) )   )


   #define  isrot_c( m, ntol, dtol )                                   \
                                                                       \
        (   isrot_c ( CONST_MAT(m), (ntol), (dtol) )   )
        
        
   #define  m2eul_c( r, axis3,  axis2,  axis1,                         \
                        angle3, angle2, angle1 )                       \
                                                                       \
        (   m2eul_c ( CONST_MAT(r), (axis3),  (axis2),  (axis1),       \
                                    (angle3), (angle2), (angle1) )   ) 
                                                                       

   #define  m2q_c( r, q )                                              \
                                                                       \
        (   m2q_c ( CONST_MAT(r), (q) )   )
        
        
   #define  mequ_c( m1, m2 )                                           \
                                                                       \
           ( mequ_c  ( CONST_MAT(m1), m2 ) )
   

   #define  mtxm_c( m1, m2, mout )                                     \
                                                                       \
        (   mtxm_c ( CONST_MAT(m1), CONST_MAT(m2), (mout) )   )


   #define  mtxv_c( m1, vin, vout )                                    \
                                                                       \
        (   mtxv_c ( CONST_MAT(m1), CONST_VEC(vin), (vout) )   )


   #define  mxmt_c( m1, m2, mout )                                     \
                                                                       \
        (   mxmt_c ( CONST_MAT(m1), CONST_MAT(m2), (mout) )   )


   #define  mxm_c( m1, m2, mout )                                      \
                                                                       \
        (   mxm_c ( CONST_MAT(m1), CONST_MAT(m2), (mout) )   )


   #define  mxv_c( m1, vin, vout )                                     \
                                                                       \
        (   mxv_c ( CONST_MAT(m1), CONST_VEC(vin), (vout) )   )


   #define  nearpt_c( positn, a, b, c, npoint, alt )                   \
                                                                       \
        (   nearpt_c ( CONST_VEC(positn), (a),  (b),  (c),             \
                       (npoint),          (alt)            )   )


   #define  npedln_c( a, b, c, linept, linedr, pnear, dist )           \
                                                                       \
        (   npedln_c ( (a),               (b),               (c),      \
                       CONST_VEC(linept), CONST_VEC(linedr),           \
                       (pnear),           (dist)                 )   )


   #define  nplnpt_c( linpt, lindir, point, pnear, dist )              \
                                                                       \
        (   nplnpt_c ( CONST_VEC(linpt), CONST_VEC(lindir),            \
                       CONST_VEC(point), (pnear), (dist )   )   )
        
        
   #define  nvc2pl_c( normal, constant, plane )                        \
                                                                       \
        (   nvc2pl_c ( CONST_VEC(normal), (constant), (plane) )  )


   #define  nvp2pl_c( normal, point, plane )                           \
                                                                       \
        (   nvp2pl_c( CONST_VEC(normal), CONST_VEC(point), (plane) )  )


   #define  oscelt_c( state, et, mu, elts )                            \
                                                                       \
        (   oscelt_c ( CONST_VEC(state), (et), (mu), (elts)  )   )


   #define  pl2nvc_c( plane, normal, constant )                        \
                                                                       \
        (   pl2nvc_c ( CONST_PLANE(plane),  (normal), (constant) )  )


   #define  pl2nvp_c( plane, normal, point )                           \
                                                                       \
        (   pl2nvp_c ( CONST_PLANE(plane),  (normal), (point) )  )


   #define  pl2psv_c( plane, point, span1, span2 )                     \
                                                                       \
        (   pl2psv_c( CONST_PLANE(plane), (point), (span1), (span2) )  )


   #define  psv2pl_c( point, span1, span2, plane )                     \
                                                                       \
        (   psv2pl_c ( CONST_VEC(point),  CONST_VEC(span1),            \
                       CONST_VEC(span2),  (plane)           )   )


   #define  rav2xf_c( rot, av, xform )                                 \
                                                                       \
        (   rav2xf_c ( CONST_MAT(rot), CONST_VEC(av), (xform) )   )
   
   
   #define  raxisa_c( matrix, axis, angle )                            \
                                                                       \
        (   raxisa_c ( CONST_MAT(matrix), (axis), (angle) )   );            

                                
   #define  reccyl_c( rectan, r, lon, z )                              \
                                                                       \
        (   reccyl_c ( CONST_VEC(rectan), (r), (lon), (z)  )   )


   #define  recgeo_c( rectan, re, f, lon, lat, alt )                   \
                                                                       \
        (   recgeo_c ( CONST_VEC(rectan), (re),   (f),                 \
                       (lon),             (lat),  (alt) )   )

   #define  reclat_c( rectan, r, lon, lat )                            \
                                                                       \
        (   reclat_c ( CONST_VEC(rectan), (r), (lon), (lat)  )   )


   #define  recrad_c( rectan, radius, ra, dec )                        \
                                                                       \
        (   recrad_c ( CONST_VEC(rectan), (radius), (ra), (dec)  )   )


   #define  recsph_c( rectan, r, colat, lon )                          \
                                                                       \
        (   recsph_c ( CONST_VEC(rectan), (r), (colat), (lon)  )   )


   #define  rotmat_c( m1, angle, iaxis, mout  )                        \
                                                                       \
        (   rotmat_c ( CONST_MAT(m1), (angle), (iaxis), (mout)  )   )


   #define  rotvec_c( v1, angle, iaxis, vout )                         \
                                                                       \
        (   rotvec_c ( CONST_VEC(v1), (angle), (iaxis), (vout)  )   )


   #define  saelgv_c( vec1, vec2, smajor, sminor )                     \
                                                                       \
        (   saelgv_c ( CONST_VEC(vec1),  CONST_VEC(vec2),              \
                       (smajor),         (sminor)         )   )


   #define  spk14a_c( handle, ncsets, coeffs, epochs )                 \
                                                                       \
        (   spk14a_c ( (handle),           (ncsets),                   \
                       CONST_VEC(coeffs),  CONST_VEC(epochs) )  ) 
   
   
   #define  spkapo_c( targ, et, ref, sobs, abcorr, ptarg, lt )         \
                                                                       \
        (   spkapo_c ( (targ),   (et),    (ref), CONST_VEC(sobs),      \
                       (abcorr), (ptarg), (lt)                   )  )


   #define  spkapp_c( targ, et, ref, sobs, abcorr, starg, lt )         \
                                                                       \
        (   spkapp_c ( (targ),   (et),    (ref), CONST_VEC(sobs),      \
                       (abcorr), (starg), (lt)                   )  )


   #define  spkw02_c( handle, body,   center, frame,  first,  last,    \
                      segid,  intlen, n,      polydg, cdata,  btime )  \
                                                                       \
        (   spkw02_c ( (handle), (body),   (center),         (frame),  \
                       (first),  (last),   (segid),          (intlen), \
                       (n),      (polydg), CONST_VEC(cdata), (btime) ) )


   #define  spkw03_c( handle, body,   center, frame,  first,  last,    \
                      segid,  intlen, n,      polydg, cdata,  btime )  \
                                                                       \
        (   spkw03_c ( (handle), (body),   (center),         (frame),  \
                       (first),  (last),   (segid),          (intlen), \
                       (n),      (polydg), CONST_VEC(cdata), (btime) ) )



   #define  spkw05_c( handle, body,   center, frame,  first,  last,    \
                      segid,  gm,     n,      states, epochs        )  \
                                                                       \
        (   spkw05_c ( (handle),  (body),   (center),   (frame),       \
                       (first),   (last),   (segid),    (gm),          \
                       (n),                                            \
                       CONST_MAT6(states),  CONST_VEC(epochs)    )   )


   #define  spkw08_c( handle, body,   center, frame,  first,  last,    \
                      segid,  degree, n,      states, epoch1, step )   \
                                                                       \
        (   spkw08_c ( (handle),  (body),   (center),   (frame),       \
                       (first),   (last),   (segid),    (degree),      \
                       (n),       CONST_MAT6(states),   (epoch1),      \
                       (step)                                     )   )


   #define  spkw09_c( handle, body,   center, frame,  first,  last,    \
                      segid,  degree, n,      states, epochs       )   \
                                                                       \
        (   spkw09_c ( (handle), (body),   (center), (frame),          \
                       (first),  (last),   (segid),  (degree),  (n),   \
                       CONST_MAT6(states), CONST_VEC(epochs)        )  )


   #define  spkw10_c( handle, body,   center, frame,  first,  last,    \
                      segid,  consts, n,      elems,  epochs       )   \
                                                                       \
        (   spkw10_c ( (handle), (body),  (center), (frame),           \
                       (first),  (last),  (segid),  CONST_VEC(consts), \
                       (n),      CONST_VEC(elems),  CONST_VEC(epochs)) )


   #define  spkw12_c( handle, body,   center, frame,  first,  last,    \
                      segid,  degree, n,      states, epoch0, step )   \
                                                                       \
        (   spkw12_c ( (handle),  (body),   (center),   (frame),       \
                       (first),   (last),   (segid),    (degree),      \
                       (n),       CONST_MAT6(states),   (epoch0),      \
                       (step)                                     )   )


   #define  spkw13_c( handle, body,   center, frame,  first,  last,    \
                      segid,  degree, n,      states, epochs       )   \
                                                                       \
        (   spkw13_c ( (handle), (body),   (center), (frame),          \
                       (first),  (last),   (segid),  (degree),  (n),   \
                       CONST_MAT6(states), CONST_VEC(epochs)        )  )





   #define  spkw15_c( handle, body,   center, frame,  first,  last,    \
                      segid,  epoch,  tp,     pa,     p,      ecc,     \
                      j2flg,  pv,     gm,     j2,     radius         ) \
                                                                       \
        (   spkw15_c ( (handle), (body),  (center), (frame),           \
                       (first),  (last),  (segid),  (epoch),           \
                       CONST_VEC(tp),     CONST_VEC(pa),               \
                       (p),      (ecc),   (j2flg),  CONST_VEC(pv),     \
                       (gm),     (j2),    (radius)                )   )


   #define  spkw17_c( handle, body,   center, frame,  first,  last,    \
                      segid,  epoch,  eqel,   rapol,  decpol       )   \
                                                                       \
        (   spkw17_c ( (handle), (body),  (center), (frame),           \
                       (first),  (last),  (segid),  (epoch),           \
                       CONST_VEC(eqel),   (rapol),  (decpol)  )   )


   #define  stelab_c( pobj, vobj, appobj )                             \
                                                                       \
        (   stelab_c ( CONST_VEC(pobj), CONST_VEC(vobj), (appobj)  )   )


   #define  sumad_c( array, n )                                        \
                                                                       \
        (   sumad_c ( CONST_VEC(array), (n)  )   )


   #define  sumai_c( array, n )                                        \
                                                                       \
        (   sumai_c ( CONST_IVEC(array), (n)  )   )


   #define  surfnm_c( a, b, c, point, normal )                         \
                                                                       \
        (   surfnm_c ( (a), (b), (c), CONST_VEC(point), (normal) )   )


   #define  surfpt_c( positn, u, a, b, c, point, found )               \
                                                                       \
        (   surfpt_c ( CONST_VEC(positn), CONST_VEC(u),                \
                       (a),               (b),               (c),      \
                       (point),           (found)                 )   )


   #define  trace_c( m1 )                                              \
                                                                       \
           ( trace_c ( CONST_MAT(m1) ) )


   #define  twovec_c( axdef, indexa, plndef, indexp, mout )            \
                                                                       \
        (   twovec_c ( CONST_VEC(axdef),  (indexa),                    \
                       CONST_VEC(plndef), (indexp), (mout) )   )


   #define  ucrss_c( v1, v2, vout )                                    \
                                                                       \
        (   ucrss_c ( CONST_VEC(v1), CONST_VEC(v2), (vout) )   )


   #define  unorm_c( v1, vout, vmag )                                  \
                                                                       \
        (   unorm_c ( CONST_VEC(v1), (vout), (vmag) )   )


   #define  unormg_c( v1, ndim, vout, vmag )                           \
                                                                       \
        (   unormg_c ( CONST_VEC(v1), (ndim), (vout), (vmag) )   )


   #define  vadd_c( v1, v2, vout )                                     \
                                                                       \
        (   vadd_c ( CONST_VEC(v1), CONST_VEC(v2), (vout) )   )


   #define  vaddg_c( v1, v2, ndim,vout )                               \
                                                                       \
        (  vaddg_c ( CONST_VEC(v1), CONST_VEC(v2), (ndim), (vout) ) )
   

   #define  vcrss_c( v1, v2, vout )                                    \
                                                                       \
        (   vcrss_c ( CONST_VEC(v1), CONST_VEC(v2), (vout) )   )


   #define  vdist_c( v1, v2 )                                          \
                                                                       \
        (   vdist_c ( CONST_VEC(v1), CONST_VEC(v2) )   )


   #define  vdistg_c( v1, v2, ndim )                                   \
                                                                       \
        (   vdistg_c ( CONST_VEC(v1), CONST_VEC(v2), (ndim) )   )


   #define  vdot_c( v1, v2 )                                           \
                                                                       \
        (   vdot_c ( CONST_VEC(v1), CONST_VEC(v2) )   )


   #define  vdotg_c( v1, v2, ndim )                                    \
                                                                       \
        (   vdotg_c ( CONST_VEC(v1), CONST_VEC(v2), (ndim) )   )


   #define  vequ_c( vin, vout )                                        \
                                                                       \
        (   vequ_c ( CONST_VEC(vin), (vout) )   )


   #define  vequg_c( vin, ndim, vout )                                 \
                                                                       \
        (   vequg_c ( CONST_VEC(vin), (ndim), (vout) )   )


   #define  vhat_c( v1, vout )                                         \
                                                                       \
        (   vhat_c ( CONST_VEC(v1), (vout) )   )


   #define  vhatg_c( v1, ndim, vout )                                  \
                                                                       \
        (   vhatg_c ( CONST_VEC(v1), (ndim), (vout) )   )


   #define  vlcom3_c( a, v1, b, v2, c, v3, sum )                       \
                                                                       \
        (   vlcom3_c ( (a), CONST_VEC(v1),                             \
                       (b), CONST_VEC(v2),                             \
                       (c), CONST_VEC(v3), (sum) )   )


   #define  vlcom_c( a, v1, b, v2, sum )                               \
                                                                       \
        (   vlcom_c ( (a), CONST_VEC(v1),                              \
                      (b), CONST_VEC(v2), (sum) )   )


   #define  vlcomg_c( n, a, v1, b, v2, sum )                           \
                                                                       \
           ( vlcomg_c ( (n), (a), CONST_VEC(v1),                       \
                             (b), CONST_VEC(v2),  (sum) )   )


   #define  vminug_c( v1, ndim, vout )                                 \
                                                                       \
       (   vminug_c ( CONST_VEC(v1), (ndim), (vout) )   )
           
           
   #define  vminus_c( v1, vout )                                       \
                                                                       \
        (   vminus_c ( CONST_VEC(v1), (vout) )   )


   #define  vnorm_c( v1 )                                              \
                                                                       \
        (   vnorm_c ( CONST_VEC(v1) )   )


   #define  vnormg_c( v1, ndim )                                       \
                                                                       \
        (   vnormg_c ( CONST_VEC(v1), (ndim) )   )


   #define  vperp_c( a, b, p )                                         \
                                                                       \
        (   vperp_c ( CONST_VEC(a), CONST_VEC(b), (p) )   )


   #define  vprjp_c( vin, plane, vout )                                \
                                                                       \
        (   vprjp_c ( CONST_VEC(vin), CONST_PLANE(plane), (vout) )   )


   #define  vprjpi_c( vin, projpl, invpl, vout, found )                \
                                                                       \
        (   vprjpi_c( CONST_VEC(vin),     CONST_PLANE(projpl),         \
                      CONST_PLANE(invpl), (vout),           (found) ) )


   #define  vproj_c( a, b, p )                                         \
                                                                       \
        (   vproj_c ( CONST_VEC(a), CONST_VEC(b), (p) )   )


   #define  vrel_c( v1, v2 )                                           \
                                                                       \
           ( vrel_c ( CONST_VEC(v1), CONST_VEC(v2) )   )
   
    
   #define  vrelg_c( v1, v2, ndim )                                    \
                                                                       \
           ( vrelg_c ( CONST_VEC(v1), CONST_VEC(v2), (ndim) )   )


   #define  vrotv_c( v, axis, theta, r )                               \
                                                                       \
        (   vrotv_c ( CONST_VEC(v), CONST_VEC(axis), (theta), (r) )   )


   #define  vscl_c( s, v1, vout )                                      \
                                                                       \
        (   vscl_c ( (s), CONST_VEC(v1), (vout) )   )


   #define  vsclg_c( s, v1, ndim, vout )                               \
                                                                       \
        (   vsclg_c ( (s), CONST_VEC(v1), (ndim), (vout) )   )


   #define  vsep_c( v1, v2 )                                           \
                                                                       \
        (   vsep_c ( CONST_VEC(v1), CONST_VEC(v2) )   )


   #define  vsepg_c( v1, v2, ndim)                                     \
                                                                       \
           ( vsepg_c ( CONST_VEC(v1), CONST_VEC(v2), ndim )  )
   
   
   #define  vsub_c( v1, v2, vout )                                     \
                                                                       \
        (   vsub_c ( CONST_VEC(v1), CONST_VEC(v2), (vout) )   )


   #define  vsubg_c( v1, v2, ndim, vout )                              \
                                                                       \
        (   vsubg_c ( CONST_VEC(v1), CONST_VEC(v2),                    \
                      (ndim),        (vout)            )   )

   #define  vtmv_c( v1, mat, v2 )                                      \
                                                                       \
        ( vtmv_c ( CONST_VEC(v1), CONST_MAT(mat), CONST_VEC(v2) ) )
   
   
   #define  vtmvg_c( v1, mat, v2, nrow, ncol )                         \
                                                                       \
        ( vtmvg_c ( CONST_VOID(v1), CONST_VOID(mat), CONST_VOID(v2),   \
                   (nrow), (ncol)                                   )  )


   #define  vupack_c( v, x, y, z )                                     \
                                                                       \
        (   vupack_c ( CONST_VEC(v), (x), (y), (z) )   )


   #define  vzero_c( v1 )                                              \
                                                                       \
        (   vzero_c ( CONST_VEC(v1) )   )


   #define  vzerog_c( v1, ndim )                                       \
                                                                       \
           (   vzerog_c ( CONST_VEC(v1), (ndim) )   )


   #define  xf2eul_c( xform, axisa, axisb, axisc, eulang, unique )     \
                                                                       \
        (   xf2eul_c( CONST_MAT6(xform), (axisa), (axisb), (axisc),    \
                      (eulang),          (unique)                  )  )  


   #define  xf2rav_c( xform, rot, av )                                 \
                                                                       \
        (   xf2rav_c( CONST_MAT6(xform), (rot), (av) )   )  


   #define  xpose6_c( m1, mout )                                       \
                                                                       \
        (   xpose6_c ( CONST_MAT6(m1), (mout) )   )


   #define  xpose_c( m1, mout )                                        \
                                                                       \
        (   xpose_c ( CONST_MAT(m1), (mout) )   )


#endif


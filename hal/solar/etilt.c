#include <math.h>
void nod(double *t, double *dpsi, double *deps);
void   etilt(double *tjd,double *oblm,double *oblt,double *eqeq,double *dpsi,double *deps)
	{
/*C
C     THIS SUBROUTINE COMPUTES QUANTITIES RELATED TO THE ORIENTATION
C     OF THE EARTH'S ROTATION AXIS AT JULIAN DATE TJD.
C
C          TJD    = TDB JULIAN DATE FOR ORIENTATION PARAMETERS (IN)
C          OBLM   = MEAN OBLIQUITY OF THE ECLIPTIC IN DEGREES AT
C                   DATE TJD (OUT)
C          OBLT   = TRUE OBLIQUITY OF THE ECLIPTIC IN DEGREES AT
C                   DATE TJD (OUT)
C          EQEQ   = EQUATION OF THE EQUINOXES IN SECONDS OF TIME AT
C                   DATE TJD (OUT)
C          DPSI   = NUTATION IN LONGITUDE IN SECONDS OF ARC AT
C                   DATE TJD (OUT)
C          DEPS   = NUTATION IN OBLIQUITY IN SECONDS OF ARC AT
C                   DATE TJD (OUT)
*/


      double  t0,t,t2,t3,tlast,seccon,obm,obt,ee,psi,eps,dabs,dcos;

      t0=2451545.0;

/*C     T0 = TDB JULIAN DATE OF EPOCH J2000.0 */
      seccon=206264.8062470964;
      tlast=0.0;

      if (abs(*tjd-tlast)<1.0e-6)
	{
	 *oblm = obm;
         *oblt = obt;
         *eqeq = ee;
         *dpsi = psi;
         *deps = eps;
	return;
	}

      t = (*tjd - t0) / 36525.0;
      t2 = t * t;
      t3 = t2 * t;

/*C     OBTAIN NUTATION PARAMETERS IN SECONDS OF ARC*/
       nod(&t,&psi,&eps);

/*
printf("t sent to nod=%.14f\n",t);
printf("psi returned from nod=%.14f\n",psi);
printf("eps returned from nod=%.14f\n",eps);
*/

/*C     COMPUTE MEAN OBLIQUITY OF THE ECLIPTIC IN SECONDS OF ARC*/
      obm = 84381.4480 - 46.8150*t - 0.00059*t2 + 0.001813*t3;

/*C     COMPUTE TRUE OBLIQUITY OF THE ECLIPTIC IN SECONDS OF ARC*/
      obt = obm + eps;

/*C     COMPUTE EQUATION OF THE EQUINOXES IN SECONDS OF TIME*/
      ee = psi / 15.0 * cos (obt/seccon);

/*     CONVERT OBLIQUITY VALUES TO DEGREES*/
      obm = obm / 3600.0;
      obt = obt / 3600.0;
      tlast = *tjd;

	*oblm=obm;
      *oblt = obt;
      *eqeq = ee;
      *dpsi = psi;
      *deps = eps;
}

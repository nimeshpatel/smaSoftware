#include <math.h>
void   etilt(double *tjd,double *oblm,double *oblt,double *eqeq,double *dpsi,double *deps);

void sidtim(double *tjdh,double *tjdl,int *k,double *gst)
	{
/*C
C     THIS SUBROUTINE COMPUTES THE GREENWICH SIDEREAL TIME
C     (EITHER MEAN OR APPARENT) AT JULIAN DATE TJDH + TJDL.
C     SEE AOKI, ET AL. (1982) ASTRONOMY AND ASTROPYSICS 105, 359-361.
C
C          TJDH   = JULIAN DATE, HIGH-ORDER PART (IN)
C          TJDL   = JULIAN DATE, LOW-ORDER PART (IN)
C                   JULIAN DATE MAY BE SPLIT AT ANY POINT, BUT
C                   FOR HIGHEST PRECISION, SET TJDH TO BE THE INTEGRAL
C                   PART OF THE JULIAN DATE, AND SET TJDL TO BE THE
C                   FRACTIONAL PART
C          K      = TIME SELECTION CODE (IN)
C                   SET K=0 FOR GREENWICH MEAN SIDEREAL TIME
C                   SET K=1 FOR GREENWICH APPARENT SIDEREAL TIME
C          GST    = GREENWICH (MEAN OR APPARENT) SIDEREAL TIME
C                   IN HOURS (OUT)
C
C
*/

      double tjd,th,tl,t0,t,t2,t3,x,eqeq,st,oblm,oblt,dpsi,deps;

      t0=2451545.0;
/*C     T0 = TDB JULIAN DATE OF EPOCH J2000.0 */

      tjd = *tjdh + *tjdl;
      th = (*tjdh - t0) / 36525.0;
      tl =  *tjdl       / 36525.0;
      t = th + tl;
      t2 = t * t;
      t3 = t2 * t;

/*C     FOR APPARENT SIDEREAL TIME, OBTAIN EQUATION OF THE EQUINOXES */
      eqeq = 0.0;
      if (*k==1)  etilt(&tjd,&oblm,&oblt,&eqeq,&dpsi,&deps);

/*
printf("tjd sent to etilt=%.14f\n",tjd);
printf("eqeq returned from etilt=%.14f\n",eqeq);
*/


      st = eqeq - 6.2e-6*t3 
			+ 0.093104*t2 
			+ 67310.54841 
			+ 8640184.812866 *tl 
			+ 3155760000.0   *tl 
			+ 8640184.812866 *th 
			+ 3155760000.0   *th;

      *gst = fmod (st / 3600.0, 24.0);
      if (*gst < 0.0) *gst = *gst + 24.0;
/*
	printf("gst sent by sidtim=%.14f\n",*gst);
*/
}

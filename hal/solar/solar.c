/*************************************************************************
solar.c
This program calculates the Sun's az and el coordinates for the
SMA site and writes these to ref. mem. for Sun avoidance monitoring.
Nimesh Patel
17 Feb 2000
This version is for the PowerPC Hal9000.
5 Oct 2001
Modified to use Charlie's driver instead of Paul's for reading the
time.
*************************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "novas.h"
#include "site.h"
#include "vme_sg_simple.h"
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "rm.h"

#define DEBUG 0

#define HOURS_PER_DEGREE 6.66666667e-2
#define	DTA          32.184	/* this is delta-T: diff betn TA and UTC */
#define	K            1.657e-3	/* K, EB, M0 and M1 are constants used */
#define	EB     	     1.671e-2	/* in calculations of et */
#define	M0         6.2399960
#define	M1 	     1.99096871e-7

#define SEC_PER_DAY 86400


/************************ Function declarations *************************/
void 
refract(double *el, int radio_flag, double temp,
	double humid, double pres);
short int 
topo_planet(double tjd, short int planet, short int earth,
	    double delta, site_info * location,
	    double *ra, double *dec, double *dis);
/***********************************************************************/

void hupHandler(int signum)
{
printf("Got HUP signal. Ignoring it.\n");
}

void
main()
{

	double tjd_upper,tjd_lower,hr;
	double          ra, dec, lst;
	double          distance;
	double          Ra0, Dec0;
	double          ha, phi, sinphi,cosphi;
	double          cosh, sinh, cosd, sind;
	double          sina, cosa;
	double          pres, temp, humid;
	short           earth = 3, sun = 10;
	site_info       geo_loc = {LATITUDE,LONGITUDE, 0.0};

	double          azv,elv;
	float           az[11], el[11];
	double          pi, secunit, milliarcsec;
	int		device_fd, irig_status,rm_status;
	int		i,antlist[RM_ARRAY_SIZE];
	double          tjd, delta;
	
	int hours,minutes,year,day;
	double seconds,museconds;
	struct vme_sg_simple_time ts;


	/* Variables for LST calculation */
	double          d1, dtupi, radian, gst;
	int             e1 = 1;	/* e1=1: apparent gst, e1=0: mean gst; in
				 * sidtim.c */
	double          c1 = 0.0;	/* this is the lower part of tjd
					 * input to sidtim.c */


	struct sigaction sa;

	/* end of variable declarations */
	
	sa.sa_handler = hupHandler; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL)) perror("sigaction");

	pi = 4.0 * atan(1.0);
	dtupi = pi * 2.0;
	radian = pi / 180.;

	secunit = 1.15740741e-5;/* day in one second */
	milliarcsec = 180. / pi * 3600000.;
	phi = LATITUDE * radian; /* convert latitude to radians */

	/* starting infinite loop */
	

	
             
	while (1) {

		/*-------------------------------time-------------------------*/

	device_fd=open("/dev/vme_sg_simple",O_RDWR);
	if(device_fd==-1) {
        perror("open()");
        exit(-1);
	}

	irig_status = read(device_fd,&ts,1);
	if(irig_status==-1) {
        perror("read()");
        exit(-1);
	}

	close(device_fd);

	hours = ts.hour;
	minutes = ts.min;
	seconds = (double) ts.sec;
	museconds = (double) ts.usec;
	seconds += (museconds/1.0e6);

	hr=(double)hours+(double)minutes/60.+seconds/3600.;

/* calculating julian day */
           tjd_upper = (double)((int)(365.25*(ts.year+4711))) +
                        (double)ts.yday+ 351.5;

           tjd_lower=hr/24.;

           tjd=tjd_upper+tjd_lower;


#if DEBUG
		printf("tjd=%lf\n",tjd);
#endif

		sidtim(&tjd, &c1, &e1, &gst);
		d1 = (gst + LONGITUDE*HOURS_PER_DEGREE) * dtupi / 24.;
		lst = fmod(d1, dtupi);
		if (lst < 0.) {
			lst += dtupi;
		}

		delta=0.;
		topo_planet(tjd, sun, earth, delta, &geo_loc, &Ra0, &Dec0, &distance);

		
#if DEBUG
		  printf("Got ra,dec: %lf %lf\n",Ra0,Dec0);
#endif
		 

		ra = Ra0 * 15.0 * pi / 180.;
		dec = Dec0 * pi / 180.;


		ha = lst - ra;
		cosh = cos(ha);
		sinh = sin(ha);
		cosd = cos(dec);
		sind = sin(dec);
		sinphi = sin(phi);
		cosphi = cos(phi);


		/* azimuth and elevation   */

		sina = -cosd * sinh;

		cosa = sind * cosphi - cosd * cosh * sinphi;

		elv = asin(sind * sinphi + cosd * cosphi * cosh);

		if (cosa == 0.0) {
			azv = 0.0;
		} else {
			azv = atan2(sina, cosa);
		}

		if (azv < 0.0) {
			azv = dtupi + azv;
		}
		/* refraction correction   */

		/* fiducial values for weather parameters */

		pres = 700.0;
		temp = 5.0;
		humid = 50.0;


		if(elv>0.) refract(&elv, 0, temp, humid, pres);

		azv = azv / radian;
		elv = elv / radian;

#if DEBUG
		printf("az=%lf el=%lf \n",azv,elv);
#endif

		for(i=0;i<11;i++)
		{
		az[i]=(float)azv;
		el[i]=(float)elv;
		}


	rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

		rm_status=rm_write(RM_ANT_ALL,"RM_SUN_AZ_DEG_F", az);
		rm_status=rm_write(RM_ANT_ALL,"RM_SUN_EL_DEG_F", el);

	rm_close();

		sleep(60);
	}			/* while(1) */

}				/* main */

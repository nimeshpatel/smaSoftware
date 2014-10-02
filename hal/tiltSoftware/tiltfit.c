/* Using svdfit to fit the tiltmeter data */
/* Nimesh A. Patel 12 Jan 98		 */
/* modified on 18 may 2000 to make it work on sis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nr.h"
#include "nrutil.h"

#define NP 7
#define MAX 20000

void            tiltfn();

float           az[MAX], el[MAX], del[MAX], daz[MAX];
float           tilt[7][MAX];

void
main(int argc, char *argv[])
{
#define NAMELEN 100
    char filename[NAMELEN];
    char fitdataFilename[NAMELEN];
    char fitresultsFilename[NAMELEN];
    float           scale_factor[6];
    int             i, idum = (-911);
    float           chisq, *x, *y, *sig, *a, *w, **cvm, **u, **v, *azres,
                   *elres;
    int             npt, tilt_number;
    FILE           *fpin, *fpdata, *fpresults;
    char           *time[20];
    float           theta, radian, tilt_minus_dc, theoretical, residual;
    static float    pi;
	float utc;
	float magnitude, angle ;
	float sunaz,sunel,cabintemp,ambienttemp,windspeed,winddir;
	int antenna, pad;

    pi = 4.0 * atan(1.0);
    radian = pi / 180.;
    if (argc < 3)
    {
	printf("Usage:\n tiltfit <filename> <tiltmeter #>\n");
	exit(0);
    }

    strcpy(filename, argv[1]);
    tilt_number = atoi(argv[2]);
    fpin = fopen(filename, "r");
    if (fpin == NULL) {
      printf("Could not open input datafile = %s\n",filename);
      exit(0);
    }
    strcpy(fitresultsFilename,"/data/engineering/tilt/fitresults");
    fpresults = fopen(fitresultsFilename, "w");
    if (fpresults == NULL) {
      printf("Could not open fitresults output file = %s\n",fitresultsFilename);
      exit(0);
    }
    chmod(fitresultsFilename,0666);
    strcpy(fitdataFilename,"/data/engineering/tilt/fitdata");
    fpdata = fopen(fitdataFilename, "w");
    if (fpdata == NULL) {
      printf("Could not open fitdata output file = %s\n",fitdataFilename);
      exit(0);
    }
    chmod(fitdataFilename,0666);

    i = 1;

    while
	(fscanf(fpin, "%s %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f",
	time, &antenna, &pad, &utc,&az[i], &el[i], &tilt[1][i], &tilt[2][i]
	,&tilt[3][i],&tilt[4][i],&sunaz,&sunel,&cabintemp,&ambienttemp,
		&windspeed,&winddir) != EOF)
    {
	i++;
    }
    fclose(fpin);

    npt = i - 1;
    x = vector(1, npt);
    y = vector(1, npt);
    sig = vector(1, npt);
    azres = vector(1, npt);
    elres = vector(1, npt);
    a = vector(1, NP);
    w = vector(1, NP);
    cvm = matrix(1, NP, 1, NP);
    u = matrix(1, npt, 1, NP);
    v = matrix(1, NP, 1, NP);

    for (i = 1; i <= npt; i++)
    {
	x[i] = (float) i;
	y[i] = (float) tilt[tilt_number][i];
    }

    svdfit(x, y, sig, npt, a, NP, u, v, w, &chisq, tiltfn, azres);
    svdvar(v, NP, w, cvm);

	magnitude =  (float)pow(((double)a[2]*(double)a[2]+(double)a[3]*(double)a[3]),0.5);
        angle=(float)atan2((double)a[2],(double)a[3]);
	angle=angle/radian;

/*
    fprintf(fpresults,"Filename: %s\n",filename);
    fprintf(fpresults, "Tiltmeter %d, %s, Elevation=%.1f deg\n", tilt_number, time, el[1]);
    fprintf(fpresults, "Tilt = %.1f\" towards azimuth = %.1f degrees\n", magnitude, angle);
    fprintf(fpresults, "DC       %6.2f %s %6.2f \"\n", a[1], "  +-", sqrt(cvm[1][1]));
    fprintf(fpresults, "Sin(az)  %6.2f %s %6.2f \"\n", a[2], "  +-", sqrt(cvm[2][2]));
    fprintf(fpresults, "Cos(az)  %6.2f %s %6.2f \"\n", a[3], "  +-", sqrt(cvm[3][3]));
    fprintf(fpresults, "Sin(2az) %6.2f %s %6.2f \"\n", a[4], "  +-", sqrt(cvm[4][4]));
    fprintf(fpresults, "Cos(2az) %6.2f %s %6.2f \"\n", a[5], "  +-", sqrt(cvm[5][5]));
    fprintf(fpresults, "Sin(3az) %6.2f %s %6.2f \"\n", a[6], "  +-", sqrt(cvm[5][5]));
    fprintf(fpresults, "Cos(3az) %6.2f %s %6.2f \"\n", a[7], "  +-", sqrt(cvm[5][5]));
    fprintf(fpresults, "rms %6.2f \"\n", pow((chisq / (npt - 1)), 0.5));
*/

	fprintf(fpresults,"%s %s %d %d %d %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %12.2f %12.2f\n", 
	filename,time,antenna,pad,tilt_number,el[1],sunaz,sunel,cabintemp,ambienttemp,windspeed,winddir,magnitude,angle,
	a[1],sqrt(cvm[1][1]),
	a[2],sqrt(cvm[2][2]),
	a[3],sqrt(cvm[3][3]),
	a[4],sqrt(cvm[4][4]),
	a[5],sqrt(cvm[5][5]),
	a[6],sqrt(cvm[5][5]),
	a[7],sqrt(cvm[5][5]),
	pow((chisq / (npt - 1)), 0.5),chisq/(npt-NP));

	printf("%s %s %d %d %d %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f %12.2f %12.2f\n", 
	filename,time,antenna,pad,tilt_number,el[1],sunaz,sunel,cabintemp,ambienttemp,windspeed,winddir,magnitude,angle,
	a[1],sqrt(cvm[1][1]),
	a[2],sqrt(cvm[2][2]),
	a[3],sqrt(cvm[3][3]),
	a[4],sqrt(cvm[4][4]),
	a[5],sqrt(cvm[5][5]),
	a[6],sqrt(cvm[5][5]),
	a[7],sqrt(cvm[5][5]),
	pow((chisq / (npt - 1)), 0.5),chisq/(npt-NP));

    for (i = 1; i <= npt; i++)
    {
	theta = az[i] * radian;
	tilt_minus_dc = tilt[tilt_number][i] - a[1];
	theoretical = a[2] * sin(theta) + a[3] * cos(theta) + a[4] * sin(2 * theta) + a[5] * cos(2 * theta) 
	+ a[6] * sin(3 * theta) + a[7] * cos(3 *
								     theta);
	residual = tilt_minus_dc - theoretical;
	fprintf(fpdata, "%.2f %.2f %.2f %.2f\n", az[i], tilt_minus_dc, theoretical, residual);
    }

    fclose(fpin);
    fclose(fpdata);
    fclose(fpresults);

    free_matrix(v, 1, NP, 1, NP);
    free_matrix(u, 1, npt, 1, NP);
    free_matrix(cvm, 1, NP, 1, NP);
    free_vector(w, 1, NP);
    free_vector(a, 1, NP);
}

/* This code is for 2-D interpolation with random, but monotonic, grid points.
   the grid points are x,y with z being the value at each (x,y). In our application,
   (x,y) is (source size, scanwdith) and z is the beam size. 

   We use a grid of (sourcesize, scanwidth) for various beam sizes generated using
   matlab. The matlab code convolved a gaussian beam of width beamsize with a tophat
   of size sourcesize in 2-D. Gaussain fits were made to the 1-D middle scan, passing 
   throuhg the peak of the resulting 2-D convolution. The width of the fitted gaussian 
   is the "scanwdith". This simulates the 1-D planet scans and gaussian fits made with 
   the SMA rpoint. The scheme here is for obtaining a beam size from such a scanwidth 
   and the source size, for aperture efficiecny calculations. In principle it is possible 
   to make this process more general by including non-circular sources (not top-hat) and
   the position angle of the scan. 

   Original version, 06 May 2005, TK.

*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>

/* void beamInterpol(int argc, char *argv[]) */
void main(int argc, char *argv[])

{
        FILE *fpi, *fpo;
	float x[755], y[755], z[755];
	float x0,y0,z0,x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4;
	float d1,d2,d3,d4, tmp;
	int i,lx,mx,hx,ly,my,hy;
	        if ( argc < 5) {
                printf("usage: beamInterpol in_file sourceSize scanWidth out_file \n");
                exit(1);
        }
	x0=atof(argv[2]);
	y0=atof(argv[3]);
	fpi = fopen(argv[1], "r");
	for (i=0; i <= 749; i++){
		fscanf(fpi, "%f %f %f %f %f %f\n", x+i, x+i, z+i, y+i, y+i, y+i);
	}
	printf("Input (x,y): %f %f\n", x0,y0);
/*	for (i=0; i<= 749; i++){
		printf("%d %lf %lf %lf \n", i, x[i], y[i], z[i]);
	}
*/
/* first do a binary search for x0
   an array size of 750 and 25 different values for x assumed */
	lx=0;
	hx=749;
	while ( (hx-lx) >= 25 ) {
		mx=(int)((lx+hx)/2);
/*		printf("x search: %d %d %d %f %f %f\n",lx,mx,hx,x[lx],x[mx],x[hx]); */
		if(x0 == x[mx]) {
			lx=mx;
			hx=mx;
		}
		if(x0 > x[mx]) lx=mx;
		if(x0 < x[mx]) hx=mx;
	}
/* Now come up to start of x0 */
	lx=lx-26;
	while (x[lx] < x0 ) lx++;
/*	printf ("%d %f\n", lx,x[lx]); */
	if (x[lx+1] > x0 ) lx=lx-25;
/*	printf ("%d %f\n", lx,x[lx]); */
/* Binary search in y */
	ly=lx;
	hy=lx+24;
        while ( (hy-ly) > 1 ) {
                my=(int)((ly+hy)/2);
/*		printf("y search: %d %d %d %f %f %f %f\n",ly,my,hy,y[ly],y[my],y[hy],y0); */
/*		printf("diff: %lf\n", y0-y[my]); */
                if ( y[my] == y0 ){
			ly=my;
			hy=my;
/*			printf("exact %d %d \n", ly,hy); */
                }
                if(y0 > y[my]) ly=my;
                if(y0 < y[my]) hy=my; 
	}
  	x1=x[ly];
	y1=y[ly];
	z1=z[ly];
	x2=x[hy];
	y2=y[hy];
	z2=z[hy];
/*	printf("%f %f %d %d %f %f %f %f %f %f\n", x0,y0,ly,hy,x1,y1,z1,x2,y2,z2); */
/* Now look for the other two corners of the "grid square" 
   the sequence of corners start from lower left and runs clockwise */
	lx=hy;
        while (x[lx] <= x0 ) lx++;
/*        printf ("%d %f\n", lx,x[lx]); */
/*        printf ("%d %f\n", lx,x[lx]); */
/* Binary search in y */
        ly=lx;
        hy=lx+24;
        while ( (hy-ly) > 1 ) {
                my=(int)((ly+hy)/2);
/*                printf("y3 search: %d %d %d %f %f %f %f\n",ly,my,hy,y[ly],y[my],y[hy],y0); */
                if(y[my] == y0) {
                        ly=my;
                        hy=my;
                } 
                if(y0 > y[my]) ly=my;
                if(y0 < y[my]) hy=my;
        }
	x3=x[ly];
        y3=y[ly];
        z3=z[ly];
        x4=x[hy];
        y4=y[hy];
        z4=z[hy];
/*        printf("%f %f %d %d %f %f %f %f %f %f\n", x0,y0,ly,hy,x3,y3,z3,x4,y4,z4); */
/*  Now compute the distance weighted sum of the corner values as the interpolated value */
	d1=pow((pow((x1-x0),2)+pow((y1-y0),2)),0.5);
	d2=pow((pow((x2-x0),2)+pow((y2-y0),2)),0.5);
	d3=pow((pow((x3-x0),2)+pow((y3-y0),2)),0.5);
	d4=pow((pow((x4-x0),2)+pow((y4-y0),2)),0.5);
	if (d1 == 0.0) z0=z1;
	else if (d2 == 0.0) z0=z2;
	else if (d3 == 0.0) z0=z3;
	else if (d4 == 0.0) z0=z4;
	else
	z0=(z1/d1+z2/d2+z3/d3+z4/d4)/(1/d1+1/d2+1/d3+1/d4);
	printf ("grid square    : \n \
		(x0, y0) Input  : %.1f %.1f \n \
		(x1, y1, z1, d1): %.1f %.1f %.1f %.2f \n \
		(x2, y2, z2, d2): %.1f %.1f %.1f %.2f \n \
		(x3, y3, z3, d3): %.1f %.1f %.1f %.2f \n \
		(x4, y4, z4, d4): %.1f %.1f %.1f %.2f \n ", \
		x0,y0,x1,y1,z1,d1,x2,y2,z2,d2,x3,y3,z3,d3,x4,y4,z4,d4);
	printf ("interpolated   :\n \
		(x0, y0, z0,   ): %.1f %.1f %.1f \n", x0,y0,z0);
	fpo = fopen(argv[4], "w");
	fprintf (fpo, "%.1f %.1f %.1f : interpolated z0; x0,y0\n", z0, x0,y0);
	fclose(fpo);
	fclose(fpi);
}

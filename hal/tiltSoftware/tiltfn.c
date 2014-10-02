#include <math.h>
extern float az[10000];
void tiltfn(x,p,np)
float x,p[];
int np;
{
	int i;
	static float pi;
	float radian;

	pi=4.0*atan(1.0);
	radian=pi/180.;

	i=(int)x;
	p[1]=1.0;
	p[2]=sin(az[i]*radian);
	p[3]=cos(az[i]*radian);
	p[4]=sin(2.0*az[i]*radian);
	p[5]=cos(2.0*az[i]*radian);
	p[6]=sin(3.0*az[i]*radian);
	p[7]=cos(3.0*az[i]*radian);
/*
	printf("%d %f %f %f %f\n",i,az[i],p[1],p[2],p[3]);
*/

}


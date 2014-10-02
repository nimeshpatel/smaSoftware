#include <math.h>
extern double radian;
double sunDistance(double az1,double el1,double az2,double el2)
{

double cosd,sind,d;
cosd=sin(el1)*sin(el2)+cos(el1)*cos(el2)*cos(az1-az2);
sind=pow((1.0-cosd*cosd),0.5);
d=atan2(sind,cosd);
printf("from sunDistance d=%f radian=%f\n",d,radian);
/*
d=d/radian;
*/
return d;
}


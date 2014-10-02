/****************************************************/
/*      CALUCULATE RMS                              */
/****************************************************/
#include <math.h>
#include <stdio.h>

float rms(float *pres,int k)
{
	int j;
        float tot2=0.0,tot=0.0;
	for(j=0;j<k;j++){
                tot2=tot2+(*pres)*(*pres);
                tot =tot+(*pres);
		pres++;
	}
printf("bunsan = %f\n", tot2/k-(tot/k)*(tot/k));	
printf("root = %f\n", sqrt(tot2/k-(tot/k)*(tot/k)));	
	return sqrt(tot2/k-(tot/k)*(tot/k));
}


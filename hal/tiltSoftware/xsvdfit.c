/* Driver for routine SVDFIT */

#include <stdio.h>
#include <math.h>
#include "nr.h"
#include "nrutil.h"

#define NPT 58
#define SPREAD 1 
#define NPOLAZ 7
#define NPOLEL 6
#define RAD 0.0174532925

void azfn();
void elfn();

float az[1000],el[1000],del[1000],daz[1000];

main()
{
	int i,idum=(-911);
	char *source[20];
	float chisq,*x,*y,*sig,*a,*w,**cvm,**u,**v,*azres,*elres;
	FILE *fpin,*fpout,*fpresults;	
	fpin=fopen("pdata","r");
	fpout=fopen("presiduals","w");
	fpresults=fopen("presults","w");
	

	x=vector(1,NPT);
	y=vector(1,NPT);
	sig=vector(1,NPT);
	azres=vector(1,NPT);
	elres=vector(1,NPT);
	a=vector(1,NPOLAZ);
	w=vector(1,NPOLAZ);
	cvm=matrix(1,NPOLAZ,1,NPOLAZ);
	u=matrix(1,NPT,1,NPOLAZ);
	v=matrix(1,NPOLAZ,1,NPOLAZ);
	for (i=1;i<=NPT;i++) {
		x[i]=(float)i;
		fscanf(fpin,"%s %f %f %f %f",source,&az[i],&el[i],&daz[i],&del[i]);
		/*printf("%f %f %f %f\n",az[i],el[i],daz[i],del[i]);*/
	y[i]=daz[i];
/*		y[i] *= (1.0+SPREAD*gasdev(&idum));
		sig[i]=y[i]*SPREAD;
*/

	}
	fclose(fpin);
	svdfit(x,y,sig,NPT,a,NPOLAZ,u,v,w,&chisq,azfn,azres);
	svdvar(v,NPOLAZ,w,cvm);
	printf("\nresults of az fit:\n\n");
	for (i=1;i<=NPOLAZ;i++)
		printf("%12.2f %s %10.2f\n",a[i],"  +-",sqrt(cvm[i][i]));
	printf("\nChi-squared %12.2f\n",chisq);

	fprintf(fpresults,"\nresults of az fit:\n\n");
	for (i=1;i<=NPOLAZ;i++)
	fprintf(fpresults,"%12.2f %s %10.2f\n",a[i],"  +-",sqrt(cvm[i][i]));
	fprintf(fpresults,"\nChi-squared %12.2f\n",chisq);


	free_matrix(v,1,NPOLAZ,1,NPOLAZ);
	free_matrix(u,1,NPT,1,NPOLAZ);
	free_matrix(cvm,1,NPOLAZ,1,NPOLAZ);
	free_vector(w,1,NPOLAZ);
	free_vector(a,1,NPOLAZ);

	a=vector(1,NPOLEL);
	w=vector(1,NPOLEL);
	cvm=matrix(1,NPOLEL,1,NPOLEL);
	u=matrix(1,NPT,1,NPOLEL);
	v=matrix(1,NPOLEL,1,NPOLEL);
	for(i=1;i<=NPT;i++) {
		y[i]=del[i];
		}

	svdfit(x,y,sig,NPT,a,NPOLEL,u,v,w,&chisq,elfn,elres);
	svdvar(v,NPOLEL,w,cvm);

	printf("\nresults of el fit:\n\n");
	for (i=1;i<=NPOLEL;i++)
	printf("%12.2f %s %10.2f\n",a[i],"  +-",sqrt(cvm[i][i]));
	printf("\nChi-squared %12.2f\n",chisq);

	fprintf(fpresults,"\nresults of el fit:\n\n");
	for (i=1;i<=NPOLEL;i++)
	fprintf(fpresults,"%12.2f %s %10.2f\n",a[i],"  +-",sqrt(cvm[i][i]));
	fprintf(fpresults,"\nChi-squared %12.2f\n",chisq);

	

	for (i=1;i<=NPT;i++){
	fprintf(fpout,"%f %f\n",azres[i],elres[i]);
	} 

	fclose(fpout);
	fclose(fpresults);
	fclose(fpin);


	free_matrix(v,1,NPOLEL,1,NPOLEL);
	free_matrix(u,1,NPT,1,NPOLEL);
	free_matrix(cvm,1,NPOLEL,1,NPOLEL);
	free_vector(w,1,NPOLEL);
	free_vector(a,1,NPOLEL);
	free_vector(sig,1,NPT);
	free_vector(y,1,NPT);
	free_vector(x,1,NPT);
	free_vector(azres,1,NPT);
	free_vector(elres,1,NPT);
}

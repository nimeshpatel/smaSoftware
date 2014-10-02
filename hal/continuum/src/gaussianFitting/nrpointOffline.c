#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
#include <termio.h>
*/
#include "cpgplot.h"
#include "nr.h"

void nrpoint(float x[],float y[],float azy[],float ely[],float azmod[],float elmod[],float sig[],int ndata,int num_gauss,int flag,int ant_num,int plotflag,char
*header)
{
	FILE *fp1,*fp2,*fp3,*fp4,*fp5;
	float rms(float *,int);
	float arg, guessed_parameters,xmin,xmax,ymin,ymax,tmp,rms_fac;
	float alamda,chisq,ochisq,**covar,**alpha,*a;
	int i,*ia,itst,j,k,l,numplot,i_maxy,i_miny,MA, NPT;
	char ans[200],f_line[200],c;
	char file_n1[60],file_n2[60],file_n3[60],file_n4[60],file_n5[60];
	char xtitle[60],ytitle[60],title[60],plotant[10];
	float xx[1600],yy[1600],yyy[1600],res[1600];

	sprintf(file_n1,"/usr/PowerPC/common/data/rpoint/offline/load.fitted.dat");
	sprintf(file_n2,"/usr/PowerPC/common/data/rpoint/offline/load.initial.dat");
	sprintf(file_n3,"/usr/PowerPC/common/data/rpoint/offline/load.temp.dat");
	sprintf(file_n4,"/usr/PowerPC/common/data/rpoint/offline/load.results.dat");
	sprintf(file_n5,"/usr/PowerPC/common/data/rpoint/offline/rpoint.ant%1d",ant_num);

	if ((fp1=fopen(file_n1,"w"))==NULL){
		printf("cannot open %s\n",file_n1);
		exit(1);
		}
	if ((fp3=fopen(file_n3,"w"))==NULL){
		printf("cannot open %s\n",file_n3);
		exit(1);
		}
	if ((fp4=fopen(file_n4,"a"))==NULL){
		printf("cannot open %s\n",file_n4);
		exit(1);
		}
	if ((fp5=fopen(file_n5,"a"))==NULL){
		printf("cannot open %s\n",file_n5);
		exit(1);
		}

	NPT=ndata;MA=num_gauss;
	printf("number of data = %d number of fitting components = %d flag = %d\n", NPT,MA/5,flag);

	ia=ivector(1,MA);
	a=vector(1,MA);
	covar=matrix(1,MA,1,MA);
	alpha=matrix(1,MA,1,MA);
	
/* read data */
	xmin=1e6;ymin=1e6;
	xmax=-1e6;ymax=-1e6;
	for (i=1;i<=NPT;i++) {
		xx[i-1]=x[i];
		yy[i-1]=y[i];
		if(xmin>=x[i]) xmin=x[i];
		if(xmax<x[i]) xmax=x[i];
		if(ymin>=y[i]){ymin=y[i];i_miny=i;}
		if(ymax<y[i]) {ymax=y[i];i_maxy=i;}
if(i<10)	printf("%d %f %f %f %f %f %f\n",i,x[i],y[i],ymin,ymax,azy[i],ely[i],sig[i]);
	fprintf(fp3,"%f %f\n",x[i],y[i]);
	}

	tmp=ymax-ymin;
	ymax=tmp*0.2+ymax;
	ymin=ymin-tmp*0.2;
	fclose(fp3);

/*    PGPLOT */
	sprintf(plotant,"%d/xs",(ant_num+10));
	if(plotflag){
	if(cpgbeg(0,plotant,1,1)!=1) exit(1);
	cpgenv(xmin,xmax,ymin,ymax,0,0); 
	cpgpt(NPT,xx,yy,2);
	cpgline(NPT,xx,yy);
	cpgend();
	}
	
/*	initial values of parameters::::::	*/

	if ((fp2=fopen(file_n2,"w"))==NULL){
		printf("cannot open %s\n",file_n2);
		exit(1);
	}

	fprintf(fp2,"%f\n",ymax-ymin);
	fprintf(fp2,"%f\n",x[i_maxy]);
	fprintf(fp2,"%f\n",20.0);
	fprintf(fp2,"%f\n",0.0);
	fprintf(fp2,"%f\n",y[1]);
	fclose(fp2);

	if ((fp2=fopen(file_n2,"r"))==NULL){
		printf("cannot open %s\n",file_n2);
		exit(1);
	}

	for(i=1;i<=MA;i++) 
	{
	fscanf(fp2,"%f\n",&guessed_parameters);
	a[i]=guessed_parameters;
	ia[i]=i;
	}

	fclose(fp2);

/*      start fitting	*/ 
		alamda = -1;
		mrqmin(x,y,sig,NPT,a,ia,MA,covar,alpha,&chisq,fgauss2,&alamda);
		k=1;
		itst=0;
		for (;;) {
			printf("\n%s %2d %17s %9.3e %10s %9.3e\n","Iteration #",k, "chi-squared:",chisq,"alamda:",alamda);
			for (i=1;i<=MA;i++) printf("%5.3e ",a[i]);
			printf("\n");
			k++;

			ochisq=chisq;
			mrqmin(x,y,sig,NPT,a,ia,MA,covar,alpha,&chisq,fgauss2,&alamda);
			if (chisq > ochisq)
				itst=0;
			else if ((fabs(ochisq-chisq) < 0.01 &&
fabs(chisq) < .1) || (k>10))
				{itst++;}
			if (itst < 4) continue;

			if ((fp2=fopen(file_n2,"w"))==NULL){
				printf("cannot open %s\n",file_n2);
				exit(1);
			}

			for (i=1;i<=MA;i++) fprintf(fp2,"%f\n",a[i]);
			for (i=1;i<=MA;i++) fprintf(fp4,"%f ",a[i]);
			fprintf(fp4,"%f ",chisq);
			printf("%f\n ",chisq);
			fprintf(fp2,"\n");
			fprintf(fp4,"\n\n");
	fclose(fp2);

	for(j=1;j<=NPT;j++){
		yyy[j-1]=0.0;
		for(k=1;k<=MA;k+=5){
			arg=(x[j]-a[k+1])/a[k+2];
       			yyy[j-1]+=a[k]*exp(-arg*arg)+a[k+3]*x[j]+a[k+4];
	}
	res[j-1]=y[j]-yyy[j-1]+a[5];
	fprintf (fp1,"%.6f %.6f %.6f %.6f\n",x[j],y[j],yyy[j-1],res[j-1]);
}
	fclose(fp1);

			alamda=0.0;
			mrqmin(x,y,sig,NPT,a,ia,MA,covar,alpha,&chisq,fgauss2,&alamda);
			rms_fac=rms(res,NPT);
			printf("\nUncertainties:\n");
			for (i=1;i<=MA;i++) printf("%8.4e ",rms_fac*sqrt(covar[i][i]));
			printf("\n");

			fprintf(fp4,"\nUncertainties:\n");
			for (i=1;i<=MA;i++) fprintf(fp4,"%8.4e ",rms_fac*sqrt(covar[i][i]));
			fprintf(fp4,"\n");
			break;
	}
fclose(fp4);

	if(flag){
		sprintf(title,"Ant %1d  AZ scan",ant_num);
		sprintf(xtitle,"Azoff (arcsec)");
	}
	else{
		sprintf(title,"Ant %1d  El scan",ant_num);
		sprintf(xtitle,"Eloff (arcsec)");
	}
	sprintf(ytitle,"Intensity (Volts)");

/*    PGPLOT */
	sprintf(title,"Ant %1d ",ant_num);
	sprintf(plotant,"%d/xs",(ant_num+10));
	if(plotflag){
	if(cpgbeg(0,plotant,1,1)!=1) exit(1);
	cpgenv(xmin,xmax,ymin,ymax,0,0); 
	cpgpt(NPT,xx,yy,2);
	cpgline(NPT,xx,yyy);
	cpgpt(NPT,xx,res,-1);
	cpglab(xtitle,ytitle,title);
	sprintf(f_line,"az= %10.4f\n",azy[i_maxy]);
	cpgmtxt("t",-2.5,0.05,0,f_line);
	sprintf(f_line,"el = %10.4f\n",ely[i_maxy]);
	cpgmtxt("t",-4.0,0.05,0,f_line);
	sprintf(f_line,"y= %10.4f\n",a[1]);
	cpgmtxt("t",-7.0,0.05,0,f_line);
	sprintf(f_line,"x = %10.4f\n",a[2]);
	cpgmtxt("t",-5.5,0.05,0,f_line);
	sprintf(f_line,"width = %10.4f\n",a[3]*2*0.83255);
	cpgmtxt("t",-8.5,0.05,0,f_line);
	sprintf(f_line,"chisq = %10.4e\n",chisq);
	cpgmtxt("t",-10.0,0.05,0,f_line);
	cpgend();
	}

	printf("recorded! \n");
	printf("azfit %s  | %10.5f %10.5f %10.5f %10.4f %10.2f +- %10.2f %8.1f %8.1f\n",header,azy[i_maxy],ely[i_maxy],a[1],a[3]*2*0.83255,a[2],rms_fac*sqrt(covar[2][2]),azmod[i_maxy],elmod[i_maxy]);
	if(flag==1) fprintf(fp5,"azfit %s  | %10.5f %10.5f %10.5f %10.4f %10.2f +- %10.2f %8.1f %8.1f\n",header,azy[i_maxy],ely[i_maxy],a[1],a[3]*2*0.83255,a[2],rms_fac*sqrt(covar[2][2]),azmod[i_maxy],elmod[i_maxy]);
	else fprintf(fp5,"elfit %s | %10.5f %10.5f %10.5f %10.4f %10.2f +- %10.2f %8.1f %8.1f \n",header,azy[i_maxy],ely[i_maxy],a[1],a[3]*2*0.83255,a[2],rms_fac*sqrt(covar[2][2]),azmod[i_maxy],elmod[i_maxy]);

	fclose(fp5);
	free_matrix(alpha,1,MA,1,MA);
	free_matrix(covar,1,MA,1,MA);
	free_ivector(ia,1,MA);
	free_vector(a,1,MA);
}


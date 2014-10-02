#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <rpc/rpc.h>
#include <pthread.h>
#include "/usr/PowerPC/applications/hal/continuum/includeFiles/chartu.h"
#include "rm.h"
#include "smapopt.h"
#define POSIX_THREADS_CALLS 
/************************************************************************
 *
 *  File Name: chartuBeam.c
 *  written by M. Saito 000709
 *  modified by M. Saito 010219 command arguments, pthread
 *  modified by M. Saito 010507 for 3.1.0 change
 *  tentative single dish radio pointing with continuum detectors
 *
 *  modified by S. Matsushita 011104 for beam pattern measurements
 *  modified by S. Matsushita 020516 for recent directory configuration
 *                                         change
 *
 * How to compile this program :
  cd /usr/PowerPC/applications/hal/continuum/src
  lynxos (for the first time)
  make 
 *
  parameter file : /common/reflmem/rm_allocation
RM_AZ_TRACKING_ERROR_F
RM_EL_TRACKING_ERROR_F
RM_ACTUAL_AZ_DEG_F
RM_ACTUAL_EL_DEG_F
RM_PMDAZ_F
RM_PMDEL_F
RM_AZOFF_D# Azoff in arcseconds (true angle on sky, double)
RM_ELOFF_D# Eloff in arcseconds (short,double)
RM_SYNCDET_VOLTS_D
RM_TOTAL_POWER_VOLTS_D
 *
 ************************************************************************/

void *write_status(), *read_status();
void usage(int exitcode, char *error, char *addl);
pthread_t write_statusTID, read_statusTID;

void usage(int exitcode, char *error, char *addl) {

	if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
	fprintf(stderr, "Usage: chartuBeam [options] \n"
		"[options] include:\n"
		"  -h or --help  (this help)\n"
		"  -a <n> or --antenna <n>  (n is the antenna number)\n"
		"  -n <***> or --filename <***>  (*** is output file name)\n");
/*		"  -r <###> or --receiver <###>  (### is receiver band [230, 345])\n");
		"  -f <###> or --frequency <###>  (### is LO frequency in GHz)\n");	*/
	exit(exitcode);
}

int main(int argc, char **argv)
{
	float Azoff_avg,Eloff_avg,Az_avg,El_avg,Azerr_avg,Elerr_avg,Azmod_avg,Elmod_avg,Data_avg;
	float xx[20000],yy[20000],zz[20000],azyy[20000],elyy[20000],azp[20000],elp[20000],xx2[20000],yy2[20000];
	int i,j,iavg;
	char c,*t,*s,*curtim,file_n1[50],filename[50],source[40];
	time_t new_time,old_time,cur_time;
	FILE *fp;
	int rm_status, antlist[RM_ARRAY_SIZE];/*variables for encoderClient*/
	float az_deg,el_deg,az_err,el_err,az_mod,el_mod,chop_angle,chop_x,chop_y,chop_z;
	double output, az_off, el_off;
	smapoptContext optCon;
	struct  smapoptOption optionsTable[] = {
		{"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"antennas",'a',SMAPOPT_ARG_INT,&ant_num,'a'},
		{"filename",'n',SMAPOPT_ARG_STRING,&filename,'n'},
		{NULL,0,0,NULL,0}
	};

	if(argc<4) usage(-1,"Insufficient number of arguments","");
	optCon = smapoptGetContext("chartuBeam", argc, argv, optionsTable,0);

	while ((c = smapoptGetNextOpt(optCon)) >= 0) {
	switch(c) {
		case 'h':
		usage(0,NULL,NULL);
		break;
	}
	}

	if((ant_num < 2) || (ant_num > 8)){
		printf("ant %d? ant 2-7\n",ant_num);
		exit(1);
	}

	/* initializing ref. mem. */
	rm_status=rm_open(antlist);
	if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_open()");
		exit(1);
	}

	chart_stat.saveflg=0;
	chart_stat.scanflg=1; /* Az scan */
	chart_stat.chopflg=0;
	chart_stat.integtime=1;
	chart_stat.quitflg=0;

	if(-1==pthread_create(&write_statusTID, NULL , write_status, (void *) &ant_num)){
		perror("main: pthread_create CommandHandler");
		exit(-1);	
	}
	pthread_detach(write_statusTID);
	sleep(1);


	while(0==chart_stat.quitflg){
 
		if(-1==pthread_create(&read_statusTID, NULL , read_status, (void *) &ant_num)){
		perror("main: pthread_create CommandHandler");
		exit(-1);	
		}
		pthread_detach(read_statusTID);

		i=0;
		while(chart_stat.saveflg==1){
			Az_avg=0;El_avg=0;Azerr_avg=0;Elerr_avg=0;iavg=1;
			Azoff_avg=0;Eloff_avg=0;Azmod_avg;Elmod_avg=0;
			old_time=time(NULL);new_time=time(NULL);
			while((new_time-old_time)<chart_stat.integtime){
				rm_status=rm_read(ant_num,"RM_ACTUAL_AZ_DEG_F",&az_deg);
		   		if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
		   		exit(1);
				}
		   		rm_status=rm_read(ant_num,"RM_ACTUAL_EL_DEG_F",&el_deg);
				if(rm_status != RM_SUCCESS) {
		   		rm_error_message(rm_status,"rm_read()");
				exit(1);
		   		}
				rm_status=rm_read(ant_num,"RM_AZ_TRACKING_ERROR_F",&az_err);
	   			if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
	   			exit(1);
				}
		   		rm_status=rm_read(ant_num,"RM_EL_TRACKING_ERROR_F",&el_err);
				if(rm_status != RM_SUCCESS) {
		   		rm_error_message(rm_status,"rm_read()");
				exit(1);
	   			}
				rm_status=rm_read(ant_num,"RM_PMDAZ_F",&az_mod);
	   			if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				rm_status=rm_read(ant_num,"RM_PMDEL_F",&el_mod);
				if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				rm_status=rm_read(ant_num,"RM_AZOFF_D",&az_off);
				if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				rm_status=rm_read(ant_num,"RM_ELOFF_D",&el_off);
				if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				if(chart_stat.chopflg) rm_status=rm_read(ant_num,"RM_SYNCDET_VOLTS_D",&output);
				else rm_status=rm_read(ant_num,"RM_TOTAL_POWER_VOLTS_D",&output);
				if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				Az_avg=((iavg-1)*Az_avg+az_deg)/iavg;
				El_avg=((iavg-1)*El_avg+el_deg)/iavg;
				Azoff_avg=((iavg-1)*Azoff_avg+az_off)/iavg;
				Eloff_avg=((iavg-1)*Eloff_avg+el_off)/iavg;
				Azerr_avg=((iavg-1)*Azerr_avg+az_err)/iavg;
				Elerr_avg=((iavg-1)*Elerr_avg+el_err)/iavg;
				Azmod_avg=((iavg-1)*Azmod_avg+az_mod)/iavg;
				Elmod_avg=((iavg-1)*Elmod_avg+el_mod)/iavg;
				Data_avg=((iavg-1)*Data_avg+output)/iavg;
				rm_status=rm_read(ant_num,"RM_SOURCE_C34",&source);
				if(rm_status != RM_SUCCESS) {
				rm_error_message(rm_status,"rm_read()");
				exit(1);
				}
				usleep(10000);
				iavg++;
				new_time=time(NULL);
			}
			printf("Ant%1d %s %3d Aoff %4.1f Eoff %4.1f Aerr%4.1f Eerr%4.1f Azmod%4.1f Elmod%4.1f Data%7.4f\n",ant_num,source,iavg,Azoff_avg,Eloff_avg,Azerr_avg,Elerr_avg,Azmod_avg,Elmod_avg,Data_avg);

			/* store data for fitting */
			azyy[i]=Az_avg;
			elyy[i]=El_avg;
			xx[i]=Azoff_avg;
			xx2[i]=Eloff_avg;
			zz[i]=Data_avg;
			yy[i]=Azerr_avg;
			yy2[i]=Elerr_avg;
			azp[i]=Azmod_avg;
			elp[i]=Elmod_avg;
			i++;

		if(-1==pthread_create(&read_statusTID, NULL , read_status, (void *) &ant_num)){
		perror("main: pthread_create CommandHandler");
		exit(-1);	
		}
		pthread_detach(read_statusTID);
		} /* end while save flag is on */

		if(i){	/*if there are data*/
			sprintf(file_n1,"/data/engineering/rpoint/ant%1d/%s",ant_num,filename);
			printf("filename=%s\n",file_n1);
			if ((fp=fopen(file_n1,"w"))==NULL){
				printf("cannot open data file\n");
				exit(1);
			}

			rm_status=rm_read(ant_num,"RM_CHOPPER_ANGLE_F",&chop_angle);
			if(rm_status != RM_SUCCESS) {
			rm_error_message(rm_status,"rm_read()");
			exit(1);
			}
			rm_status=rm_read(ant_num,"RM_CHOPPER_X_MM_F",&chop_x);
			if(rm_status != RM_SUCCESS) {
			rm_error_message(rm_status,"rm_read()");
			exit(1);
			}
			rm_status=rm_read(ant_num,"RM_CHOPPER_Y_MM_F",&chop_y);
			if(rm_status != RM_SUCCESS) {
			rm_error_message(rm_status,"rm_read()");
			exit(1);
			}
			rm_status=rm_read(ant_num,"RM_CHOPPER_Z_MM_F",&chop_z);
			if(rm_status != RM_SUCCESS) {
			rm_error_message(rm_status,"rm_read()");
			exit(1);
			}
	
			t=ctime(&new_time);
			fprintf(fp,"#%s#%s scnflg %1d chpflg %1d itgtime %1d %5.3f %5.3f %5.3f %5.3f\n",t, source,chart_stat.scanflg,chart_stat.chopflg,chart_stat.integtime,chop_angle,chop_x,chop_y,chop_z);
			for(j=0;j<i-1;j++){
				fprintf(fp,"%10.5f %10.5f %7.2f %7.2f %7.4f %7.4f %7.2f %7.2f %7.2f\n",azyy[j],elyy[j],xx[j],xx2[j],zz[j],yy[j],yy2[j],azp[j],elp[j]);
			}
			fclose(fp);
		}

		cur_time=time(NULL);
		curtim=ctime(&cur_time);
		printf("type 'chartuCommand -a %1d -q' to quit %s",ant_num,curtim);
		sleep(1);
	} /* end while */

	printf("\n");

	/* close ref. mem. */
	rm_status=rm_close(); 
	if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_close()");
		exit(1);
	}
}

void *read_status(){
	FILE *fps;
	char  file_n2[40];
 
	sprintf(file_n2,"/data/engineering/rpoint/ant%1d/stat.ant%1d",ant_num,ant_num);
	if ((fps=fopen(file_n2,"r"))==NULL)
	{
		printf("cannot open status file\n");
		exit(1);
	}
 
	fscanf(fps,"%d %d %d %d %d\n",&chart_stat.saveflg, &chart_stat.scanflg, &chart_stat.chopflg, &chart_stat.integtime, &chart_stat.quitflg);
/*	printf("Ant%1d read status = %d %d %d %d %d ",ant_num,chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime, chart_stat.quitflg);*/
	fclose(fps);
}

void *write_status(){
	FILE *fps;
	char  file_n2[40];
 
	sprintf(file_n2,"/data/engineering/rpoint/ant%1d/stat.ant%1d",ant_num,ant_num);
	if ((fps=fopen(file_n2,"w"))==NULL)
	{
		printf("cannot open status file\n");
		exit(1);
	}
 
/*	printf("write %d %d %d %d %d\n",chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime, chart_stat.quitflg);*/
	fprintf(fps,"%d %d %d %d %d\n",chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime, chart_stat.quitflg);
	fflush(fps);
	fclose(fps);
}

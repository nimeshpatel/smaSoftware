#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/file.h>
#include <ctype.h>
#include "/usr/PowerPC/applications/hal/holo/includeFiles/holo.h"
#include "rm.h"
/* #include <fcntl.h>
#include <errno.h>
*/


/************************************************************************
 *
 *  File Name: holo3.c 
 *  written by M. Saito 000314 (with N. Patel and T. K. Sridharan)
 *  modified by Trung and M. Saito 000614 
 *  modified by M. Saito 000708 
 *
 *  Description: holography at 3mm
 *
 ************************************************************************/
/*WARNING there are many sleep commands and some command are excuted twice */

int main(int argc, char **argv)
{
    FILE       *fp_raw_data;
    char	command[35],file_name[80];
    int		j, k, l,ant_scn,n_size, n_start, offsetunit;
    long	i,i_delay,n_delay;
    int		istatus,icount;
    int		amp_read[2000],phase_read[2000];
    time_t      start_time, cur_time, Start_time;
    float       az_step, el_step, azoff, eloff, freq,az_bore,el_bore; 
    float       az_off,el_off;
    double      az_enc,el_enc;
    double      az_read[2000],el_read[2000];
    int rm_status, antlist[RM_ARRAY_SIZE];/*variables for encoderClient*/
    short	currentAzoff;
    short	handshake;
    struct tm	ts;	
    double      currentAzoffD;
    unsigned short m1009,m1010;

/**********get command line arguments and initializations**************/

	if(argc<6)
	{
	printf("Usage: holo <antNo> <mapSize(64 or 128)> <scanSpeed(>0)> <frequency> <n_delay> <start line#:optional>\n");
	exit(0);
	}

	ant_scn=atoi(argv[1]);
	n_size=atoi(argv[2]);
	offsetunit=atoi(argv[3]);
	freq=atof(argv[4]);
	n_delay=atoi(argv[5]);
	if (argc==7) {
	n_start=atoi(argv[6]);
	} else n_start=0;

	az_step=7734.0/freq;el_step=az_step; 
	azoff=0; eloff=0;
    	start_time = time(NULL);
	ts = *localtime(&start_time);
	sprintf(file_name, "/application/data/holo/%02d%02d_%02d%02d.holo%1d",ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min,ant_scn);
	printf("freq=%4.1fGHz grid=%4.1f\" filename %s\n",freq,az_step,file_name);

	if(ant_scn<2||ant_scn>5)
	{
	printf("Ant%1d!? ant# should be 2 or 3\n",ant_scn);
	exit(1);
	}

	if(ant_scn==2){
		az_bore=AZ_BORE2;
		el_bore=EL_BORE2;
	}
	else if(ant_scn==3){
		az_bore=AZ_BORE3;
		el_bore=EL_BORE3;
	}
	else if(ant_scn==4){
		az_bore=AZ_BORE4;
		el_bore=EL_BORE4;
	}
	else if(ant_scn==5){
		az_bore=AZ_BORE0;
		el_bore=EL_BORE0;
		ant_scn=3; /* debug */
	}

/* fine adjustment of the boresight position if you want */
	if (ant_scn==2){
	az_off = 0.0;
	el_off = 0.0;
	az_bore = az_bore + az_off;
	el_bore = el_bore + el_off;
	}

	if (ant_scn==3){
	az_off = 0.0;
	el_off = 0.0;
	az_bore = az_bore + az_off;
	el_bore = el_bore + el_off;
	}

	if (ant_scn==4){
	az_off = 0.0;
	el_off = 0.0;
	az_bore = az_bore + az_off;
	el_bore = el_bore + el_off;
	}

	fp_raw_data = fopen(file_name,"w");
	if(fp_raw_data==NULL){
		printf("cannot open the data file\n");
		exit(1);
	}

/* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

/* configure for interrupt through RM */
 	rm_status=rm_monitor(ant_scn,"RM_HOLO_ENC_HNDSHK_S");
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_monitor()");
                exit(1);
        }

	printf("checking the encoderServer status\n");
	handshake = 0;

        rm_status=rm_write_notify(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
                } 

	sleep(1);

        rm_status=rm_read(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
                }  

	if (handshake==0) 
	{
	printf("encoder Server is not running on acc%1d!\n",ant_scn);
	exit(-1);
	}

/*********************Get the antenna ready for*************************/
/* go to starting position  */

	printf ("Going to bore-sight position \n");
        sprintf(command, "azoff -a %d -s 0", ant_scn);
	system (command);
        sprintf(command, "eloff -a %d -s 0", ant_scn);
	system (command);
	sprintf(command, "azel -a %d -w -z %f -e %f", ant_scn, az_bore, el_bore);
	printf ("command: %s \n", command);
	system (command);
	sprintf(command, "offsetUnit -a %d -s %d", ant_scn, -offsetunit); 
	system (command);
	printf ("Hit Return to continue:");
	fgetc(stdin);
	printf ("Starting raster on antenna %d\n", ant_scn);
	azoff=((float)n_size/2.+3.)*az_step;
	eloff=((float)n_size/2.)*el_step - n_start*el_step;
	sprintf(command, "azoff -a %d -w -s %f", ant_scn, azoff); 
	printf ("command: %s ", command);
	system (command);
	sleep(2.0);
	sprintf(command, "eloff -a %d -w -s %f", ant_scn, eloff); 
	printf ("%s \n", command);
	system (command);
	sleep(2.0);

/*********************Start taking data*************************/
	printf ("Start raster! \n");
    	start_time = time(NULL);
	Start_time = start_time;

/* Elevation Loop */
	for (j=n_start; j<n_size; j++){  /*do one row*/ 
		i=0; 
	currentAzoff=0;
    		start_time = time(NULL);
		sprintf(command, "azscan -a %d", ant_scn);
		printf ("%s \n", command);

		istatus=system (command);

		printf("%6d %6d  %6f\n",istatus,currentAzoff,azoff);
		printf("%6d  %6f \n",currentAzoff,azoff); fflush(stdout);
		while((1.0*currentAzoff)>(-azoff)){
		for (i_delay=1; i_delay<n_delay; i_delay++){}
/*  usleep(1); */
/* read encoders using encoderClient */
			encoderClient(ant_scn,&az_enc,&el_enc,&m1009,&m1010); 

                        az_read[i]=az_enc;el_read[i]=el_enc;
                        amp_read[i]=(int)m1009;
                        phase_read[i]=(int)m1010;

		        rm_status=rm_read(ant_scn,"RM_AZOFF_D",&currentAzoffD);
		        if(rm_status != RM_SUCCESS) {
	                rm_error_message(rm_status,"rm_read()");
	                exit(1);
       			}/*if end*/
			currentAzoff=(short)currentAzoffD;
/* printf("line# %3d %3d %4d %6.2f %6.2f %6.2f %5d %5d\n",j,i,currentAzoff,azoff,az_enc,el_enc,amp_read[i],phase_read[i]); fflush(stdout); */
			i++;
		}/*while end*/
		sprintf(command, "stopScan -a %d", ant_scn);
	      	system (command);
		sleep(1); 
		printf ("command: %s \n", command); 
	      	system (command);
		sleep(1); 
		eloff = eloff - el_step;
		sprintf(command, "eloff -a %d -s %f", ant_scn, eloff);
		system (command);
		sleep(1); 
		printf ("command: %s ", command);fflush(stdout);
/*		system (command);
		usleep(10); */
		sprintf(command, "azoff -a %d -s %f", ant_scn, azoff);
		system (command);
		usleep(100000);
		printf ("command: %s ", command);fflush(stdout);
/*		system (command);*/

                for(k=2;k<i;k++){
			fprintf(fp_raw_data,"%10.6f %10.6f %6d %6d %3d\n" ,\
			el_read[k],az_read[k],amp_read[k],phase_read[k],j);
			fflush(fp_raw_data);
    		}/*for end*/

		/* Place for including online dispaly command */
		printf ("Flush the data\n"); 
    		cur_time=time(NULL);
    		printf ("%3dth line: Stop time: %d time taken: %d secs samples: %d\n", \
		j, cur_time, cur_time-start_time, i);
		sleep(8); 
    	}/* for end; El loop done */

	printf ("Start: %d Stop: %d Time taken: %d secs\n", Start_time, cur_time, cur_time - Start_time);
        sprintf(command, "azoff -a %d -s 0", ant_scn);
	system (command);
	usleep(100000);
	printf ("command: %s ", command);
        sprintf(command, "eloff -a %d -s 0", ant_scn);
	system (command);
	usleep(100000);
	printf ("command: %s\n", command);

/*********************clear up everything*************************/
	fclose(fp_raw_data);
/* release the RM interrupt waiting */
        rm_status=rm_clear_monitor();
        if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_clear()");
	}
	printf ("DONE!\n");
	exit(0);
}				/* end main */

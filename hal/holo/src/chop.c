#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/file.h>
#include <ctype.h>
#include "rm.h"

/************************************************************************
 *
 *  File Name: chop.c 
 *  written by M. Saito 000707 
 *  modified by M. Saito 000829 
 *  modified by M. Saito 010705 for lynxos 3.1.0 
 *
  lynxos
  gcc -o chop chop.c -I$COMMONINC $COMMONLIB/rm.o
 *
 *
  parameter file:/export/home/lynx/common/reflmem/rm_allocation
 *
 ************************************************************************/

/* global variables */

int main(int argc, char **argv)
{
    FILE	*fp;
    long	i,l;
    int ant_scn,rm_status, antlist[RM_ARRAY_SIZE];/*variables for encoderClient*/
    short	handshake;
    unsigned short	num_of_dat,num_of_ave;
    float	chop_angle,chop_x,chop_y,chop_z;
    float	chop_angle_av,chop_x_av,chop_y_av,chop_z_av,output_av;
    double 	output;

	if(argc<5)
	{
	printf("Usage: chop <filename> <antNo> <#data> <#average(10-30)>\n"); 
	exit(0);
	}

	ant_scn=atoi(argv[2]);
	num_of_dat=atoi(argv[3]);
	num_of_ave=atoi(argv[4]);

	fp = fopen(argv[1],"w");
	if(fp==NULL){
		printf("cannot open the data file\n");
		exit(1);
	}

/* initializing ref. mem. */
	rm_status=rm_open(antlist);
	if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_open()");
		exit(1);
	}


	for(l=0;l<num_of_dat;l++){
    	chop_angle_av=0.;chop_x_av=0.;chop_y_av=0.;chop_z_av=0.;output_av=0.;
	for(i=0;i<num_of_ave;i++){
		handshake=0;
		/* send interrupt to antenna computer requesting for readings*/
		rm_status=rm_write_notify(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_write_notify()");
		exit(1);
		}
		usleep(1);
	
		/* wait for interrupt for data-ready */
		rm_status=rm_read(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read_wait()");
		exit(1);
		}

		rm_status=rm_read(ant_scn,"RM_TOTAL_POWER_VOLTS_D",&output);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}
		rm_status=rm_read(ant_scn,"RM_CHOPPER_ANGLE_F",&chop_angle);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}
		rm_status=rm_read(ant_scn,"RM_CHOPPER_X_MM_F",&chop_x);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}
		rm_status=rm_read(ant_scn,"RM_CHOPPER_Y_MM_F",&chop_y);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}
		rm_status=rm_read(ant_scn,"RM_CHOPPER_Z_MM_F",&chop_z);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}

		output_av+=output;
    		chop_angle_av+=chop_angle;
		chop_x_av+=chop_x;
		chop_y_av+=chop_y;
		chop_z_av+=chop_z;
	}
		output_av/=num_of_ave;
    		chop_angle_av/=num_of_ave;
		chop_x_av/=num_of_ave;
		chop_y_av/=num_of_ave;
		chop_z_av/=num_of_ave;
		printf("%4d %6.2f %6.2f %6.2f %7.4f\n",l,chop_x_av,chop_y_av,chop_z_av,output_av);
		fprintf(fp,"%4d %6.2f %6.2f %6.2f %7.4f\n",l,chop_x_av,chop_y_av,chop_z_av,output_av);
	}

	fclose(fp);
}

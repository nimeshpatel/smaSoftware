#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/file.h>
#include <ctype.h>
/*#include "holo.h"*/
#include "rm.h"

/************************************************************************
 *
 *  File Name: chop_vvm.c 
 *  written by M. Saito 000707 
 *  modified by M. Saito 000829 
 *  modified by M. Saito 010705 
 *
  lynxos
  gcc -o chop_vvm chop_vvm.c -I$COMMONINC $COMMONLIB/encoderClient.o $COMMONLIB/rm.o
 *
 *
  parameter file:$COMMONLIB/../reflmem/rm_allocation
 *
 ************************************************************************/

/* global variables */

union ad_union {
        short word;
        char byte[2];
        };

int main(int argc, char **argv)
{
    FILE	*fp;
   /* union       ad_union amp_vvm[10], pha_vvm[10]; */
    union       ad_union amp_vvm, pha_vvm; 
    long	i,l,i_delay,n_delay;
    int ant_scn,rm_status, antlist[RM_ARRAY_SIZE];/*variables for encoderClient*/
	int adc0_fd, adc1_fd, adc0_rd, adc1_rd, count;
	int OFLAG;
	int amp,pha,amp_sum,pha_sum;
    short	handshake;
    unsigned short	m1009,m1010,num_of_dat,output,num_of_ave;
    float	chop_angle,chop_x,chop_y,chop_z,chopperPosition[4];
    float	chop_angle_av,chop_x_av,chop_y_av,chop_z_av,output_av,amp_av, pha_av;
    double az_enc,el_enc,az_av,el_av;

	count=2; /* set how many counts we read from A/D board device */

	if(argc<6)
	{
	printf("Usage: chop_vvm <filename> <antNo> <#data> <#average(10-30)> <n_delay> \n"); 
	exit(0);
	}

	ant_scn=atoi(argv[2]);
	num_of_dat=atoi(argv[3]);
	num_of_ave=atoi(argv[4]);
	n_delay=atoi(argv[5]);

	chop_angle=0;
	chop_x=0;
	chop_y=0;
	chop_z=0;

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
/* configure for interrupt through RM */
        rm_status=rm_monitor(ant_scn,"RM_HOLO_ENC_HNDSHK_S");
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_monitor()");
                exit(1);
        }
/* no Enc
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
        printf("encoderServer is not running on acc%1d!\n",ant_scn);
        exit(-1);
        }
*/
/* open Device tables */
        OFLAG=O_RDWR|O_NDELAY;
        adc0_fd = open("/dev/xVME564-0", OFLAG, 0);
        if(adc0_fd<0){printf("cannot open device 0\n");exit(1);}
        adc1_fd = open("/dev/xVME564-1", OFLAG, 0);
        if(adc1_fd<0){printf("cannot open device 1\n");exit(1);}

	for(l=0;l<num_of_dat;l++){
    	chop_angle_av=0.;chop_x_av=0.;chop_y_av=0.;chop_z_av=0.;output_av=0.;
	amp_sum=0;pha_sum=0;amp_av=0.;pha_av=0.;az_av=0.;el_av=0.;
	for(i=0;i<num_of_ave;i++){
		/* send interrupt to antenna computer requesting for readings*/
/*		handshake=0; 
		rm_status=rm_write_notify(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_write_notify()");
		exit(1);
		}
		usleep(1); */
	
		/* wait for interrupt for data-ready */
	/*	rm_status=rm_read(ant_scn,"RM_HOLO_ENC_HNDSHK_S",&handshake);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read_wait()");
		exit(1);
		}
*/

                        for (i_delay=1; i_delay<n_delay; i_delay++){}
                        adc0_rd=read(adc0_fd, amp_vvm.byte, count);
                        if(adc0_rd<=0){
                                printf("cannot read device 0\n");
                                exit(1);
                                close(adc0_fd);
                                close(adc1_fd);
                        }
/* usleep(1); */
                        adc1_rd=read(adc1_fd, pha_vvm.byte, count);
                        if(adc1_rd<=0){
                                printf("cannot read device 0\n");
                                exit(1);
                                close(adc0_fd);
                                close(adc1_fd);
                    }
/* read encoders using encoderClient */
/* no Enc!
                encoderClient(ant_scn,&az_enc,&el_enc,&m1009,&m1010);
*/

/* new chopper variables in RM, NaP 13 Nov 2003 */

	        rm_status=rm_read(ant_scn,"RM_CHOPPER_POS_MM_V4_F",chopperPosition);
		if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
		}

		chop_x=chopperPosition[0];
		chop_y=chopperPosition[1];
		chop_z=chopperPosition[2];
		chop_angle=chopperPosition[3];

/*		rm_status=rm_read(ant_scn,"RM_CHOPPER_ANGLE_F",&chop_angle);
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
*/
		amp=amp_vvm.word;
 		pha=pha_vvm.word;
		amp_sum+=amp;
		pha_sum+=pha;
/* no Enc !
		az_av+=az_enc;
		el_av+=el_enc;
*/
    		chop_angle_av+=chop_angle;
		chop_x_av+=chop_x;
		chop_y_av+=chop_y;
		chop_z_av+=chop_z;
/*		printf("%8.4f %7.4f %4d %6.2f %6.2f %6.2f %5d %5d\n",az_enc,el_enc,l,chop_x_av,chop_y_av,chop_z_av,amp,pha);*/
	}
		az_av/=num_of_ave;
		el_av/=num_of_ave;
		amp_av=amp_sum/num_of_ave;
		pha_av=pha_sum/num_of_ave;
    		chop_angle_av/=num_of_ave;
		chop_x_av/=num_of_ave;
		chop_y_av/=num_of_ave;
		chop_z_av/=num_of_ave;
		printf("%8.4f %7.4f %4d %6.2f %6.2f %6.2f %6.2f %7.1f %7.1f\n",az_av,el_av,l,chop_x_av,chop_y_av,chop_z_av,chop_angle_av,amp_av,pha_av);
		fprintf(fp,"%8.4f %7.4f %4d %6.2f %6.2f %6.2f %6.2f %7.1f %7.1f\n",az_av,el_av,l,chop_x_av,chop_y_av,chop_z_av,chop_angle_av,amp_av,pha_av);
	}

	fclose(fp);
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/file.h>
#include <ctype.h>
#include "holo.h"
#include "rm.h" 

/* Unknown Date: oroginal version, TK */
/* 26 Feb 14: modified to allow any number of antennas < 3 -TK. */

/* #define N_ANTENNAS 11
lynxos
  gcc -o boreChopper boreChopper.c -I$COMMONINC -I../includeFiles $COMMONLIB/rm.o 
  parameter file:$COMMONLIB/../reflmem/rm_allocation */

int main(int argc, char **argv)
{
FILE *fpBore;
/* float az,el, azBore[29],elBore[29]; */
float  azBore,elBore,chopZ, chop_x, chop_y,chopperPosition[4];
int ant1,ant2,antR, antlist[RM_ARRAY_SIZE], rm_status,i,pad1,pad2,padR;
char command[35];
        if(argc<2)
        {       
        printf("Usage: boreChopper <ant1> [<ant2>] [<antR>]\n");
        exit(0);
        }
fpBore = fopen("/application/holo/boreZ.tmp","r");

ant1=atoi(argv[1]);
if (argc>2) ant2=atoi(argv[2]);
if (argc>3) antR=atoi(argv[3]);
printf("%s:\n",argv[0]);
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }
fscanf(fpBore,"%f %f %f",&azBore, &elBore, &chopZ);
	sprintf(command, "azel -a %d -z %f -e %f", ant1, azBore, elBore);
	printf("command: %s\n", command);
     system (command);
	sprintf(command, "antennaWait -a %d", ant1);
	printf("%s\n", command);
	system(command);
/* antennaWait doesnt seem to work! All antennas have arrived message is
printed well before the antennas reach the commanded az, el; so
increase sleep to 1 min; also for nat2 and antR below. 27 Sep 10*/
/*	sleep(5); */
	sleep(60);
                rm_status=rm_read(ant1,"RM_CHOPPER_POS_MM_V4_F",chopperPosition);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
                }
                chop_x=chopperPosition[0];
                chop_y=chopperPosition[1];
	sprintf(command, "focusCurveOff -a %d", ant1);
	printf("%s\n", command);
      system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d  -x  %f -y %f", ant1, chop_x, chop_y);
	printf("%s\n", command);
       system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d -f -z %f", ant1, chopZ);
	printf("%s\n", command);
       system (command);
if (argc < 3) {
	fclose(fpBore);
	exit(0);
}
fscanf(fpBore,"%f %f %f",&azBore, &elBore, &chopZ);
	sprintf(command, "azel -a %d -z %f -e %f", ant2, azBore, elBore);
	printf("%s\n", command);
	system(command);
	sprintf(command, "antennaWait -a %d", ant2);
	printf("%s\n", command);
	system(command);
/*	sleep(5); */
	sleep(60);
                rm_status=rm_read(ant2,"RM_CHOPPER_POS_MM_V4_F",chopperPosition);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
                }
                chop_x=chopperPosition[0];
                chop_y=chopperPosition[1];
	sprintf(command, "focusCurveOff -a %d", ant2);
	printf("%s\n", command);
        system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d  -x  %f -y %f" , ant2, chop_x, chop_y);
	printf("%s\n", command);
       system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d -f -z %f", ant2, chopZ);
	printf("%s\n", command);
       system (command);

if (argc < 4) {
	fclose(fpBore);
	exit(0);
}

fscanf(fpBore,"%f %f %f",&azBore, &elBore, &chopZ);
	sprintf(command, "azel -a %d -z %f -e %f", antR, azBore, elBore);
	printf("%s\n", command);
	system(command);
	sprintf(command, "antennaWait -a %d", antR);
	printf("%s\n", command);
	system(command);
/*	sleep(5); */
	sleep(60);
                rm_status=rm_read(antR,"RM_CHOPPER_POS_MM_V4_F",chopperPosition);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
                }
                chop_x=chopperPosition[0];
                chop_y=chopperPosition[1];
	sprintf(command, "focusCurveOff -a %d", antR);
	printf("%s\n", command);
       system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d -x  %f -y %f", antR, chop_x, chop_y);
	printf("%s\n", command);
        system (command);
	sleep(20);
        sprintf(command, "positionChopper -a  %d -f -z %f", antR, chopZ);
	printf("%s\n", command);
       system (command);
	fclose(fpBore);
}

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

/* #define N_ANTENNAS 11
lynxos
  gcc -o padidtest padidtest.c -I$COMMONINC $COMMONLIB/rm.o 
  parameter file:$COMMONLIB/../reflmem/rm_allocation */

int main(int argc, char **argv)
{
FILE *fp_pid, *fpBore;
char PadID1,PadID2,PadIDR;
/* float az,el, azBore[29],elBore[29]; */
float  az_bore,el_bore,az_bore2,el_bore2,az_boreR,el_boreR;
float chopperZ,chopperZ2,chopperZR;
int ant1,ant2,antR, antlist[RM_ARRAY_SIZE], rm_status,i,pad1,pad2,padR;
        if(argc<4)
        {       
        printf("Usage: test <ant1> <ant2> <antR>\n");
        exit(0);
        }
/* fp_pid = fopen("padIDConf","r");
for (i=0;i<19;i++){
fscanf(fp,"%f %f",azBore, elBore, chopperZ);
}
	fclose(fp_pid);
*/
fpBore = fopen("/application/holo/boreZ.tmp","w");
ant1=atoi(argv[1]);
ant2=atoi(argv[2]);
antR=atoi(argv[3]);
printf("%s:\n",argv[0]);
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }
rm_status=rm_read(ant1,"RM_PAD_ID_B",&PadID1); 
rm_status=rm_read(ant2,"RM_PAD_ID_B",&PadID2); 
rm_status=rm_read(antR,"RM_PAD_ID_B",&PadIDR); 
printf("Antenna %d is on pad %d \n", ant1,PadID1); 
printf("Antenna %d is on pad %d \n", ant2,PadID2); 
printf("Antenna %d is on pad %d \n", antR,PadIDR); 

        fp_pid = fopen("/application/holo/includeFiles/padBoreFocus.conf","r");
        for (i=0;i<PadID1;i++){
        fscanf(fp_pid,"%d %f %f %f",&pad1, &az_bore, &el_bore, &chopperZ);
/*	printf("%d %d %f %f %f\n", i, pad1, az_bore, el_bore, chopperZ);  */
        }
	fclose(fp_pid);
        fp_pid = fopen("/application/holo/includeFiles/padBoreFocus.conf","r");
        for (i=0;i<PadID2;i++){
        fscanf(fp_pid,"%d %f %f %f",&pad2, &az_bore2, &el_bore2, &chopperZ2);
        }
	fclose(fp_pid);
        fp_pid = fopen("/application/holo/includeFiles/padBoreFocus.conf","r");
        for (i=0;i<PadIDR;i++){
        fscanf(fp_pid,"%d %f %f %f",&padR, &az_boreR, &el_boreR, &chopperZR);
        }
	fclose(fp_pid);

/* az=azBore[PadID];
el=elBore[PadID];
*/
printf("%f %f %f\n%f %f %f\n%f %f %f\n", az_bore,el_bore,chopperZ,az_bore2,el_bore2,chopperZ2,az_boreR,el_boreR,chopperZR);
fprintf(fpBore,"%f %f %f\n%f %f %f\n%f %f %f\n", az_bore,el_bore,chopperZ,az_bore2,el_bore2,chopperZ2,az_boreR,el_boreR,chopperZR);
}

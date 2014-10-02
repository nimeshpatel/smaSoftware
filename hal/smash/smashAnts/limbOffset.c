/***************************************************************
*
* limbOffset.c
* 
* NAP 29 Jan 2005.
* gives raoff or decoff by planet radius according to input direction
*
* NAP 14 Feb 2005.
* added an argument for arbitrary PA offset
****************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <smapopt.h>
#include <math.h>
#include "rm.h"
#include "commonLib.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))


void usage(int exitcode, char *error, char *addl) {

        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: limbOffset [options] --direction n,s,e,w>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -n or --nomosaic to not append offsets to sourcename\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "  -p <degrees> arbitrary parallactic angle for offset\n"
                "  -d or --direction (center,north,south,east,west (or c,n,s,e,w))\n"); 
        exit(exitcode);
}

int main(int argc, char *argv[])  
{

	char c,*direction,command_n[30];
/*      char messg[100]; */
	double arcseconds,positionangle;
	double pi,radian;
	short pmac_command_flag=0;
	short error_flag=RUNNING;
	int gotdirection=0,rm_status,antlist[RM_ARRAY_SIZE];
	int gotpa=0;
	int gotnomosaic=0;
	short gotMosaicFlag=1;
	double raoff=0.;
	double decoff=0.;
	double raoff_sign=0; 
	double decoff_sign=0;
	int i, antenna;
	smapoptContext optCon;
        int gotantenna=0,antennaArray[SMAPOPT_MAX_ANTENNAS+1];
	int antennas[SMAPOPT_MAX_ANTENNAS+1];
        int trackStatus=0,iant;
        int tracktimestamp,timestamp;
	int numberOfAnts=0;

 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h',"Help"},
                {"nomosaic",'m',SMAPOPT_ARG_NONE,0,'m',"No mosaicing-
source name will not change."},
                {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a',
		"a comma-separated list of antennas"},
                {"p",'p',SMAPOPT_ARG_DOUBLE,&positionangle,'p',
		"an arbitrary parallactic angle in degrees, for offsets in RA/DEC"},
                {"direction",'d',SMAPOPT_ARG_STRING,&direction,'d',
		"direction string: north,south ..."},
 		SMAPOPT_AUTOHELP
                {NULL,0,0,NULL,0}

        };

	pi = 4.0*atan(1.0);
	radian=pi/180.;

 if(argc<2) usage(-1,"Insufficient number of arguments","direction required.");

        optCon = smapoptGetContext("limbOffset", argc, argv, optionsTable,0);

        while ((c = smapoptGetNextOpt(optCon)) >= 0) {

        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;

               case 'a':
		gotantenna=1;
                break;

               case 'n':
		gotnomosaic=1;
                break;

                case 'd':
                gotdirection=1;
                break;

                case 'p':
                gotpa=1;
                break;

                }

        }

if((gotdirection==1)&&(gotpa==1))
usage(-3,"Specify either position angle or direction","but not both");

if((gotdirection==1)&&(gotpa==0)) {
if (!strcmp(direction,"north")||!strcmp(direction,"n")) decoff_sign=1.0; 
else if (!strcmp(direction,"south")||!strcmp(direction,"s")) decoff_sign=-1.0; 
else if (!strcmp(direction,"east")||!strcmp(direction,"e")) raoff_sign=1.0; 
else if (!strcmp(direction,"west")||!strcmp(direction,"w")) raoff_sign=-1.0; 
else if (!strcmp(direction,"center")||!strcmp(direction,"c")) {
					raoff_sign=0.0; 
					decoff_sign=0.0;
					}
else 
	{
	if(gotpa==0)	
	usage(-2,"Invalid direction","expecting n,s,e,w or
north,south,east,west");
	}
}

if((gotpa==1)&&(gotdirection==0)) {
raoff_sign=sin(positionangle*radian);
decoff_sign=cos(positionangle*radian);
}


       if(gotantenna==1) {
                numberOfAnts=0;
                 for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
                    if(antennaArray[iant]==1) {
                    antennas[numberOfAnts]=iant;
                    numberOfAnts++;
                    }
                 }
		 if (numberOfAnts < 1) {
		   printf("No antennas specified.  Nothing done.\n");
		   exit(1);
		 }
        }

	for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };
        command_n[0]='1'; /* send '1' command */

	 if((gotdirection!=1)&&(gotpa!=1)) usage(-2,"No direction or position
angle specified","direction or position angle
is required .\n");

        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
                smapoptStrerror(c));
        }
         smapoptFreeContext(optCon);

	/* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

	
        if (gotantenna==0) {
	getAntennaList(antennaArray);

        }

        for(iant=1;iant<=8;iant++) {
	
        if(antennaArray[iant]==1) {
        antenna=iant;

          /* check if track is running on this antenna */

        rm_status=rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
	rm_status=rm_read(antenna,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if(abs(tracktimestamp-timestamp)>3L) {
        trackStatus=0;
        printf("Track is not running on antenna %d.\n",antenna);
        }
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

        if(trackStatus==1) {

	 if(gotnomosaic) {
        gotMosaicFlag=0;
        rm_status = rm_write(antenna,"RM_MOSAIC_NAME_FLAG_S",&gotMosaicFlag);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
                }
        } else {
        gotMosaicFlag=1;
        rm_status = rm_write(antenna,"RM_MOSAIC_NAME_FLAG_S",&gotMosaicFlag);
                if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
                }
        }

	
	/* get planet diameter from rm */
	rm_status=rm_read(antenna,"RM_PLANET_DIAMETER_ARCSEC_D",
			&arcseconds);
	raoff=raoff_sign*arcseconds/2.0;
	decoff=decoff_sign*arcseconds/2.0;

	rm_status=rm_write(antenna, "RM_RAOFF_ARCSEC_D",&raoff);
	 if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
	rm_status=rm_write(antenna, "RM_DECOFF_ARCSEC_D",&decoff);
	 if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

        rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
                                        &command_n);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

	pmac_command_flag=0;

	rm_status=rm_write_notify(antenna,"RM_SMARTS_PMAC_COMMAND_FLAG_S",
                                        &pmac_command_flag);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
	}

        error_flag=RUNNING;
 
/*
	rm_status=rm_read(antenna, "RM_SMARTS_COMMAND_STATUS_S",&error_flag);
         if(rm_status != RM_SUCCESS) {
         rm_error_message(rm_status,"rm_read()");
         exit(1);
         }

        if(error_flag==ERROR)
        {
        printf("Error from track:\n");
        rm_status=rm_read(antenna,
                "RM_TRACK_MESSAGE_C100",messg);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
        }
        printf("%s\n",messg);
        }
*/
	
	} /* if track is running */
        } /* if antenna is online */
        } /* for 8 antennas */
return(0);
}

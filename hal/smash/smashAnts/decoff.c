/***************************************************************
*
* decoff.c
* 
* NAP 18 SEP 2002.
*
****************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <smapopt.h>
#include "rm.h"
#include "commonLib.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))


void usage(int exitcode, char *error, char *addl) {

        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: decoff [options] --arcseconds (or -s) <arcseconds>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -n or --nomosaic  to not append offsets to sourcename\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n");
        exit(exitcode);
}

int main(int argc, char *argv[])  
{

	char c,*arcseconds,command_n[30];
/*      char messg[100]; */
	short pmac_command_flag=0;
	short error_flag=RUNNING;
	int gotdecoff=0,rm_status,antlist[RM_ARRAY_SIZE];
	double decoff;
	int i, antenna;
	smapoptContext optCon;
        int gotantenna=0,antennaArray[SMAPOPT_MAX_ANTENNAS+1];
	int gotnomosaic=0;
	short gotMosaicFlag=0;
	int antennas[SMAPOPT_MAX_ANTENNAS+1];
        int trackStatus=0,iant;
        int tracktimestamp,timestamp;
	int numberOfAnts=0;

	

 
        struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"nomosaic",'n',SMAPOPT_ARG_NONE,0,'n'},
                {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
                {"arcseconds",'s',SMAPOPT_ARG_STRING,&arcseconds,'s'},
                {NULL,0,0,NULL,0}
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least
decoff(arcseconds) required.");

        optCon = smapoptGetContext("decoff", argc, argv, optionsTable,0);

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

                case 's':
                gotdecoff=1;
		decoff = atof(arcseconds);
                break;

                }


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

	 if(gotdecoff!=1) usage(-2,"No offset specified","Az (arcseconds) is 
	required .\n");

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

        for(iant=1;iant<=ANT_ARRAY_SZ;iant++) {
	
        if(antennaArray[iant]==1) {
        antenna=iant;

          /* check if track is running on this antenna */

        rm_status=rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
	rm_status=rm_read(antenna,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if ((abs(tracktimestamp-timestamp)>3L) &&
	    (antenna <= 8)) {
        trackStatus=0;
        printf("Track is not running on antenna %d.\n",antenna);
        }
        if ((abs(tracktimestamp-timestamp)<=3L) ||
	    (antenna > 8)) trackStatus=1;

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
        } /* for ANT_ARRAY_SZ antennas */
return(0);
}
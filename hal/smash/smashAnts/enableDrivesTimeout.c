/***************************************************************
*
* enableDrivesTimeout.c
* 
* NAP 25 Jan 2005
*
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"
#include "commonLib.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))


void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: disableDrivesTimeout [options] \n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                " (or comma-separated list of specific antennas)\n"
                " (or range of antennas e.g. 2..6)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


int main(int argc, char *argv[])  
{

	char c;
	int rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int iant, antenna;
	int antennaArray[SMAPOPT_MAX_ANTENNAS+1],
                antennas[SMAPOPT_MAX_ANTENNAS+1];
	int numberOfAnts=0;

        int gotantenna=0;
	short disableDrivesFlag=0;


	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
                {NULL,0,0,NULL,0}
        };


        optCon = smapoptGetContext("closeM3", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
 
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
               case 'a':
		gotantenna=1;
                break;
		}
 
        }

 
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
        else {
	getAntennaList(antennaArray);
        }


 
	  for(iant=1;iant<=8;iant++) {

        if(antennaArray[iant]==1) {
        antenna=iant;

           rm_status=rm_write(antenna,"RM_DRIVES_TIMEOUT_FLAG_S",
                                        &disableDrivesFlag);
           if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
           }
 
        } /* if antenna is online */
        } /* for 8 antennas */
return(0);
}

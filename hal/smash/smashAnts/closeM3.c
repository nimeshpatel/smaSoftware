/***************************************************************
*
* closeM3.c
* 
* NAP 3 March 2000 revised for PowerPC
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

char zero = 0;

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: closeM3 [options] \n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


int main(int argc, char *argv[])  
{

	char c,command_n[30];
/*      char messg[100]; */
	int rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int i,iant ;
	int antennaArray[SMAPOPT_MAX_ANTENNAS+1],
                antennas[SMAPOPT_MAX_ANTENNAS+1];
	int numberOfAnts=0;

        int gotantenna=0;


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
	for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };

 
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
		rm_write(iant, "RM_M3CMD_B", &zero);
	    }
	}

 
    return(0);
}

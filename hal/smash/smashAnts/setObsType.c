/***************************************************************
*
* setObsType.c
* 
* SMAsh command for setting Observation type for a given project/source
* NAP 
* 27 Nov 2002
*
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"
#include "dsm.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: setObsType [options] --type <sourcename>\n"
                "[options] include:\n"
		"--type -t <main,flux,bandpass,gain,ipoint,planet,holography>\n"
		"--epoch -e <1950./2000.>\n"
		"--velocity or -v <velocity km/s (only vlsr for now)>]\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "                  (default: all online antennas)\n");
        exit(exitcode);
}

void main(int argc, char *argv[])  
{
	FILE *fp;
	char c,*source,Source[34],messg[100], command_n[30];
	char *type;
	short i, pmac_command_flag=0; 
	short error_flag=RUNNING, sourcelength;
	int gotsource=0,gottype=0,rm_status,antlist[RM_ARRAY_SIZE];
	int dsm_status;
	int sourceType=0;
	short sourceTypeShort=0;
	int gotantenna=0,gotcoordinates=0,gotvelocity=0;
	smapoptContext optCon;	
	int antenna;
	int antennaArray[]={0,0,0,0,0,0,0,0,0};
	int iant,ipad;
	int tracktimestamp,timestamp;
	char line[BUFSIZ];
	int trackStatus=0;
	short antennaStatus = 0;

	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antenna",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"source",'s',SMAPOPT_ARG_STRING,&source,'s'},
                {"type",'t',SMAPOPT_ARG_STRING,&type,'t'},
                {NULL,0,0,NULL,0}
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least
source- name required.");
 
        optCon = smapoptGetContext("setObsType", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
                case 'a':
		gotantenna=1;
                break;
 
                case 's':
                gotsource=1;
                break;

                case 't':
                gottype=1;
                break;
 
                }
 
        }
 
        if(gotsource!=1) usage(-2,"No source specified","Source name is required .\n");

	/* check if source type is valid */
	if(gottype==0)  sourceType=0;
	else {
	gottype=-1;
	if(!strcmp(type,"main")) {sourceType=0x0; gottype=1;}
	if(!strcmp(type,"flux")) {sourceType=0x1; gottype=1;}
	if(!strcmp(type,"bandpass")) {sourceType=0x2; gottype=1;}
	if(!strcmp(type,"gain")) {sourceType=0x4; gottype=1;}
	if(!strcmp(type,"ipoint")) {sourceType=0x8; gottype=1;}
	if(!strcmp(type,"planet")) {sourceType=0x10; gottype=1;}
	if(!strcmp(type,"holography")) {sourceType=0x20; gottype=1;}
	}
        if(gottype==-1) usage(-3,"Invalid source type."," Source type should be either of main,flux,bandpass,gain,ipoint,holography\n");

 
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

	/* initialize dsm */
	  dsm_status = dsm_open();
	  if(dsm_status!= DSM_SUCCESS) {
	    dsm_error_message(dsm_status, "dsm_open");
	    exit(-1);
	  }

	


	if(gotantenna==1) {
	antennaArray[antenna]=1;
	} 
	else {
	/* check which antennas are having ONLINE status (as set
	   by the project command */
	for(iant=1;iant<=8;iant++) {
	i=0;
        while(antlist[i] != RM_ANT_LIST_END)
        	if(iant == antlist[i++]) {
		rm_read(iant,"RM_ANTENNA_STATUS_S",&antennaStatus);
		if(antennaStatus==1) antennaArray[iant]=1;
		if(antennaStatus==0) antennaArray[iant]=0;
		}
        }
	}

	sourcelength=strlen(source);
	strcpy(Source,source);
	for(i=0;i<30;i++) {
	command_n[i]=0x0;
	}
	command_n[0]='n';/*0x6e;*/ /* send 'n' command */
	command_n[1]=0x0;
	pmac_command_flag=0;

	
	

	dsm_status=dsm_write_notify("m5","DSM_AS_SOURCE_C34",Source);
        if(dsm_status != RM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }
	dsm_status=dsm_write_notify("m5","DSM_AS_SOURCE_TYPE_L",&sourceType);
        if(dsm_status != RM_SUCCESS) {
                dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
        }


	for(iant=1;iant<=8;iant++) {

	if(antennaArray[iant]==1) {
	antenna=iant;

	  /* check if track is running on this antenna */
        
        rm_status=rm_read(antenna,"RM_OBSTYPE_L",&sourceType);
        rm_status=rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
	rm_status=rm_read(antenna,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if(abs(tracktimestamp-timestamp)>3L) {
	trackStatus=0;
	printf("Track is not running on antenna %d.\n",antenna);
	}
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

	if(trackStatus==1) {

	
	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_LENGTH_S",
				&sourcelength);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_C34",Source);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

	rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
					command_n);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
	
	rm_status=rm_write_notify(antenna,"RM_SMARTS_PMAC_COMMAND_FLAG_S",
					&pmac_command_flag);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
        }

	sourceTypeShort=(short)sourceType;
	rm_status=rm_write(antenna,"RM_OBSTYPE_S",&sourceTypeShort);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
	}

	} /* if track is running */
	} /* if antenna is online */
	} /* for 8 antennas */
}

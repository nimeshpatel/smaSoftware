/***************************************************************
*
* observe.c
* 
* SMAsh command for tracking an astronomical source
* NAP 
* 1 March 2000
*
* This version is for the PowerPC
* Command line arguments are now parsed using the smapopt library.
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
#include "dDS.h"
#include "smadata.h"
#include "rm.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

int newObject(void);
int wakeup_smadata(void);

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: observe [options] --source (or -s) <sourcename>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -w or --wait    for waiting to acquire source\n"
                "                  (default: nowait)\n"
                "  -n or --norpc   to avoid communicating to DDS and\n"
                "                  smadata about source change \n"
                "                  (default: rpc)\n"
                "  -f or --fits    to save data as FITS (default: no saving)\n" 
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "                  (default: all antennas)\n");
        exit(exitcode);
}

void main(int argc, char *argv[])  
{
	char c,*source,Source[34],messg[100], command_n[30];
	short i, pmac_command_flag=0; 
	short error_flag=RUNNING, sourcelength;
	int rpc=1,wait=0,gotsource=0,rm_status,fits=0,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;	
	int antenna;

	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"norpc",'n',SMAPOPT_ARG_NONE,0,'n'},
                {"fits",'f',SMAPOPT_ARG_NONE,0,'f'},
                {"wait",'w',SMAPOPT_ARG_NONE,0,'w'},
                {"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"source",'s',SMAPOPT_ARG_STRING,&source,'s'},
                {NULL,0,0,NULL,0}
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least
source- name required.");
 
        optCon = smapoptGetContext("observe", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
 
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
                case 'n':
                rpc=0;
                break;

                case 'f':
                fits=1;
                break;
 
                case 'a':
                break;
 
                case 'w':
                wait=1;
    		printf("Waiting to acquire the source...\n");
                break;
 
                case 's':
                gotsource=1;
                break;
 
                }
 
        }
 
        if(gotsource!=1) usage(-2,"No source specified","Source name is required .\n");
 
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
	

	sourcelength=strlen(source);
	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_LENGTH_S",
				&sourcelength);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

	strcpy(Source,source);

	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_C34",Source);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

	for(i=0;i<30;i++) {
	command_n[i]=0x0;
	}

	/* send 'n' command */
	command_n[0]='n';/*0x6e;*/
	command_n[1]=0x0;
	rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
					command_n);
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

/* Now wait until the source has been reached.
*/

	error_flag=RUNNING;

	if(rpc)
	{
	/* inform Taco's dDSServer of new source */
	newObject();
	}

	if(fits)
	{
	/* inform JunHui's smadata_svc of new source */
	if(wakeup_smadata()!=OK)
	fprintf(stderr,"Error from wakeup_smadata.\n");
	}
	
	sleep(1);


         rm_status=rm_read(antenna,
		"RM_SMARTS_COMMAND_STATUS_S",&error_flag);
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

	if(wait)
	{
		while(error_flag==RUNNING)
		{
          	 rm_status=rm_read(antenna,
			"RM_SMARTS_COMMAND_STATUS_S",&error_flag);
       		 if(rm_status != RM_SUCCESS) {
       	         rm_error_message(rm_status,"rm_read()");
       	         exit(1);
       		 }
		sleep(1);
		}
	if(error_flag==OK) printf("antenna has reached the source.\n");
	}
}

/*
    newObject.c

	This is from Taco- see email of 1 Dec 99.

    This function sends an RPC client request to the DDS server, telling it
  to request new coordinates from the coordinates server.   The client
  handle is created and destroyed each time this function is called, which
  increases the overhead somewhat, but should make the code more robust.
*/


/*
  statusCheck checks the status returned by the client call to the DDS
  server.
*/
int statusCheck(dDSStatus *status)
{
  if (status == NULL) {
    fprintf(stderr,
"NULL pointer returned from client call to DDS server.   Server terminated?\n"
	    );
    return(ERROR);
  }
  if (status->status != DDS_SUCCESS) {
    fprintf(stderr, "DDS server returned error status, reason = %d\n",
      status->reason);
    return(ERROR);
  }
  return(OK);
}

int newObject(void)
{
  CLIENT *cl;
  dDSCommand command;
  int returnCode;

  /* First get client handle */

  if (!(cl = clnt_create("dds", DDSPROG, DDSVERS, "tcp"))) {
    clnt_pcreateerror("dds");
    return(ERROR);
  }

  command.antenna = DDS_ALL_ANTENNAS;
  command.receiver = DDS_ALL_RECEIVERS;
  command.command = DDS_GET_COORDS;
  returnCode = statusCheck(ddsrequest_1(&command, cl));

  clnt_destroy(cl);
  return(returnCode);
}

/* This code is the client for Jun-Hui's smadata_svc. Jun-Hui
sent this code by email on 16 Dec 1999 */
int wakeup_smadata(void)
{
         retjob *result, Wakeup;
 
 
         CLIENT  *SMADATA_cl;    /* client handle */
 
/* creates the "client handle" to SMADATA */
   if (!(SMADATA_cl = clnt_create(SMADATA,SMADATAPROG,SMADATAVERS,"tcp")))
    {
            clnt_pcreateerror(SMADATA);
                exit(1);
       }
 
           result = sour_wakeup_1(&Wakeup, SMADATA_cl);
 
/*         printf( "in wakeup client\n");
*/
           return(OK);
}


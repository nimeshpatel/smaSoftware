/***************************************************************
*
* chopperCommand.c
* 
* NAP 23 March 2000 revised for Antennas >= 4
*
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: chopperCommand [options] --command (or -c) <command>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (note: antenna number must be >= 4)\n");
        exit(exitcode);
}


void main(int argc, char *argv[])  
{
	char c,messg[100],*command,command_n[30];
	short pmac_command_flag=1;
	short error_flag=RUNNING;
	int gotc=0,rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int i,antenna;


	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"command",'c',SMAPOPT_ARG_STRING,&command,'c'},
                {NULL,0,0,NULL,0}
        };

	 for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };



 if(argc<2) usage(-1,"Insufficient number of arguments","At least
a  command is  required.");
 
        optCon = smapoptGetContext("chopperCommand", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
 
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
               case 'a':
                break;
 
                case 'c':
                gotc=1;
                break;
                }
 
 
        }
 
 if(gotc!=1) usage(-2,"No command specified","A command
for the chopper is required .\n");

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
 
	strcpy(command_n,command);
   /* send the command */
        rm_status=rm_write(antenna,"RM_CHOPPER_SMASH_COMMAND_C30",
                                        &command_n);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
 
        pmac_command_flag=1;

rm_status=rm_write_notify(antenna,"RM_CHOPPER_SMASH_COMMAND_FLAG_S",
                                        &pmac_command_flag);
 
     if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
        }
 
        error_flag=RUNNING;
 
        sleep(1);
 
}

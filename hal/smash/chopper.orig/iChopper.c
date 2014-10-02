/***************************************************************
*
* iChopper.c
* 
* RWW 4 September 2003 Interactively issue PMAC commands and standard
* chopper commands.
*
****************************************************************/
static char rcsid[] = "$Id: choppersmash.c$";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smapopt.h"
#include "rm.h"

#define  PMAC_RTN_SIZE 128

/* Definitions which should be in readline/readline.h, but aren't in LynxOS */
void readline_initialize_everything(void);
char *readline(char *);
void add_history(char *);
int read_history(char *);
int write_history(char *);
#define HIST_FILE "./.iChopper_history"

int ant = -1;
int rm_antlist[RM_ARRAY_SIZE];
short pmac_command_flag = 2;

void usage(void) {
    fprintf(stderr, "Usage: iChopper ant_number (1-8)\n");
    exit(1);
}


int main(int argc, char *argv[]) {
    int i;
    int rm_status;
    char command[100];
    char response[PMAC_RTN_SIZE];
    char var_name[RM_NAME_LENGTH];
    char *line, prompt[12];

    if(argc != 2) usage();
    ant = atoi(argv[1]);
    if(ant < 0 || ant > 8) usage();

    /* initializing ref. mem. */
    rm_status=rm_open(rm_antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }
    /* start listening to ref. mem. interrupts for SMAsh commands*/
    rm_status=rm_monitor(ant, "RM_CHOPPER_PMAC_RESPONSE_C128");
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"monitor PMAC response");
        exit(1);
    }
    for(i = 0; ; i++) {
        if(rm_antlist[i] == RM_ANT_LIST_END) {
            fprintf(stderr, "No reflective memory for antenna %d\n", ant);
            exit(2);
        }
        if(ant == rm_antlist[i]) break;
    }
    sprintf(prompt, "chopper %d >", ant);
    readline_initialize_everything();
    read_history(HIST_FILE);

    for(;;) {

	line = readline(prompt);
	if(! *line) continue;
	add_history(line);
        sscanf(line, "%s", command);
	free(line);
	if(strcmp(command, "q") == 0) exit(0);

        /* send the command */
        rm_status=rm_write(ant,"RM_CHOPPER_SMASH_COMMAND_C30",
                           &command);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_write()");
            exit(1);
        }
        rm_status=rm_write_notify(ant,"RM_CHOPPER_SMASH_COMMAND_FLAG_S",
                                  &pmac_command_flag);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_write_notify()");
            exit(1);
        }
        rm_status=rm_read_wait(&i,var_name, &response);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"Reading PMAC response");
            exit(1);
        }
	for(i = 0; i < PMAC_RTN_SIZE; i++) {
	    if(response[i] == '\r') response[i] = '\n';
	    if(response[i] == 0) break;
	}
	printf("%s", response);

    }
    return(0);
}

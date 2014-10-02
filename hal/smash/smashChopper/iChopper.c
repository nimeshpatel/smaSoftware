/***************************************************************
*
* iChopper.c
* 
* RWW 4 September 2003 Interactively issue PMAC commands and standard
* chopper commands.
*
****************************************************************/
static char rcsid[] = "$Id: iChopper.c,v 1.4 2008/01/15 21:56:57 rwilson Exp $";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smapopt.h"
#include "rm.h"
#include "chopperControl.h"

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
short pmac_command_flag = ICHOPPER_CMD;
int rm_status;
char command[30];
char response[PMAC_RTN_SIZE];
char var_name[RM_NAME_LENGTH];

void usage(void) {
    fprintf(stderr, "Usage: iChopper ant_number (1-8)\n");
    exit(1);
}

/* iChopper.c */
void SendCommandGetReturn(char *cmd);
void PrintStatus(char motor);

int main(int argc, char *argv[]) {
    int i;
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
    line = 0;
    readline_initialize_everything();
    read_history(HIST_FILE);

    for(;;) {

	if(line) free(line);
	line = readline(prompt);
	if(line == 0 || strcmp(line, "q") == 0) {
	    write_history(HIST_FILE);
	    exit(0);
	}
	if(! *line) continue;
	add_history(line);

	if(strncmp(line, "status", 6) == 0) {

	    SendCommandGetReturn("#1?");
	    PrintStatus('X');
	    /* A 1 sec sleep sometimes fails.  I don't understand. */
	    sleep(2);
	    SendCommandGetReturn("#2?");
	    PrintStatus('Y');
	    sleep(2);
	    SendCommandGetReturn("#3?");
	    PrintStatus('Z');
	    sleep(2);
	    SendCommandGetReturn("#4?");
	    PrintStatus('T');
	} else {
	    SendCommandGetReturn(line);
	    for(i = 0; i < PMAC_RTN_SIZE; i++) {
		if(response[i] == '\r') response[i] = '\n';
		if(response[i] == 0) break;
	    }
	    printf("%s", response);
	}

    }
    return(0);
}

void SendCommandGetReturn(char *cmd) {
	int antn;

	strncpy(command, cmd, 29);
	command[29] = 0;
/*      sscanf(line, "%s", command); */

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
        rm_status=rm_read_wait(&antn, var_name, &response);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"Reading PMAC response");
            exit(1);
        }
}

void PrintStatus(char motor) {
    char outstr[80];
    int len, new;
    struct STAT {
	int c, b;
	char msg[16];
    } *sp, stat[] = {
	{0, 8, "Active"},
	{0, 4, "In Pos Lim"},
	{0, 2, "In Neg Lim"},
	{1, 4, "Open Loop"},
	{2, 2, "Commanded Vel 0"},
	{3, 4, "Search for Home"},
	{8, 4, "Amp Enabled"},
	{9, 8, "Stopped on lmt"},
	{9, 4, "Home Complete"},
	{11, 8, "Amplifier Fault"},
	{11, 4, "Ftl follow err"},
	{11, 2, "Wrn follow err"},
	{11, 1, "In Position"}
    };
#define NUMERRS (sizeof(stat) / sizeof (struct STAT))

    sprintf(outstr, "%c Motor:", motor);
    len = strlen(outstr);
    for(sp = stat; sp < &stat[NUMERRS]; sp++) {
	if(response[sp->c] & sp->b) {
	    new = strlen(sp->msg);
	    if(len + new > 77) {
		printf("%s\n", outstr);
		len = 8;
		outstr[7] = 0;
	    }
	    if(len > 8)
		strcat(outstr, ", ");
	    else
		strcat(outstr, " ");
	    strcat(outstr, sp->msg);
	    len += new;
	}
    }
    if(len > 4) printf("%s\n", outstr);
    
}

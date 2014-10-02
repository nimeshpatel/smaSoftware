#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include "smapopt.h"
#include "dsm.h"
#include "commonLib.h"

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)."},
            {"chop",'c',SMAPOPT_ARG_NONE, 0, 'c',"Use if chopping"},
            {"freerun",'f',SMAPOPT_ARG_NONE, 0, 'f',
		"Use if not chopping (default)"},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
            {NULL,0,0,NULL,0,0},
	    {"Antenna list defaults to antennas in array"}
        };
smapoptContext optCon;
int antennasGiven = 0;
char host[8] = "acc1";

int main(int argc, char *argv[]) {
	char *extra, msg[32];
	int status, ant;
	short value = 3;

	optCon = smapoptGetContext("setcdmode", argc, argv, optionsTable,
		SMAPOPT_CONTEXT_EXPLAIN);
    while ((status = smapoptGetNextOpt(optCon)) != -1) {
        switch(status) {
        case 'a':
            antennasGiven = 1;
            break;
        case 'c':
	    value = 2;
            break;
        case 'f':
	    value = 3;
            break;
        case 'h':
	    smapoptPrintHelp(optCon, stderr, 0);
	    putc('\n', stderr);
            exit(0);
	default:
	    fprintf(stderr, "%s: %s\n", smapoptBadOption(optCon, 0),
		smapoptStrerror(status));
            exit(0);
        }

    }
    if((extra = smapoptGetArg(optCon)) != 0) {
	fprintf(stderr, "!! Extra argument %s !!\n", extra);
	smapoptPrintHelp(optCon, stderr, 0);
	putc('\n', stderr);
	exit(0);
    }
    smapoptFreeContext(optCon);
    if(!antennasGiven) {
        getAntennaList(requestedAntennas);
    }
    for(ant = 1; ant < ANT_LIST_SZ; ant++) {
	printf(" %d", requestedAntennas[ant]);
    }
    printf("\n");
	printf("Will set cd mode to %d\n", value);

#if 1
    if((status = dsm_open()) != DSM_SUCCESS) {
	dsm_error_message(status, "Dsm open failed");
	exit(1);
    }
    for(ant = 1; ant < ANT_LIST_SZ; ant++) {
	if(requestedAntennas[ant]) {
	    host[3] = '0' + ant;
	    if((status = dsm_write_notify(host, "DSM_AH_CONT_DET_MODE_S",
			&value)) != DSM_SUCCESS) {
		sprintf(msg, "setcdmode: DSM write for %s failed", host);
		dsm_error_message(status, msg);
		exit(2);
	    }
	}
    }
#endif
    return(0);
}

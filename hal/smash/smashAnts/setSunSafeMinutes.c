/***************************************************************
*
* setSunSafeMinutes.c
* 
* SMAsh command to set the required minimum number of minutes ahead of the
* sun for an antenna to track if it is in the path of the sun avoidance
* zone.
*
* RWW  28 July 2005
*
****************************************************************/
static char rcsid[] = "$Id:";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smapopt.h"
#include "rm.h"
#include "commonLib.h"

#define VERBOSE 0

static char sunSafeMinutesv[] = "RM_REQUIERD_SUN_SAFE_MINUTES_S";
int minutes = 120;

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
int antennasGiven = 0;
int rm_antlist[RM_ARRAY_SIZE];

static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)."},
	    {"minutes", 'm', SMAPOPT_ARG_INT, &minutes, 0,
		"required minutes ahead of sun (120)"},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
            {NULL,0,0,NULL,0,0}
        };
smapoptContext optCon;

void usage(void) {

    smapoptPrintHelp(optCon, stderr, 0);
    fprintf(stderr, "\nAntenna list defaults to antennas in array\n");
}

int main(int argc, char *argv[]) {
    int i, ant, j, rm_status;
    short sunSafeMinutes;
    char c, messg[64];

    optCon = smapoptGetContext(argv[0], argc, argv, optionsTable,0);

    while ((c = smapoptGetNextOpt(optCon)) >= 0) {
        switch(c) {
        case 'a':
            antennasGiven = 1;
            break;
        case 'h':
            usage();
            exit(0);
        }

    }
    if(c < -1) {
      fprintf(stderr, "%s: %s\n",
            smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
            smapoptStrerror(c));
    }
    smapoptFreeContext(optCon);

    /* initializing ref. mem. */
    rm_status=rm_open(rm_antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }
    /* If anetnna option not given, default to those in array */
    if(!antennasGiven) {
        getAntennaList(requestedAntennas);
    }


#if VERBOSE
    printf("Antennas with RM:");
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
        printf(" %d", ant);
    }
        printf("\nRequested Antennas:");
        for(i = 1; i < ANT_LIST_SZ; i++) {
            printf(" %d", requestedAntennas[i]);
        }
    putchar('\n');
#endif VERBOSE
    for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	if(requestedAntennas[ant]) {
	    rm_antlist[j++] = ant;
	}
    }
    rm_antlist[j] = RM_ANT_LIST_END;

    printf("Antennas to be commanded: ");
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	printf(" %d", ant);
    }
    printf("\n");

    sunSafeMinutes = minutes;
    /* Now issue the commands to each antenna through rm. */
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	rm_status = rm_write(ant, sunSafeMinutesv, &sunSafeMinutes);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Writing sunSafeMinutes to ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
    }
    return(0);
}

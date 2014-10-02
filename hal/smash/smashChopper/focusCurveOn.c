/***************************************************************
*
* focusCurevOn.c
* 
* SMAsh commands to control tracking of the subreflector to the
* the optimum focus position and automatic energizing of the drives
* to keep the correct position.
* RWW 
* 19 February 2004
*
****************************************************************/
static char rcsid[] = "$Id: focusCurveOn.c,v 1.4 2008/01/15 22:03:47 rwilson Exp $";

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

/* Subreflector tracking control flag.
 * -1, no automatic updates
 * 0 automatic updates, but don't follow the focus curve
 * 1 keep the subreflector in position and follow the focus curve.
 */
char focusCurveFlagv[] =    "RM_CHOPPER_FOCUS_CURVE_FLAG_L";

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
int antennasGiven = 0;
int rm_antlist[RM_ARRAY_SIZE];

/* Names under which this program might be called */
char names[][14] = {"freezeFocus", "focusCurveOff", "focusCurveOn"};
#define NUMNAMES (sizeof(names) / sizeof(names[0]))

static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)."},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
            {NULL,0,0,NULL,0,0}
        };
smapoptContext optCon;

/* focusCurve.c */

void usage(void) {

    smapoptPrintHelp(optCon, stderr, 0);
    fprintf(stderr, "\nAntenna list defaults to antennas in array\n");
}

int main(int argc, char *argv[]) {
    int flag, i, ant, j, rm_status;
    char c, *cp, messg[48];;

    if((cp = strrchr(argv[0], '/')) == NULL) {
	cp = argv[0];
    } else {
	cp++;
    }
    for(flag = 0; flag < NUMNAMES; flag++) {
	if(strcmp(names[flag], cp) == 0) break;
    }
    if(flag == NUMNAMES) {
	fprintf(stderr, "The command %s is not implimented\n", argv[0]);
	exit(1);
    }
    flag--;
    optCon = smapoptGetContext(cp, argc, argv, optionsTable,0);

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

    /* Now issue the command to each chopperControl through rm. */
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	rm_status = rm_write(ant, focusCurveFlagv, &flag);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Writing smash command to ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
    }
    return(0);
}

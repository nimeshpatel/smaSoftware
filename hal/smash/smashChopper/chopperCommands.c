/***************************************************************
*
* chopperCommands.c
* 
* SMAsh commands for controlling antenna choppers through reflective
* memory using the chopperControl daemon.
* RWW 
* 9 September 2003
*
****************************************************************/
static char rcsid[] = "$Id: chopperCommands.c,v 1.9 2008/01/15 22:03:46 rwilson Exp $";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smapopt.h"
#include "rm.h"
#include "commonLib.h"
#include "chopperControl.h"

#define VERBOSE 0

/* Chopper system control and status RM variables */
static char cmdv[] = "RM_CHOPPER_SMASH_COMMAND_C30";
static char cmdFlagv[] = "RM_CHOPPER_SMASH_COMMAND_FLAG_S";
#if 0
char statusBitsv[] = "RM_CHOPPER_STATUS_BITS_C16";
static char unixTimev[] = "RM_UNIX_TIME_L";
#endif

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
int antennasGiven = 0;
int rm_antlist[RM_ARRAY_SIZE];
int unixTime;
short int one = 1;

/* Names under which this program might be called */
char names[][12] = {"initC", "farC", "resetC", "restartC",
	"startC", "stopC", "stowC", "driftC",
	"homeC", "checkC", "nearC", "positionC"};
#define NUMNAMES (sizeof(names) / sizeof(names[0]))

static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)."},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
            {NULL,0,0,NULL,0,0}
        };
smapoptContext optCon;

/* chopperCommands.c */
void GetUnixTime(void);

#if 0
void errorOut(char *error, char *addl) {
    if (error) fprintf(stderr, "\n%s: %s.  For help use -h.\n",
                           error, addl);
    exit(1);
}
#endif

void usage(void) {

    smapoptPrintHelp(optCon, stderr, 0);
    fprintf(stderr, "\nAntenna list defaults to antennas in array\n");
}

int main(int argc, char *argv[]) {
    int i, ant, j, rm_status;
    char c, *cp, smashCommand[30], messg[48];;

    if((cp = strrchr(argv[0], '/')) == NULL) {
	cp = argv[0];
    } else {
	cp++;
    }
    for(i = 0; i < NUMNAMES; i++) {
	if(strncmp(names[i], cp, strlen(names[i])) == 0) break;
    }
    if(i == NUMNAMES) {
	fprintf(stderr, "The command %s is not implimented\n", argv[0]);
	exit(1);
    }
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

    strncpy(smashCommand, cp, sizeof(smashCommand));
    /* Now issue the commands to each chopperControl through rm. */
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	rm_status = rm_write(ant, cmdv, smashCommand);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Writing smash command to ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
	rm_status = rm_write_notify(ant, cmdFlagv, &one);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Writing smash command flag to ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
    }



#if 0
    for(ant = 1, i = 0; ant < ANT_LIST_SZ; ant++) {
	if(ant < rm_antlist[i]) {
	    requestedAntennas[ant] = 0;
	} else {
	    i++;
	}
    }

    GetUnixTime();
    /* Limit list to requested antennas with active deiced */
    for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
        int deicedTime;
        if( requestedAntennas[ant]) {
            rm_status = rm_read(ant, statusTimestampv, &deicedTime);
            if(rm_status != RM_SUCCESS) {
                sprintf(ts, "Reading deiced time from antenna %d", ant);
                rm_error_message(rm_status, ts);
            }
#if VERBOSE
            printf("Antenna %d unixtime %d, deicedTime %d\n", ant,
                   unixTime, deicedTime);
#endif VERBOSE
            rm_antlist[j++] = ant;
        }
    }
#endif

    return(0);
}

#if 0
void GetUnixTime(void) {
    int i;

    for(i = 0; rm_antlist[i] != RM_ANT_LIST_END; i++) {
        if(rm_read(rm_antlist[i], unixTimev, &unixTime) == RM_SUCCESS) {
            return;
        }
    }
}
#endif

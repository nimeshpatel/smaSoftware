/***************************************************************
*
* postionChopper.c
* 
* SMAsh command for positioning antenna choppers in x, y, z, and tilt
* It uses reflective memory to contact the chopperControl daemon.
* RWW 
* 19 September 2003
*
****************************************************************/
static char rcsid[] = "$Id:";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "smapopt.h"
#include "rm.h"
#include "commonLib.h"
#include "chopperControl.h"

#define VERBOSE 0

/* Chopper system control and status RM variables */
char cmdv[] = "RM_CHOPPER_SMASH_COMMAND_C30";
char cmdFlagv[] = "RM_CHOPPER_SMASH_COMMAND_FLAG_S";
char farPosMmv[] =      "RM_CHOPPER_FAR_POS_MM_V4_F";
char cmdPosMmv[] =        "RM_CHOPPER_CMD_POS_MM_V4_F";
char cmdPosTypev[] =    "RM_CHOPPER_CMD_POS_TYPE_C1";
float maxCmdPosMm[4] = {MAX_Xmm, MAX_Ymm, MAX_Zmm, MAX_Tmm};
float minCmdPosMm[4] = {MIN_Xmm, MIN_Ymm, MIN_Zmm, MIN_Tmm};
#if 0
char statusBitsv[] = "RM_CHOPPER_STATUS_BITS_C16";
static char unixTimev[] = "RM_UNIX_TIME_L";
#endif

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
int rm_antlist[RM_ARRAY_SIZE];
int unixTime;
short int one = 1;
double argPos[4];
const double cnt2mm[4] = {X_MM_PER_COUNT, Y_MM_PER_COUNT, Z_MM_PER_COUNT,
	TILT_ARCSEC_PER_COUNT};
int antennasGiven = 0;
int posGiven[4] = {0,0,0,0};
char smashCommand[30] = "position";

/* Control of conversions of position arguments for 4 cases */
int conversionControl = 0;
#define USE_COUNTS 1
#define USE_FAR 2

static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)"},
            {"counts",'c',SMAPOPT_ARG_NONE,0,'c',
		"Interpret positions and tilt in counts"},
            {"far",'f',SMAPOPT_ARG_NONE,0,'f',
		"Interpret positions with respect to the far position"},
            {"x",'x',SMAPOPT_ARG_DOUBLE,argPos,'x',
		"X position of the subreflector"},
            {"y",'y',SMAPOPT_ARG_DOUBLE,&argPos[1], 'y',
		"Y position of the subreflector"},
            {"z",'z',SMAPOPT_ARG_DOUBLE,&argPos[2],'z',
		"Z position of the subreflector"},
            {"tilt",'t',SMAPOPT_ARG_DOUBLE,&argPos[3],'t',
		"Tilt on chop axis of the subreflector"},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
            {NULL,0,0,NULL,0,0}
        };
smapoptContext optCon;

/* positionChopper.c */
void GetUnixTime(void);

void errorOut(char *error, char *addl) {
    if (error) fprintf(stderr, "\n%s: %s.  For help use -h.\n",
                           error, addl);
    exit(1);
}

void usage(void) {

    smapoptPrintHelp(optCon, stderr, 0);
    fprintf(stderr, "\nAntenna list defaults to antennas in array\n"
	"Positions not given remain unchanged\n");
}

int main(int argc, char *argv[]) {
    int i, ant, j, rm_status;
    float cmdPosMm[4], farPosMm[4];
    char c, messg[48];

    optCon = smapoptGetContext("positionChopper", argc, argv, optionsTable,0);

    while ((c = smapoptGetNextOpt(optCon)) >= 0) {
        switch(c) {
        case 'a':
            antennasGiven = 1;
            break;
	case 'x':
	    posGiven[0] = 1;
            break;
	case 'y':
	    posGiven[1] = 1;
            break;
	case 'z':
	    posGiven[2] = 1;
            break;
	case 't':
	    posGiven[3] = 1;
            break;
	case 'c':
	    conversionControl |= USE_COUNTS;
            break;
	case 'f':
	    conversionControl |= USE_FAR;
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
#if 1
    /* Now issue the commands to each chopperControl through rm. */
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	rm_status = rm_read(ant, cmdPosMmv, cmdPosMm);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Reading current cmd position for ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
	if(conversionControl & USE_FAR) {
	    rm_status = rm_read(ant, farPosMmv, farPosMm);
	    if(rm_status != RM_SUCCESS) {
		sprintf(messg, "Reading current far position for ant %d", ant);
		rm_error_message(rm_status, messg);
	    }
	}
	for(j = 0; j < 4; j++) {
	    if(posGiven[j]) {
		switch(conversionControl) {
		case 0:
		    cmdPosMm[j] = argPos[j];
		    break;
		case USE_COUNTS:
		    cmdPosMm[j] = argPos[j] * cnt2mm[j];
		    break;
		case USE_FAR:
		    cmdPosMm[j] = argPos[j] + farPosMm[j];
		    break;
		case USE_COUNTS | USE_FAR:
		    cmdPosMm[j] = argPos[j] * cnt2mm[j] + farPosMm[j];
		    break;
		}
		if(cmdPosMm[j] > maxCmdPosMm[j] ||
			cmdPosMm[j] < minCmdPosMm[j]){
		    fprintf(stderr, "Position out of range, CANCELED.\n");
		    exit(2);
		}
	    }
	}
	rm_status = rm_write(ant, cmdPosMmv, cmdPosMm);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Writing cmd position for ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
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

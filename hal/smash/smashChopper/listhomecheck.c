#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rm.h"
#include "commonLib.h"
#include "chopperControl.h"

/* Chopper system control and status RM variables */
char homeChkv[] =       "RM_CHOPPER_HOME_CHECK_RESULTS_V3_S";
char homeChkTimev[] =   "RM_CHOPPER_HOME_CHECK_TIME_L";
char pmacResetTimev[] = "RM_CHOPPER_PMAC_RESET_TIME_L";
short homeCheckResults[3];
int homeCheckTime, pmacResetTime;

int rm_antlist[RM_ARRAY_SIZE];

int main(int argc, char *argv[]) {
    int ant, i, rm_status;
    char messg[64];

    /* initializing ref. mem. */
    rm_status=rm_open(rm_antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }

    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	rm_status = rm_read(ant, homeChkv, homeCheckResults);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Reading home check results for ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
	rm_status = rm_read(ant, homeChkTimev, homeCheckTime);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Reading home check time for ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
	rm_status = rm_read(ant, pmacResetTimev, pmacResetTime);
	if(rm_status != RM_SUCCESS) {
	    sprintf(messg, "Reading PMAC reset time for ant %d", ant);
	    rm_error_message(rm_status, messg);
	}
	printf("Home check results for ant %d - %d %d %d\n", ant,
	    homeCheckResults[0], homeCheckResults[1], homeCheckResults[2]);
    }
}

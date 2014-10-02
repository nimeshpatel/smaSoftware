/***************************************************************
*
* getAntList.c
* 
* Utility program to parse a comma and ellipsis separated list of
* antennas for shell and Perl SMAsh commands.  
* RWW 
* 14 November 2003
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

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
int present(char *search,char *token);
static int requestedAntennas[ANT_LIST_SZ];
int antennasGiven = 0;
int rm_antlist[RM_ARRAY_SIZE];
int nolog = 0;

static struct smapoptOption optionsTable[] = {
            {"antenna",'a',SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',
                "Antennas (Comma and ellipsis separated list)."},
            {"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
	    {"log", 'l', SMAPOPT_ARG_NONE, &nolog, 'l', 
	     "Do not write the command to the SMAshLog"},
            {NULL,0,0,NULL,0,0}
        };
smapoptContext optCon;

/* getAntList.c */

void usage(void) {
    smapoptPrintHelp(optCon, stderr, 0);
    fprintf(stderr, "\nAntenna list defaults to antennas in array\n");
    fprintf(stderr,"Use -l to prevent the command from being logged to the SMAshLog\n");
}

int main(int argc, char *argv[]) {
    int i, ant, j, rm_status;
    char c, *cp;;

    if((cp = strrchr(argv[0], '/')) == NULL) {
	cp = argv[0];
    } else {
	cp++;
    }
    for (i=0; i<argc; i++) {
      if (present(argv[i],"-l")||present(argv[i],"--log")) {
	nolog = 1;
      }
    }
    if (nolog==1) {
      optCon = smapoptGetContext(cp, argc, argv, optionsTable, SMAPOPT_CONTEXT_NOLOG);
    } else {
      optCon = smapoptGetContext(cp, argc, argv, optionsTable,0);
    }
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

#if VERBOSE
    printf("Antennas to be commanded: ");
#endif VERBOSE
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	printf(" %d", ant);
    }
#if VERBOSE
    printf("\n");
#endif VERBOSE
    exit(0);
}

int present(char *search,char *token) {
  if (strstr(search,token) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

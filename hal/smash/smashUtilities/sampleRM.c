#include <math.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <resource.h>
#include <errno.h>
/* If this is put ahead of math.h and sys/types.h, it hides some definitions */
#define _POSIX_SOURCE 1
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <malloc.h>
#include "rm.h"
#include "smapopt.h"

int antlist[RM_ARRAY_SIZE],rm_status;

#if FOR_HAL
static int antennaArray[SMAPOPT_MAX_ANTENNAS + 1];
#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))
#endif /* FOR_HAL */
int antennasGiven = 0;
int sleepSec = 1;
int useHex = 0;
int numPasses = 1000000;
int showTime = 0;
int startTime, unixTime;
int seriesMS;
static struct	smapoptOption optionsTable[] = {
#if FOR_HAL
    {"antenna",'a',SMAPOPT_ARG_ANTENNAS, antennaArray,'a',
	"Antennas (Comma and ellipsis separated list)." },
#endif /* FOR_HAL */
    {"number", 'n', SMAPOPT_ARG_INT, &numPasses, 0,
	"Number of readings to take, default forever"},
    {"seconds", 's', SMAPOPT_ARG_INT, &sleepSec, 0,
	"Seconds between reading, default 1"},
    {"time", 't', SMAPOPT_ARG_NONE, &showTime, 0,
	"Put seconds since starting in column 1"},
    {"timeseries", 'T', SMAPOPT_ARG_INT, &seriesMS, 0,
	"Interpret a 1 or 2 dim variable as a multicolumn time series.  "
	"Argument is milliseconds between samples.  This only works with "
	"a single variable and a single antenna."},
    {"hex", 'x', SMAPOPT_ARG_NONE, &useHex, 0, "Print ints in hex"},
	SMAPOPT_AUTOHELP
    {NULL,0,0,NULL,0,0},
    {"\nSamplerm will sample a set of RM arguments for each of a set "
	"of antennas\nrepeatedly and write them out in an ascii table.\n"
	"RM variable names (with indices as needed) should follow the options\n"
        "listed below.  Array axes without an index or an empty index will be\n"
        "written out completely.  Arrays of up to 2 dimensions are handled.\n"
	"Indices are more general than single numbers and may include comma\n"
	"and elepsis separated values.  Since the shell treats '[' and ']'\n"
	"specially, variables with indices should be quoted.  As a\n"
	"convenience, all variables may be included within a single pair\n"
	"of double quotes.  For more information see\n"
	"\"SMA Operating Procedures->Troubleshooting\"\n"
	}
};
smapoptContext optCon;

/* Tokens for the list of index values in il1 & il2.  Numbers are >= 0 */
#define IEND -1
#define IRANGE -2
struct VAR {
    char *name;		/* Name as the rm routines want to see it */
    int dim1, dim2;	/* Dimensions of up to two axes of an array */
    short *il1, *il2;	/* Points to a list of indices &/or ranges */
    int type;		/* Type of variable (F, S, etc.) */
    int baseSize;	/* Size of each value */
    struct VAR *next;	/* points to next variable or NULL */
}
*vars = 0, *curVar = 0;
int maxVarSize = 0;
void *rmbuf;

/* sampleRM.c */
void ParseName(char *name);
void ParseIndex(char *cp, short **pil);
int GetToken(char **pcp);
void PrintVar(void);
void PrintVarTimeSeries(void);
void PrintValue(int i1, int i2);
void PrintVarList(void);
void PrintIL(short *il);

int main(int argc, char *argv[]) {
    int n, ant;
    char *cp;

    optCon = smapoptGetContext(NULL, argc, argv, optionsTable,
                               SMAPOPT_CONTEXT_EXPLAIN |
                               SMAPOPT_CONTEXT_NOLOG); /* |
           SMAPOPT_CONTEXT_POSIXMEHARDER); */
    while ((n = smapoptGetNextOpt(optCon)) >= 0) {
        switch(n) {
        case 'a':
            antennasGiven = 1;
            break;
        }

    }
    if(n < -1) {
        fprintf(stderr, "%s: %s\n",
                smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
                smapoptStrerror(n));
        exit(1);
    }
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        fprintf(stderr, "I can't do anything without reflective memory\n");
        exit(1);
    }
#if FOR_HAL
    for(ant = 1, n = 0; ant < ANT_ARRAY_SZ; ant++) {
        if(antlist[n] == ant) {
	    n++;
	} else {
            antennaArray[ant] = 0;
        }
    }
    printf("#Ants: ");
    for(ant = 0, n = 0; ant < ANT_ARRAY_SZ; ant++) {
        if(antennaArray[ant]) {
            printf("%d  ", ant);
	    n++;
	}
    }
    if(seriesMS > 0 && n > 1) {
	fprintf(stderr, "Can not specify multiple antennas with timeseries\n");
	putchar('\n');
	exit(1);
    }
#else
    ant = 0;
#endif /* FOR_HAL */
    printf("\n#RM vars: ");
    while((cp = smapoptGetArg(optCon)) != NULL) {
	char *cp2;
	int inname = 0;

	for(cp2 = cp;; cp2++) {
	    if(!inname) {
		if(*cp2 == 0) break;
		if(isspace(*cp2)) {
		    continue;
		} else {
		    inname = 1;
		    cp = cp2;
		}
	    }
	    if(*cp2 == 0) {
		ParseName(cp);
		break;
	    } else if(isspace(*cp2)) {
		*cp2 = 0;
		inname = 0;
		ParseName(cp);
	    } else {
		*cp2 = toupper(*cp2);
	    }
	}
    }
    printf("\n");
    smapoptFreeContext(optCon);
#if FOR_HAL
    if(antennasGiven == 0 || vars == 0) {
        fprintf(stderr, "Must specify at least one antenna and "
                "one rm variable\n");
#else /* FOR_HAL */
    if(vars == 0) {
        fprintf(stderr, "Must specify at least one rm variable\n");
#endif /* FOR_HAL */
        exit(1);
    }
    if(seriesMS > 0 && vars->next != 0) {
	fprintf(stderr, "Only one variable can be given with timeseries\n");
	exit(1);
	
    }
#if 0
    PrintVarList();
    printf("antennasGiven %d sleepSec %d numPasses %d useHex %d\n",
           antennasGiven, sleepSec, numPasses, useHex);
#endif


    if((rmbuf = malloc(maxVarSize)) == NULL) {
	fprintf(stderr, "sampleRM: malloc failed\n");
	exit(5);
    }
#if FOR_HAL
    rm_status = rm_read(antlist[0],"RM_UNIX_TIME_L", &startTime);
#else /* FOR_HAL */
    rm_status = rm_read(RM_ANT_0,"RM_UNIX_TIME_L", &startTime);
#endif /* FOR_HAL */
    printf("#Starting time %d\n", startTime);
    while(numPasses--) {
#if FOR_HAL
	rm_status = rm_read(antlist[0],"RM_UNIX_TIME_L", &unixTime);
#else /* FOR_HAL */
	rm_status = rm_read(RM_ANT_0,"RM_UNIX_TIME_L", &unixTime);
#endif /* FOR_HAL */
	if(showTime) {
	    printf("%d ", unixTime - startTime);
	}
#if FOR_HAL
        for(ant = 0; ant < ANT_ARRAY_SZ; ant++) {
            if(antennaArray[ant]) {
#endif /* FOR_HAL */
                for(curVar = vars; curVar; curVar = curVar->next) {
                    rm_status=rm_read(ant, curVar->name, rmbuf);
                    if(rm_status != RM_SUCCESS) {
			char msg[128];

			putchar('\n');
			sprintf(msg, "Rm error reading %s", curVar->name);
                        rm_error_message(rm_status,msg);
			exit(5);
                    } else {
			if(seriesMS > 0) {
			    PrintVarTimeSeries();
			} else {
			    PrintVar();
			}
                    }
                }
#if FOR_HAL
            }
        }
#endif /* FOR_HAL */
	if(seriesMS <= 0) putchar('\n');
        if(numPasses)
            sleep(sleepSec);
    }
    return(0);
}

void ParseName(char *name) {
    char *end, *cp;
    struct VAR *newVar;
    int size;

    if(strncmp(name, "RM_", 3) != 0) {
        fprintf(stderr, "%s is not a reflective memory variable\n", name);
        exit(1);
    }
    printf("%s ", name);
    if((newVar = (struct VAR *)malloc(sizeof(struct VAR))) == NULL) {
	fprintf(stderr, "sampleRM: malloc failed\n");
	exit(5);
    }
    if(vars == 0) {
        vars = newVar;
    } else {
        curVar->next = newVar;
    }
    newVar->next = 0;
    curVar = newVar;
    end = name + strlen(name) - 1;
    newVar->name = name;
    /* Find, evaluate, and strip off any indexes */
    if(*end == ']') {
        while(*--end != '[') {
            if(end <= name) {
		putchar('\n');
                fprintf(stderr, "Corrupt index in %s\n", name);
                exit(2);
            }
        }
	ParseIndex(end + 1, &newVar->il1);
        if(*--end == ']') {
            while(*--end != '[') {
                if(end <= name) {
		    putchar('\n');
                    fprintf(stderr, "Corrupt index in %s\n", name);
                    exit(2);
                }
            }
	    ParseIndex(end + 1, &newVar->il2);
            end--;
	} else {
	    ParseIndex("]", &newVar->il2);
        }
        end[1] = 0;
    } else {
	ParseIndex("]", &newVar->il1);
	ParseIndex("]", &newVar->il2);
    }

    /* Now get the variable type */
    for(cp = end; cp[-1] != '_' && cp > name; cp--)
        ;
    newVar->type = *cp;
    switch(newVar->type) {
    case 'B':
        newVar->baseSize = 1;
        break;
    case 'C':
        if(isdigit(cp[1])) {
            newVar->baseSize = atoi(&cp[1]);
        } else {
            newVar->baseSize = 1;
        }
        break;
    case 'D':
        newVar->baseSize = sizeof(double);
        break;
    case 'F':
        newVar->baseSize = sizeof(float);
        break;
    case 'L':
        newVar->baseSize = sizeof(int);
        break;
    case 'S':
        newVar->baseSize = sizeof(short);
        break;
    default:
        fprintf(stderr, "Variable type %c not recognized\n", newVar->type);
        exit(2);
    }
    if(newVar->type != 'C' && cp != end) {
        printf("Extra chars at end of name\n");
        exit(3);
    }
    newVar->dim1 = newVar->dim2 = 1;
    cp -= 2;	/* skip over the '_' */
    if(isdigit(*cp)) {
        while(isdigit(*cp))
            cp--;
        if(*cp == 'V' && cp[-1] == '_') {	/* found a dimension */
            newVar->dim1 = atoi(&cp[1]);
            cp -= 2;	/* skip over the '_' */
            if(isdigit(*cp)) {
                while(isdigit(*cp))
                    cp--;
                if(*cp == 'V' && cp[-1] == '_') {	/* found a dimension */
                    newVar->dim2 = atoi(&cp[1]);
                }
            }
        }
    }
    if(*newVar->il1 == IEND) {
	if(newVar->dim1 > 1) {
	    newVar->il1[0] = IRANGE;
	    newVar->il1[1] = 0;
	    newVar->il1[2] = newVar->dim1 - 1;
	    newVar->il1[3] = IEND;
      } else {
	    newVar->il1[0] = 0;
	    newVar->il1[1] = IEND;
      }
    }
    if(*newVar->il2 == IEND) {
	if(newVar->dim2 > 1) {
	    newVar->il2[0] = IRANGE;
	    newVar->il2[1] = 0;
	    newVar->il2[2] = newVar->dim2 - 1;
	    newVar->il2[3] = IEND;
      } else {
	    newVar->il2[0] = 0;
	    newVar->il2[1] = IEND;
      }
    }
    size = newVar->baseSize * newVar->dim1 * newVar->dim2;
    if(size > maxVarSize)
        maxVarSize = size;
}

#define ILSIZE 6
void ParseIndex(char *cp, short **pil) {
    int t1, t2;
    int ilSize, ilNext;

    ilSize = ILSIZE;
    ilNext = 0;
    if((*pil = (short *)malloc(ILSIZE * sizeof(short))) == NULL) {
	fprintf(stderr, "sampleRM: malloc failed\n");
	exit(5);
    }
/* printf("\nParseIndex called with %s\n", cp); */
    t1 = GetToken(&cp);
    for(;;) {
	if(ilSize - ilNext < 3) {
	    if((*pil = (short *)realloc(*pil, ilSize += ILSIZE)) == NULL) {
		fprintf(stderr, "sampleRM: realloc failed\n");
		exit(5);
	    }
	}
	if(t1 == IEND) {
	    (*pil)[ilNext] = t1;
	    return;
	}
	if(t1 >= 0) {
	    t2 = GetToken(&cp);
	    if(t2 == IRANGE) {
		(*pil)[ilNext++] = t2;
		(*pil)[ilNext++] = t1;
		if((t2 = GetToken(&cp)) < 0) {
		    fprintf(stderr, "incomplete range of indexes in %s\n",
			curVar->name);
		    exit(6);
		}
	    } else {
		(*pil)[ilNext++] = t1;
	    }
	    t1 = t2;
	}
    }
}

int GetToken(char **pcp) {
    int t;
    char *cp = *pcp;

    if(*cp == ']') {
	t = IEND;
    } else if(*cp == '.' && cp[1] == '.') {
	cp += 2;
	t = IRANGE;
    } else if(!isdigit(*cp)) {
	putchar('\n');
	fprintf(stderr, "Corrupt index in %s\n", curVar->name);
	exit(6);
    } else {
	t = *cp - '0';
	while(isdigit(*++cp)) {
	    t *= 10;
	    t += *cp - '0';
	}
	if(*cp == ',') cp++;
    }
    *pcp = cp;
    return(t);
}

void PrintVar(void) {
    int i1, i1Start, i1End;
    int i2, i2Start, i2End;
    short *il1, *il2;

    il2 = curVar->il2;
    for(; *il2 != IEND; il2++) {
	if(*il2 == IRANGE) {
	    i2Start = *++il2;
	    i2End = *++il2;
	} else {
	    i2Start = i2End = *il2;
	}
	if(i2Start < 0 || i2End >= curVar->dim2) {
	    putchar('\n');
	    fprintf(stderr, "Index out of range for %s\n", curVar->name);
	    exit(7);
	}
	for(i2 = i2Start; i2 <= i2End; i2++) {
	    il1 = curVar->il1;
	    for(; *il1 != IEND; il1++) {
		if(*il1 == IRANGE) {
		    i1Start = *++il1;
		    i1End = *++il1;
		} else {
		    i1Start = i1End = *il1;
		}
		if(i1Start < 0 || i1End >= curVar->dim1) {
		    putchar('\n');
		    fprintf(stderr, "Index out of range for %s\n",
			    curVar->name);
		    exit(7);
		}
		for(i1 = i1Start; i1 <= i1End; i1++) {
		    PrintValue(i1, i2);
		}
	    }
	}
    }
}

void PrintVarTimeSeries(void) {
    int i1, i1Start, i1End;
    int i2, i2Start, i2End;
    short *il1, *il2;

    il1 = curVar->il1;
    for(; *il1 != IEND; il1++) {
	if(*il1 == IRANGE) {
	    i1Start = *++il1;
	    i1End = *++il1;
	} else {
	    i1Start = i1End = *il1;
	}
	if(i1Start < 0 || i1End >= curVar->dim1) {
	    putchar('\n');
	    fprintf(stderr, "Index out of range for %s\n", curVar->name);
	    exit(7);
	}
	for(i1 = i1Start; i1 <= i1End; i1++) {
	    il2 = curVar->il2;
	    printf("%7.3f ", unixTime - startTime + (double)i1 * seriesMS / 1000.);
	    for(; *il2 != IEND; il2++) {
		if(*il2 == IRANGE) {
		    i2Start = *++il2;
		    i2End = *++il2;
		} else {
		    i2Start = i2End = *il2;
		}
		if(i2Start < 0 || i2End >= curVar->dim2) {
		    putchar('\n');
		    fprintf(stderr, "Index out of range for %s\n",
			    curVar->name);
		    exit(7);
		}
		for(i2 = i2Start; i2 <= i2End; i2++) {
		    PrintValue(i1, i2);
		}
	    }
	    putchar('\n');
	}
    }
}

void PrintValue(int i1, int i2) {
    int i;
    void *arg;

    arg = rmbuf + (i2*curVar->dim1 + i1) * curVar->baseSize;
    switch(curVar->type) {
    case 'B':
        printf((useHex)? "0x%x  ": "%d  ", *(char *)arg);
        break;
    case 'C':
        for(i = 0; i < curVar->baseSize; i++) {
            putchar(*(char *)arg);
            arg++;
        }
        return;
        break;
    case 'D':
        printf("%g  ", *(double *)arg);
        break;
    case 'F':
        printf("%g  ", *(float *)arg);
        break;
    case 'L':
        printf((useHex)? "0x%x  ": "%d  ", *(int *)arg);
        break;
    case 'S':
        printf((useHex)? "0x%x  ": "%d  ", *(short *)arg);
        break;
    }
}

#if 0
void PrintVarList(void) {
    for(curVar = vars; curVar; curVar = curVar->next) {
        printf("%s  d1 %d d2 %d i1 %d i2 %d tp %c sz %d\n",
               curVar->name, curVar->dim1, curVar->dim2, curVar->i1,
               curVar->i2, curVar->type, curVar->baseSize);
    }
}

void PrintIL(short *il) {
    for(; *il != IEND; il++) {
	printf("%d ", *il);
    }
    printf("-1\n");
}
#endif

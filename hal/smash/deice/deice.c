/***************************************************************
*
* deice.c
* 
* SMAsh command for controlling antenna deicing through reflective
* memory and deiced.
* RWW 
* 3 July 2001
*
* $Log: deice.c,v $
* Revision 1.13  2008/01/15 17:21:53  rwilson
* update naming with debuf, OpTel defaule smapopt
*
* Revision 1.12  2007/02/20 22:34:40  rwilson
* password protection for high levels, full power on subreflector, don't control 9 and 10
*
* Revision 1.11  2005/09/26 14:13:55  rwilson
* Updated dependencies for ld
*
* Revision 1.10  2004/04/19 20:07:40  rwilson
* Add Peak Power warning, add expected values to -t mode
*
* Revision 1.9  2003/12/03 20:02:49  rwilson
* fixed special control of chopper case
*
* Revision 1.8  2003/11/30 19:25:16  rwilson
* Chopper heat default full when general on
*
* Revision 1.7  2003/04/14 13:43:54  rwilson
* less verbose
*
* Revision 1.6  2003/04/01 18:38:05  rwilson
* small changes
*
* Revision 1.5  2003/02/20 22:33:05  rwilson
* added test mode
*
* Revision 1.4  2003/01/31 20:24:30  rwilson
* update
*
* Revision 1.3  2002/11/20 03:13:05  rwilson
* working versions
*
* Revision 1.2  2002/11/13 20:36:22  rwilson
* chenged to depend
*
* Revision 1.1.1.1  2002/10/30 19:55:32  rwilson
* deice control and monitor
*
****************************************************************/
static char rcsid[] = "$Id: deice.c,v 1.13 2008/01/15 17:21:53 rwilson Exp $";

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smapopt.h"
#include "rm.h"
#include "deiced.h"

#define KEY (0x559966aa)
#define NUMANTENNAS 8
#define VERBOSE 0

/* Deice system conntrol and current report */
static char powerCmdv[] = "RM_DEICE_POWER_CMD_V5_S";
static char cmdTimestampv[] = "RM_DEICE_CMD_TIMESTAMP_L";
static char currentv[] = "RM_DEICE_CURRENT_V3_F";
static char sysStatusv[] = "RM_DEICE_STATUS_BITS_L";
static char statusTimestampv[] = "RM_DEICE_TIMESTAMP_L";
static char padidv[] = "RM_PAD_ID_B";
static char unixTimev[] = "RM_UNIX_TIME_L";

static int testFlag = 0;
static int mainPwr = -1, subreflectorPwr = -1, chopperPwr = -1, opTelPwr = -1;
#define MAX_MAIN_PWR 6
static int zone = -1;
static int generalPower = -1;
static int antennaArray[SMAPOPT_MAX_ANTENNAS + 1];
#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))
static int antennasGiven = 0;
static int powerGiven = 0;
static int rm_antlist[RM_ARRAY_SIZE];
static int unixTime;
static int peakPwr[MAX_MAIN_PWR + 1] =
	{0, 5470, 10939, 15553, 20167, 24781, 29394};

static struct	smapoptOption optionsTable[] = {
	{"antenna",'a',SMAPOPT_ARG_ANTENNAS, antennaArray,'a',
		"Antennas (Comma and ellipsis separated list).  "
		"Default all with deice control and not in the hangar"},
	{"main",'m',SMAPOPT_ARG_INT,&mainPwr,'s',
    	    "# of deice phases surface and quad leg heaters will be on (0-6)"},
	{"subreflector",'s',SMAPOPT_ARG_INT,&subreflectorPwr,'s',
    	    "# of deice phases subreflector heaters will be on (0-8)"},
	{"chopper",'c',SMAPOPT_ARG_INT,&chopperPwr,'s',
    	    "# of deice phases chopper case heaters will be on (0-8)"},
	{"opTel",'o',SMAPOPT_ARG_INT,&opTelPwr,'s',
	    "# of deice phases optical telescope heaters will be on (0-8)"},
	{"zone",'z',SMAPOPT_ARG_INT,&zone,0,
	    "Single zone to turn on continuously (1, 11)"},
	{"test",'t',SMAPOPT_ARG_NONE,&testFlag,0,
	    "Print heater currents for each zone of each antenna"},
	{"help",'h',SMAPOPT_ARG_NONE,0,'h',""},
/*	SMAPOPT_AUTOHELP */
	{NULL,0,0,NULL,0,0}
};	  
smapoptContext optCon;

/* deice.c */
void SetUp(int *ip, int high, char *name);
static void RunTests(void);
void GetUnixTime(void);

void errorOut(char *error, char *addl) {
	if (error) fprintf(stderr, "\n%s: %s.  For help use -h.\n",
		error, addl);
	exit(1);
}

void usage(void) {

	smapoptPrintHelp(optCon, stderr, 0);
	fprintf(stderr,
	    "\nOr to set all zones to the same fractional power:\n\n"
	    "\tdeice [-a<n>]  <PowerLevel(0-10)>\n\n");
}

int main(int argc, char *argv[]) {
	int i, ant, j, rm_status, totPwr;
	short powerCmd[5];
	char c, *cp, ts[64];

	optCon = smapoptGetContext("deice", argc, argv, optionsTable,0);

	while ((c = smapoptGetNextOpt(optCon)) >= 0) {
	    switch(c) {
	    case 'a':
		antennasGiven = 1;
		break;
	    case 's':
		powerGiven++;
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
	    exit(1);
	}
	if((cp = smapoptGetArg(optCon)) != 0) {
	    powerGiven++;
	    generalPower = atoi(cp);
	}
	smapoptFreeContext(optCon);

	/* initializing ref. mem. */
	rm_status=rm_open(rm_antlist);
	if(rm_status != RM_SUCCESS) {
	    rm_error_message(rm_status,"rm_open()");
	    exit(1);
	}

	if(testFlag == 0 && powerGiven <= 0 && zone < 0) {
	    errorOut("Insufficient arguments" ,"Some action required");
	}
	if(zone > 0 && powerGiven) {
	    errorOut("Over Specified", "Can't specify zone with a power");
	}

	if(generalPower < -1 || generalPower > 10) {
	    printf("If general power level given, it must be between 0"
		" and 10\n");
	    exit(1);
	}
	if(generalPower > 3) {
	    char passwd[10] = "";
	    char secret[] = "$KtoHECO\n";

	    printf("You have asked for a deice level above 3 which is almost\n"
	    "never necessary.  If you are sure you want to do this, enter the\n"
	    "secret password > ");
	    fflush(stdout);
/*	    scanf("%9s", passwd); This hangs on a null return */
	    read(0, passwd, 9);
	    if(strcmp(passwd, secret) != 0) {
	      printf("Sorry, no match.  The password is given in the "
	        "documentation\n\"Running the Deice System on SMA Antennas\".  "
	        "Please try level 3 or lower\nnow and get the password if it "
		"is really needed.\n");
	      exit(1);
	    }
	}
	SetUp(&mainPwr, MAX_MAIN_PWR, "Main Power");
	SetUp(&chopperPwr, 8, "Chopper Power");
	SetUp(&subreflectorPwr, 8, "Subreflector Power");
	SetUp(&opTelPwr, 8, "OpTel Power");
	if(zone > 0) {
	    if(zone > ZONES) {
		printf("Zone must be between 1 and 11\n");
		exit(1);
	    }
	    mainPwr = - zone;
	}

	if(antennasGiven == 0) {
	    for(ant = 0; ant < ANT_ARRAY_SZ; ant++) {
		rm_status = rm_read(ant, padidv, &c);
		antennaArray[ant] = c != 28;
	    }
	}

#if VERBOSE
	printf("Requested Antennas:");
	for(i = 1; i < ANT_ARRAY_SZ; i++) {
	    printf(" %d", antennaArray[i]);
	}
	printf("\ntestFlag = %d  zone = %d  generalPower = %d\n",
		testFlag, zone, generalPower);
	printf("mainPwr = %d subreflectorPwr = %d  chopperP = %d"
		"  opTelPwr = %d\n",
		mainPwr, subreflectorPwr, chopperPwr, opTelPwr);
	printf("Antennas with RM:");
	for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	    printf(" %d", ant);
	}
	putchar('\n');
#endif VERBOSE

	totPwr = 0;
	GetUnixTime();
	/* Limit list to requested antennas with active deiced */
	for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	    int deicedTime;

	    if(ant > 8) {
		break;
	    }
	    if( antennaArray[ant]) {
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
		totPwr += peakPwr[mainPwr];
	    } else {
		rm_status = rm_read(ant, powerCmdv, powerCmd);
		if(rm_status == RM_SUCCESS && powerCmd[MAIN_P] > 0 &&
			powerCmd[MAIN_P] < MAX_MAIN_PWR) {
		    totPwr += peakPwr[powerCmd[MAIN_P]];
		}
	    }
	}
	rm_antlist[j] = RM_ANT_LIST_END;
	printf("Antennas to be controlled by this command:");
	for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	    printf(" %d", ant);
	}
	putchar('\n');
	if(testFlag) {
	    if(mainPwr != 0 || chopperPwr != 0 || subreflectorPwr != 0 ||
			opTelPwr != 0 ) {
		printf("You may not specify a power with a test\n");
		exit(1);
	    }
	    RunTests();
	    exit(0);
	}

	printf("Peak deice power for the array will be %d Watts\n", totPwr);
	if(totPwr > 100000) {
	    printf("This action may be expensive.  Do you wnat to continue?\n");
	    scanf("%5s", ts);
	    if(ts[0] != 'y' && ts[0] != 'Y') {
		printf("No change will be made!\n");
		exit(1);
	    }
	}
	for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	    powerCmd[MAIN_P] = mainPwr;
	    powerCmd[CHOP_P] = chopperPwr;
	    powerCmd[SUBR_P] = subreflectorPwr;
	    powerCmd[OPTEL_P] = opTelPwr;
	    powerCmd[CHECK_P] = CHECKWORD(powerCmd); 
	    rm_status = rm_write(ant, powerCmdv, powerCmd);
printf("Notifying %d\n",  ant);
	    rm_status |= rm_write_notify(ant, cmdTimestampv, &unixTime);
	    if(rm_status != RM_SUCCESS) {
		printf("rm_write error occurred for antenna %d\n", ant);
	    }
	}

	return(0);
}

void SetUp(int *ip, int high, char *name) {
	double scale;

	scale = 0.1 * high;
	if(generalPower >= 0 && *ip == -1) {
	    if(strncmp(name, "Chop", 4) == 0 || strncmp(name, "Subr", 4) == 0) {
		if(generalPower > 0) *ip = high;
	    } else {
		*ip = (int)(scale * generalPower + 0.49);
	    }
	} else if(*ip > high || *ip < -1) {
	    printf("%s is out of range (%d), must be between 0 and %d\n",
		name, *ip, high);
	    exit(1);
	}
	if(*ip < 0)
	    *ip = 0;
}

static void RunTests(void) {
#if 1
	float current[RM_ARRAY_SIZE][3];
	int statusTimestamp[RM_ARRAY_SIZE];
	unsigned int status[RM_ARRAY_SIZE];
	short powerCmd[5];
	int i, zone, z, ant, rm_status;
	unsigned int expectedStatus;
	char errorString[64];
	static char *zoneName[] = {"None", "Dish", "Quadrupod",
	    "Chopper Case", "Subreflector", "M3"};
	static float expectedCurrent[][3] = {
	    {0.0, 0.0, 0.0},		/* Zero Reading */
	    {13.15, 12.65, 12.65},	/* Dish surface */
	    {14.98, 14.98, 15.62},	/* Quad legs */
	    {0.6, 0.4, 0},		/* Chopper case */
	    {0, 0, 0.4},		/* Subreflector */
	    {0.7, 0, 0}			/* OpTel */
	};

	for(i = MAIN_P; i < CHECK_P; i++) {
	    powerCmd[i] = 0;
	}
	for(i = 0; i < ANT_ARRAY_SZ; i++) {
	    antennaArray[i] = 0;
	}
	for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	    antennaArray[ant] = 1;
	}
	printf("\n         Zone Type   Z Ph ");
	for(i = 1; i <= NUMANTENNAS; i++) {
	    printf("%3d  ", i);
	}
	printf(" Expected\n");
	for(zone = 0; zone <= 12; zone++) {
	    if(zone > 0 && zone < 12) {
		powerCmd[MAIN_P] = -zone;
	    } else {
		powerCmd[MAIN_P] = 0;
	    }
	    if(zone == 0) z = 0;
	    else if(zone < 7) z = 1;
	    else if(zone < 9) z = 2;
	    else z = zone - 6;
	    powerCmd[CHECK_P] = CHECKWORD(powerCmd); 
	    GetUnixTime();
	    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
		sprintf(errorString, "Setting zone of ant %d", ant);
		rm_status = rm_write(ant, powerCmdv, powerCmd);
		if(rm_status != RM_SUCCESS) {
		    rm_error_message(rm_status, errorString);
		}
/* printf("Notifying %d\n",  ant); */
		rm_status = rm_write_notify(ant, cmdTimestampv, &unixTime);
		if(rm_status != RM_SUCCESS) {
		    rm_error_message(rm_status, errorString);
		}
	    }
	    if(zone == 12) {
		exit(0);
	    }
	    sleep(5);
	    rm_status = rm_read(RM_ANT_ALL, statusTimestampv, statusTimestamp);
	    if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status, "reading deiced status timestamps");
	    }
	    rm_status = rm_read(RM_ANT_ALL, currentv, current);
	    if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status, "reading deice currents");
	    }
	    rm_status = rm_read(RM_ANT_ALL, sysStatusv, status);
	    if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status, "reading deiced status bits");
	    }
	    expectedStatus = (zone > 0)? 0x7000 | (1 << zone): 0;
	    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
		if(statusTimestamp[ant] <= unixTime || status[ant] !=
			expectedStatus) {
		    printf("antenna %d error: timestamp %d sec after cmd, "
			"status = %4x - expected %4x\n", ant,
			statusTimestamp[ant] - unixTime, status[ant],
			expectedStatus);
		}
	    }
	    for(i = 0; i < 3; i++) {
		printf("%19s %2d %c", (expectedCurrent[z][i] == 0.0)?
			"N/C": zoneName[z], zone, 'A' + i);
		for(ant = 1; ant <= NUMANTENNAS; ant++) {
		    if(antennaArray[ant]) {
			printf("%5.1f", current[ant][i]);
		    }else {
			printf("     ");
		    }
		}
		printf("%7.1f\n", expectedCurrent[z][i]);
	    }
	}
#endif
}

void GetUnixTime(void) {
	int i;

	for(i = 0; rm_antlist[i] != RM_ANT_LIST_END; i++) {
	    if(rm_read(rm_antlist[i], unixTimev, &unixTime) == RM_SUCCESS) {
		return;
	    }
	}
}

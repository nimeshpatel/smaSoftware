/* Datathread is an optional thread of servo to collect servo performance
 * data and write it to a file as svdata does.  Having this function
 * in servo will allow collecting data with a SMAsh command on hal.
 *
 * Two RM variables are used to control data taking:
 *    RM_SERVO_DATA_CONTROL_S Is to be set to the number of seconds of
 *	data to collect by the SMAsh command and decremented each second
 *	by datathread.
 *  RM_SERVO_DATA_FILE_C8 is the first part of the file name where the
	data should be written.  datathread will append unixtime to it.
 */
static char rcsid[] = "$Id:$";

#include <resource.h>
#define _POSIX_SOURCE
#include <unistd.h>
/* This is a non-POSIX function, so it is not seen in unistd.h */
extern unsigned int usleep    _AP((time_t));
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <servo.h>
#include "tsshm.h"

#define MAS (3600000)

static char azState = 0, elState = 0;

void *openshm(char *name, int size);

int main(int argc, char *argv[]) {
	TrackServoSHM *tsshm;		/* Local pointer to shared memory */
	int started;
	int nSamp, when;
	int nextSample;
	SAMPLE *samples, *sp;
	FILE *fp;

	/* Deal with command line args */
	when = 's';
	while(argc > 3) {
	    if(argv[1][0] == '-') {
		when = argv[1][1];
		argc--;
		argv++;
	    } else {
		break;
	    }
	}
	if(argc != 3 || (when != 's' && when != 'i')) {
	    fputs("Usage: svdata [-when] sample_len(sec) file_name\n"
		"where when can take on the values:\n"
		"i - Start immediately\n"
		"s - Start on the next state change (default)\n"
		,stderr);
	    exit(1);
	}
	started = (when == 'i')? 1: 0;
	nSamp = atoi(argv[1]) * 100;	/* the number of samples */
/*	printf("when = %c, nSamp = %d, fn = %s\n", when, nSamp, argv[2]); */
	if((samples = (SAMPLE *)malloc(nSamp*sizeof(SAMPLE))) == NULL) {
	    fputs("Could not malloc enough memory for recording test data\n",
		stderr);
	    exit(1);
	}

	setpriority(PRIO_PROCESS, (0), (45));
/*	setprio(0, 75); */
	tsshm = OpenShm(TSSHMNAME, TSSHMSZ);
	tsshm->sampOut = PREV_SAMP(tsshm->sampIn); /* Set samp buffer empty */
	usleep(100000);
	if(SEMPTY) {
	    fputs("Servo doesn't seem to be running\n", stderr);
	    exit(1);
	}
	if(! started) {
	    INC_OUT;
	    azState =  SO.azState;
	    elState =  SO.elState;
	}

	if((fp = fopen(argv[2], "w")) == NULL) {
	    fprintf(stderr, "svdata: Couldn't open %s for writing\n", argv[2]);
	    exit(1);
	}

	/* If we are not starting immediately, wait for the state to change */
	if(! started) fprintf(stderr,
	    "Waiting for a state change from (%1d %1d)\n", azState, elState);
	while(! started) {
	    if(SEMPTY) {
		fputs("Servo has stopped\n", stderr);
		exit(1);
	    }
	    while(! SEMPTY) {
		INC_OUT;
		if(azState != SO.azState || elState != SO.elState) {
			started = 1;
			break;
		}
	    }
	    usleep(100000);
	}
	fprintf(stderr, "Starting to collect data, state (%1d %1d)\n",
		SO.azState, SO.elState);
	/* Now collect the data */
	for(nextSample = 0; nextSample < nSamp; nextSample++) {
	    if(SEMPTY) {
		usleep(100000);
		if(SEMPTY) {
		    fputs("Servo has stopped\n", stderr);
		    break;
		}
	    }
	    INC_OUT;
	    samples[nextSample] = SO;
	}

	fputs("Writing out data\n", stderr);

/* Definition of the data columns:
Column  Content
1       time in seconds
2       scb Status (bit mapped)
3       Az command from (dmy)track
4       Az command out of the command shaper
5       Az from the fine encoder
6       Az Velocity commanded by servo
7       Az Velocity from the tachometer
8       Az Torque in Amperes (average of the two motors)
9       State of the Az command shaper
10      El command from track
11      El command out of the command shaper
12      El from the fine encoder12
13      El Velocity commanded by servo
14      El Velocity from the tach
15      El Torque (Amps)
16      State of the El command shaper
*/

	/* Write out the data */
#define TESTING_LIM_VS_TACH 0
#define PO(x) sp->x*(1.0/MAS)		/* Pos and vel in degrees */
#define TO(x) sp->x*(70./32768.0)	/* Torque in Amps */
	for(sp = samples; sp < &samples[nextSample]; sp++) {
	    fprintf(fp, "%7.3f %2d"
		"% 11.5f %11.5f %11.5f %8.4f %8.4f %5.1f %1d"	/* Az values */
		"% 11.5f %11.5f %11.5f %8.4f %8.4f %5.1f %1d"	/* El values */
		"\n", sp->msec*0.001, sp->scbStatus,
		PO(curAz), PO(shpAz), PO(encAz),
#if TESTING_LIM_VS_TACH
		PO(cmdAzVel), (double)(sp->tachAzVel), (double)(sp->azTorq), sp->azState,
#else
		PO(cmdAzVel), PO(tachAzVel), TO(azTorq), sp->azState, 
#endif
		PO(curEl), PO(shpEl), PO(encEl),
#if TESTING_LIM_VS_TACH
		PO(cmdElVel), (double)(sp->tachElVel), (double)(sp->elTorq), sp->elState
#else
		PO(cmdElVel), PO(tachElVel), TO(elTorq), sp->elState
#endif
	    );
	}
	return(0);
}

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
/* This is a non-POSIX function, so it is not seen in unistd.h */
extern unsigned int usleep    _AP((time_t));
#include <string.h>
void bzero(void *s, int n);	/* This should be in string.h, but isn't */
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>
#include "vme_sg_simple.h"
#include "smadaemon.h"

#define USEMAIN 0

struct vme_sg_simple_time ttime;
int ttfd;			/* File descriptor for the TrueTime device */
int irig_error_count = 0, irig_error_seen = 0;
int ttTimeoutCount = 0;
static int shutdownSignal = 0;		/* signal main program to shutdown */
static int gotQuit = 0;			/* Return with QUITRTN */
double lastTimeStep;		/* Time (sec) from the prev clock rtn to now */
int reportTrueTimeTimeout = 0;
int msec, day;

/* Set the ttime structure to the next even 10 msec time.  GetNextTickTime
 * first reads the current time and then rounds up
 */
void GetNextTickTime(void) {
	int stat;

	if(( stat = read(ttfd, &ttime, sizeof(ttime))) < 0) {
	    fprintf(stderr, "Error %d reading TrueTime\n", stat);
	}
	if(ttime.input_reference_error || (ttime.phase_locked ^ 1)) {
	    irig_error_seen = 1;
	}
	/* Round the usec up to the next even 10msec.  Require 50usec spare */
/*
	ttime.usec += 10050;
	ttime.usec /= 10000;
	ttime.usec *= 10000;
*/
	ttime.usec += 10000; 
	if(ttime.usec >= 1000000) {
	  ttime.usec -= 1000000;
	  if(++ttime.sec >= 60) {
	    ttime.sec = 0;
	    if(++ttime.min >= 60) {
	      ttime.min = 0;
	      if(++ttime.hour >= 24) {
		ttime.hour = 0;
		if(++ttime.yday >= ((ttime.year % 4) == 0)? 366: 365) {
		  ttime.yday = 0;
		  ttime.year++;
		}
	      }
	    }
	  }
	}
}

/* Get time, read encoder and convert values */
void GetPosTime() {
	int stat;
	static int oldusec = -1;
	int usecDiff;
	int ttTimeout;

	GetNextTickTime();
	usecDiff = ttime.usec - oldusec;
	if(usecDiff < 0)
	    usecDiff += 1000000;
	ttTimeout = 0;
	/* if we would skip a tick, don't wait for the TrueTime */
	if(usecDiff < 15000) {
	  if(ioctl(ttfd, VME_SG_SIMPLE_WAIT_UNTIL, &ttime) < 0) {
	    switch(errno) {
	    case EAGAIN:
		fprintf(stderr,
		    "servo could not wait on TrueTime because it was busy\n");
		/* Recover by reading the time below and skip the wait */
		break;
	    case EINTR:
		/* We received an interrupt and will stop anyway, so don't
		 * worry. */
		fprintf(stderr,
		    "SILENT TrueTime returned an interrupt,"
		    " shutdown %d, quit %d\n",
		    shutdownSignal, gotQuit);
		break;
	    case ETIMEDOUT:
		ttTimeoutCount++;
		if(reportTrueTimeTimeout) {
		    ttTimeout = 1;
		}
		break;
	    default:
		fprintf(stderr, "The TrueTime wait returned error %d  ", errno);
		break;
	    }
	  }
	}
	/* get the current time in any case */
	if((stat = read(ttfd, &ttime, sizeof(ttime))) < 0) {
	    fprintf(stderr, "Error %d reading TrueTime\n", stat);
	}
	if(ttime.input_reference_error || (ttime.phase_locked ^ 1)) {
	    irig_error_seen = 1;
	}
	lastTimeStep = (ttime.usec - oldusec) / 1e6;	/* Time step (sec) */
	if(lastTimeStep < 0) {
	    lastTimeStep += 1;
	}
	if(ttTimeout) {
	    fprintf(stderr, "TrueTime timeout: Time step is %.6f\n",
		    lastTimeStep);
	}
	oldusec = ttime.usec;

	/* Convert the time now */
	msec = ((ttime.hour * 60 + ttime.min) * 60 + ttime.sec) * 1000 +
		(ttime.usec + 500) / 1000;
	day = ttime.yday;
}
#if USEMAIN
int main(int argc, char *argv[]) {
	ttime.timeout_ticks = 2;  	/* This margin should be safe */

	ttfd = open("/dev/vme_sg_simple", O_RDWR, 0);
	if(ttfd <= 0) {
	    fprintf(stderr, "Error opening TrueTime - /dev/vme_sg_simple\n");
	    exit(SYSERR_RTN);
	}
	printf("Starting infinite loop\n");
	while(shutdownSignal == 0) {
	    GetPosTime();
	    printf("Day %d, msec %d\n", day, msec);
	}
	return(0);
}
#endif 

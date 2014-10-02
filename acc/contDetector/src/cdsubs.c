/* Subrouteins for accessing the acromag counter */

#define USE_MAIN 1

#include <math.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#if 0
#include <sys/mman.h>
#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <smem.h>
#endif
#include <fcntl.h>
#include <sys/ioctl.h>
#include "/usr/PowerPC/applications/acc/contDetector/include/contDetector.h"
#include "/usr/PowerPC/applications/acc/contDetector/include/contDetectordrvr.h"

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd = 0;				/* continuum detector fd */

/* Open the cdontinuum detector device and set up the integration time
 * in microseconds.  If the device is already open, just set the
 * integration time.
 */
int openCD(int intTime) {
    contDetectorioc_t cdIoctlArg;

    if(cdfd == 0) {
	if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
	    fprintf(stderr, "Can not open %s, I quit\n", cdName);
	    perror("");
	    return(0);
	}
    }
    cdIoctlArg.mode = IND;
    cdIoctlArg.intTime = intTime;
    if(ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg) < 0) {
	    perror("Setting cont det mode failed");
	    return(0);
    }
    return(1);
}

void closeCD(void) {
    if(cdfd != 0) {
	close(cdfd);
	cdfd = 0;
    }
}

int readCD(int *loRx, int *hiRx) {
    contDetector_result_t cdOut;

    if(read(cdfd, &cdOut, sizeof(cdOut)) != sizeof(cdOut)) {
	perror("Reading cont det");
	return(0);
    }
    *loRx = cdOut.lowRx;
    *hiRx = cdOut.highRx;
    return(1);
}

#if USE_MAIN
/* Test main program */
int main(int argc, char *argv[]) {
    int lowRx, highRx;
    int i;

    openCD(100000);
    for(i = 0; i < 5; i++) {
	readCD(&lowRx, &highRx);
	printf("low cnts %d,  high cnts %d\n", lowRx, highRx);
    }
}
#endif /* USE_MAIN */

/* Synchronous detector and total power monitor working from the V/F and
 * Acromag counter interface to the continuum detectors.
 * Bob Wilson 6/30/2004
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h> /* threads */
#include <sys/file.h> /* for device open */
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <stderrUtilities.h>
#include "smadaemon.h" /* for QUIT_RTN */
#include "rm.h" /* reflective memory */
#include "contDetector.h"
#include "patchPanelBits.h"

#define SYNCDET_PRIO 100

int antlist[RM_ARRAY_SIZE];
int ant = RM_ANT_0;
int unixTime;

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;

/* Things for the Unidig device */
char udName[] = "/dev/iPUniDig_D";
int udfd;

int main(int argc, char*argv[]) {
    int rm_status;

    DAEMONSET
    setpriority(PRIO_PROCESS, (0), (SYNCDET_PRIO));

    /* initializing ref. mem. */
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        fprintf(stderr,"Could not open reflective memory.");
        exit(QUIT_RTN);
    }
    /* Open the contDetector and unidig devices */
    if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
	fprintf(stderr, "Can not open %s, I quit\n", cdName);
	exit(1);
    }
    if((udfd = open(udName, O_RDONLY, 0)) < 0) {
	fprintf(stderr, "Can not open %s, I quit\n", udName);
	exit(1);
    }
    return(0);
}

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <smem.h>
#include "contDetector.h"
#include "contDetectordrvr.h"

#define SHMEM_NAME "Acromag"

#define VERBOSE 3
#define TIMING 0
#if TIMING
int exTime;
void StartTime(void);
void StopTime(char *name);
#endif /* TIMING */

ACROMAG *cntr;

volatile int busError = 0;
void signalBusError(void) {
	busError = 1;
}


int main(int argc, char *argv[]) {
	char *smp;
	int i, module;
	struct sigaction sa;

	sa.sa_handler = signalBusError;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGBUS, &sa, NULL)) {
	    perror("sigaction");
	    return(-1);
	}


	/* Set up the IP Carrier in a shared memory segment */
	i = VME_A16_PHYS_ADDRESS  + ACROMAG_CARRIER_BASE;
#if VERBOSE > 1
	printf("About to smem_create for Phys address 0x%x\n", i);
#endif /* VERBOSE > 1 */

	if(smem_remove(SHMEM_NAME) < 0) {
	    perror("smem_remove failed");
	}
	smp = (char *)smem_create(SHMEM_NAME, (char *)i, 0x1000,
		SM_READ | SM_WRITE);
	if(smp == NULL) {
	    perror("smem_create failed");
	    exit(1);
	}
#if VERBOSE > 1
	printf("smp = 0x%x\n", (int)smp);
#endif /* VERBOSE > 1 */
    for(module = 0; module < 4; module++) {
	printf("\nTrying module at %d\n", module);
	cntr = ((ACROMAG *)smp) + module;
        busError = 0;
        i = cntr->ascii[0];
	if (busError) {
	    printf("Nothing seen at 0x%x\n", module);
            continue;
        }
	for(i = 0; i < 4; i++) {
	    putchar(cntr->ascii[i]);
	}
	for(i = 4; i < 12; i++) {
	    printf(" 0x%x", 0xff & cntr->ascii[i]);
	}
	printf("\nMfrg code 0x%x, Prod code 0x%x, #bytes 0x%x, crc 0x%x\n",
		cntr->manufacturer & 0xff, cntr->modelNo & 0xff,
		cntr->idBytes & 0xff, cntr->crc & 0xff);
/*	cntr->intVect = 0x005a; */
	printf("intVect read back is 0x%x\n", cntr->intVect & 0xff);
    }
    return(0);
}

#if TIMING
struct timeval tv1, tv2;
struct timezone tz;

void StartTime(void) {
	gettimeofday(&tv1, &tz);
}

void StopTime(char *name) {
	gettimeofday(&tv2, &tz);
	exTime = (int)(tv2.tv_usec - tv1.tv_usec);
}
#endif /* TIMING */

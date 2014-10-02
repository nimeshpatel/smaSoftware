#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#include "iPUniDig_D.h"

#define TIMING 1
#define SBC_RESET_W      0x000001
#define SERVO_ESTOP_W    0x000002
#define OVERTEMP_R       0x000004
#define CHOPPER_SYNC_W   0x000008
#define TRIGGER_LEVEL_R  0x000010
#define TRIGGER_PULSE_R  0x000020
#define CHOPPER_RESET_W  0x000040
#define TRIGGER_SPARE_R  0x000080
#define ESTOP_BYPASS_R   0x000100

double exTime;
static int readMask = 0x1ff;

/* chopperBits.c */
void StartTime(void);
void StopTime(void);

int main(int argc, char *argv[]) {
	int fd, status;
	int count, bits, oldBits;

	fd = open("/dev/iPUniDig_D", O_RDWR);
	if (fd < 0) {
	    printf("Error returned from open, errno = %d\n", errno);
	    exit(-1);
	}
	if ((status = ioctl(fd, IOCTL_SETIN, &readMask)) < 0) {
	    printf("Error %d returned from 2nd ioctl call, errno = %d\n",
		status, errno);
	    exit(-1);
	}

	if(argc > 1) {
	    count = 4 * atoi(argv[1]);
/*	    printf("count = %d\n", count); */
	} else {
	    count = 1;
	}
	StartTime();
	while(count) {
	    if(status = read(fd, (char *)(&bits), 4) < 0) {
		perror("chopperBits: error reading bits");
		exit(1);
	    }
	    bits >>= 3;
	    bits &= 0x7;
	    if(bits != oldBits) {
		count--;
		oldBits = bits;
		StopTime();

		printf("%10.6f %1d\n", exTime, bits);
/*		if(count % 10 == 0)
		    putchar('\n'); */
	    }
/*	    usleep(20000); */
	}
}

#if TIMING
struct timeval tv1, tv2;
struct timezone tz;

void StartTime(void) {
	gettimeofday(&tv1, &tz);
}

void StopTime(void) {
	gettimeofday(&tv2, &tz);
	exTime = tv2.tv_sec - tv1.tv_sec + (double)(tv2.tv_usec - tv1.tv_usec) / 1000000.;
}
#endif /* TIMING */

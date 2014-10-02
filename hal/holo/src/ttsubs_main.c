#include <stdio.h>
#include "smadaemon.h"
#include "vme_sg_simple.h"
#include <sys/file.h>

extern int day, msec, ttfd;
extern struct vme_sg_simple_time ttime;

int main(int argc, char *argv[]) {
	ttime.timeout_ticks = 2;  	/* This margin should be safe */

	ttfd = open("/dev/vme_sg_simple", O_RDWR, 0);
	if(ttfd <= 0) {
	    fprintf(stderr, "Error opening TrueTime - /dev/vme_sg_simple\n");
	    exit(SYSERR_RTN);
	}
	printf("Starting infinite loop\n");
	while(1) {
	    GetPosTime();
	    printf("Day %d, msec %d\n", day, msec);
	}
	return(0);
}

#include <sys/types.h>
#include <sys/utsname.h>
#include <resource.h>
#include <errno.h>
#define _POSIX_SOURCE 1
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include "vme_sg_simple.h"

#define TESTSIGTIMEDWAIT 1
#if TESTSIGTIMEDWAIT
	sigset_t sigs = {{0,0}};
	struct timespec timeout= {0, 10000000};
	siginfo_t extra;
#endif

int main(int argc, char *argv[]) {
	struct vme_sg_simple_time ttime, ttime2;
	int ttfd;		/* File descriptor for the TrueTime device */
	int i;
	int signo;

	setpriority(PRIO_PROCESS, (0), 100);
	if((ttfd = open("/dev/vme_sg_simple", O_RDWR, 0)) < 0) {
	    perror("Can't open TrueTIme");
	    exit(-1);
	}
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10000000;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    for(i = 0; i < 10; i++) {
#if 1
	if(read(ttfd, &ttime, sizeof(ttime)) < 0) {
	    perror("Trouble reading the truetime");
	    return(-1);
	}
#endif
#if TESTSIGTIMEDWAIT
	if((signo = sigtimedwait(&sigs, &extra, &timeout)) < 0) {
	    switch(errno) {
	    case EAGAIN:
		break;
	    case EINTR:
		printf("Received SIGINT\n");
		exit(0);
	    case EINVAL:
		printf("invalid tv_nsec = %d\n", (int)timeout.tv_nsec);
		exit(1);
	    }
	} else {
	    printf("Signal %d arrived\n", signo);
	    exit(0);
	}
#if 1
	if(read(ttfd, &ttime2, sizeof(ttime2)) < 0) {
	    perror("Trouble reading the truetime");
	    return(-1);
	}
	printf("usec before %6d, after %6d\n", ttime.usec, ttime2.usec);
	for(signo = 0; signo < 240000; signo++) {
	    
	}
#endif
#else
	printf("day = %d %d:%d:%d.%06d\n", ttime.yday, ttime.hour,
		ttime.min, ttime.sec, ttime.usec);
	usleep(10000);
#endif
    }
	return(0);
}

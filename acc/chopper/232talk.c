/* This came from Taco, modified by RWW
 * Two threaded connection to the Chopper PMAC which can be used to
 * upload  plcs, programs, etc. from the PMAC or for a basic terminal
 * interface.
 */
#include <sys/file.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <rpc/rpc.h>
#include <resource.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <timers.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "iPOctal232.h"

pthread_t readTid;
int readFD;
int timeOutJiffies = 1000;		/* 10 sec. */
int showHex = 0;

void *readThread(void *arg) {
    int status, len;
    char buffer;

    len = 0;
    while(1) {
        status = read(readFD, &buffer, 1);
        if (status == -1) {
            perror("readThread: read error, trying again");
        } else {
#if 0
            printf("Received 0x%x\r\n", (int)buffer);
            fflush(stdout);
#else
            if(showHex) {
                if(isprint(buffer)) {
                    putchar(buffer);
		    len++;
                } else {
		    if(len) {
			putchar(' ');
			len = 0;
		    }
		    switch(buffer) {
		    case 6:
			puts("0x6(ACK)");
			break;
		    case 7:
			puts("0x7(BEL)");
			break;
		    case 0xd:
			puts("0xd(CR)");
			break;
		    default:
			printf("0x%02x\n", buffer);
		    }
                }
            } else {
                if(buffer == '\r') {
                    putchar('\n');
                    fflush(stdout);
                } else {
                    putchar(buffer);
                }
            }
#endif
        }
    }
}

int main(argc, argv)
int argc;
char **argv;
{
    int fd, status;
    char message;
    pthread_attr_t readAttr;
    struct sched_param fifo_param;

    if(argc > 1) {
        if(strncmp(argv[1], "-x", 2) == 0) {
            showHex = 1;
	} else {
            printf("Usage: 232talk [-h] [-x]\n use -x for hex representation"
                   " of all control chars from the PMAC\n");
	    exit(1);
        }
    }
    fd = open("/dev/iPOctal232-3", O_RDWR)
         ;
    if (fd == -1) {
        perror("mainThread: Opening serial line");
        exit(-1);
    }
    status=ioctl(fd,IOCTL_CHANGE_TIMEOUT,&timeOutJiffies);
    if(status)
        perror("Setting timeout for PMAC response");
    readFD = fd;

    /* Posix 1c version of setting the scheduling */

    if (pthread_attr_init(&readAttr) != 0) {
        perror("pthread_attr_init(readAttr)");
        exit(-1);
    }
    pthread_attr_setinheritsched(&readAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&readAttr, SCHED_FIFO);
    fifo_param.sched_priority = 20;
    pthread_attr_setschedparam(&readAttr, &fifo_param);

    if (pthread_create(&readTid, &readAttr, readThread, (void *) 12) != 0) {
        perror("pthread_create(readAttr)");
        exit(-1);
    }
    while (1) {
        int byte;

        if((read(0, &message, 1)) <= 0) {
            fprintf(stderr, "Hit end of file\n");
            exit(0);
        }
        byte = (int)message;
        if (byte == 4)
            exit(0);
        if (byte == 10) {
            byte = 13;
            message = (char)byte;
        }
        write(fd, &message, 1);
    }
    close(fd);
}

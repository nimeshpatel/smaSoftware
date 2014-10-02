#include <math.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
#define _POSIX_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sched.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <smem.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "contDetector.h"
#include "contDetectordrvr.h"
#include "vme_sg_simple.h"

/* Definitions which should be in readline/readline.h, but aren't in LynxOS */
void readline_initialize_everything(void);
char *readline(char *);
void add_history(char *);
int read_history(char *);
int write_history(char *);
#define HIST_FILE "./.testcntr_history"


#define SHMEM_NAME "Acromag"

#define CHOPPERPRIO 20
#define VERBOSE 0
#define TIMING 0
#if TIMING
double exTime;
void StartTime(void);
void StopTime(void);
#endif /* TIMING */

/* Device pointers */
void *iPCarrier;
char *unidig;
ACROMAG *cntr;

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;				/* continuum detector fd */
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;
int ttfd;			/* File descriptor for the TrueTime device */
struct vme_sg_simple_time ttime;
int lowStart, lowEnd, highStart, highEnd;

volatile int busError = 0;
FILE *pFile = NULL;			/* Print file */
int waitCnt;
int chop;
static int gotINT = 0;			/* Return with QUITRTN */
int repeats = 10;

/* testTiming.c */
void Delay(int cnt);
void WriteDACs(int *values);
void Wait(int usec);
void WDWait(int maxusec);
void Print(void);
void ReadCD();
double ReadTT();
void signalBusError(void);
void usage(void);
static void SigIntHndlr(int signo);

int main(int argc, char *argv[]) {
	double startTime, curTime;
	int i, r;
	struct sigaction sa;
	char *line, cmdChar, *ip;

	readline_initialize_everything();
	read_history(HIST_FILE);
	sa.sa_handler = signalBusError;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGBUS, &sa, NULL)) {
	    perror("sigaction");
	    return(-1);
	}
	/* Open the contDetector and unidig devices */
	if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
	    fprintf(stderr, "Can not open %s, I quit\n", cdName);
	    exit(1);
	}
	if((ttfd = open("/dev/vme_sg_simple", O_RDWR, 0)) < 0) {
	    perror("Can't open TrueTIme");
	    exit(-1);
	}

	if(signal(SIGINT, SigIntHndlr) == SIG_ERR) {
	    fprintf(stderr, "Error setting INT signal disposition\n");
	    exit(-1);
	}
	ttime.timeout_ticks = 2;  	/* This margin should be safe */

	/* This main loop communicates with the user */
	for(;;) {
	    line = readline("ctr: ");
	    if(*line) {			/* Readline removes the '\n' */
		add_history(line);

		/* At this point the main part of the program should run.  */
	        cmdChar = *line;
		/* Skip the remainder of the first word */
	        for(ip = 1 + line; ! isspace(*ip); ip++) ;
		while(isspace(*ip))
		    ip++;
		switch(cmdChar) {
		case 'c':
		    cdIoctlArg.mode = TIMING_TEST;
		    cdIoctlArg.intTime = 300000;
		    ioctl(cdfd, EIOCSETMODE, &cdIoctlArg);
		    ReadCD();
		    startTime = ReadTT();
		    for(i = 0; i < repeats; i++) {
		    do{
			ReadCD();
		    } while(cdOut.status != 2);
		    curTime = ReadTT();
		    lowEnd = (curTime - startTime)*1e6;
		    lowStart = lowEnd - cdOut.lowRx;
		    ReadCD();
		    curTime = ReadTT();
		    highEnd = (curTime - startTime)*1e6;
		    highStart = highEnd - cdOut.highRx;
		    Print();
		    }
		    break;
		case 'f':
		    if(pFile != NULL) {
			fclose(pFile);
		    }
		    if((pFile = fopen(ip, "w")) == NULL) {
			perror("error opening");
		    }
		    break;
		case 'h':
		    usage();
		    break;
		case 'i':
		    break;
		case 'r':
		    r = sscanf(ip, "%d", &repeats);
		    fprintf(stderr, "repeats = %d\n", repeats);
		    break;
		case 'q':
		    write_history(HIST_FILE);
		    return(0);
		default:
		    printf("??\n");
		    break;
		} /* end of the main loop */
	    }
	    free(line);
	}
	fprintf(stderr, "!!! Control should not get here !!!\n");
	return(0);
}

void Delay(int cnt) {
	while(--cnt) ;
}

void Wait(int usec) {
	for(; usec > 0; usec -= 20000) {
	    usleep(2);
	    if(gotINT) return;
	}
}

void WDWait(int maxusec) {
	for(waitCnt = 0; waitCnt < maxusec; waitCnt += 20000) {
	    if(cntr->rb[4] == 0 && cntr->rb[5] == 0)
		break;
	    usleep(20000);
	}
}

void ReadCD() {
    if(read(cdfd, &cdOut, sizeof(cdOut)) < 0) {
	perror("Reading the acromag");
	exit(1);
    }
}

double ReadTT() {
    if(read(ttfd, &ttime, sizeof(ttime)) < 0) {
	perror("Trouble reading the truetime");
	return(-1);
    }
    return((ttime.hour*60 +ttime.min)*60 + ttime.sec + ttime.usec*1e-6);
}

void Print(void) {

	printf("%8d  %8d  %8d  %8d  0x%x\n",
		lowStart, lowEnd, highStart, highEnd,
		cdOut.status);
	if(pFile) {
	    fprintf(pFile, "%8d  %8d  %8d  %8d  0x%x\n",
		lowStart, lowEnd, highStart, highEnd,
		cdOut.status);
	}
#if 0
	if(pFile) {
	    double i1, i2;

	    StopTime();
	    if(chopperRun) {
	    } else {
		i1 = cdOut.lowRx * 1000. / (double)intTime;
		i2 = cdOut.highRx * 1000. / (double)intTime;
		fprintf(pFile, "%8.3f  %8.3f %8.3f  %6d %6d %6d  %6d %6d\n",
		    exTime, i1, i2, intTime, cdOut.lowRx, cdOut.highRx,
		    dacValues[phase][0], dacValues[phase][1]);
	    }
	}
#endif
}

void signalBusError(void) {
	busError = 1;
}

void usage(void) {
	fprintf(stderr, "The commands are:\n"
	    "c \t\tRun in chopper mode for repeat cycles\n"
	    "d\t\tDelay +- from triggering counter to chopper bits\n"
	    "f [Name]\tClose any open file and open Name for printing if "
	    "given\n"
	    "h\t\tType this help message\n"
	    "i \t\tRun in independent mode for repeat cycles\n"
	    "q\t\tquit\n"
	    "r repeat\tset the number of cycles to run (-1 -> forever)\n"
	    "v v1 v2 v3 v4 \tset two D/A values for each chopper phase\n"
	    "V dv1 dv2\tset increments for the two D/A values\n"
	    "t time\tSet integration time in usec\n"
	    );
}

/* Subroutine to handle SIGINT (^C) interrupts and shut down gracefully */
static void SigIntHndlr(int signo) {
	gotINT = 1;
}

#if TIMING
struct timeval tv1, tv2;
struct timezone tz;

void StartTime(void) {
	gettimeofday(&tv1, &tz);
}

void StopTime(void) {
	gettimeofday(&tv2, &tz);
	exTime = (double)(tv2.tv_sec - tv1.tv_sec) +
		(double)(tv2.tv_usec - tv1.tv_usec) / 1e6;
}
#endif /* TIMING */

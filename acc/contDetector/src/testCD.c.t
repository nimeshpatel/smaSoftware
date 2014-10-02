#include <math.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
#define _POSIX_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
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
#include "iPUniDig_E.h"

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
#define TIMING 1
#if TIMING
double exTime;
void StartTime(void);
void StopTime(void);
#endif /* TIMING */

/* Device pointers */
void *iPCarrier;
char *unidig;
ACROMAG *cntr;
/* UnidigE definitions for generating free_run/chopper_bar (bit 0) and a
 * fake chopper_in_position signal (bit 1) */
#define UD_MODULE 1
#define UD_CR 0xd
#define UD_LOW_BITS 0x1
#define ENABLE_OUTPUT  0x04 /* Set the control register bit to enable output */

/* IP-OptoDA16 definitions for producing fake receiver outputs */
#define DAC_MODULE 0
#define CHANSEL 1
#define STATUSDAC 3
#define DATAREG 4
#define LOADDAC 7
#define AUTOLOAD 0x80
#define DAC_BUSY 1
#define DAC_RANGE 2
#define DAC_ERROR 4
char *dacp;

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;				/* continuum detector fd */
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;

/* Things for the Unidig device */
char udName[] = "/dev/iPUniDig_D";
int udfd;				/* unidigD fd */
int udIoctlArg;

/* Vars for a thread to generate 'chopper signals" on the unidig */
int chopperRun = 0;
sigset_t chopperSigSet = { {(1 << (SIGCONT - 1)), 0 } };
pthread_t chopperTid;
int chopperPid;
#if _POSIX_VERSION >= 199506
	struct sched_param param;
#else  /* _POSIX_VERSION >= 199506 */
	pthread_attr_t attr;
#endif /* _POSIX_VERSION >= 199506 */

volatile int busError = 0;
FILE *pFile = NULL;			/* Print file */
int intTime0 = 10000, intTime, intTimeIncr = 0;
int waitCnt;
int chop;
static int gotINT = 0;			/* Return with QUITRTN */
#define NUM_PHASES 2
int dacValues[NUM_PHASES][2] = {{10000, 1000}, {1000, 10000}};
int dacInc0, dacInc1;
int dacSave0, dacSave1;
int repeats = 1;
int phase;
int chopDelay = 100;

/* testCD.c */
void Delay(int cnt);
void Wait(int usec);
void WDWait(int maxusec);
void Print(void);
void iInt();
void cInt();
void signalBusError(void);
void usage(void);
static void SigIntHndlr(int signo);
void *Chopper(void *dmy);
void WriteChopBits(int bits);

int main(int argc, char *argv[]) {
	int i, r, address;
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
	if((udfd = open(udName, O_RDONLY, 0)) < 0) {
	    fprintf(stderr, "Can not open %s, I quit\n", cdName);
	    exit(1);
	}

	if(signal(SIGINT, SigIntHndlr) == SIG_ERR) {
	    fprintf(stderr, "Error setting INT signal disposition\n");
	    exit(-1);
	}

#if 0
	/* Start the chopper thread */
#if _POSIX_VERSION >= 199506
	if(pthread_create(&chopperTid, NULL, Chopper, (void *)0) < 0) {
	    perror("Failed to create chopper");
	    exit(1);
	}
	param.sched_priority = CHOPPERPRIO;
	pthread_setschedparam(chopperTid, SCHED_DEFAULT, &param);
#else  /* _POSIX_VERSION >= 199506 */
	pthread_attr_create(&attr);
	if(pthread_create(&chopperTid, attr, Chopper, (void *)0) < 0) {
	    perror("Failed to create chopper");
	    exit(1);
	}
	pthread_setprio(chopperTid, CHOPPERPRIO);
#endif /* _POSIX_VERSION >= 199506 */
	chopperPid = BUILDPID(0, chopperTid);
#endif /* 0 */

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
		    chopperRun = 1;
		    cdIoctlArg.mode = CHOP;
		    cdIoctlArg.intTime = intTime = intTime0;
		    ioctl(cdfd, EIOCSETMODE, &cdIoctlArg);
		    for(r = repeats; r != 0; r--) {
			if(read(cdfd, &cdOut, sizeof(cdOut)) != sizeof(cdOut)) {
			    perror("Reading cont det");
			}
			if(gotINT) {
			    gotINT = 0;
			    fprintf(stderr, "Received SIGINT\n");
			    goto endC;
			}
			Print();
		    }
		    chopperRun = 0;
endC:
		    break;
		case 'd':
		    r = sscanf(ip, "%d", &chopDelay);
		    fprintf(stderr, "chopDelay = %d\n", chopDelay);
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
		    cdIoctlArg.mode = IND;
		    intTime = intTime0;
#if 0
		    i = 1;
		    if(write(udfd, &i, 3) < 3) {
			fprintf(stderr, "Error writing to unidig\n");
		    }
#endif
		    phase = 0;
		    StartTime();
		    for(r = repeats; r != 0; r--) {
			cdIoctlArg.intTime = intTime;
			ioctl(cdfd, EIOCSETMODE, &cdIoctlArg);
			if(read(cdfd, &cdOut, sizeof(cdOut)) != sizeof(cdOut)) {
			    perror("Reading cont det");
			}
			Print();
			if(gotINT) {
			    gotINT = 0;
			    fprintf(stderr, "Received SIGINT\n");
			    goto endI;
			}
			intTime += intTimeIncr;
			if(intTime < 100)
			    intTime = 100;
		    }
endI:
		    break;
		case 'r':
		    r = sscanf(ip, "%d", &repeats);
		    fprintf(stderr, "repeats = %d\n", repeats);
		    break;
		case 'u':
		    if((r = read(udfd, &i, 3)) != 3) {
			fprintf(stderr, "bad read of unidig %d\n", r);
		    }
		    printf("unidig = 0x%06x\n", i);
		    break;
		case 'v':
		    if(isdigit(*ip) || *ip == '-') {
			sscanf(ip, "%d %d %d %d",
				&dacValues[0][0], &dacValues[0][1],
				&dacValues[1][0], &dacValues[1][1]);
		    }
		    for(i = 0; i < 2; i++) {
		    }
		    printf("Phase 1 D/A values are %d, %d  Phase 2 %d, %d\n",
			dacValues[0][0], dacValues[0][1],
			dacValues[1][0], dacValues[1][1]);
		    break;
		case 'V':
		    if(isdigit(*ip) || *ip == '-') {
			sscanf(ip, "%d %d", &dacInc0, &dacInc1);
		    }
		    printf("Dac increments are %d %d\n", dacInc0, dacInc1);
		    break;
		case 't':
		    r = sscanf(ip, "%d %d", &intTime0, &intTimeIncr);
		    if(r > 0 && (intTime0 <= 0 || intTime0 > 60000000)) {
			fprintf(stderr, "Bad Integration Time (%d) given use "
				"10000\n", intTime0);
			intTime0 = 10000;
			if(r < 2)
			    intTimeIncr = 0;
		    } else {
			fprintf(stderr, "int time = %d usec. increment is %d "
			"per cycle\n", intTime0, intTimeIncr);
		    }
		    break;
#if 0
		case 'u':
		    unidig[0] = 0xa5;
		    unidig[1] = 0x5f;
		    printf("unidig readback 0x%x, direct 0x%x\n",
		    *(unsigned short *)unidig, *(unsigned short *)(unidig + 4));
		    break;
#endif
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

void Print(void) {
	printf("%8d %8d   %8d %8d  %5d %5d  0x%x\n", intTime,
		cdOut.timer, cdOut.lowRx, cdOut.highRx,
		dacValues[phase][0], dacValues[phase][1],
		cdOut.status);
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
}

#if 1
void iInt() {
	cntr->initValue[4] = intTime >> 16;
	cntr->initValue[5] = intTime & 0xffff;
	cntr->triggerCntl = 0x2a;
}

#else

void iInt() {
	cntr->cntl[1] = INPUT_ACTIVE_HIGH | USE_32_BITS | EXT_CLOCK;
	cntr->cntl[3] = INPUT_ACTIVE_HIGH | USE_32_BITS | EXT_CLOCK;
	cntr->initValue[3] = 0;
	cntr->initValue[2] = 0;
	cntr->initValue[1] = 0;
	cntr->initValue[0] = 0;
	cntr->cntl[1] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	cntr->cntl[3] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	cntr->cntl[5] = OUTPUT_ACTIVE_HIGH | USE_32_BITS;
	cntr->initValue[4] = intTime >> 16;
	cntr->initValue[5] = intTime & 0xffff;
	cntr->cntl[5] = WATCH_DOG_MODE | OUTPUT_ACTIVE_HIGH | USE_32_BITS;
	cntr->triggerCntl = 0x2a;
/*	Delay(1000);
	cntr->triggerCntl = 0x20; */
}
#endif

void cInt() {
#if 0
	cntr->initValue[4] = 0;
	cntr->initValue[5] = 0;
#endif
	cntr->triggerCntl = 0x2a;
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

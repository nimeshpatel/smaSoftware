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
#include "rm.h"

/* Definitions which should be in readline/readline.h, but aren't in LynxOS */
void readline_initialize_everything(void);
char *readline(char *);
void add_history(char *);
int read_history(char *);
int write_history(char *);
#define HIST_FILE "./.testcntr_history"
#define CHOPPERPRIO 20
#define VERBOSE 0
#define TIMING 1
#if TIMING
double exTime;
void StartTime(void);
void StopTime(void);
#endif /* TIMING */

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;				/* continuum detector fd */
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;

/* Things for the Unidig device */
char udName[] = "/dev/iPUniDig_D";
int udfd;				/* unidigD fd */
#include "patchPanelBits.h"

FILE *pFile = NULL;			/* Print file */
int intTime0 = 10000, intTime, intTimeIncr = 0;
int waitCnt;
int chopPosn;
static int gotINT = 0;			/* Return with QUITRTN */
#define NUM_PHASES 2
int repeats = 1;
int subIntervals = 100;
int chopperRun = 0;
int rm_status;
int antlist[RM_ARRAY_SIZE];

/* testCD.c */
void Delay(int cnt);
void Wait(int usec);
void Print(void);
void signalBusError(void);
void usage(void);
static void SigIntHndlr(int signo);
void *Chopper(void *dmy);
void WriteChopBits(int bits);

int main(int argc, char *argv[]) {
	int i, v, r, s;
	int sumTime, sumLow, sumHigh;
	char *line, cmdChar, *ip;

	readline_initialize_everything();
	read_history(HIST_FILE);

	/* Open the contDetector and unidig devices */
	if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
	    fprintf(stderr, "Can not open %s, I quit\n", cdName);
	    exit(1);
	}
	if((udfd = open(udName, O_RDONLY, 0)) < 0) {
	    fprintf(stderr, "Can not open %s, I quit\n", udName);
	    exit(1);
	}
    /* initializing ref. mem. */
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }

	if(signal(SIGINT, SigIntHndlr) == SIG_ERR) {
	    fprintf(stderr, "Error setting INT signal disposition\n");
	    exit(-1);
	}
	setpriority(PRIO_PROCESS, 0, 100);

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
		    if(read(udfd, &v, 3) != 4) {
			fprintf(stderr, "bad read of unidig\n");
			break;
		    }
		    if((v & CONT_DET_MODE_W) != 0) {
			printf("Cont det mode not set to chop\n");
			break;
		    }
		    chopperRun = 1;
		    cdIoctlArg.mode = CHOP;
		    cdIoctlArg.intTime = intTime = intTime0;
		    ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
		    for(r = repeats; r != 0; r--) {
			for(i = 0; i < 20; i++) {
			    if((read(udfd, &v, 4)) != 4) {
				fprintf(stderr, "bad read of unidig\n");
			    } else if((v & TRIGGER_SPARE_R) == 0) {
				goto ReadC;
			    }
			    usleep(1);
			}
			printf("Chopper always in position, can't start\n");
			chopperRun = 0;
			break;
ReadC:
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
		    cdIoctlArg.intTime = intTime;
		    ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
		    StartTime();
		    for(r = repeats; r != 0; r--) {
			if(read(cdfd, &cdOut, sizeof(cdOut)) != sizeof(cdOut)) {
			    perror("Reading cont det");
			}
			Print();
			if(gotINT) {
			    gotINT = 0;
			    fprintf(stderr, "Received SIGINT\n");
			    goto endi;
			}
			intTime += intTimeIncr;
			if(intTime < 100)
			    intTime = 100;
		    }
endi:
		    break;
		case 'I':
		    cdIoctlArg.mode = IND;
		    intTime = intTime0 / subIntervals;
		    cdIoctlArg.intTime = intTime;
		    ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
		    StartTime();
		    for(r = repeats; r != 0; r--) {
			sumTime = 0;
			sumLow = 0;
			sumHigh = 0;
			for(s = 0; s < subIntervals; s++) {
			    if(read(cdfd, &cdOut, sizeof(cdOut)) !=
				    sizeof(cdOut)) {
				perror("Reading cont det");
			    }
			    sumTime += cdOut.timer;
			    sumLow += cdOut.lowRx;
			    sumHigh += cdOut.highRx;
			}
			cdOut.timer = sumTime ;
			cdOut.lowRx = sumLow ;
			cdOut.highRx = sumHigh ;
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
		case 's':
		    r = sscanf(ip, "%d", &subIntervals);
		    fprintf(stderr, "subIntervals = %d\n", subIntervals);
		    break;
		case 'r':
		    r = sscanf(ip, "%d", &repeats);
		    fprintf(stderr, "repeats = %d\n", repeats);
		    break;
		case 'u':
		    if((r = read(udfd, &i, 3)) != 4) {
			fprintf(stderr, "bad read of unidig %d\n", r);
		    }
		    printf("unidig write bits = 0x%06x", i);
		    if((r = read(udfd, &i, 4)) != 4) {
			fprintf(stderr, "bad read of unidig %d\n", r);
		    }
		    printf("  read bits = 0x%06x\n", i);
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

void Print(void) {
	int v;
	float gunnBias;

	if((read(udfd, &v, 4)) != 4) {
	    fprintf(stderr, "bad read of unidig\n");
	}
	chopPosn = (v >> 4) & 0xb;
            rm_status=rm_read(RM_ANT_0,"RM_GUNN_BIAS_F", &gunnBias);
            if(rm_status!=RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
            }
	printf("%8d %8d   %8d %8d  %d %7.4f\n", intTime,
		cdOut.timer, cdOut.lowRx, cdOut.highRx, chopPosn, gunnBias);
	if(pFile) {
	    double i1, i2;

	    StopTime();
	    if(chopperRun) {
		i1 = cdOut.lowRx * 1000. / (double)cdOut.timer;
		i2 = cdOut.highRx * 1000. / (double)cdOut.timer;
		fprintf(pFile, "%8.3f  %8.3f %8.3f  %6d %6d %6d  %d\n",
		    exTime, i1, i2, cdOut.timer, cdOut.lowRx, cdOut.highRx,
		    chopPosn);
	    } else {
		i1 = cdOut.lowRx * 1000. / (double)intTime;
		i2 = cdOut.highRx * 1000. / (double)intTime;
		fprintf(pFile, "%8.3f  %8.3f %8.3f  %6d %6d %6d %7.4f\n",
		    exTime, i1, i2, intTime, cdOut.lowRx, cdOut.highRx,
		    gunnBias);
	    }
	}
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

#include <math.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
/* #define _POSIX_SOURCE */
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
#include "contDetector.h"
#include "contDetectordrvr.h"

/* Definitions which should be in readline/readline.h, but aren't in LynxOS */
void readline_initialize_everything(void);
char *readline(char *);
void add_history(char *);
int read_history(char *);
int write_history(char *);
#define HIST_FILE "./.testcntr_history"


#define SHMEM_NAME "Acromag"

#define VERBOSE 0
#define TIMING 0
#if TIMING
int exTime;
void StartTime(void);
void StopTime(char *name);
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

volatile int busError = 0;
FILE *pFile = NULL;			/* Print file */
int intTime0 = 10000, intTime, intTimeIncr = 0;
int waitCnt;
int chop;
static int gotINT = 0;			/* Return with QUITRTN */
#define NUM_PHASES 2
int dacValues[NUM_PHASES][2] = {{10000, 1000}, {1000, 10000}};
int repeats = 1;
int phase;
int chopDelay = 100;

/* testcntr.c */
void Delay(int cnt);
void iSetCntl(void);
void cSetCntl(void);
void WriteDACs(int *values);
void Wait(int usec);
void WDWait(int maxusec);
void Print(void);
void iInt();
void cInt();
void signalBusError(void);
void usage(void);
static void SigIntHndlr(int signo);

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

	/* Set up the IP Carrier in a shared memory segment */
	address = VME_A16_PHYS_ADDRESS  + ACROMAG_CARRIER_BASE;
#if VERBOSE > 1
	printf("About to smem_create for Phys address 0x%x\n", address);
#endif /* VERBOSE > 1 */
	if(smem_remove(SHMEM_NAME) < 0) {
	    perror("smem_remove failed");
	}
	iPCarrier = smem_create(SHMEM_NAME, (char *)address, 0x1000,
		SM_READ | SM_WRITE);
	if(iPCarrier == NULL) {
	    perror("smem_create failed");
	    exit(1);
	}
	dacp = (char *)iPCarrier + IP_MODULE_OFFSET * DAC_MODULE;
	unidig = iPCarrier + IP_MODULE_OFFSET * UD_MODULE;
	unidig[UD_CR] = ENABLE_OUTPUT;
	cntr = iPCarrier + IP_MODULE_OFFSET * ACROMAG_MODULE;
#if VERBOSE > 1
	printf("Acromag Module mapped to = 0x%x\n", (int)cntr);
#endif /* VERBOSE > 1 */
        busError = 0;
        i = cntr->ascii[0];
	if (busError) {
	    printf("Nothing seen at 0x%x\n", address);
            exit(1);
        }
	printf("ID Contents: ");
	for(i = 0; i < 4; i++) {
	    putchar(cntr->ascii[i]);
	}
	printf(", Mfrg code 0x%x, Prod code 0x%x, #bytes 0x%x, crc 0x%x\n",
		cntr->manufacturer, cntr->modelNo, cntr->idBytes,
		cntr->crc);
	cntr->intPend = 0;
	if(signal(SIGINT, SigIntHndlr) == SIG_ERR) {
	    fprintf(stderr, "Error setting INT signal disposition\n");
	    exit(-1);
	}
#if 0
	for(;;) {
	    WriteDACs(dacValues[0]);
	    Wait(20000);
	    WriteDACs(dacValues[1]);
	    Wait(20000);
	    if(gotINT) exit(0);
	}
#endif

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
		    cSetCntl();
		    intTime = intTime0;
		    for(r = repeats; r != 0; r--) {
			for(phase = 0; phase < NUM_PHASES; phase++) {
			    unidig[UD_LOW_BITS] = 0;
			    WriteDACs(dacValues[phase]);
			    Wait(20000);
			    if(chopDelay < 0) {
			        unidig[UD_LOW_BITS] = CHOPPER_IN_POSITION ;
			        Delay(-chopDelay);
			        cInt();
			    } else {
			        cInt();
			        Delay(chopDelay);
			        unidig[UD_LOW_BITS] = CHOPPER_IN_POSITION ;
			    }
			    Wait(intTime);
			    unidig[UD_LOW_BITS] = 0;
			    if(gotINT) {
				gotINT = 0;
				fprintf(stderr, "INTerrupt\n");
				goto endc;
			    }
			    Print();
			}
			intTime += intTimeIncr;
			if(intTime < 100)
			    intTime = 100;
		    }
endc:
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
		    iSetCntl();
		    intTime = intTime0;
		    unidig[UD_LOW_BITS] = FREE_RUN;
		    phase = 0;
		    WriteDACs(dacValues[0]);
		    for(r = repeats; r != 0; r--) {
			iInt();
			WDWait(intTime + 20000);
			if(gotINT) {
			    gotINT = 0;
			    fprintf(stderr, "INTerrupt\n");
			    goto endI;
			}
			Print();
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
		case 'u':
		    unidig[0] = 0xa5;
		    unidig[1] = 0x5f;
		    printf("unidig readback 0x%x, direct 0x%x\n",
		    *(unsigned short *)unidig, *(unsigned short *)(unidig + 4));
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

/* Subroutine to set the counter control registers for independent (32-bit)
 * operation */
void iSetCntl(void) {
	int i;

	cntr->cntl[5] = WATCH_DOG_MODE | OUTPUT_ACTIVE_HIGH | USE_32_BITS;
	cntr->cntl[1] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK | DEBOUNCE_INPUT;
	cntr->cntl[3] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	for(i = 0; i < 6; i++) {
	    cntr->initValue[i] = 0;
	}
}

/* Subroutine to set the counter control registers for independent (32-bit)
 * operation */
void cSetCntl(void) {
	int i;

	cntr->cntl[5] = PULSE_WIDTH_MODE | USE_32_BITS;
	cntr->cntl[1] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK | DEBOUNCE_INPUT;
	cntr->cntl[3] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	for(i = 0; i < 6; i++) {
	    cntr->initValue[i] = 0;
	}
}

void WriteDACs(int *values) {
	int i, s1, s2, s3, s4;

	dacp[STATUSDAC] = 0;
	s1 = dacp[STATUSDAC];
	*(short *)(dacp + DATAREG) = values[1];
	s2 = dacp[STATUSDAC];
	dacp[CHANSEL] = 0;
	s3 = dacp[STATUSDAC];
	for(i = 0; i < 10000; i++) {
	    if(gotINT) exit(0);
	    if(((s4 = dacp[STATUSDAC]) & DAC_BUSY) == 0)
		break;
	}
/*	printf("Delay = %d status %d %d %d %d\n", i, s1, s2, s3, s4); */
	*(short *)(dacp + DATAREG) = values[0];
	dacp[CHANSEL] = AUTOLOAD + 1;
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
	int c2, c4, c6;

	c2 = cntr->rb[0] << 16 | cntr->rb[1];
	c4 = cntr->rb[2] << 16 | cntr->rb[3];
	c6 = cntr->rb[4] << 16 | cntr->rb[5];
	printf("%8d %8d   %8d %8d  %5d %5d  %d\n", intTime, c6, c2, c4,
		dacValues[phase][0], dacValues[phase][1], waitCnt);
	if(pFile) {
	    fprintf(pFile, "%8d %8d   %8d %8d  %5d %5d  %d\n",
		intTime, c6, c2, c4,
		dacValues[phase][0], dacValues[phase][1], waitCnt);
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

void StopTime(char *name) {
	gettimeofday(&tv2, &tz);
	exTime = (int)(tv2.tv_usec - tv1.tv_usec);
}
#endif /* TIMING */

#include <curses.h>
/* Compensate for incomplete LynxOS curses.h file */
extern int mvprintw    _AP((int, int, const char *fmt, ...));
#include <math.h>
#include <termio.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "rm.h"
#include "deiced.h"

int quit = 0;
int line;

/* Reflective memory variables */
int rm_open_status;

/* Number of SMA antennas with deice */
#define NUMANTENNAS 8
int antennas[NUMANTENNAS] = {0,0,0,0,0,0,0,0};
short powerCmd[RM_ARRAY_SIZE][5];
unsigned int cmdTimestamp[RM_ARRAY_SIZE];
float current[RM_ARRAY_SIZE][3];
unsigned int status[RM_ARRAY_SIZE];
unsigned int phase[RM_ARRAY_SIZE];
unsigned int statusTimestamp[RM_ARRAY_SIZE];
unsigned int oldTimestamp[RM_ARRAY_SIZE];
unsigned int unixTime;
int unixTimeAntenna;
#define VALID_CMD_TIME (14*24*3600)		/* Limit old values to 2 weeks */
#define VALID_STATUS_TIME (600)			/* Limit status to 10 min */

struct rm_var {
	void *a;
	char *name;
} rv[] = {
	{powerCmd, "RM_DEICE_POWER_CMD_V5_S"},
	{cmdTimestamp, "RM_DEICE_CMD_TIMESTAMP_L"},
	{current, "RM_DEICE_CURRENT_V3_F"},
	{status, "RM_DEICE_STATUS_BITS_L"},
	{phase, "RM_DEICE_PHASE_L"},
	{statusTimestamp, "RM_DEICE_TIMESTAMP_L"}
};
#define NUMVARS (sizeof(rv) / sizeof(rv[0]))


/* deicemon.c */
static void SigHndlr(int signo);
void OpenRM(void);
void ReadRM(void);

int main(int argc, char *argv[]) {
	char ch;
	struct termio   tio;
	int ant, stat, i;
	char statStr[] = "                                 ";

	OpenRM();
	initscr();
	if(signal(SIGQUIT, SigHndlr) == SIG_ERR) {
	    fprintf(stderr, "Error setting QUIT signal disposition\n");
	    exit(1);
	}
	ioctl(0, TCGETA, &tio);
	tio.c_lflag &= ~ECHO;
	tio.c_lflag &= ~ICANON;
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 0;
	ioctl(0, TCSETA, &tio);

	mvaddstr(0,3,"State of the deice systems\n");
	mvaddstr(2,0,"Ant   Power Level   Sys        State of Zones           "
		"  Phase Currents\n");
	mvaddstr(3,0,"Num Sur Cho Sub Opt Pha Op Su Ch Q2 Q1 D6 D5 D4 D3 D2 D1"
		"  A     B     C\n");
	while(! quit) {
	    ReadRM();
	    for(ant = 1; ant <= NUMANTENNAS; ant++) {
		if(antennas[ant - 1] == 0) {
		    continue;
		}
		i = unixTime - cmdTimestamp[ant];
		if(i > VALID_CMD_TIME || i < -100) { /* Commands are invalid */
		    mvprintw(3 + ant * 2, 0, " %1d   ?   ?   ?   ?   ", ant);
		} else {
		    mvprintw(3 + ant * 2, 0, " %1d  %2d  %2d  %2d  %2d   ", ant,
			powerCmd[ant][MAIN_P], powerCmd[ant][CHOP_P],
			powerCmd[ant][SUBR_P], powerCmd[ant][OPTEL_P] );
		}

		i = unixTime - statusTimestamp[ant];
		if(i > VALID_STATUS_TIME || i < -100) { /* Status is invalid */
		    mvprintw(3 + ant * 2, 21, " Status is stale\n");
		    continue;
		}else {
		    stat = status[ant];
		    for(i = 0; i < ZONES; i++) {
			statStr[i*3] = (stat & (1 << (11 - i)))? '1': '0';
		    }
		    mvprintw(3 + ant * 2, 21, "%d  %s%-4.1f  %-4.1f  %-4.1f\n",
			phase[ant], statStr, current[ant][0],
			current[ant][1], current[ant][2]);
		}
	    }

	    refresh();
	    sleep(1);
	    while(read(0, &ch, 1)) {
		if(ch == 'q')
		    goto DONE;
		}
	}
DONE:
	endwin();
	return(0);
}


/* Subroutine to handle SIGINT (^C) interrupts and shut down gracefully */
static void SigHndlr(int signo) {
	quit = 1;
}

void OpenRM(void) {
	int antlist[RM_ARRAY_SIZE];
	int *ip;

	rm_open_status=rm_open(antlist);
	if(rm_open_status != RM_SUCCESS) {
	    rm_error_message(rm_open_status,"rm_open()");
	    exit(1);
	}
	for(ip = antlist; *ip != RM_ANT_LIST_END; ip++) {
	    antennas[*ip - 1] = 1;
	}
	unixTimeAntenna = antlist[0];
}

void ReadRM(void) {
	int i;
	int rm_rtn;

	for(i = 0; i < NUMVARS; i++) {
	    rm_rtn=rm_read(RM_ANT_ALL, rv[i].name, rv[i].a);
	    if(rm_rtn != RM_SUCCESS) {
		fprintf(stderr, "rm_read of ");
		rm_error_message(rm_rtn, rv[i].name);
	    }
	}
	rm_rtn=rm_read(unixTimeAntenna, "RM_UNIX_TIME_L", &unixTime);
	if(rm_rtn != RM_SUCCESS) {
	    fprintf(stderr, "rm_read of ");
	    rm_error_message(rm_rtn, "RM_UNIX_TIME_L");
	}
}

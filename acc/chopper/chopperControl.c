/* Chopper Control and Monitor Daemon */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h> /* threads */
#include <sys/file.h> /* for device open */
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <commonLib.h>
#include <stderrUtilities.h>

#include "chopperControl.h"
#include "smadaemon.h" /* for QUIT_RTN */
#include "rm.h" /* reflective memory */
#include "iPOctal232.h"

#define COMPLAIN 0
#define LISTCMD 1

#define PMAC_RTN_SIZE 128
#define PMAC_WRITE_SIZE 80

#define CONTROL_PRIO 20
#define MONITOR_PRIO 18
#define pmacrwp(x, y) pmacrw(x, y); SendMessageToRM(x)

/* chopperControl.c */
void *ChopperPMAC();
void *ChopperMonitor();
/* void pmacw(char *command); */
void pmacrw(char *command, char *rtn);
void pmacw(char *command);
void ReadConfigFile(void);
void ReadFarCounts(void);
void ReadNearCounts(void);
int MoveXyz(int *newPos);
int UpdateXyz(void);
int StowChopper(void);
void ReadCmdPos(void);
void ReadCaptureRegs(int * pos);
void ReadCaptureFlags(int * flags);
void ReadXyzEncoders(int * pos);
int MoveTilt(int t);
int CheckHome(void);
void ResetPMAC( void);
void RestartMotors();
void HomeChopper(void);
void InitChopper(void);
void MeasureDrift(void);
#if LISTCMD
void ListToFile(char *cmd);
#endif
void DownloadFile(char *fn);
void WriteCmdPos(char type);
void CheckPmacCmd(char *cp);
void SendMessageToRM(char *messg);


/* Global variables */

/* Chopper system control and status RM variables */
char pmacResponsev[] = 	"RM_CHOPPER_PMAC_RESPONSE_C128";
char statusBitsv[] =	"RM_CHOPPER_STATUS_BITS_V16_B";
char monTimestampv[] =	"RM_CHOPPER_MONITOR_TIMESTAMP_L";
char cmdv[] =		"RM_CHOPPER_SMASH_COMMAND_C30";
char cmdFlagv[] = 	"RM_CHOPPER_SMASH_COMMAND_FLAG_S";
char choppingFlagv[] = 	"RM_CHOPPING_FLAG_S";
char focusCurveFlagv[] 	= "RM_CHOPPER_FOCUS_CURVE_FLAG_L";
char focusCurveMmv[] = 	"RM_CHOPPER_FOCUS_CURVE_MM_V4_F";
char eldrivestatev[] =	"RM_EL_DRV_STATE_B";
char trackelv[] = 	"RM_TRACK_EL_F";
char encelv[] =		"RM_ENCODER_EL_F";

char posMmv[] =		"RM_CHOPPER_POS_MM_V4_F";
char cmdPosMmv[] =	"RM_CHOPPER_CMD_POS_MM_V4_F";
char cmdPosTypev[] =	"RM_CHOPPER_CMD_POS_TYPE_C2";
char farPosMmv[] = 	"RM_CHOPPER_FAR_POS_MM_V4_F";
char homeChkv[] = 	"RM_CHOPPER_HOME_CHECK_RESULTS_V3_S";
char homeChkTimev[] =	"RM_CHOPPER_HOME_CHECK_TIME_L";
char pmacResetTimev[] =	"RM_CHOPPER_PMAC_RESET_TIME_L";
char chopperTemperaturev[] = "RM_CHOPPER_TEMPERATURE_F";

char unixTimev[] =	"RM_UNIX_TIME_L";
enum eprintfTokens {PMAC_COMMAND, PMAC_RESPONSE, STATUS_BITS,
                    MONITOR_TIMESTAMP, SMASH_COMMAND, SMASH_COMMAND_FLAG,
		    YZ_CORRECTION_FLAG, POS, CMD_POS, CMD_POS_TYPE,
		    HOME_CHECK_RESULTS, HOME_CHECK_TIME,
                    PMAC_RESET_TIME, UNIX_TIME};

const double cnt2mm[4] = {
                             X_MM_PER_COUNT, Y_MM_PER_COUNT, Z_MM_PER_COUNT,
                             TILT_ARCSEC_PER_COUNT
                         };

const double mm2cnt[4] = {
                             X_COUNTS_PER_MM, Y_COUNTS_PER_MM, Z_COUNTS_PER_MM,
                             TILT_COUNTS_PER_ARCSEC
                         };
const int updateCounts[] = {
                               X_UPDATE_COUNTS, Y_UPDATE_COUNTS, Z_UPDATE_COUNTS
                           };

/* Constants for monitor to use for updating */
#define XYZ_ERROR 1
#define TILT_OPEN_LOOP 2

int timeOutJiffies = 300;		/* 3 sec. */
char instDir[] = "/instance/chopperPMAC/";

/* Variables written by both threads */
int updateCountdown = 0;

/* Variables written by the main thread */
int antlist[RM_ARRAY_SIZE];
int ant = RM_ANT_0;

int pmacfd;
char pmacrtn[PMAC_RTN_SIZE];

pthread_t ChopperMonitorTID;
int stopMonitor = 0;
pthread_mutex_t pmacMutex = PTHREAD_MUTEX_INITIALIZER;
char dmyVarName[RM_NAME_LENGTH];

short pmac_command_flag;
short homeCheckResults[3];
/* These 'pos' variables contain raw counts.
 * The standard value for pos[3] is TILT_HOME = 32768 */
int pos[4], cmdPos[4], farPos[4], nearPos[4], focusCurve[4];
float cosCoef[4], sinCoef[4];
float posMm[4], cmdPosMm[4], farPosMm[4], focusCurveMm[4];
char cmdPosType[2];
int cmdPosWritten;
FILE *listfp = 0; /* If this is not NULL, pmac return will be written to it */

/* Variables written by the monitor thread */
int unixTime;
unsigned char statusBits[16];

int p0status, p1status, p2status, p3status, p20status;
int updatesNeeded = 0, updateTries = 0;
short chopperMotionStatus=0;
double subx_counts, suby_counts, subz_counts, subtilt_counts;
int focusCurveFlag;
float elevation;
char elDriveState;

int main(int argc, char*argv[]) {
    int rm_status;

    DAEMONSET
    setpriority(PRIO_PROCESS, (0), (CONTROL_PRIO));

    /* initializing ref. mem. */
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        fprintf(stderr,"Could not open reflective memory.");
        exit(QUIT_RTN);
    }
    rm_status=rm_write(RM_ANT_0, focusCurveMmv, focusCurveMm);

    /* setup to listen to ref. mem. interrupts for SMAsh commands*/
    rm_status=rm_monitor(RM_ANT_0, cmdFlagv);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_monitor of smash cmd flag");
        exit(QUIT_RTN);
    }

    /* opening the chopper pmac serial comm device driver */
    pmacfd = open("/dev/iPOctal232-3",O_RDWR);
    if(pmacfd == -1) {
        perror("Opening pmac serial line");
        exit(QUIT_RTN);
    }
    rm_status=ioctl(pmacfd,IOCTL_CHANGE_TIMEOUT,&timeOutJiffies);
    if(rm_status)
        perror("Setting timeout for PMAC response");

    ReadConfigFile();
    /* Start the monitor thread */
    if (pthread_create(&ChopperMonitorTID, NULL, ChopperMonitor,
                       (void *)0) != 0) {
        perror("pthread_create ChopperMonitor");
        exit(QUIT_RTN);
    }

    /* use the original thread for controlling the chopper */
    usleep(500000);
    ChopperPMAC();

    return 0;
}


void *ChopperPMAC() {
    char pmac_command[30];
    int rm_status, i;

    /* Clear any prior commands */
    pmac_command_flag = 0;
    rm_status = rm_write(RM_ANT_0,cmdFlagv,&pmac_command_flag);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_write smash cmd flag");
        exit(SYSERR_RTN);
    }
    ReadCmdPos();
    for(;;) {
        pmac_command_flag = 0;
        rm_status=rm_write(RM_ANT_0,cmdFlagv,&pmac_command_flag);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_write() pmac_command_flag");
            exit(SYSERR_RTN);
        }
        rm_status=rm_read_wait(&i, dmyVarName, &pmac_command_flag);
        /* If the monitor thread detects that an update is needed, it does
         * an rm_release_read(), causing RM_REL_PREMATURE */
        if(rm_status == RM_REL_PREMATURE) {
            static int oldfocusCurveFlag = -7;

            pmac_command_flag = AUTO_UPDATE;
	    if(updatesNeeded & TILT_OPEN_LOOP) {
		RestartMotors();
	    }
	    if(updatesNeeded & XYZ_ERROR) {
		if(focusCurveFlag > 0 || oldfocusCurveFlag != focusCurveFlag) {
		    MoveXyz(cmdPos);
		    oldfocusCurveFlag = focusCurveFlag;
		} else {
		    UpdateXyz();
		}
	    }
            /* If a command came in while updating, do it */
            rm_read(RM_ANT_0,cmdFlagv,&pmac_command_flag);
            if(pmac_command_flag == 0) {
                continue;
	    }
        } else if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read_wait for pmac_command_flag");
            exit(SYSERR_RTN);
        }

        rm_status=rm_read(RM_ANT_0,cmdv, pmac_command);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read() chopper SMASH command");
            exit(SYSERR_RTN);
        }
        cmdPosWritten = 0;

        /* fprintf(stderr, "chopperControl: recieved command %s\n",
        pmac_command); */
        /* check if it is a farChopper or other complex command*/
        if(!strncmp(pmac_command,"far", 3)) {
            ReadConfigFile();
            MoveXyz(farPos);
            MoveTilt(farPos[3]);
            WriteCmdPos('F');
        } else if(!strncmp(pmac_command,"near", 4)) {
            ReadConfigFile();
            MoveXyz(nearPos);
            MoveTilt(nearPos[3]);
            WriteCmdPos('N');
        } else if(!strncmp(pmac_command,"stop", 4)) {
            pmacrwp("p2=8", pmacrtn);
            sleep(3);
            pmacrwp("p2=5", pmacrtn);
        } else if(!strncmp(pmac_command,"start", 5)) {
	    if(p20status == 2) {
		strcpy(pmacrtn, "Error, already chopping\n");
	    } else {
        	pmacrwp("p2=7", pmacrtn);
        	sleep(2);
	    }
        } else if(!strncmp(pmac_command,"stow", 4)) {
	    StowChopper();
        } else if(!strncmp(pmac_command,"position", 8)) {
            int needsmove = 0;
            int newpos;

            rm_status = rm_read(RM_ANT_0, cmdPosMmv, cmdPosMm);
            for(i = 0; i < 3; i++) {
                if((newpos = cmdPosMm[i] * mm2cnt[i]) != cmdPos[i]) {
                    cmdPos[i] = newpos;
                    needsmove = 1;
                }
            }
            if(needsmove)
                MoveXyz(cmdPos);
            if((newpos = cmdPosMm[3] * mm2cnt[3] + TILT_HOME) != cmdPos[3]) {
                if(p20status != 2) {	/* Not Chopping */
                    MoveTilt(newpos);
                } else {
                    cmdPos[3] = newpos;
                }
            }
            WriteCmdPos('O');
        } else if(!strncmp(pmac_command,"init", 4)) {
            InitChopper();
	    MeasureDrift();
        } else if(!strncmp(pmac_command,"drift", 5)) {
	    MeasureDrift();
        } else if(!strncmp(pmac_command,"check", 5)) {
            CheckHome();
        } else if(!strncmp(pmac_command,"home", 4) ||
                  !strcmp(pmac_command, "P2=1") ||
                  !strcmp(pmac_command, "p2=1")) {
            HomeChopper();
        } else if(!strncmp(pmac_command,"restart", 7)) {
            RestartMotors();
        } else if(!strncmp(pmac_command,"download", 8)) {
            DownloadFile(&pmac_command[9]);
#if LISTCMD

        } else if(!strncmp(pmac_command,"list", 4)) {
            ListToFile(pmac_command);
#endif
            /* Must not allow "$$$***" as that erases PMAC's memory */
        } else if(!strncmp(pmac_command,"$$$", 3) ||
                  !strncmp(pmac_command, "reset", 5)) {
            ResetPMAC();
        } else if(!strncmp(pmac_command,"save", 4)) {
	    pmacw("save");
	    strcpy(pmacrtn, "Wait for PMAC to restart\n");
        } else if(pmac_command_flag == ICHOPPER_CMD) {
            CheckPmacCmd(pmac_command);
            pmacrwp(pmac_command, pmacrtn);
        } else {
            if(chopperMotionStatus==0) {
                CheckPmacCmd(pmac_command);
                pmacrwp(pmac_command, pmacrtn);
            } else if(chopperMotionStatus==1) {
                SendMessageToRM("Chopper busy. Command failed. ");
            } else if(chopperMotionStatus==-1) {
                SendMessageToRM("Chopper error. Command failed. ");
            }
        }
        updateTries  = 0;	/* allow more update replies after a cmd */

        if(pmac_command_flag == ICHOPPER_CMD) {
            if(rm_write_notify(RM_ANT_0,pmacResponsev,&pmacrtn) != RM_SUCCESS) {
                eprintf(PMAC_RESPONSE, "rm_write of pmacResponse failed\n");
            }
        }
        if(!cmdPosWritten) {
            WriteCmdPos('O');
        }
        sleep(1);
    } /* infinite while loop */
}

void *ChopperMonitor() {
    char response[64];
    short chopper_status, choppingFlag;
    int rm_status;
    int motor;
    int motorErrors;
    int it1, it2, n;
    float driverTemp;
    char *elevationv;

    setpriority(PRIO_PROCESS, (0), (MONITOR_PRIO));
    while(1) {
        if(stopMonitor)
            goto idle;
        /* get the chopper tilt, position and status */
        pmacrw("#1p", response);
        posMm[0] = (pos[0] = atof(response)) * cnt2mm[0];
        pmacrw("#2p", response);
        posMm[1] = (pos[1] = atof(response)) * cnt2mm[1];
        pmacrw("#3p", response);
        posMm[2] = (pos[2] = atof(response)) * cnt2mm[2];
        pmacrw("#4p", response);
        posMm[3] = ((pos[3] = atof(response)) - TILT_HOME) * cnt2mm[3];
        pmacrw("p0", response);
        p0status=atoi(response);
        pmacrw("p1", response);
        p1status=atoi(response);
        pmacrw("p2", response);
        p2status=atoi(response);
        pmacrw("p3", response);
        p3status=atoi(response);
        pmacrw("p20", response);
        p20status=atoi(response);
	pmacrw("M305M405", response);
/*
 * M305, M405 Temperatures at the voice coil drivers.  The sensors are AD590
 * which source 1E-6 Amp/Kelvin and work into a load of 20.19 KOhm.  This is
 * 0.02019Volt/K.  The PMAC A/D converter is a signed 16 bit converter read as
 * as an unsigned value.  So 32768 corresponds to 0Volts and 65536 to 10V.
 * Counts = 32768 + (T(deg C) + 273.2)*0.02019*3276.8
 * T(Deg. C) = (Counts-50840)/66.16
 */
	sscanf(response, "%d %d", &it1, &it2);
	if(abs(it1 - it2) > 330) {	/* if sensors differ by > 5 deg*/
	    n = 0;
	    if(it1 > 50178 && it1 < 52163) { /* if -10 < t < 20 */
		driverTemp = it1;
		n = 1;
	    }
	    if(it2 > 50178 && it2 < 52163) {
		driverTemp += it2;
		n++;
	    }
	    if(n) driverTemp /= n;
	    else driverTemp = (it1 + it1)/2;
	} else {
	    driverTemp = (it1 + it1)/2;
	}
	driverTemp = (driverTemp - 50840) / 66.16;

        if((p0status==0)&&(p3status==0))
            chopperMotionStatus=0;
        else if((p0status==3)||(p3status==3))
            chopperMotionStatus=-1;
        else if((p0status==4)||(p3status==1))
            chopperMotionStatus=1;

        /* pass the chopper variables to monitor programs through RM */

        rm_status=rm_read(RM_ANT_0, focusCurveFlagv, &focusCurveFlag);
	if(rm_status == RM_SUCCESS && focusCurveFlag &&
            rm_read(RM_ANT_0, eldrivestatev, &elDriveState) == RM_SUCCESS &&
	    elDriveState > 2) {
		elevationv = trackelv;
	} else {
		elevationv = encelv;
	}
	if(rm_status == RM_SUCCESS && focusCurveFlag &&
        	rm_read(RM_ANT_0, elevationv, &elevation) == RM_SUCCESS &&
		elevation > 10 && elevation < 90) {

            focusCurveMm[1] = cnt2mm[1] * (focusCurve[1] =
	       cosCoef[1] * (cos(elevation * (M_PI / 180.)) - M_SQRT1_2));
            focusCurveMm[2] = cnt2mm[2] * (focusCurve[2] =
	       sinCoef[2] * (sin(elevation * (M_PI / 180.)) - M_SQRT1_2));
	} else {
            focusCurve[1] = focusCurveMm[1] = 0;
            focusCurve[2] = focusCurveMm[2] = 0;
	}
	rm_status=rm_write(RM_ANT_0, focusCurveMmv, focusCurveMm);
	statusBits[UPDATE_STATUS] = focusCurveFlag;

        choppingFlag = (p20status == 2)? 1: 0;
        rm_status=rm_write(RM_ANT_0, choppingFlagv, &choppingFlag);

        if(abs(pos[3]-farPos[3]) > TILT_ERROR_COUNTS) {
            chopper_status=1;
        } else {
            chopper_status=0;
        }

        if(abs(pos[0]-farPos[0]-focusCurve[0]) > X_ERROR_COUNTS)
            chopper_status |= 8;
        if(abs(pos[1]-farPos[1]-focusCurve[1]) > Y_ERROR_COUNTS)
            chopper_status |= 4;
        if(abs(pos[2]-farPos[2]-focusCurve[2])> Z_ERROR_COUNTS)
            chopper_status |= 2;
#if 0
fprintf(stderr, "Y_ERROR_COUNTS %d, Y_UPDATE_COUNTS %d\n", Y_ERROR_COUNTS, Y_UPDATE_COUNTS);
fprintf(stderr, "pos %d, far %d, fc %d\n", pos[1], farPos[1], focusCurve[1]);
#endif
        statusBits[P0] = p0status;
        statusBits[P1] = p1status;
        statusBits[P2] = p2status;
        statusBits[P3] = p3status;
        statusBits[P20] = p20status;
        statusBits[POS_ERR_BITS] = chopper_status;

        motorErrors = 0;
        for(motor = 1; motor < 5; motor++) {
            static char motorStatusCmd[4] = "#1?";

            motorStatusCmd[1] = '0' + motor;
            statusBits[X_MOTOR_STATUS - 1 + motor] = 0 ;
            pmacrw(motorStatusCmd, response);
            if(2 & response[0])
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_POS_LIM ;
            if(4 & response[0])
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_NEG_LIM ;
            if(4 & response[1]) {
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_OL;
		if(motor == 4) motorErrors |= TILT_OPEN_LOOP;
            }
            if(motor != 4 && !(4 & response[9]))
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_NOT_HOME;
            if(!(1 & response[11])) {
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_NOT_IN_POS;
            }
            if(4 & response[11])
                statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_FFE ;
            if(motor != 4) {
                if(!(4 & response[9]))
                    statusBits[X_MOTOR_STATUS - 1 + motor] |= MOTOR_NOT_HOME;
                if(abs(pos[motor] - cmdPos[motor] - focusCurve[motor]) >
                        updateCounts[motor])
                    motorErrors |= XYZ_ERROR;
            }
        }
        /* If the command thread is idle, the chopper is homed and XYZ is ok
         * signal command thread if there are motor errors */
        if(pmac_command_flag == 0 && p0status == 0 && p3status == 0 &&
                focusCurveFlag >= 0) {
            if(updateCountdown <= 0) {
                if(motorErrors) {
                    if(updateTries < 10) {
                        if(updateCountdown-- < -2) {
                            updatesNeeded = motorErrors;
                            rm_release_read();
                            updateTries++;
                        }
                    } else {
                        if(updateTries == 10) {
                            fprintf(stderr, "Update failed after %d tries, "
                                    "no more updates\n", updateTries);
                            updateTries++;
                        }
                    }
                } else {
                    if(updateCountdown < 0)
                        updateCountdown++;
                    if(updateTries > 0 && updateTries < 10) {
                        updateTries--;
                    }
                }
            } else {
                updateCountdown--;
            }
        }

        rm_status=rm_write(RM_ANT_0,statusBitsv,&statusBits);
        if(rm_status != RM_SUCCESS) {
            eprintf(STATUS_BITS, "rm_write of status bits failed\n");
        }

        /*        rm_status=rm_write(RM_ANT_0,posv, pos); */
        rm_status=rm_write(RM_ANT_0,posMmv, posMm);

        rm_status=rm_read(RM_ANT_0, unixTimev, &unixTime);
        rm_status=rm_write(RM_ANT_0, chopperTemperaturev, &driverTemp);
        rm_status=rm_write(RM_ANT_0,monTimestampv,&unixTime);
	goto monitor_sleep;
idle:
        rm_status=rm_read(RM_ANT_0, unixTimev, &unixTime);
monitor_sleep:
        sleep(1);
    }
    pthread_detach(ChopperMonitorTID);
    pthread_exit((void *) 0);
}

void pmacrw(char *command, char *rdBuf) {
    int i;
    int nbytes;
    char inChar;
    char buffer[PMAC_WRITE_SIZE];
    int status;
    static char clearit[] = "\r";
    int trycnt;
    static int failures = 0;

    pthread_mutex_lock(&pmacMutex);
    trycnt = 0;
tryAgain:
    trycnt++;
    status=ioctl(pmacfd, IOCTL_FLUSH, &buffer);
    if(status)
        perror("Flushing serial line buffer");
    strcpy(buffer,command);
    strcat(buffer,"\r");
    if((status = write(pmacfd, buffer,strlen(buffer))) < 0) {
        if(status == -2) {
#if COMPLAIN
            fprintf(stderr, "Writing to PMAC failed: excessive waiting time\n");
#endif
        } else {
            perror("Writing to PMAC");
        }
	write(pmacfd, clearit, sizeof(clearit));
        if(trycnt < 3) {
            usleep(100000);
            goto tryAgain;
        } else {
	    fprintf(stderr, "chopperControl can not write to the PMAC,"
			" Quitting to restart\n");
	    exit(SYSERR_RTN);
        }
    }
    inChar=' ';
    i=0;
    for(;;) {
        nbytes=read(pmacfd,&inChar,1);
        if(nbytes < 0) {
#if COMPLAIN
            if(trycnt < 4) {
                sprintf(buffer, "Sent PMAC %s - response", command);
                perror(buffer);
            }
#endif
            write(pmacfd, clearit, sizeof(clearit));
            usleep(100000);
            if(trycnt < 2) {
		usleep(100000);
                goto tryAgain;
            } else {
                if(++failures > 19) {
                    fprintf(stderr, "No PMAC response after 20 trys!  "
			"Continuing to try\n");
		    failures = 0;
                }
		sleep(6);
                goto tryAgain;
            }
        }
        failures = 0;
        trycnt = 0;
        if(inChar == 6) {
            goto done;
        } else if(inChar==0x7) {
            int j;

            rdBuf[i++] = inChar;
            for(j = 0; j < 7; j++) {
                nbytes = read(pmacfd,&inChar,1);
                if(nbytes <= 0)
                    goto done;
                if(inChar == 0xd)
                    break;
                rdBuf[i++] = buffer[j] = inChar;
            }
            buffer[j] = 0;
            fprintf(stderr, "Cmd %s -> Error return from PMAC - %s\n",
                    command, buffer);
            break;
        } else {
	    if(inChar == '\r') inChar = '\n';
            if(listfp != NULL) {
                if(putc(inChar, listfp) == EOF) {
                    perror("Writing log file:");
                    listfp = 0;
                }
            }
            if(i < PMAC_RTN_SIZE - 8) {
                rdBuf[i]=inChar;
                i++;
            }
        }
    }
done:
    rdBuf[i]=0x0;
    pthread_mutex_unlock(&pmacMutex);
    return;
}

#if 1
void pmacw(char *command) {
    int status;
    char buffer[50];

    pthread_mutex_lock(&pmacMutex);
    status=ioctl(pmacfd,IOCTL_FLUSH,&buffer);
    if(status)
        fprintf(stderr,
                "chopper pmac buffer flushing status=%d\n",status);
    strcpy(buffer,command);
    strcat(buffer,"\r");
    write(pmacfd, buffer,strlen(buffer));
    sleep(20);
    pthread_mutex_unlock(&pmacMutex);
}
#endif

void ReadConfigFile(void) {
    FILE *fp;
    char line[80], *cp;
    int file;
    int rm_status;
    int i, axis, usecounts = 0;
    float value;
    static char fname[][40] = {
                                  "/application/configFiles/chopper.conf",
                                  "/instance/configFiles/chopper.conf"
                              };

    for(file = 0; file < 2; file++) {
        if((fp = fopen(fname[file], "r")) == NULL) {
            fprintf(stderr, "ReadConfigFile could not open %s\n", fname[file]);
            exit(QUIT_RTN);
        }
        while(fgets(line, sizeof(line), fp) != NULL) {
            for(cp = line; *cp; cp++) {
                if(*cp == '\n') {
                    *cp = 0;
                    break;
                }
            }
            if(*line == NULL || *line == '#')
                continue;
            for(cp = line; *cp != 0; cp++) {
                if(*cp == ' ') {
                    *cp++ = 0;
                    break;
                }
            }
            while(*cp == ' ')
                cp++;	/* skip to next word or end */
            if(strcmp(line, "PMAC") == 0) {
                pmacrwp(cp, pmacrtn);
            } else if(strcmp(line, "COUNTS") == 0) {
                usecounts = 1;
            } else {
                if((axis = *cp - 'x') < 0)
                    axis += ('x' - 't' + 3);
                if(sscanf(++cp, "%f", &value) != 1 || axis < 0 || axis > 3) {
                    fprintf(stderr, "Bad line %s in %s\n", line, fname[file]);
                } else {
                    if(! usecounts) {
                        value *= mm2cnt[axis];
                    }
                    if(axis == 3)
                        value += 32768;
                    if(strcmp(line, "FAR") == 0) {
                        farPos[axis] = floor(value + 0.5);
                    } else if(strcmp(line, "NEAR") == 0) {
                        nearPos[axis] = floor(value + 0.5);
                    } else if(strcmp(line, "COS_EL_COEF") == 0) {
                        cosCoef[axis] = value;
                    } else if(strcmp(line, "SIN_EL_COEF") == 0) {
                        sinCoef[axis] = value;
                    } else {
                        fprintf(stderr, "unrecognized line %s in %s\n",
                                line, fname[file]);
                    }
                }
            }
        }
        fclose(fp);
        for(i = 0; i < 4; i++) {
            farPosMm[i] = (farPos[i] - ((i == 3)? TILT_HOME: 0)) * cnt2mm[i];
        }
        rm_status=rm_write(RM_ANT_0,farPosMmv,farPosMm);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"writing CHOPPER_FAR_POS_MM");
        }
    }
#if 0
    for(i = 0; i < 4; i++) {
        fprintf(stderr, "%1d %6d %6d %10.3f %10.3f\n", i, farPos[i], nearPos[i], cosCoef[i], sinCoef[i]);
    }
#endif
}

int MoveXyz(int *newPos) {
    char pmaccommand[30];
    int i;

    sprintf(pmaccommand, "P4=%dP5=%dP6=%dP55=%dP56=%d",
	newPos[0] + focusCurve[0], newPos[1] + focusCurve[1],
	newPos[2] + focusCurve[2], newPos[1], newPos[2]);
    pmacrwp(pmaccommand, pmacrtn);
    for(i = 0; i < 3; i++) {
        cmdPos[i] = newPos[i];
    }
    sleep(1);		/* Let me see the command */
#if 0

    pmacrwp("P2=3", pmacrtn);
    sleep(2);		/* Wait for the monitor thread to get a new p3status */
    for(i = 0; i < 30; i++) {
        if(p3status == 0) { /* IN_POS_XYZ */
            return(0);
        } else if(p3status == 3) { /* POS_ERROR */
            SendMessageToRM("XYZ move error");
            return(1);
        }
        sleep(1);
    }
    SendMessageToRM("XYZ move timeout");
    return(1);
#endif

    return(UpdateXyz());
}

int UpdateXyz(void) {
    int i;

    pmacrwp("P2=3", pmacrtn);
    sleep(2);		/* Wait for the monitor thread to get a new p3status */
    for(i = 0; i < 60; i++) {
        if(p3status == 0) { /* IN_POS_XYZ */
            updateCountdown = 2;
            return(0);
        } else if(p3status == 3) { /* POS_ERROR */
            SendMessageToRM("XYZ move error");
            return(1);
        }
        sleep(1);
    }
    SendMessageToRM("XYZ move timeout");
    return(1);
}

int StowChopper(void) {
    int i;

    updateCountdown = 100000;
    pmacrwp("P2=2", pmacrtn);
    sleep(2);		/* Wait for the monitor thread to get a new p3status */
    for(i = 0; i < 60; i++) {
        if(p3status == 0) { /* IN_POS_XYZ */
            updateCountdown = 2;
	    pmacrwp("P3=5", pmacrtn);
            return(0);
        } else if(p3status == 3) { /* POS_ERROR */
            SendMessageToRM("XYZ move error in stow");
            return(1);
        }
        sleep(1);
    }
    SendMessageToRM("XYZ move timeout in stow");
    return(1);
}

void ReadCmdPos(void) {
    pmacrw("P4P55P56P21", pmacrtn);
    sscanf(pmacrtn, "%d %d %d %d", cmdPos, cmdPos + 1, cmdPos + 2,
           cmdPos + 3);
}

void ReadCaptureRegs(int * pos) {
    pmacrwp("M103M203M303", pmacrtn);
    sscanf(pmacrtn, "%d%d%d", pos, pos+1, pos+2);
}

void ReadCaptureFlags(int * flags) {
    pmacrwp("M117M217M317", pmacrtn);
    sscanf(pmacrtn, "%d%d%d", flags, flags+1, flags+2);
}

void ReadXyzEncoders(int * pos) {
    pmacrwp("M101M201M301", pmacrtn);
    sscanf(pmacrtn, "%d%d%d", pos, pos+1, pos+2);
}

int MoveTilt(int t) {
    char pmaccommand[30];
    int i;

    sprintf(pmaccommand,"p21=%d", t);
    pmacrwp(pmaccommand, pmacrtn);
    cmdPos[3] = t;
    sleep(1);		/* Let me see the command */
    pmacrwp("P2=5", pmacrtn);
    sleep(2);		/* Wait for the monitor thread to get a new p20status */
    for(i = 0; i < 30; i++) {
        if(p20status == 0) { /* IN_POS_XYZ */
            /*            pmacrwp("I425=$92C00C", pmacrtn); */
            return(0);
        } else if(p20status == 3) { /* POS_ERROR */
            SendMessageToRM("Tilt move error");
            return(1);
        }
        sleep(1);
    }
    SendMessageToRM("Tilt move timeout");
    return(1);
}

int CheckHome(void) {
    char messg[48];
    static int negPos[3] = {-500, -500, -500};
    static int posPos[3] = {500, 500, 500};
    int pos1[3], pos2[3];
    int i, err;

    if(p0status != 0) {
        SendMessageToRM("Chopper not homed, can not check home\n");
        return(1);
    }
    if(MoveXyz(negPos) != 0)
        return(1);
    WriteCmdPos('C');
    ReadCaptureRegs(pos1);		/* Reset Captured Flag */
    if(MoveXyz(posPos) != 0)
        return(1);
    WriteCmdPos('C');
    ReadCaptureFlags(pos1);
    if((pos1[0] & pos1[1] & pos1[2]) == 0) {
        strcpy(messg, "Failed to capture ");
        if(pos1[0] == 0)
            strcat(messg, "X");
        if(pos1[1] == 0)
            strcat(messg, "Y");
        if(pos1[2] == 0)
            strcat(messg, "Z");
        strcat(messg, " flags\n");
        SendMessageToRM(messg);
        sleep(3);
    }
    ReadCaptureRegs(pos1);
    ReadXyzEncoders(pos2);
    err = 0;
    for(i = 0; i < 3; i++) {
        pos1[i] += pos[i] - pos2[i];
        homeCheckResults[i] = (pos1[i] < -32767)? -32767:
                              ((pos1[i] > 32767)? 32767: pos1[i]);
        homeCheckResults[i] = pos1[i];
        if(abs(pos1[i]) > 3)
            err++;
    }
    sprintf(pmacrtn, "Home shifts are %d %d %d\n", pos1[0],
            pos1[1], pos1[2]);
    SendMessageToRM(pmacrtn);
    if(rm_write(RM_ANT_0, homeChkv, homeCheckResults) != RM_SUCCESS) {
        eprintf(HOME_CHECK_RESULTS, "rm_write of homeCheckResults failed\n");
    }
    if(rm_write(RM_ANT_0, homeChkTimev, &unixTime) != RM_SUCCESS) {
        eprintf(HOME_CHECK_TIME, "rm_write of homeChkTime failed\n");
    }
    return(err);
}

void ResetPMAC( void) {
    char resetcmd[] = "$$$\r";

    SendMessageToRM(resetcmd);
    stopMonitor = 1;
    usleep(500000);
    write(pmacfd, resetcmd, 4);
#if 0
    sleep(1);
    write(pmacfd, "\r", 1);
#endif
    sleep(7);			/* Wait until plc2 has started completely */
    chopperMotionStatus = 0;
    ReadConfigFile();
    stopMonitor = 0;
    pmacrtn[0] = 0;
    if(rm_write(RM_ANT_0, pmacResetTimev, &unixTime) != RM_SUCCESS) {
        eprintf(PMAC_RESET_TIME, "rm_write of PMAC reset time failed\n");
    }
}

void RestartMotors() {
    pmacrwp("P2=8", pmacrtn);
    sleep(3);
    pmacrwp("#4J/", pmacrtn);
    sleep(1);
    pmacrwp("P2=5", pmacrtn);
    pmacrwp("&1A", pmacrtn);
    pmacrwp("p3=0", pmacrtn);
}

void HomeChopper(void) {
    int i, pass;

    for(pass = 0; pass < 2; pass++) {
        pmacrwp("&1A", pmacrtn);
        sleep(1);
        pmacrwp("P2=1", pmacrtn);
        sleep(10);
        for(i = 0; i < 120; i++) {
            if(p0status!= 4)
                break;
            sleep(1);
        }
        sleep(2);
        for(i = 0; i < 10; i++) {
            if(p20status != 1)
                break;
            sleep(1);
        }
        if(p0status == 0)
            break;
    }
    /* Temporary code for antenna 6 */
    if(p0status == 0 && p3status == 4)
        pmacrwp("P3=0", pmacrtn);
    if(p0status == 0 && p20status != 0)
        MoveTilt(TILT_HOME);
    WriteCmdPos('H');
}

void InitChopper(void) {
    if(p0status != 0) {
        ResetPMAC();
        HomeChopper();
    } else {
        if(CheckHome()) {
            ResetPMAC();
            HomeChopper();
        }
        sleep(2);
    }
    if(p0status == 0) {
        ReadConfigFile();
        MoveXyz(farPos);
        MoveTilt(farPos[3]);
        WriteCmdPos('F');
    } else {
        return;
    }
    if(p3status != 0) {
        MoveXyz(farPos);
        WriteCmdPos('F');
    }
    if(p20status != 0) {
        MoveXyz(farPos);
        WriteCmdPos('F');
    }
}

void MeasureDrift(void) {
    int xbias, ybias, zbias;
    int startTime, startPos[3];
    int saveFlag;
    int i;
    short drifts[4];
    char cmd[48];

    saveFlag = pmac_command_flag;
    pmac_command_flag = HOLD_OFF_UPDATE;
    stopMonitor=1;
    usleep(20000);
    /* Read the DAC_BIAS2 values */
    pmacrwp("P40..42", pmacrtn);
    sscanf(pmacrtn, "%d %d %d", &xbias, &ybias, &zbias);
    /* Set the DAC_BIAS2s to servo on values */
    sprintf(cmd, "i179=%di279=%di379=%d", xbias, ybias, zbias);
    pmacrwp(cmd, pmacrtn);
    /* Save recent time and positions */
    for(i = 0; i < 3; i++) {
	startPos[i] = pos[i];
    }
    startTime = unixTime;
    /* activate the amplifiers and kill the servos, so each axis will drift */
    pmacrwp("m5..7=1#1k#2k#3k", pmacrtn);
    stopMonitor = 0;
    sleep(10);
    /* Activate the servos again at current position */
    pmacrwp("#1J/#2J/#3J/", pmacrtn);
    stopMonitor=1;
    usleep(20000);
    drifts[3] = unixTime - startTime;
    for(i = 0; i < 3; i++) {
	drifts[i] = pos[i] - startPos[i];
	if(drifts[i] < 100 || drifts[i] > 3000) {
	    static char msg[] = "  Axis bias should be adjusted\n";
	    static char axis[] = "XYZ";

	    msg[0] = axis[i];
	    fprintf(stderr, "MAIL=adiven@sma.hawaii.edu Chopper %s", msg);
/*	    sendOpMessage(OPMSG_WARNING, 19, 60, msg); */
	}
    }
    pmac_command_flag = saveFlag;
    stopMonitor = 0;
    rm_write(RM_ANT_0, "RM_CHOPPER_XYZ_DRIFT_V4_S", drifts);
    rm_write(RM_ANT_0, "RM_CHOPPER_DRIFT_TIMESTAMP_L", &unixTime);
    UpdateXyz();
    sprintf(pmacrtn, "%d %d %d %d\n", drifts[0], drifts[1], drifts[2],
	    drifts[3]);
}

#if LISTCMD
/* List the gather buffer or anything else the PMAC's list command lists to
 * the user and a file.  Only the first 128 chars are transferred to the user.
 * The whole command is sent to the PMAC after a file in the instance/acc/n/
 * chopperPMAC directory is created or truncated for writing.  The file name
 * is the second word of the command.  Thus "list gather" will create and
 * write in a file named gather.  "list plc2" in file plc2, but "list plc 2
 * would create a file named plc.
 */
void ListToFile(char *cmd) {
    char *cp, *lp, lbuf[PMAC_WRITE_SIZE];
    stopMonitor = 1;
#if 1
    /* Put the directory name in lbuf and find the end */
    strcpy(lbuf, instDir);
    for(lp = lbuf; *lp; lp++) 
	;
    /* find the word following "list" and add to the directory name */
    for(cp = cmd + 4; *cp == ' ' || *cp == '\t'; cp++)
        ;
    while(*cp && *cp != ' ' && *cp != '\t') {
	*lp++ = *cp++;
    }
    *lp = 0;
    /* If listfp is not NULL, pmacrw will put everything received from the
     * PMAC in it. */
    if((listfp = fopen(lbuf, "w")) == NULL) {
        sprintf(pmacrtn, "Could not create or open for writing %s", lbuf);
        stopMonitor = 0;
        return;
    }
#endif
    /* Finally send the PMAC the command and pmacrw will write the result
     * into the file. */
    pmacrw(cmd, pmacrtn);
#if 1
    fclose(listfp);
#endif
    listfp = NULL;
    stopMonitor = 0;
}
#endif

void DownloadFile(char *fn) {
    FILE *fp;
    int c, inComment, line;
    char *cp, lbuf[PMAC_WRITE_SIZE];

    stopMonitor = 1;
    while(*fn == ' ' || *fn == '\t')
        fn++;
    strcpy(lbuf, instDir);
    strcat(lbuf, fn);
    if((fp = fopen(lbuf, "r")) == NULL) {
        sprintf(pmacrtn, "chopperControl could not open %s", lbuf);
        stopMonitor = 0;
        return;
    }
    for(c = 0, line = 0; c!= EOF; line++) {
        inComment = 0;
        for(cp = lbuf; (c = getc(fp)) != '\n' && c != EOF; ) {
            if(c == ';')
                inComment = 1;
            if(inComment)
                continue;
            if(c != ' ' && c != '\t')
                *cp++ = c;
            if(cp >= &lbuf[PMAC_WRITE_SIZE - 2]) {
                pmacrw("close", pmacrtn);
                sprintf(pmacrtn, "Line %d too long downloading %s\n", line, fn);
                fclose(fp);
                stopMonitor = 0;
                return;
            }
        }
        if(cp > lbuf) {	/* We have a line with content for the PMAC */
            *cp = 0;
            pmacrw(lbuf, pmacrtn);
            if(pmacrtn[0] == 0x7) {
                pmacrw("close", lbuf);
                sprintf(lbuf, " while downloading line %d of %s\n", line, fn);
                strcat(pmacrtn, lbuf);
                fclose(fp);
                stopMonitor = 0;
                return;
            }
        }
    }
    fclose(fp);
    stopMonitor = 0;
}

void WriteCmdPos(char type) {
    int i, rm_status;

    cmdPosType[0] = type;
    for(i = 0; i < 4; i++) {
        cmdPosMm[i] = (cmdPos[i] - ((i == 3)? TILT_HOME: 0)) * cnt2mm[i];
    }
    rm_status = rm_write(RM_ANT_0, cmdPosMmv, cmdPosMm);
    /*    rm_status = rm_write(RM_ANT_0, cmdPosv, cmdPos); */
    rm_status = rm_write(RM_ANT_0, cmdPosTypev, &cmdPosType);
    cmdPosWritten = 1;
}

/* Check through a Pmac command string for strings matching
 * [pP][4-6] *= *-?[0-9]+ which would be a change in command position
 * where 4 is x, 5 y, and 6 is z.
 * If found, set that axis in cmdPos to the new value minus the focusCurve
 *  for that axis.
 */
void CheckPmacCmd(char *cp) {
    int state, c, axis, value;
    char *cp2;

    for(state = 0; (c = *cp++); ) {
Restart:
        switch(state) {
        case 0:
            if(c == 'p' || c == 'P')
                state = 1;
            break;
        case 1:
            if(c > '3' && c < '7') {
                state = 2;
                axis = c - '4';
            } else {
                state = 0;
                /* This char immediately followed a 'p', so is not a candidate
                 * for starting the next match */
                break;
            }
            break;
        case 2:
            if(c == ' ')
                break;
            if(c != '=') {
                state = 0;
                goto Restart;	/* This char might start a new sequence */
            }
            value = strtol(cp, &cp2, 10);
            if(cp2 == cp || value > 150000 || value < -150000)
                break;
            cmdPos[axis] = value - focusCurve[axis];
            cp = cp2;
        }
    }
}

void SendMessageToRM(char *messg) {
    int rm_status;
    char message[100];

    strncpy(message, messg, sizeof(message));
    rm_status=rm_write(RM_ANT_0,"RM_TRACK_MESSAGE_C100",message);
#if 0

    fprintf(stderr, "chopperControl: wrote message %s "
            "rm_status = %d\n", message, rm_status);
#endif

}

#if 0
void SigAlrmHndlr(int signo) {
    int buffer;

    if(ioctl(pmacfd,IOCTL_READ_ABORT,&buffer) < 0)
        perror("Aborting read");
    fprintf(stderr, "Alarm waiting for PMAC response\n");
}
#endif

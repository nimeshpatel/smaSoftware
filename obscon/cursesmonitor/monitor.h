#define DEWAR_TEMP_4K_STAGE 2
#define NUMBER_OF_UPSs_ON_OBSCON 10
//#define NUMBER_OF_UPSs_ON_COLOSSUS 4
#define WACKO_OFFSET 129600 /* 36 degrees */
#define SYNCDET_CHANNELS_WACKO_LOW -10   /* uW */
#define SYNCDET_CHANNELS_WACKO_HIGH 100  /* uW */
#define WACKO_PROJECT_ID_MAX 10000000
 #define TIME_DIFFERENTIAL_STANDOUT 0.1 /* seconds */
#define SUN_DISTANCE_LIMIT 40.0 
#define NUMANTS 8
#define MIN_ANTENNA_PAD 1
#define MAX_ANTENNA_PAD 24
#define NUMBER_OF_RECEIVERS 8
#define MAX_NUMBER_ANTENNAS 10
#define LO_BOARD_UNKNOWN_TYPE -1
enum {LO_BOARD_A1_TYPE = 0, LO_BOARD_A2_TYPE, LO_BOARD_B1_TYPE,
        LO_BOARD_B2_TYPE, LO_BOARD_C_TYPE, LO_BOARD_D_TYPE,
        LO_BOARD_E_TYPE, LO_BOARD_F_TYPE, LO_BOARD_OFF};

/* monitor.c */
extern char *getLoBoardTypeStringBrief(int type);
extern int translateBandNumberToGHz(int band);
extern float computeYearFraction(int day, int month, int year);
extern int oldDate(char *string, int rightnow);
extern int computeMonthFrom3CharString(char *month);
;
extern float yigTuneCurveA;
extern float yigTuneCurveB;
extern float c1dcTargetIF1[MAX_NUMBER_ANTENNAS+1];
extern float c1dcTargetIF2[MAX_NUMBER_ANTENNAS+1];
extern float power200MHzTarget[MAX_NUMBER_ANTENNAS+1];
extern float power200MHzTarget2[MAX_NUMBER_ANTENNAS+1];
extern float cont2det2target[MAX_NUMBER_ANTENNAS+1];
extern float cont1det2target[MAX_NUMBER_ANTENNAS+1];
extern float mrgRfPowerTarget[MAX_NUMBER_ANTENNAS+1];
extern int deadAntennas[MAX_NUMBER_ANTENNAS+1];
extern int antsAvailable[MAX_NUMBER_ANTENNAS+1];
extern int ignoreHeatedLoad[MAX_NUMBER_ANTENNAS+1];
extern int numberAntennas; /* 8 for SMA, 10 for eSMA (wide screen option) */
enum {IFLO_UNITS_VOLTS = 0, IFLO_UNITS_MILLIWATT, IFLO_UNITS_DBM, IFLO_UNITS_LAST};

/* arrayMonitor.c */
extern int parseReceiverName(char *dummyString1);
extern void initialize();
extern int call_dsm_read(char *machine, char *variable, void *ptr, time_t *tstamp);
#ifndef _DSM_H
#include "dsm.h"
#endif
extern int call_dsm_structure_get_element(dsm_structure *ds, char *name, void *ptr);
extern double sunDistance(double az1,double el1,double az2,double el2);
#define CAL_VANE_STALE 10 /* seconds */
#define MINIMUM_SCREEN_WIDTH_FOR_ESMA 100
#define MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE 120
extern int pageOffset;
extern int searchMode;
extern char searchString[100];
enum {UR_MESSAGES=0, UR_SMASHLOG, UR_CORRELATOR, UR_STDERR, UR_LAST};
extern int upperRightWindow;

#define CORR_MONITOR_LOG "/rootfs/logs/corr_monitor_log.txt"
#define SMASH_LOG "/rootfs/logs/SMAshLog"
#define STDERR_LOG "/rootfs/logs/stderr"
#define MESSAGES_LOG "/rootfs/logs/messages"
extern int checkAsiaaIdleTime(void);
extern int checkObsconIdleTime(void);
extern void printObsconhpIdleTime(int idleMinutes);
extern int checkObsconhIdleTime(void);
extern int checkObsconhpIdleTime(void);
extern int checkObsconcIdleTime(void);
extern int checkIdleTime(char *filename, int ostype);
enum {OBSCON_INFO_STALE=-2, OBSCON_NO_ONE_LOGGED_ON};
extern void printAsiaaIdleTime(int idleMinutes);
extern void printObsconhIdleTime(int idleMinutes);
extern void printObsconIdleTime(int idleMinutes);
extern void printObsconcIdleTime(int idleMinutes);
extern void printIdleTime(int idleMinutes);
int present(char *a, char *b);

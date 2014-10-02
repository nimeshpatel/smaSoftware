#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "s_cmd2.h"
#include "tsshm.h"
#include "dsm.h"
#include "monitor.h"
#include "gps8d.h"

#define CHECKDSMERRORS 0

enum disp_lines{status_line = 4, pos_avg_line = 6, phase_freq_line = 8,
	phase_freq_line2 = 10, lat_lon_line = 12, tt_lat_lon_line = 14,
	leap_line = 16,
	sat_line = 18, el_line, az_line, snr_line};

extern char *toddtime(time_t *, char *); /* located in dewarpage.c */
extern char *hsttime(time_t *, char *); /* located in dewarpage.c */
#ifndef LINUX
extern int mvprintw    _AP((int, int, const char *fmt, ...));
#endif
static char colossus[] = "colossus";
static char pos_avg_mode_str[][16] = {"not averaging", "averaging",
	"Known position"};
static char positioning_mode_str[][12] = {"interrupted", "2-D only", "3-D"};
static int badMsgsShown = 0;
static dsm_structure rpt;

void gpsPage(int count) {
  time_t system_time;
  char timeString[32]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  time_t timeStamp, prevTimeStamp = 0;
  int i, col;

  unsigned char dummyByte, dummyByte2;
  short dummyShort, dummyShort2;
  long dummyLong;
  float dummyFloat, dummyFloat2;
  double dummyDouble, dummyDouble2;

#if CHECKDSMERRORS
  int dsmErrors;
#endif /* CHECKDSMERRORS */

  if(rpt.size == 0) {
      if((i = dsm_structure_init(&rpt, "GPSD_REPORT_X")) != DSM_SUCCESS) {
	dsm_error_message(i, "Initializing Dsm structure failed");
	exit(1);
      }
  }
  if ((count % 20) == 0) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
     move(1,1);
    refresh();
  }
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("%s UT, %s HT",timeString,timeString2);

  move(status_line, 0);
  call_dsm_read(colossus, "GPSD_REPORT_X", &rpt, &timeStamp);
  if(abs(timeStamp - system_time) > 10) {
    printw("Data from gps8d is stale\n");
    goto DONE;
  }
  if(timeStamp == prevTimeStamp) goto DONE;
  prevTimeStamp = timeStamp;

  call_dsm_structure_get_element(&rpt, "GPS8_STATUS_B", &dummyByte);
  if(dummyByte & 8) {
    printw("Valid Time ");
  } else {
    standout();
    printw("Invalid Time ");
    standend();
  }
  if(dummyByte & 4) {
    standout();
    printw("GPS Not Locked ");
    standend();
  }
  if(dummyByte & 2) {
    standout();
    printw("Local Time ");
    standend();
  }
  if(dummyByte & 1) {
    printw("Leap Year ");
  }
  call_dsm_structure_get_element(&rpt, "GPS8_OUT_STATUS_B", &dummyByte);
  if(dummyByte != 0) {
    standout();
    printw("Bad output 0x%x", dummyByte);
    standend();
  }
  call_dsm_structure_get_element(&rpt, "GPS8_CTL_STATUS_B", &dummyByte);
  if(dummyByte != 0) {
    standout();
    printw("Bad Control status 0x%x", dummyByte);
    standend();
  }
  printw("\n");

  move(pos_avg_line, 0);
  call_dsm_structure_get_element(&rpt, "GPS8_POS_AVG_MODE_B", &dummyByte);
  call_dsm_structure_get_element(&rpt, "GPS8_NUM_POS_SAMPLES_B", &dummyByte2);
  if(dummyByte > 3)
	dummyByte = 0;
  printw("Position mode: %s, %d samples", pos_avg_mode_str[dummyByte],
	  dummyByte2);
  call_dsm_structure_get_element(&rpt, "GPS8_OP_MODE_B", &dummyByte);
  if(dummyByte != 1) printw(", 2-D Only");
  call_dsm_structure_get_element(&rpt, "GPS8_POS_STATUS_B", &dummyByte);
  if(dummyByte > 3)
	dummyByte = 0;
  printw(", %s\n", positioning_mode_str[dummyByte - 1]);

  call_dsm_structure_get_element(&rpt, "GPS8_AVG_PHASE_F", &dummyFloat);
  call_dsm_structure_get_element(&rpt, "GPS8_FREQ_CTLR_VALUE_F", &dummyFloat2);
  move(phase_freq_line, 0);
  printw("Avg Phase %.0fns, Freq ctl value %.0f, status ",
	dummyFloat, dummyFloat2);
  call_dsm_structure_get_element(&rpt, "GPS8_PHASE_FREQ_STATUS_B", &dummyByte);
  if(dummyByte == 1)
    printw("good");
  else
    printw("0x%02x", dummyByte);
  call_dsm_structure_get_element(&rpt, "GPS8_PLL_CONSTRAINT_B", &dummyByte);
  printw(", PLL Constraint ");
  if(dummyByte == 8)
    printw("good\n");
  else
    printw("0x02x\n", dummyByte);
  move(phase_freq_line2, 0);
  call_dsm_structure_get_element(&rpt, "GPS8_LAST_FREQ_CORR_TIME_L", &dummyLong);
  toddtime((time_t *)&dummyLong, timeString);
  call_dsm_structure_get_element(&rpt, "GPS8_LAST_FREQ_CORR_F", &dummyFloat);
  call_dsm_structure_get_element(&rpt, "GPS8_FREQ_TREND_F", &dummyFloat2);
  printw("Last Freq corr %.1e at %s Trend %.1e\n", dummyFloat, timeString, dummyFloat2);

  call_dsm_structure_get_element(&rpt, "GPS8_UNIX_TIME_D", &dummyDouble);
  dummyLong = dummyDouble;
  dummyDouble -= dummyLong;
  toddtime((time_t *)&dummyLong, timeString);
  bcopy(timeString + 19, timeString + 23, 6); 
  sprintf(timeString + 19, ".%03d", (int)(0.5 + dummyDouble * 1000.));
  timeString[23] = ' ';
/*  timeString[28] = '\0'; */
  move(2,0);
  call_dsm_structure_get_element(&rpt, "GPS8_GPS_MINUS_UTC_B", &dummyByte);
  printw("Last UT read from GPS8 %s\n",
	  timeString);

  move(3,0);
  call_dsm_structure_get_element(&rpt, "TT_PHASE_LOCKED_S", &dummyShort);
  if(dummyShort != 1) {
    printw("TrueTime phase lock = %d\n", dummyShort);
  } else {
    call_dsm_structure_get_element(&rpt, "TT_UNIX_TIME_D", &dummyDouble);
    dummyLong = dummyDouble;
    dummyDouble -= dummyLong;
    toddtime((time_t *)&dummyLong, timeString);
    bcopy(timeString + 19, timeString + 23, 6); 
    sprintf(timeString + 19, ".%03d", (int)(0.5 + dummyDouble * 1000.));
    timeString[23] = ' ';
/*    timeString[28] = '\0'; */
    printw("              TrueTime %s\n", timeString);
  }

  move(leap_line, 0);
  call_dsm_structure_get_element(&rpt, "GPS8_NEXT_LEAP_SECOND_L", &dummyLong);
  toddtime((time_t *)&dummyLong, timeString);
  printw("Next Leap sec %s\n", timeString);

  call_dsm_structure_get_element(&rpt, "GPS8_LAT_D", &dummyDouble);
  call_dsm_structure_get_element(&rpt, "GPS8_LON_D", &dummyDouble2);
  call_dsm_structure_get_element(&rpt, "GPS8_ALT_F", &dummyFloat);
  call_dsm_structure_get_element(&rpt, "GPS8_DOP_F", &dummyFloat2);
  call_dsm_structure_get_element(&rpt, "GPS8_GPS_MINUS_UTC_B", &dummyByte);
  move(lat_lon_line, 0);
  printw("GPS8 Lat %.6f   Long %.6f  Alt %.1f   DOP %.2f  GPS-UTC %d sec.\n",
	dummyDouble, dummyDouble2, dummyFloat, dummyFloat2, dummyByte);

  call_dsm_structure_get_element(&rpt, "TT_LAT_D", &dummyDouble);
  call_dsm_structure_get_element(&rpt, "TT_LON_D", &dummyDouble2);
  call_dsm_structure_get_element(&rpt, "TT_ALT_F", &dummyFloat);
  call_dsm_structure_get_element(&rpt, "TT_NUM_SATS_S", &dummyShort);
  call_dsm_structure_get_element(&rpt, "TT_SELF_TEST_S", &dummyShort2);
  move(tt_lat_lon_line, 0);
  printw("TT   Lat %.6f   Long %.6f  Alt %.1f    %d sats   Self Test %s\n",
	dummyDouble, dummyDouble2, dummyFloat, dummyShort,
	(dummyShort2 == 0)? "OK": "BAD");

  call_dsm_structure_get_element(&rpt, "GPS8_SATS_V12_V4_B", sats);
  move(sat_line, 0);
  printw("Satellite #");
  move(el_line, 0);
  printw("Elevation  ");
  move(az_line, 0);
  printw("Azimuth    ");
  move(snr_line, 0);
  printw("SNR        ");
  col = 12;
  for(i = 0; i < 12; i++) {
    if(sats[i][NUM] > 0) {
	move(sat_line, col);
	printw("%3d ", sats[i][NUM]);
	move(el_line, col);
	printw("%3d ", sats[i][EL]);
	move(az_line, col);
	printw("%3d ", 2*sats[i][AZ_OVER_TWO]);
	move(snr_line, col);
	printw("%3d ", sats[i][SNR]);
	col += 5;
    } else {
	move(sat_line, col);
	printw("\n");
	move(el_line, col);
	printw("\n");
	move(az_line, col);
	printw("\n");
	move(snr_line, col);
	printw("\n");
	break;
    }
  }
  move(23,20);
  call_dsm_structure_get_element(&rpt, "GPS8_BAD_MSGS_B", &dummyByte);
  if(dummyByte != 0) {
    standout();
    printw("%d messages from gps8 with errors\n", dummyByte);
    standend();
    badMsgsShown = 1;
  } else if(badMsgsShown) {
    badMsgsShown = 0;
    printw("\n");
  }
DONE:
  move(23,0);
  refresh();
}

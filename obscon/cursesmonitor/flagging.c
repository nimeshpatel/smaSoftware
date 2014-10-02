#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "monitor.h"
#include "upspage.h"
#include "coherence.h"
#include "dDSCursesMonitor.h" 
#include "scanFlags.h"
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 1

extern int quit;
extern int verbose;

void flagging(int count)
{
  static int firstCall = TRUE;
  long dSMScanFlags[11][2];
  int ant, s;

  int doWeCare[11];
  char string1[100];
  time_t timestamp, curTime;

  if ((count % 60) == 1) {
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
  getAntennaList(doWeCare);
  s = dsm_read("hal9000",
	       "DSM_SCAN_FLAGS_V11_V2_L", 
	       (char *)dSMScanFlags,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_SCAN_FLAGS_V11_V2_L");
    exit(-1);
  }
  strcpy(string1, asctime(gmtime(&timestamp)));
  string1[strlen(string1)-1] = (char)0;
  move(0,0);
  printw("          Flag values for the scan stored at %s", string1);
  curTime = time((long *)0);
  strcpy(string1, asctime(gmtime(&curTime)));
  string1[strlen(string1)-1] = (char)0;
   move (1,32);
  printw("current time %s", string1);
  move(2,0);
  printw("   Condition         1     2     3     4     5     6     7     8     9     10");
  move(3,0); printw("1  Ave. Tracking   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_AVE_TRACKING) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_AVE_TRACKING) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(4,0); printw("2  Bad Samples     ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_BAD_SAMPLES) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_BAD_SAMPLES) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(5,0); printw("3  Cal. Vane       ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_CAL_VANE) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_CAL_VANE) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(6,0); printw("4  Chopper Pos.    ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_CHOPPER_POS) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_CHOPPER_POS) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(7,0); printw("5  Coord Mismatch  ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_COORD_MISMATCH) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_COORD_MISMATCH) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(8,0); printw("6  Dewar Warm      ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_DEWAR_WARM) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_DEWAR_WARM) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(9,0); printw("7  Drives Off      ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_DRIVES_OFF) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_DRIVES_OFF) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(10,0); printw("8  Feed Mismatch   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_FEED_MISMATCH) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_FEED_MISMATCH) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(11,0); printw("9  IRIG Time       ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_IRIG_TIME) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_IRIG_TIME) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(12,0); printw("10 M3 Door Closed  ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_M3_CLOSED) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_M3_CLOSED) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(13,0); printw("11 Optical Point.  ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_OPTICAL) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_OPTICAL) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(14,0); printw("12 Peak Tracking   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_PEAK_TRACKING) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_PEAK_TRACKING) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(15,0); printw("13 PLL Unlocked    ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_UNLOCKED_PLL) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_UNLOCKED_PLL) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(16,0); printw("14 Shadowed Ant.   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_SHADOWING) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_SHADOWING) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(17,0); printw("15 Source Change   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_SOURCE_CHANGE) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_SOURCE_CHANGE) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(18,0); printw("16 Source Mismatch ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_SOURCE_MISMATCH) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_SOURCE_MISMATCH) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(19,0); printw("17 Track Stale     ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_TRACK_STALE) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_TRACK_STALE) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(20,0); printw("18 Wacky Offsets   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_WACKY_OFFSETS) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_WACKY_OFFSETS) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(21,0); printw("19 BDA Conf/Power  ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else if ((dSMScanFlags[ant][0] & SFLAG_DRO_UNLOCKED) == 0)
      printw("disab ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_DRO_UNLOCKED) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(22,0); printw("   Operator Flag   ");
  for (ant = 1; ant <= 10; ant++)
    if (!doWeCare[ant])
      printw("----- ");
    else {
      if (dSMScanFlags[ant][1] & SFLAG_OPERATOR) {
	standout(); printw("BAD"); standend(); printw("   ");
      } else
	printw("good  ");
    }
  move(0,79);
  refresh();
  return;
}

#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "monitor.h"
#include "10MHz_Select.h"
#define PRINT_DSM_ERRORS 0

extern dsm_structure mRGControl, mRGStatus;

void mRG(int count)
{
  short locked[2], sb[2], comb[2], nTweeks[2], short1, short2[2], short4[4];
  int s, rx, i;
  float tuningVoltage[2];
  float iFCntrlVoltage[2];
  float quadVoltage[2];
  float stressVoltage[2];
  float levelVoltage[2];
  double reqFreq[2];
  double offsetFreq[2];
  char string1[100];
  time_t timestamp, curTime, lastCommand[2], lastTweek[2];
  char source;

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
  move(0,20);
  s = dsm_read("m5",
	       "MRG_CONTROL_X", 
	       (char *)&mRGControl,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - MRG_CONTROL_X");
  }
  s = dsm_read("m5",
	       "MRG_STATUS_X", 
	       (char *)&mRGStatus,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - MRG_STATUS_X");
  }
  curTime = time(NULL);
  strcpy(string1, asctime(gmtime(&curTime)));
  string1[24] = (char)0;
  printw("MRG status on %s UT", string1);
  move(1,0);
  printw("                      Low Freq Rx           High Freq Rx");  move(2,0);
  printw("Requested Frequency   ");
  s = dsm_structure_get_element(&mRGControl,
				"REQ_FREQ_V2_D",
				(char *)&reqFreq);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - REQ_FREQ_V2_D");
  }
   for (rx = 0; rx < 2; rx++) {
    if (reqFreq[rx] > 10.0e+9 || reqFreq[rx] < 5.0e+9)
      printw("wacko          ");
    else
      printw("%11.9f GHz", reqFreq[rx]*1.0e-9);
    printw("       ");
  }
  move(5,0);
  printw("Lock Status           ");
  s = dsm_structure_get_element(&mRGControl,
				"YIG_LOCKED_V2_S", 
				(char *)&locked);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - YIG_LOCKED_V2_S");
  }
  for (rx = 0; rx < 2; rx++) {
    if (locked[rx]) {
      printw("locked  ");
    } else {
      standout();
      printw("UNLOCKED");
      standend();
    }
    printw("              ");
  }
  move(4,0);
  printw("Tuning voltage      ");
  s = dsm_structure_get_element(&mRGControl,
				"CNTRL_VOLTAGE_V2_F", 
				(char *)&tuningVoltage);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - CNTRL_VOLTAGE_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(tuningVoltage[rx]) > 100)
      printw("wacko   ");
    else
      printw("%8.4f", tuningVoltage[rx]);
    printw("              ");
  }
  move(3,0);
  printw("Estimated Frequency ");
  for (rx = 0; rx < 2; rx++) {
    double estFreq = (tuningVoltage[rx]+yigTuneCurveB)/yigTuneCurveA;
    if (estFreq < 0 || estFreq > 100) {
      printw("wacko");
    } else {
      printw("%8.4f",estFreq);
    }
    printw("              ");
  }
  move(6,0);
  printw("Level Cntrl Voltage ");
  s = dsm_structure_get_element(&mRGControl,
				"LEVEL_CNTRL_V2_F", 
				(char *)&iFCntrlVoltage);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(iFCntrlVoltage[rx]) > 100)
      printw("wacko   ");
    else
      printw("%8.4f", iFCntrlVoltage[rx]);
    printw("              ");
  }
  move(7,0);
  printw("PLL Quad voltage    ");
  s = dsm_structure_get_element(&mRGControl,
				"QUAD_V2_F", 
				(char *)&quadVoltage);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(quadVoltage[rx]) > 100)
      printw("wacko   ");
    else
      printw("%8.4f", quadVoltage[rx]);
    printw("              ");
  }
  move(8,0);
  printw("PLL Stress voltage  ");
  s = dsm_structure_get_element(&mRGControl,
				"STRESS_V2_F", 
				(char *)&stressVoltage);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(stressVoltage[rx]) > 100)
      printw("wacko   ");
    else
      printw("%8.4f", stressVoltage[rx]);
    printw("              ");
  }
  move(9,0);
  printw("YIG Power Testpoint ");
  s = dsm_structure_get_element(&mRGControl,
				"LEVEL_V2_F", 
				(char *)&levelVoltage);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(levelVoltage[rx]) > 100)
      printw("wacko   ");
    else
      printw("%8.4f", levelVoltage[rx]);
    printw("              ");
  }
  move(10,0);
  printw("PLL Lock Sideband     ");
  s = dsm_structure_get_element(&mRGControl,
				"SB_V2_S", 
				(char *)&sb);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (sb[rx] < 1)
      printw("lower");
    else
      printw("upper");
    printw("                 ");
  }
  move(11,0);
  printw("Comb setting          ");
  s = dsm_structure_get_element(&mRGControl,
				"COMB_V2_S", 
				(char *)&comb);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LEVEL_CNTRL_V2_F");
  }
  for (rx = 0; rx < 2; rx++) {
    if (comb[rx] > 10 || comb[rx] < 0)
      printw("wacko");
    else
      printw("%d GHz", comb[rx]);
    printw("                 ");
  }
  move(12,0);
  printw("Offset Frequency     ");
  s = dsm_structure_get_element(&mRGControl,
				"OFFSET_SYNTH_V2_D", 
				(char *)&offsetFreq);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - YIG_LOCKED_V2_S");
  }
  for (rx = 0; rx < 2; rx++) {
    if (fabs(offsetFreq[rx]) > 10.0e+9)
      printw("wacko          ");
    else
      printw("%11.6f MHz", offsetFreq[rx]*1.0e-6);
    printw("       ");
  }
  move(13,0);
  printw("Last commanded at     ");
  s = dsm_structure_get_element(&mRGControl,
				"CMD_TIME_V2_L", 
				(char *)&lastCommand);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - YIG_LOCKED_V2_S");
  }
  for (rx = 0; rx < 2; rx++) {
    strncpy(string1, ctime(&lastCommand[rx])+4, 15);
    string1[15] = (char)0;
    printw("%s", string1);
    printw("       ");
  }
  move(14,0);
  printw("Last tweeked at       ");
  s = dsm_structure_get_element(&mRGControl,
				"LAST_TWEEK_V2_L", 
				(char *)&lastTweek);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - YIG_LOCKED_V2_S");
  }
  for (rx = 0; rx < 2; rx++) {
    if (lastTweek[rx] != 0) {
      strncpy(string1, ctime(&lastTweek[rx])+4, 15);
      string1[15] = (char)0;
      printw("%s", string1);
    } else
      printw("Never tweeked  ");
    printw("       ");
  }
  move(15,0);
  printw("Tweeks since lock     ");
  s = dsm_structure_get_element(&mRGControl,
				"N_TWEEKS_V2_S", 
				(char *)&nTweeks);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - YIG_LOCKED_V2_S");
  }
  for (rx = 0; rx < 2; rx++) {
    printw("%2d", nTweeks[rx]);
    printw("                    ");
  }
  move(17,0);
  s = dsm_read("m5","DSM_AS_10MHZ_SOURCE_B",&source,&timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_read - DSM_AS_10MHZ_SOURCE_B");
    }
  }
  printw("10MHz Status bits:");
  printw("    reference source = ");
  switch (source) {
  case SOURCE_10MHZ_GPS:
    printw("GPS");
    break;
  case SOURCE_10MHZ_MASER:
    printw("maser");
    break;
  case SOURCE_10MHZ_FREERUN:
    printw("free-running");
    break;
  default:
    standout();
    printw("wacko");
    standend();
    break;
  }
  printw(" (last update = ");
  printAgeNoStandoutNoTrailingSpace(curTime,timestamp);
  printw(")\n");

  move(18,0);
  printw("HPs: ");
  s = dsm_structure_get_element(&mRGStatus,
				"10MHZ_TO_OFFSETS_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 10MHZ_TO_OFFSETS_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   To 2nd LOs: ");
  s = dsm_structure_get_element(&mRGStatus,
				"10MHZ_TO_2ND_LO_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 10MHZ_TO_2ND_LO_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   To 1st LOs: ");
  s = dsm_structure_get_element(&mRGStatus,
				"10MHZ_TO_1ST_LO_V2_S", 
				(char *)&short2);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 10MHZ_TO_1ST_LO_V2_S");
  }
  if (short2[0] == 1)
    printw(" OK,");
  else {
    standout();printw("BAD,");standend();
  }
  if (short2[1] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   2nd LO Synth: ");
  s = dsm_structure_get_element(&mRGStatus,
				"10MHZ_TO_2ND_LO_SYNTH_V4_S", 
				(char *)&short4);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 10MHZ_TO_2ND_LO_SYNTH_V4_S");
  }
  for (i = 0; i < 3; i++)
    if (short4[i] == 1)
      printw(" OK,");
    else {
      standout();printw("BAD,");standend();
    }
  if (short4[3] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  move(19,0);
  printw("4-Way Dist: ");
  s = dsm_structure_get_element(&mRGStatus,
				"10MHZ_4WAY_V4_S", 
				(char *)&short4);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_structure_get_element - 10MHZ_4WAY_V4_S");
    }
  }
  for (i = 0; i < 3; i++)
    if (short4[i] == 1) {
      printw(" OK,");
    } else {
      standout();printw("BAD,");standend();
    }
  if (short4[3] == 1) {
    printw(" OK");
  } else {
    standout();printw("BAD");standend();
  }
  s = dsm_read("colossus","DSM_MASER_STATUS_B",&source,&timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_read - DSM_MASER_STATUS_B");
    }
  }
  printw("    Maser AC power = ");
  if ((curTime-timestamp) > 60) {
    standout();
    printw("stale");
    standend();
  } else {
    switch (source) {
    case 0:
      /*
	standout();
      */
      printw("off");
      standend();
      break;
    case 1:
      printw("on");
      break;
    default:
      printw("wacko");
      break;
    }
  }
  move(20,0);
  printw("1st LO Ref: (Lower) ");
  s = dsm_structure_get_element(&mRGStatus,
				"1ST_LO_REF_V4_S", 
				(char *)&short4);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 1ST_LO_REF_V4_S");
  }
  if (short4[0] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw(" (upper) ");
  if (short4[1] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw(" (VXI Crate lower) ");
  if (short4[2] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw(" (VXI Crate upper) ");
  if (short4[3] == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  move(21,0);
  printw("Other status bits:");
  move(22,0);
  printw("Ortel Power");
  s = dsm_structure_get_element(&mRGStatus,
				"LOW_POWER_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LOW_POWER_S");
  }
  if (short1 == 1) { /* changed to 1 by Todd on 24Feb2005 */
    printw(" OK");
  } else {
    standout();printw("BAD");standend();
  }
  printw("   Ortel Alarm");
  s = dsm_structure_get_element(&mRGStatus,
				"FO_XMITER_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - FO_XMITER_S");
  }
  if (short1 == 1) { /* changed to 1 by Todd on 24Feb2005 */
    printw(" OK");
  } else {
    standout();printw("BAD");standend();
  }
  printw("   200 MHz ");
   s = dsm_structure_get_element(&mRGStatus,
				 "200MHZ_PRESENT_S", 
				 (char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 200MHZ_PRESENT_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   1 GHz ");
  s = dsm_structure_get_element(&mRGStatus,
				"1GHZ_PRESENT_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 1GHZ_PRESENT_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   52 MHz PLL ");
  s = dsm_structure_get_element(&mRGStatus,
				"52MHZ_LOCK_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 52MHZ_LOCK_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  move(23,0);
  printw("100 MHz PLL Lock ");
   s = dsm_structure_get_element(&mRGStatus,
				 "100MHZ_PLL_LOCK_S", 
				 (char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 100MHZ_PLL_LOCK_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   Power Before PLL ");
   s = dsm_structure_get_element(&mRGStatus,
				 "100MHZ_PRESENT_PREPLL_S", 
				 (char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 100MHZ_PRESENT_PREPLL_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  printw("   Power After PLL ");
  s = dsm_structure_get_element(&mRGStatus,
				"100MHZ_PRESENT_POSTPLL_S", 
				(char *)&short1);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - 100MHZ_PRESENT_POSTPLL_S");
  }
  if (short1 == 1)
    printw(" OK");
  else {
    standout();printw("BAD");standend();
  }
  move(0,0);
  refresh();
  return;
}

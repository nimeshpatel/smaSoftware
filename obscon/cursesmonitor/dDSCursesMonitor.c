#define DEBUG 0
#include <curses.h>
#include <math.h>
#ifdef LINUX
#include <bits/nan.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "monitor.h"
#include "astrophys.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0
#include "dDSCursesMonitor.h"

extern dsm_structure dDSStatusStructure;

float lookupBaselineLengthByAntenna(int i, int j);
float baselineLengthByAntenna(int i, int j, double x[N_ANTENNAS+1], 
			      double y[N_ANTENNAS+1], double z[N_ANTENNAS+1],
			      double u[N_ANTENNAS+1], double v[N_ANTENNAS+1],
			      double frequency);
void computeBaselineLengths(double x[N_ANTENNAS+1],
			    double y[N_ANTENNAS+1],
                            double z[N_ANTENNAS+1],
			    double u[N_ANTENNAS+1],
                            double v[N_ANTENNAS+1],
			    double frequency);
static float baselineLengths[N_ANTENNAS+1][N_ANTENNAS+1];
typedef struct {
  int a;
  int b;
  float length;
} BLINE;
static BLINE sortedBaseline[101];
char antChar(int i);

extern int baselineInfo;
int quit = FALSE;

int localVerbose = FALSE;
int ddprintf(const char *format, ...) {
  extern int localVerbose;
  va_list ap;
  int s=0;

  if (localVerbose) {
    va_start(ap, format);
    s = vfprintf(stderr, format, ap);
    va_end(ap);
    fflush(stderr);
  }

  return(s);
}

void rad2HHMMSS(double angle, int doSigned, int decimalPlaces, char *result)
{
  int hH, mM, sign, turns;
  int pointer = 0;
  double sS, hours, residue, reducedAngle;
  char formatString[20];

  turns = (int)(angle / (2.0 * M_PI));
  reducedAngle = angle - ((double)turns * 2.0 * M_PI);
  if (reducedAngle < 0.0) {
    sign = -1;
    reducedAngle *= -1.0;
  } else
    sign = 1;
  hours = reducedAngle * 12.0 / M_PI;
  hH = (int)hours;
  residue = hours - (double)hH;
  mM = (int)(residue*60.0);
  residue = residue - ((double)mM)/60.0;
  sS = residue * 3600.0;
  if (doSigned) {
    if (sign < 0)
      result[pointer] = '-';
    else
      result[pointer] = '+';
    pointer++;
  }
  sprintf(formatString,"%%02d:%%02d:%%0%d.%df",
	  decimalPlaces+3, decimalPlaces);
  if (fabs(hH) > 99 || fabs(mM) > 99 || fabs(sS) > 99) {
    sprintf(&result[pointer]," wacko        ");
  } else {
    sprintf(&result[pointer],formatString, hH, mM, sS);
  }
}

void rad2DDMMSS(double angle, int doSigned, int ninetyMax,
		int decimalPlaces, char *result)
{
  int dD, mM, sign, turns;
  int pointer = 0;
  double sS, degrees, residue, reducedAngle;
  char formatString[20];

  if (ninetyMax) {
    turns = (int)(angle / (0.5 * M_PI));
    reducedAngle = angle - ((double)turns * 0.5 * M_PI);
  } else {
    turns = (int)(angle / (2.0 * M_PI));
    reducedAngle = angle - ((double)turns * 2.0 * M_PI);
  }
  if (reducedAngle < 0.0) {
    sign = -1;
    reducedAngle *= -1.0;
  } else
    sign = 1;
  degrees = reducedAngle * 180.0 / M_PI;
  dD = (int)degrees;
  residue = degrees - (double)dD;
  mM = (int)(residue*60.0);
  residue = residue - ((double)mM)/60.0;
  sS = residue * 3600.0;
  if (doSigned) {
    if (sign < 0)
      result[pointer] = '-';
    else
      result[pointer] = '+';
    pointer++;
  }
  if (ninetyMax)
    sprintf(formatString,"%%02d:%%02d:%%0%d.%df",
	    decimalPlaces+3, decimalPlaces);
  else
    sprintf(formatString,"%%03d:%%02d:%%0%d.%df",
	    decimalPlaces+3, decimalPlaces);
  if (fabs(dD) > 359 || fabs(mM) > 99 || fabs(sS) > 99) {
    sprintf(&result[pointer]," wacko        ");
  } else {
    sprintf(&result[pointer],formatString, dD, mM, sS);
  }
}

void dDSDisplay(int count, int rx)
{
  int isAntennaInArray[20];
  int startline;
  int s,k,a,b, i,j,ctr,cr;
  double dayFraction, hA, lST, rA, dec, uT1MUTC, refLong, refLat, refRadius;
  short gunn[N_RECEIVERS+1], realWeather;
  short walshMod[N_ANTENNAS+1][N_RECEIVERS], walshDemod[N_ANTENNAS+1][N_RECEIVERS];
  double fringeRate[N_RECEIVERS+1][N_ANTENNAS+1];
  double lastDelay[N_ANTENNAS+1], atmCorr[N_ANTENNAS+1][N_RECEIVERS];
  double pDry, pH2O, tAtm, N;
  double trackingFrequency[N_RECEIVERS+1];
  double u[N_ANTENNAS+1], v[N_ANTENNAS+1], w[N_ANTENNAS+1];
  double x[N_ANTENNAS+1], y[N_ANTENNAS+1], z[N_ANTENNAS+1];
  char string1[100], string2[100], string3[100], string4[100];
  time_t timestamp;

  if (DEBUG) {
    /*    printw("rx=%d\n",rx);*/
    refresh();
  }
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
  s = dsm_read("newdds",
	       "DDS_TO_HAL_X", 
	       &dDSStatusStructure,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS || 1)
      dsm_error_message(s, "dsm_read - DDS_TO_HAL_X");
    quit = TRUE;
    return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"HA_D", 
				(char *)&hA);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - HA_D");
    quit = TRUE;
    return;
  }
  if (!timestamp) {
    move(12,9);
    printw(
	   "DDS status has not been written - DDS software is not running.");
    move(23,79);
    refresh();
    return;
  }
  move(0,0);
  if (rx == 0) {
    printw(
	   "--------------------------- Low Frequency Receiver --- ( +/- to change Rx ) ----");
  } else if (rx == 1) {
    printw(
	   "--------------------------- High Frequency Receiver --- ( +/- to change Rx ) ---");
  } else {
    printw("        strange receiver number = %d ",rx);
  }
  if (DEBUG) {
    refresh();
  }
  move(1,0);
  if (DEBUG) {
    printw("rx=%d\n",rx);
    refresh();
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"UT1MUTC_D", 
				(char *)&uT1MUTC);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LST_D");
    quit = TRUE;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"DAYFRAC_D", 
				(char *)&dayFraction);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - DAYFRAC_D");
    quit = 1; 
    return;
  }
  strcpy(string1, asctime(gmtime(&timestamp)));
  string1[strlen(string1)-1] = (char)0;
  printw("| DDS Status on %s  Day Frac: %8.6f  UT1-UTC: %6.3f  |",
	 string1, dayFraction, uT1MUTC);
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"FREQ_V3_D", 
				(char *)&trackingFrequency);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - DSM_DDS_FREQ_V3_D");
    quit = 1; return;
  }
    s = dsm_structure_get_element(&dDSStatusStructure,
				  "GUNN_V3_S", 
				  (char *)&gunn);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "dsm_structure_get_element - GUNN_V3_S");
      quit = 1; return;
    }
  if (DEBUG) refresh();
  move(2,0);
  if ((trackingFrequency[2] < 150.0e9) || 
      (trackingFrequency[2] > 1000.0e9))
    trackingFrequency[2] = 0.0;
  printw("| LO%d Freq: %10.6f Gunn%d Mult: %d     delta earth %10.0f Hz             |",
	 1+rx,
	 trackingFrequency[1+rx]*1.0e-9, 
	 1+rx,
	 gunn[1+rx],
	 trackingFrequency[0]);
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"REFLONG_D", 
				(char *)&refLong);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - REFLONG_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"REFLAT_D", 
				(char *)&refLat);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - REFLAT_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"REFRAD_D", 
				(char *)&refRadius);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - REFRAD_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(3,0);
  rad2DDMMSS(refLong, 0, 0, 2, string1);
  rad2DDMMSS(refLat, 0, 0, 2, string2);
  printw("| ARP\tLong: %s\tLat: %s\tRadius: %8.3f (km)  |",
	 string1, string2, refRadius*1.0e-3);
  if (DEBUG) refresh();
 
  s = dsm_structure_get_element(&dDSStatusStructure,
				"RA_D", 
				(char *)&rA);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - RA_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"DEC_D", 
				(char *)&dec);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - DEC_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"LST_D", 
				(char *)&lST);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - LST_D");
    quit = 1; return;
  }
  rad2HHMMSS(lST, 0, 2, string1);
  rad2HHMMSS(hA, 1, 2, string2);
  rad2HHMMSS(rA, 0, 2, string3);
  rad2DDMMSS(dec, 1, 1, 1, string4);
  if (DEBUG) refresh();
  move(4,0);
  printw("| LST: %s  HA: %s  RA: %s  Dec: ",
	 string1, string2, string3);
  if (strlen(string4) > 12) {
    standout();
    printw("wacko");
    standend();
    printw("      ");
  } else {
    printw("%s",string4);
  }
  printw("        |");
  if (DEBUG) refresh();
  move(5,0);
  s = dsm_structure_get_element(&dDSStatusStructure,
				"P_DRY_D", 
				(char *)&pDry);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - P_DRY_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"P_WATER_D", 
				(char *)&pH2O);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - P_WATER_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"T_ATM_D", 
				(char *)&tAtm);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - T_ATM_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"N_D", 
				(char *)&N);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - N_D");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"REAL_WEATHER_S", 
				(char *)&realWeather);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - REAL_WEATHER_S");
    quit = 1; return;
  }
  printw("| Real Weather: %d   pDry: %5.1f   pH2O: %5.2f   tAtm: %5.1f   N: %f",
	 realWeather, pDry, pH2O, tAtm, N);
  move(5,79);
  printw("|");
  move(6,0);
  printw(
	 "--------------------------------------------------------------------------------");
  if (DEBUG) refresh();
  move(7,0);
  printw("Antenna      1");
  for (i=2; i<=numberAntennas; i++) {
    switch (i) {
    case CSO_ANTENNA_NUMBER:
      printw("      CSO");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("     JCMT");
      break;
    default:
      printw("        %d",i);
    }
  }
  if (DEBUG) refresh();
  move(8,0);
  s = dsm_structure_get_element(&dDSStatusStructure,
				"FR_V3_V11_D", 
				(char *)&fringeRate);
  if (DEBUG) {
    /*    printw("read fringeRate ");*/
    refresh();
  }
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_structure_get_element - FR_V3_V11_D");
    }
    quit = 1;
    return;
  }
  for (i = 1; i <= N_ANTENNAS; i++) {
    if (rx < 0 || rx > 1) {
      rx = 0;
    }
    if (DEBUG) {
      /*      printw("i=%d,rx=%d ",i,rx);*/
      refresh();
    }
    if (fringeRate[1+rx][i] > 10000.0) {
#ifdef LINUX
      fringeRate[1+rx][i] = NAN;
#else
      fringeRate[1+rx][i] = _NAN;
#endif
    }
  }
  printw("Fringe  ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++)
    if (antsAvailable[i])
      if ((fringeRate[1+rx][i] == fringeRate[1+rx][i]) &&
	  (fringeRate[1+rx][i] > -1000.0) &&
	  (fringeRate[1+rx][i] < 1000.0))
	printw("%8.5f ", fringeRate[1+rx][i]*(double)gunn[1+rx]);
      else
	printw("  wacko  ");
    else
      printw("   ----- ");
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"DEL_V11_D", 
				(char *)&lastDelay);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_structure_get_element - DEL_V11_D");
    }
    quit = 1;
    return;
  }
  move(9, 0);
  printw("Delay   ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++) {
    if (antsAvailable[i]) {
      if (fabs(lastDelay[i])>1.0e9) {
	printw("  wacko  ");
      } else {
	printw("%8.2f ", lastDelay[i]*1.0e9);
      }
    } else {
      printw("   ----- ");
    }
  }
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"ATM_V11_V2_D", 
				(char *)&atmCorr);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - ATM_V11_D");
    quit = 1; return;
  }
  for (i = 0; i <= N_ANTENNAS; i++) {
    if (i != 10)
      if (atmCorr[i][rx] >= 0.0) {
	atmCorr[i][rx] = -0.0000001;
      }
  }
  if (DEBUG) refresh();
  move(10, 0);
  printw("Atmos.  ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++)
    if (antsAvailable[i])
      printw("%8.3f ", -atmCorr[i][rx]*180.0/M_PI);
    else
      printw("   ----- ");
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"U_V11_D", 
				(char *)&u);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - U_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(11, 0);
  printw("u       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++)
    if (antsAvailable[i])
      printw("%8.3f ", u[i]);
    else
      printw("   ----- ");
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"V_V11_D", 
				(char *)&v);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - V_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(12, 0);
  printw("v       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++)
    if (antsAvailable[i])
      printw("%8.3f ", v[i]);
    else
      printw("   ----- ");
   if (DEBUG) refresh();
 s = dsm_structure_get_element(&dDSStatusStructure,
			       "W_V11_D", 
			       (char *)&w);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - W_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(13, 0);
  printw("w       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++)
    if (antsAvailable[i])
      printw("%8.3f ", w[i]);
    else
      printw("   ----- ");
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"X_V11_D", 
				(char *)&x);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - X_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(14, 0);
  printw("X       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++) {
    if (antsAvailable[i]) {
      if (fabs(x[i]) >= 10000) {
	printw("  wacko  ");
      } else {
	printw("%8.3f ", x[i]);
      }
    } else {
      printw("   ----- ");
    }
  }
  if (DEBUG) refresh();
  s = dsm_structure_get_element(&dDSStatusStructure,
				"Y_V11_D", 
				(char *)&y);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - Y_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(15, 0);
  printw("Y       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++) {
    if (antsAvailable[i]) {
      if (fabs(y[i]) >= 10000) {
	printw("  wacko  ");
      } else {
	printw("%8.3f ", y[i]);
      }
    } else {
      printw("   ----- ");
    }
    if (DEBUG) refresh();
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"Z_V11_D", 
				(char *)&z);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - Z_V11_D");
    quit = 1; return;
  }
  if (DEBUG) refresh();
  move(16, 0);
  printw("Z       ");
  if (DEBUG) refresh();
  for (i = 1; i <= numberAntennas; i++) {
    if (antsAvailable[i]) {
      if (fabs(z[i]) >= 10000) {
	printw("  wacko  ");
      } else {
	printw("%8.3f ", z[i]);
      }
    } else {
      printw("   ----- ");
    }
  }
  move(17,0);
  s = dsm_structure_get_element(&dDSStatusStructure,
				"WALSH_MOD_V11_V2_S", 
				(char *)&walshMod);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - WALSH_MOD_V11_V2_S");
    quit = 1; return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
				"WALSH_DEMOD_V11_V2_S", 
				(char *)&walshDemod);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - WALSH_DEMOD_V11_V2_S");
    quit = 1; return;
  }
  printw("Walsh M/D");
  for (i = 1; i <= numberAntennas; i++) {
    if (antsAvailable[i]) {
      if ((walshMod[i][rx] < 0) || (walshMod[i][rx] > 16)) {
	printw("wck");
      } else {
	if ((walshMod[i][rx] == 0) || (walshMod[i][rx] != walshDemod[i][rx]))
	  standout();
	printw("%3d", walshMod[i][rx]);
      }
      standend();
      printw("/");
      if ((walshDemod[i][rx] < 0) || (walshDemod[i][rx] > 16)) {
	printw("wck");
      } else {
	if ((walshDemod[i][rx] == 0) || (walshMod[i][rx] != walshDemod[i][rx]))
	  standout();
	printw("%3d", walshDemod[i][rx]);
      }
      standend();
      printw("  ");
    } else {
      printw("  -----  ");
    }
  }
  if (DEBUG) refresh();

  /* Todd has hijacked the rest of this page for devious mischief */

  computeBaselineLengths(x,y,z,u,v,trackingFrequency[1+rx]);
  startline = 18;
  move(startline,0);
  switch (baselineInfo) {
  case UNPROJECTED:
    printw("Unprojected baselines sorted by antenna number (highlighted = in the project): ");
    break;
  case UNPROJECTED_SORTED_BY_LENGTH:
    printw("Unprojected baselines sorted by length (highlighted = in the project):         ");
    break;
  case PROJECTED_SORTED_BY_LENGTH:
    printw("Projected baselines sorted by length (highlighted = in the project):           ");
    break;
  case PROJECTED:
    printw("Projected baselines sorted by antenna number (highlighted = in the project):   ");
    break;
  case MAS:
    printw("Resolution in milliarcsec sorted by antenna number (highlighted = in project): ");
    break;
  case MAS_SORTED_BY_LENGTH:
  default:
    printw("Resolution in milliarcsec sorted by beamsize (highlighted = in the project):   ");
  }
  move(++startline,0);
  ctr = 0;
  getAntennaList(isAntennaInArray);
  cr = 0;
  for (i=1; i<numberAntennas; i++) {
    for (j=i+1; j<=numberAntennas; j++) {
      switch (baselineInfo) {
      case PROJECTED:
      case UNPROJECTED:
      case MAS:
	if (antsAvailable[i] && antsAvailable[j]) {
	  switch (baselineInfo) {
	  case PROJECTED:
	  case UNPROJECTED:
	    if (isAntennaInArray[i] && isAntennaInArray[j]) {
	      standout();
	    }
	    printw("%c-%c =%3.0fm",antChar(i),antChar(j),
		   lookupBaselineLengthByAntenna(i,j));
	    break;
	  case MAS:
	    if (isAntennaInArray[i] && isAntennaInArray[j]) {
	      standout();
	    }
	    printw("%c-%c %5.0f",antChar(i),antChar(j),
		   lookupBaselineLengthByAntenna(i,j));
	    break;
	  } /* end switch */
	  standend();
	  printw("  ");
	  if (++ctr % (numberAntennas-1) == 0) {
	    cr++;
	    move(++startline,0);
	    /*	    printw("\n");*/
	  }
	} /* end if */
	break;
      case PROJECTED_SORTED_BY_LENGTH:
      case UNPROJECTED_SORTED_BY_LENGTH:
      case MAS_SORTED_BY_LENGTH:
	k = i*numberAntennas+j;
	a = sortedBaseline[k].a;
	b = sortedBaseline[k].b;
	if (antsAvailable[a] && antsAvailable[b]) {
	  switch (baselineInfo) {
	  case PROJECTED_SORTED_BY_LENGTH:
	  case UNPROJECTED_SORTED_BY_LENGTH:
	    if (isAntennaInArray[a] && isAntennaInArray[b]) {
	      standout();
	    }
	    printw("%c-%c =%3.0fm",antChar(a),antChar(b),
		   sortedBaseline[k].length);
	    break;
	  case MAS_SORTED_BY_LENGTH:
	    if (isAntennaInArray[a] && isAntennaInArray[b]) {
	      standout();
	    }
	    printw("%c-%c %5.0f",antChar(a),antChar(b),
		   sortedBaseline[k].length);
	    break;
	  }
	  standend();
	  printw("  ");
	  if (++ctr % (numberAntennas-1) == 0) {
	    cr++;
	    move(++startline,0);
	    /*	    printw("\n");*/
	  }
	} /* end if */
	break;
      } /* end switch() */  
    } /* end of 'for j' */
  } /* end of 'for i' */
  clrtoeol();
  if (cr < 5) {
    /* if there are only 9 antennas in the array (i.e. with RM running), then
     * we still have room to show the help line at the bottom */
    move(23,0);
    switch (baselineInfo) {
    case UNPROJECTED_SORTED_BY_LENGTH:
      printw("Type p for projected baselines, \" for resolution in mas, n to sort by number");
      break;
    case PROJECTED_SORTED_BY_LENGTH:
      printw("Type u for unprojected baselines, \" for resolution in mas, n to sort by number");
      break;
    case MAS_SORTED_BY_LENGTH:
      printw("Type u for unprojected baselines, p for projected baselines, n to sort by number");
      break;
    case UNPROJECTED:
      printw("Type p for projected baselines, \" for resolution in mas, l to sort by length");
      break;
    case PROJECTED:
      printw("Type u for unprojected baselines, \" for resolution in mas, l to sort by length");
      break;
    case MAS:
    default:
      printw("Type u for unprojected baselines, p for projected baselines, l to sort by length");
    }
  }
  refresh();
}

char antChar(int i) {
  switch (i) {
  case CSO_ANTENNA_NUMBER:
    return('C');
  case JCMT_ANTENNA_NUMBER:
    return('J');
  default:
    return(i-1+'1');
  }
}

float lookupBaselineLengthByAntenna(int i, int j) {
  return(baselineLengths[i][j]);
}

float baselineLengthByAntenna(int i, int j, double x[N_ANTENNAS+1], 
			      double y[N_ANTENNAS+1], double z[N_ANTENNAS+1],
			      double u[N_ANTENNAS+1], double v[N_ANTENNAS+1],
			      double frequency) {
  float length;
  float lambda;

  switch (baselineInfo) {
  case UNPROJECTED:
  case UNPROJECTED_SORTED_BY_LENGTH:
    length = pow(pow(x[i]-x[j],2)+pow(y[i]-y[j],2)+pow(z[i]-z[j],2),0.5);
    break;
  case PROJECTED:
  case PROJECTED_SORTED_BY_LENGTH:
    length = pow(pow(u[i]-u[j],2)+pow(v[i]-v[j],2),0.5);
    break;
  case MAS:
  case MAS_SORTED_BY_LENGTH:
  default:
    if (frequency != 0.0) {
      length = pow(pow(u[i]-u[j],2)+pow(v[i]-v[j],2),0.5);
      lambda = SPEED_OF_LIGHT / frequency;
      length = lambda / length;
      length = (length * 180.0 / M_PI) * 3600000.0;
    } else
      length = 0.0;
  }
  return(length);
}

void computeBaselineLengths(double x[N_ANTENNAS+1],
			    double y[N_ANTENNAS+1], 
                            double z[N_ANTENNAS+1],
			    double u[N_ANTENNAS+1], 
                            double v[N_ANTENNAS+1],
			    double frequency) {
  int n,i,j,k,nextk;
  int swap;
  float fswap;

  for (i=1; i<=numberAntennas; i++) {
    for (j=i+1; j<=numberAntennas; j++) {
      baselineLengths[i][j] = baselineLengthByAntenna(i,j,x,y,z,u,v,frequency);
    }
  }
  /* sort them */
  for (i=1; i<=(numberAntennas-1); i++) {
    for (j=i+1; j<=numberAntennas; j++) {
      k = i*numberAntennas+j;
      sortedBaseline[k].a = i;
      sortedBaseline[k].b = j;
      sortedBaseline[k].length = baselineLengths[i][j];
    }
  }
  for (n=1; n<(numberAntennas*(numberAntennas-1))/2; n++) {
    for (i=1; i<numberAntennas; i++) {
      for (j=i+1; j<=numberAntennas; j++) {
	if (i==(numberAntennas-1) && j==numberAntennas) continue;
	k = i*numberAntennas+j;
	if (j==numberAntennas) {
	  nextk = (i+1)*numberAntennas+(i+2);
	} else {
	  nextk = i*numberAntennas+j+1;
	}
	if (sortedBaseline[k].length > sortedBaseline[nextk].length) {
	  fswap = sortedBaseline[k].length;
	  sortedBaseline[k].length = sortedBaseline[nextk].length;
	  sortedBaseline[nextk].length = fswap;
	  swap = sortedBaseline[k].a;
	  sortedBaseline[k].a = sortedBaseline[nextk].a;
	  sortedBaseline[nextk].a = swap;
	  swap = sortedBaseline[k].b;
	  sortedBaseline[k].b = sortedBaseline[nextk].b;
	  sortedBaseline[nextk].b = swap;
	}
      }
    }
  }
}

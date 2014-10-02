#define NO_PRINT 0
#define YES_PRINT 1
#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "rm.h"
#include "s_cmd2.h"
#include "monitor.h"
#include "upspage.h"
#include "dsm.h"

#define D2R (M_PI/180.0)

extern void checkForNulls(char *string);
void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign);
double sunDistance(double az1,double el1,double az2,double el2);

void hangarPage(int count)
{
  long hangarLight;
  short dec_dum_sign, dec_cat_sign;
  unsigned int projectID;
  int rms, dsm_status, ra_cat_h, ra_cat_m, dec_cat_d, dec_cat_m, ant;
  int lowestAntenna, antennaInArray[11];
  int antCheckedOut[11] = {0,0,0,0,0,0,0,0,0,0,0};
  float dummyFloat, epoch, dummyFloat16[16];
  double ra_cat_disp, dec_cat_disp, ra_cat_s, dec_cat_s, sun_az, sun_el;
  char string1[100];
  char projectPI[30], observer[30], source[34];
  char description[256], operatingLocation[256];
  char antUsers[11][80], scheme[11][256];
  time_t timestamp, curTime, lastCommand[2], lastTweek[2], unixTime;
  FILE *antennaUser;

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
  lowestAntenna = getAntennaList(antennaInArray);
  antennaUser = fopen("/global/projects/antennaUser", "r");
  if (antennaUser != NULL) {
    int nTokens;
    char antUser[80];
    
    while ((nTokens = fscanf(antennaUser, "%d %s", &ant, antUser)) == 2) {
      if ((ant > 0) && (ant < 9)) {
	int ii;

	antCheckedOut[ant] = 1;
	strcpy(antUsers[ant], antUser);
	ii = 0;
	while ((antUser[0] != '\n') && (!feof(antennaUser))) {
	  antUser[0] = scheme[ant][ii++] = (char)getc(antennaUser);
	}
	scheme[ant][ii] = (char)0;
      }
    }
     fclose(antennaUser);
  }
  move(0,13);
  curTime = time(NULL);
  strcpy(string1, asctime(gmtime(&curTime)));
  string1[24] = (char)0;
  printw("Antenna Status Summary at %s UT", string1);
  move(1,0);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_ID_L",&projectID,&timestamp);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_DESCRIPTION_C256", description,&timestamp);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_PI_C30",projectPI,&timestamp);
  /*
  dsm_status = dsm_read("m5", "DSM_AS_PROJECT_ID_L", &projectID, &timestamp);
  dsm_status = dsm_read("m5", "DSM_AS_PROJECT_DESCRIPTION_C256",
		         description, &timestamp);
  */
  checkForNulls(description);
  description[60] = (char)0;
  printw("Project: ");
  if (projectID < 0 || projectID > WACKO_PROJECT_ID_MAX) {
    printw("wacko");
  } else {
    printw("%d ", projectID);
  }
  printw(" - %s", description);
  move(2,0);
  /*
  dsm_status|=dsm_read("m5", "DSM_AS_PROJECT_PI_C30", projectPI, &timestamp);
  checkForNulls(projectPI);
  dsm_status|=dsm_read("m5", "DSM_AS_PROJECT_OBSERVER_C30",
		       observer, &timestamp);
  dsm_status|=dsm_read("m5", "DSM_AS_PROJECT_OPERATINGLOCATION_C256",
		       operatingLocation, &timestamp);
  */
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_OBSERVER_C30",observer,&timestamp);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_OPERATINGLOCATION_C256",operatingLocation,&timestamp);
  checkForNulls(operatingLocation);
  checkForNulls(observer);
  printw("Observer: %s  Project PI: %s  Location: %s",
	 observer, projectPI, operatingLocation);
  if (lowestAntenna > 0) {
    char dec_sign_char;
    int i;
    double lst_disp, ha, ra_disp;

    move(3,0);
    rms = rm_read(lowestAntenna,"RM_SOURCE_C34",source);
    rms=rm_read(lowestAntenna,"RM_EPOCH_F",&epoch);
    rms=rm_read(lowestAntenna,"RM_RA_CAT_HOURS_F",&dummyFloat);
    ra_cat_disp=(double)dummyFloat;
    rms=rm_read(lowestAntenna,"RM_DEC_CAT_DEG_F",&dummyFloat);
    dec_cat_disp=(double)dummyFloat;
    hms(&ra_cat_disp,&ra_cat_h,&ra_cat_m,&ra_cat_s,&dec_dum_sign);
    hms(&dec_cat_disp,&dec_cat_d,&dec_cat_m,&dec_cat_s,&dec_cat_sign);
    dec_cat_d = abs(dec_cat_d);
    rms=rm_read(lowestAntenna, "RM_RA_APP_HR_D", &ra_disp);
    rms=rm_read(lowestAntenna, "RM_LST_HOURS_F", &dummyFloat);
    lst_disp=(double)dummyFloat;
    ha = lst_disp - ra_disp;
    i = strlen(source) - 1;
    while ((i > 0) && (source[i] == ' '))
      source[i--] = (char)0;
    if (dec_cat_sign < 0)
      dec_sign_char = '-';
    else
      dec_sign_char = '+';
    printw("Source: %s  RA(%4.0f) %02d:%02d:%6.3f  Dec(%4.0f) %c%02d:%02d:%5.2f  HA: %6.3f",
	   source, epoch, ra_cat_h, ra_cat_m, ra_cat_s,
	   epoch, dec_sign_char, dec_cat_d, dec_cat_m, dec_cat_s, ha);
    rms = rm_read(lowestAntenna, "RM_UNIX_TIME_L", &unixTime);
  } else {
    lowestAntenna = 1;
  }
  move(4,0);
  rms=rm_read(lowestAntenna,"RM_SUN_AZ_DEG_F",&dummyFloat);
  sun_az=(double)dummyFloat;
  rms=rm_read(lowestAntenna,"RM_SUN_EL_DEG_F",&dummyFloat);
  sun_el=(double)dummyFloat;
  printw("Sun: Azim = %5.1f  Elev = %4.1f", sun_az, sun_el);
  move(4,40);
  printw("hangar light updated: ");
  dsm_status = dsm_read("colossus", "DSM_HANGAR_LIGHT_L",
		       &hangarLight, &timestamp);
  printIntervalNoTrailingSpace(time((long *)0)-timestamp, 1);

  move(6,0);
  printw("Ant Pad   Az    El  Sun Dist      Status     Drives   Dewar Temp");
  for (ant = 1; ant <= 8; ant++) {
    move(6+ant*2, 0);
    printw(" %d", ant);
    if (!antsAvailable[ant]) {
      printw("      Not Deployed");
    } else {
      char dummyByte;
      short dummyShort;
      int staleness;
      float dewarTemp;
      double az, el, sunDist;

      rms = rm_read(ant,"RM_PAD_ID_B",&dummyByte);
      rms = rm_read(ant,"RM_ACTUAL_AZ_DEG_F",&dummyFloat);
      az = (double)dummyFloat;
      rms = rm_read(ant,"RM_ACTUAL_EL_DEG_F",&dummyFloat);
      el = (double)dummyFloat;
      sunDist = sunDistance(az*D2R, el*D2R, sun_az*D2R, sun_el*D2R);
      printw("  %2d %6.1f  %4.1f  %5.1f     ",
	     dummyByte, az, el, sunDist);
      if (antennaInArray[ant])
	printw(" In Project");
      else if (antCheckedOut[ant])
	printw("Checked Out");
      else
	printw("       Idle");
      rms = rm_read(ant,"RM_SERVO_TIMESTAMP_L",&timestamp);
      staleness = (unixTime - timestamp);
      if (staleness > 10) {
	printw("Servo Stale");
	if (staleness < 1000)
	  printw("%03d", staleness);
	else
	  printw("***");
      } else {
	char faultState;
	short driveStatus;
	long scbFaultWord;
	static char dstat[][14] = { "      off  ", "      on   ", "      fault",
				    "    lockout", "     single", "     EL LMT", "     AZLMT",
				    "    one tac", " ESTOP     ", "  ???      " };

	rm_read(ant, "RM_SCB_FAULTWORD_L", &scbFaultWord);
	rm_read(ant,"RM_ANTENNA_DRIVE_STATUS_B",&dummyByte);
	rm_read(ant,"RM_SERVO_FAULT_STATE_B",&faultState);
	if (faultState==0)
	  driveStatus=(short)dummyByte;
	if (faultState>0)
	  driveStatus=(short)(faultState+1);
	
	switch(driveStatus) {
	case 0:
	  if(scbFaultWord & (1 << AIR_PRESSURE_SWITCH_FAULT)) {
	    printw("  AZBRAKE");
	  } else {
	    printw(dstat[0]);
	  }
	  break;
	case 1:
	  printw(dstat[1]);
	  break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	  printw(dstat[driveStatus]);
	  break;
	default:
	  printw(dstat[5]);
	  break;
	} /* end switch */
      }
      (void)rm_read(ant,"RM_DEWAR_TEMPS_V16_F",&dummyFloat16);
      dewarTemp = dummyFloat16[DEWAR_TEMP_4K_STAGE];
      if (fabs(dewarTemp) >= 1000) {
	printw("   wacko");
      } else {
	printw("   %5.1f", dewarTemp);
      }
    }
    if (antCheckedOut[ant]) {
      move(7+ant*2, 0);
      printw("    %s is using ant %d for %s", antUsers[ant], ant, scheme[ant]);
    }
  }
  move(0,79);
  refresh();
}







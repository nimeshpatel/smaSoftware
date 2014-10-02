#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "esma.h"
#include "rm.h"
#include "dsm.h"
#include "upspage.h"
#include "monitor.h"
#include "antMonitor.h"
#include "chopperControl.h"
#include "opticsMonitor.h"
#include "s_cmd2.h"  /* for HANGAR_PAD_ID */
#define DEBUG 0
#if DEBUG
  #define movemacro(x,y) refresh();move(x,y);
#else
  #define movemacro(x,y) move(x,y)
#endif
extern double sunDistance(double az1,double el1,double az2,double el2);
extern double timeDifferential(int antennaNumber);
extern int colorFlag;
void ComputeSolarRADec(double tjd_disp);
typedef struct {
  double rightAscension;
  double declination;
} SOLAR;
SOLAR solarPosition;
char homeStateStrings[][4] = {"OK ", "NO ", "   ", "ERR", "MOV"};
char XYZorChopStateStrings[][4] =  {"OK ", "MOV", "CH ", "ERR", "OL"};
void screen(char *source,double *lst_disp,double *utc_disp,double *tjd_disp,
	double *ra_disp,double *dec_disp, 
	double *ra_cat_disp, double *dec_cat_disp,
	double *az_disp,
	double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,double *tiltx,
	double *tilty,float *pressure, float *temperature, float *humidity,
	double *az_error, double *el_error,double *scan_unit, char *messg,
	float *posMm,
	float *syncdet_channels,float *cont1det1,float *cont2det1, 
        int *integration, float *windspeed, 
	float *winddirection,float *refraction, float *pmdaz, float *pmdel,
	double *smoothed_tracking_error, double *tsys, int *antennaNumber,
	float *plsAzRdg, int *radio_flag,short *padid,double *planetdistance,
	double *Az_cmd, double *El_cmd,double *raoff, double *decoff,
	    unsigned char *statusBits);

int antDisplay(int ant, int icount, int pageMode, ANTENNA_FLAGS *flags) {
  char dummyByte;
  float plsAzRdg;
  char messg[100];
  char lastcommand[100];
  int rms;
  /*
    struct sigaction action, old_action; int sigactionInt;
  */
  
  extern double radian;
  
  char source[SOURCE_CHAR_LEN] ;
  double lst_disp, utc_disp, tjd_disp;
  double ra_disp, dec_disp, ra_cat_disp, dec_cat_disp;
  double az_disp, el_disp; 
  double azoff, eloff; 
  double raoff,decoff;
  double az_actual_corrected, el_actual_disp, tiltx;
  double tilty; 
  float pressure, temperature, humidity;
  double az_error, el_error;
  double scan_unit; 
  float posMm[4];		/* x, y, z, and t of chopper n mm and arcsec */
  unsigned char statusBits[16];
  int integration=0; 
  float windspeed ,winddirection, refraction, pmdaz, pmdel;
  double smoothed_tracking_error, tsys; 
  int radio_flag; 
  double planetdistance,Az_cmd, El_cmd; 
  double sunaz,sunel;
  float dummyFloat;
  double dummyDouble;
  short dummyShort;
  short padid;
  float cont1det1, cont2det1;
  float syncdet_channels[2];
  double timeError;
  char host[6];
  long estopBypass;
  char airPressureSwitch;
  char azDriveState;
  time_t timestamp;
  short disableDrivesFlag;

  if (pageMode == ANTENNA_PAGE_CHECK_ONLY) {
    timeError = timeDifferential(ant);
    flags->flags = flags->timeDifferential = flags->estopBypass = flags->driveTimeoutDisabled = 0;
    flags->wackoOffsets = flags->azBrakeManualRelease = 0;
    if (fabs(timeError) > TIME_DIFFERENTIAL_STANDOUT) {
      flags->timeDifferential = 1;
    }
    sprintf(host,"acc%d",ant);
    rms = rm_read(ant,"RM_DRIVES_TIMEOUT_FLAG_S",&disableDrivesFlag);
    flags->driveTimeoutDisabled = disableDrivesFlag;
    dsm_read(host,"DSM_ESTOP_BYPASS_L",(char *)&estopBypass, &timestamp);
    if (estopBypass==0) {
      flags->estopBypass = 1;
    }
    rms=rm_read(ant,"RM_SCB_AZIM_BRAKE_RELEASED_B",(char *)&airPressureSwitch);
    rms=rm_read(ant,"RM_AZ_DRV_STATE_B",    (char *)&azDriveState);
    if (airPressureSwitch==1 && azDriveState==0) {
      flags->azBrakeManualRelease = 1;
    }
    rms=rm_read(ant,"RM_AZOFF_D",&azoff);	
    rms=rm_read(ant,"RM_ELOFF_D",&eloff);	
    rms=rm_read(ant,"RM_RAOFF_ARCSEC_D",&raoff);	
    rms=rm_read(ant,"RM_DECOFF_ARCSEC_D",&decoff);	
    if (fabs(azoff) > WACKO_OFFSET ||
	fabs(eloff) > WACKO_OFFSET ||
	fabs(raoff) > WACKO_OFFSET ||
	fabs(decoff) > WACKO_OFFSET
	) {
      flags->wackoOffsets = 1;
    }
    flags->flags = flags->timeDifferential | flags->estopBypass | flags->azBrakeManualRelease | flags->wackoOffsets | flags->driveTimeoutDisabled;
    return(0);
  }

	  rms=rm_read(ant,"RM_PAD_ID_B",&dummyByte);
	  if(rms!= RM_SUCCESS) {
	    rm_error_message(rms,"padid");
	    exit(1);
	  }
	  padid = (short)dummyByte;

  if (deadAntennas[ant]) {
    if ((icount%30) == 0) {
      initscr();
#ifdef LINUX
      clear();
#endif
      refresh();
    }
    movemacro(11, 13);
    printw("Antenna %d ",ant);
    if (ant == CSO_ANTENNA_NUMBER) {
      printw("(CSO) ");
    }
    if (ant == JCMT_ANTENNA_NUMBER) {
      printw("(JCMT) ");
    }
    printw("is out of service - no data are available");
    if (padid == HANGAR_PAD_ID) {
      movemacro(13,16);
      printw("(it has the hangar ID connector attached)");
    }
    movemacro(0,0);
    refresh();
    return(0);
  }
          rms=rm_read(ant,"RM_AZOFF_D",&azoff);	
	  rms=rm_read(ant,"RM_ELOFF_D",&eloff);	
          rms=rm_read(ant,"RM_RAOFF_ARCSEC_D",&raoff);	
	  rms=rm_read(ant,"RM_DECOFF_ARCSEC_D",&decoff);	
	  rms=rm_read(ant,"RM_SUN_AZ_DEG_F",&dummyFloat);
	  sunaz=(double)dummyFloat;
	  rms=rm_read(ant,"RM_SUN_EL_DEG_F",&dummyFloat);
	  sunel=(double)dummyFloat;
	  sunaz=sunaz*radian;
	  sunel=sunel*radian;
	  
	  /*
	    rms=rm_read(ant,"RM_CALIBRATION_WHEEL_S",&cal_wheel_status);
	  */
	  
	  rms=rm_read(ant,"RM_SMARTS_OFFSET_UNIT_ARCSEC_S",&dummyShort);
	  scan_unit=(double)dummyShort;
	  rms=rm_read(ant,"RM_SYNCDET2_CHANNELS_V2_F",syncdet_channels);
	  rms=rm_read(ant,"RM_CONT1_DET1_F",&cont1det1);
	  rms=rm_read(ant,"RM_CONT2_DET1_F",&cont2det1);
	  rms=rm_read(ant,"RM_REFRACTION_RADIO_FLAG_B",&dummyByte);
	  radio_flag=(int)dummyByte; 
	  
	  rms=rm_read(ant,"RM_WEATHER_TEMP_F",&temperature);
	  rms=rm_read(ant,"RM_WEATHER_HUMIDITY_F",&humidity);
	  rms=rm_read(ant,"RM_WEATHER_MBAR_F",&pressure);
	  rms=rm_read(ant,"RM_WEATHER_WINDSPEED_F",&windspeed);
	  windspeed *= 0.44704; /* convert from mph to m/s */
	  rms=rm_read(ant,"RM_WEATHER_WINDDIR_F",&winddirection);
	  rms=rm_read(ant,"RM_TSYS_D",&tsys);
	  rms=rm_read(ant,"RM_COMMANDED_AZ_DEG_F",&dummyFloat);
	  az_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_COMMANDED_EL_DEG_F",&dummyFloat);
	  el_disp=(double)dummyFloat;
	  
	  rms=rm_read(ant, "RM_AZ_TRACKING_ERROR_F",&dummyFloat);
	  az_error=(double)dummyFloat;
	  rms=rm_read(ant, "RM_EL_TRACKING_ERROR_F",&dummyFloat);
	  el_error=(double)dummyFloat;
	  rms=rm_read(ant,"RM_RA_APP_HR_D", &ra_disp);
	  rms=rm_read(ant,"RM_DEC_APP_DEG_D",&dec_disp);
	  rms=rm_read(ant,"RM_REFRACTION_ARCSEC_D",&dummyDouble);
	  refraction=(float)dummyDouble;
	  rms=rm_read(ant,"RM_PLANET_DISTANCE_AU_D", &planetdistance);
	  rms=rm_read(ant,"RM_SOURCE_C34",source);
	  rms=rm_read(ant,"RM_LST_HOURS_F",&dummyFloat);
	  lst_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_UTC_HOURS_F",&dummyFloat);
	  utc_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_TJD_D",&tjd_disp);
	  rms=rm_read(ant,"RM_RA_CAT_HOURS_F",&dummyFloat);
	  ra_cat_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_DEC_CAT_DEG_F",&dummyFloat);
	  dec_cat_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_ACTUAL_AZ_DEG_F",&dummyFloat);
	  az_actual_corrected=(double)dummyFloat;
	  rms=rm_read(ant,"RM_PLS_AZ_DEG_F",&plsAzRdg);
	  rms=rm_read(ant,"RM_ACTUAL_EL_DEG_F",&dummyFloat);
	  el_actual_disp=(double)dummyFloat;
	  rms=rm_read(ant,"RM_PMDAZ_F",&pmdaz);
	  rms=rm_read(ant,"RM_PMDEL_F",&pmdel);
	  rms=rm_read(ant,"RM_TRACK_MESSAGE_C100",messg);
	  rms=rm_read(ant,"RM_TRACK_LAST_COMMAND_C100",lastcommand);

#if 0
	  rms=rm_read(ant,"RM_CHOPPER_X_MM_F",&rm_chopper_x_mm);
	  rms=rm_read(ant,"RM_CHOPPER_Y_MM_F",&rm_chopper_y_mm);
	  rms=rm_read(ant,"RM_CHOPPER_Z_MM_F",&rm_chopper_z_mm);
	  rms= rm_read(ant,"RM_CHOPPER_ANGLE_F",
		       &rm_chopper_tilt_arcsec);
	  rms=rm_read(ant,"RM_CHOPPER_X_COUNTS_D",&subx_counts);
	  rms=rm_read(ant,"RM_CHOPPER_Y_COUNTS_D",&suby_counts);
	  rms=rm_read(ant,"RM_CHOPPER_Z_COUNTS_D",&focus_counts);
	  rms= rm_read(ant,"RM_CHOPPER_TILT_COUNTS_D",
		       &subtilt_counts);
#endif
	  rms=rm_read(ant,"RM_CHOPPER_POS_MM_V4_F",posMm);
	  rms=rm_read(ant,"RM_CHOPPER_STATUS_BITS_V16_B",statusBits);
/*
	  rms=rm_read(ant,"RM_CHOPPER_STATUS_S",&chopper_status);
	  subx_counts =  (double)rm_chopper_x_mm* 1000.;
	  suby_counts =  (double)rm_chopper_y_mm* 500.;
	  focus_counts =  (double)rm_chopper_z_mm* 2000.;
	  subtilt_counts = ((double)rm_chopper_tilt_arcsec/ 0.009)+32768;
*/
	  
	  

	  rms=rm_read(ant,"RM_TILTX_ARCSEC_D",&tiltx);
	  rms=rm_read(ant,"RM_TILTY_ARCSEC_D",&tilty);
	  
	  rms=rm_read(ant,"RM_AZOFF_D",&azoff);
	  rms=rm_read(ant,"RM_ELOFF_D",&eloff);
	  rms=rm_read(ant,"RM_PLANET_DISTANCE_AU_D", &planetdistance);
	  rms=rm_read(ant,"RM_COMMANDED_AZ_DEG_F",&dummyFloat);
	  Az_cmd=(double)dummyFloat;
	  rms=rm_read(ant,"RM_COMMANDED_EL_DEG_F",&dummyFloat);
	  El_cmd=(double)dummyFloat;


	  screen(source, &lst_disp, &utc_disp, &tjd_disp, &ra_disp, &dec_disp,
		 &ra_cat_disp, &dec_cat_disp,
		 &az_disp, &el_disp, &icount, &azoff, &eloff, 
		 &az_actual_corrected, &el_actual_disp, &tiltx, &tilty, 
		 &pressure, &temperature, &humidity,
		 &az_error, &el_error, &scan_unit, messg, posMm,
		 syncdet_channels, &cont1det1,&cont2det1, &integration,
		 &windspeed, &winddirection, &refraction, 
		 &pmdaz, &pmdel, &smoothed_tracking_error, &tsys,
		 &ant,&plsAzRdg,&radio_flag,&padid,
		 &planetdistance,&Az_cmd,&El_cmd,&raoff,&decoff, statusBits);

	return 0;
}

void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign);
void af(int *i,char s[2]);

void screen(char *source,double *lst_disp,double *utc_disp,double *tjd_disp,
	double *ra_disp,double *dec_disp, 
	double *ra_cat_disp, double *dec_cat_disp,
	double *az_disp,
	double *el_disp,int *icount,double *azoff,double *eloff,
	double *az_actual_corrected,double *el_actual_disp,double *tiltx,
	double *tilty,float *pressure, float *temperature, float *humidity,
	double *az_error, double *el_error,double *scan_unit, char *messg,
	float *posMm,
	float *syncdet_channels,float *cont1det1,float *cont2det1, 
        int *integration, float *windspeed, 
	float *winddirection,float *refraction, float *pmdaz, float *pmdel,
	double *smoothed_tracking_error, double *tsys, int *antennaNumber,
	float *plsAzRdg, int *radio_flag,short *padid,double *planetdistance,
	double *Az_cmd, double *El_cmd,double *raoff, double *decoff,
	unsigned char *statusBits)
{
  int antlist[RM_ARRAY_SIZE];
  OPTICS_FLAGS opticsFlags;
  short az_act_sign,az_sign,el_sign,dec_dum_sign,dec_app_sign,dec_cat_sign;
  short presentSunSafeMinutes, requiredSunSafeMinutes;
  char str[3];
  int lsth,lstm,ra_cat_h,ra_cat_m,dec_cat_d,dec_cat_m;
  int dec_app_d,dec_app_m;
  int ra_app_h,ra_app_m;
  int utch,utcm;
  int feed,rms;
  double a1,a2;
  int az_cmd_d,az_cmd_m,el_cmd_d,el_cmd_m;
  int az_act_d,az_act_m,el_act_d,el_act_m;
  double ra_cat_s,dec_cat_s,ra_app_s,dec_app_s,lsts,utcs;
  double az_cmd_s,el_cmd_s,az_act_s,el_act_s;
#if 0
  static int idleMinutes;
  static int idlehMinutes;
  static int idlecMinutes;
  static int idleaMinutes;
#endif
  short vvmlock1,vvmlock2;
  int lstsi,utcsi,az_cmd_si,az_act_si,el_cmd_si,el_act_si;
  double sunaz,sunel,sunD;
  double ha;
  double timeError;
  char host[6];
  double planetdiameter;
  char airPressureSwitch;
  int wackoSource, i;
  char azDriveState;
  char padIDIsFake;
  long  syncdet2timestamp;
  long  unixTime;
  long estopBypass;
  time_t timestamp;
  extern double radian;
  float syncdetStats[6];
  float azRockerCWLimit, azRockerCCWLimit, cwLimit, ccwLimit;
  float dummyFloat;
  float dummyFloat2[2];
  double tiltCoeff[6], tiltAzoff, tiltEloff;
  double cmdEpoch;
  float actualEpoch;
  char messgbuf[40];
  short disableDrivesFlag;
  short pair=2;
  float tiltVolts[4];
 double pmaztiltAzoff,pmaztiltEloff;
 double tiltSX,tiltSY, tiltGX,tiltGY;
 double datiltAzoff, datiltEloff;
 int tiltCorrFlag;
	
  short foreground,background;

  ha=*lst_disp-*ra_disp;

  if((*icount%30)==0) {
    initialize();
    ComputeSolarRADec(*tjd_disp);	
  }
  hms(ra_disp,&ra_app_h,&ra_app_m,&ra_app_s,&dec_dum_sign);
  hms(dec_disp,&dec_app_d,&dec_app_m,&dec_app_s,&dec_app_sign);
  hms(ra_cat_disp,&ra_cat_h,&ra_cat_m,&ra_cat_s,&dec_dum_sign);
  hms(dec_cat_disp,&dec_cat_d,&dec_cat_m,&dec_cat_s,&dec_cat_sign);
  hms(lst_disp,&lsth,&lstm,&lsts,&dec_dum_sign);
  hms(utc_disp,&utch,&utcm,&utcs,&dec_dum_sign);
  hms(az_disp,&az_cmd_d,&az_cmd_m,&az_cmd_s,&az_sign);
  hms(el_disp,&el_cmd_d,&el_cmd_m,&el_cmd_s,&el_sign);
  hms(az_actual_corrected,&az_act_d,&az_act_m,&az_act_s,&az_act_sign);
  hms(el_actual_disp,&el_act_d,&el_act_m,&el_act_s,&dec_dum_sign);
  
  
  
  lstsi=(int)lsts;
  utcsi=(int)utcs;
  az_cmd_si=(int)az_cmd_s;
  el_cmd_si=(int)el_cmd_s;
  az_act_si=(int)az_act_s;
  el_act_si=(int)el_act_s;
  
  /* box(stdscr, '|','-'); */
  
  /*
    movemacro(1,59);
    printw("M3:%7s",m3StateString);
  */
  sprintf(host,"acc%d",*antennaNumber);
  dsm_read(host,"DSM_ESTOP_BYPASS_L",(char *)&estopBypass, &timestamp);
  if (estopBypass==0 && *antennaNumber <= 8) {
    movemacro(1,2);
    standout();
    printw("Antenna computer E-stop bypassed in the cabin!");
    standend();
  } else {
    movemacro(1,2);
    printw("                                              ");
  }
  rms = rm_read(*antennaNumber,"RM_DRIVES_TIMEOUT_FLAG_S",&disableDrivesFlag);
  if (disableDrivesFlag==1) {
    movemacro(1,56);
    standout();
    printw("Drive timeout disabled");
    standend();
  } else {
    movemacro(1,56);
    printw("                      ");
  }
  rms =rm_read(*antennaNumber,"RM_SCB_AZIM_BRAKE_RELEASED_B",
	       (char *)&airPressureSwitch);
  rms =rm_read(*antennaNumber,"RM_AZ_DRV_STATE_B",
	       (char *)&azDriveState);
  if (airPressureSwitch==1 && azDriveState==0) {
    movemacro(18,31);
    standout();
    printw("Azimuth brake released manually in the cabin!");
    standend();
  } else {
    movemacro(18,30);
    printw("                                             ");
  }

  movemacro(2,2);
  
  printw("SMA Antenna-%d    tracking   ",*antennaNumber);
  movemacro(3,2);
  switch (*antennaNumber) {
  case CSO_ANTENNA_NUMBER:
    printw("on pad: CSO");
    break;
  case JCMT_ANTENNA_NUMBER:
    printw("on pad: JCMT");
    break;
  default:
    printw("on pad: ");
    if (*padid < MIN_ANTENNA_PAD || *padid > MAX_ANTENNA_PAD) {
      standout();
    }
    printw("%d",*padid);
    standend();
    rms =rm_read(*antennaNumber,"RM_PAD_ID_IS_FAKE_B",(char *)&padIDIsFake);
    if (padIDIsFake == 1) {
      printw(" (");
      standout();
      printw("software ID");
      standend();
      printw(")");
    } else if (padIDIsFake == 0) {
      printw(" (hardware ID)");
    } else {
      standout();
      printw(" (wacko ID)");
      standend();
    }
  }

  rms=rm_read(*antennaNumber,"RM_PLANET_DIAMETER_ARCSEC_D",
		  &planetdiameter);
  if (planetdiameter > 0) {
    movemacro(3,27);
    if (planetdiameter > 2500) {
      printw("Diam=wacko");
    } else {
      printw("Diam=%.1f\"",planetdiameter);
    }
  }
  movemacro(4,5);
  addstr("LST");
  move (4,15);
  addstr("UTC");
  movemacro(4,25);
  addstr("TJD");
  movemacro(2,32);
  if (strlen(source) > SOURCE_CHAR_LEN) {
    printw("wacko");
  } else { 
    wackoSource = 0;
    for (i=0; i<strlen(source); i++) {
      if (source[i] < 0) {
	printw("wacko");
	wackoSource = 1;
	break;
      }
    }
    if (wackoSource == 0) {
      addstr(source);
    }
  }
  move (5,3);
  af(&lsth,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,6);
  af(&lstm,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,9);
  af(&lstsi,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,13);
  af(&utch,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,16);
  af(&utcm,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,19);
  af(&utcsi,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(5,23);
  printw("%lf",*tjd_disp);
  movemacro(6,3);
  printw("H.A.: ");
#if DEBUG
  refresh();
#endif  
  if (fabs(ha) < 100) {
    printw("%+.4f",ha);
  } else {
    printw("wacko");
  }
#if DEBUG
  refresh();
#endif  
  movemacro(6,22);
  timeError = timeDifferential(*antennaNumber);
  if (fabs(timeError) > TIME_DIFFERENTIAL_STANDOUT) {
    standout();
  }
  printw("Error: ");
  if (fabs(timeError) < 100000) {
    printw(" %+.3f sec",timeError);
  } else {
    printw(" wacko ");
  }
  standend();
#if DEBUG
  refresh();
#endif  
  
  movemacro(7,2);
  rms = rm_read(*antennaNumber,"RM_CMD_EPOCH_YEAR_D",&cmdEpoch);
  if (cmdEpoch > 2000 || cmdEpoch < -1) {
    standout();
  }
  if (cmdEpoch > 9999.5 || cmdEpoch < -999.5) {
    printw("wacko");
  } else {
    if (fabs(cmdEpoch) < 1) { 
      printw(" none");
    } else {
      printw("J%4.0f",cmdEpoch);
    }
  }
  standend();
  printw("/");
  rms = rm_read(*antennaNumber,"RM_EPOCH_F",&actualEpoch);
  if (actualEpoch > 2000 || actualEpoch < 1950) {
    standout();
  }
  if (actualEpoch > 9999.5 || actualEpoch < 999.5) {
    printw("wacko");
  } else {
    printw("J%4.0f",actualEpoch);
  }
  standend();
#if DEBUG
  refresh();
#endif  
  movemacro(7,16);
  addstr("RA");
  movemacro(7,30);
  addstr("DEC");
  movemacro(8,2);
  addstr("CATALOG");
  movemacro(8,12);
  af(&ra_cat_h,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,15);
  af(&ra_cat_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,18);
  if (fabs(ra_cat_s) >= 100) {
    printw("wacko");
  } else {
    printw("%06.3f",ra_cat_s);
  }
  movemacro(8,27);
  if (dec_cat_sign>0) addch('+');
  if (dec_cat_sign<0) addch('-');
  if (dec_cat_sign==0) addch(' ');
  af(&dec_cat_d,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,31);
  af(&dec_cat_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(8,34);
  if (fabs(dec_cat_s) >= 100) {
    printw("wacko");
  } else {
    printw("%05.2f",dec_cat_s);
  }
  
  movemacro(9,2);
  printw("APPARENT");
  movemacro(9,12);
  af(&ra_app_h,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,15);
  af(&ra_app_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,18);
  if (fabs(ra_app_s) >= 100) {
    standout();
    printw("wacko");
    standend();
  } else {
    printw("%06.3f",ra_app_s);
  }
  movemacro(9,27);
  if(dec_app_sign>0)addch('+');
  if(dec_app_sign<0)addch('-');
  if(dec_app_sign==0)addch(' ');
  af(&dec_app_d,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,31);
  af(&dec_app_m,str);
  if (isdigit(str[0]) == 0 || isdigit(str[1]) == 0) {
    printw("wa");
  } else {
    addch(str[0]);
    addch(str[1]);
  }
  movemacro(9,34);
  if (fabs(dec_app_s) >= 100) {
    standout();
    printw("wacko");
    standend();
  } else {
    printw("%05.2f",dec_app_s);
  }
  
  movemacro(10,2);
  printw("RaDecOFFSET(\")");
  if (fabs(*raoff) > WACKO_OFFSET) {
    movemacro(10,19);
    standout();
    printw("wacko");
    standend();
  } else {
    if (fabs(*raoff) < 0.01 && fabs(*raoff) > 0) {
      movemacro(10,15);
      standout();
      printw("%.2e ",*raoff);
      standend();
    } else {
      movemacro(10,19);
      printw("%.2f ",*raoff);
    }
  }
  if (fabs(*decoff) > WACKO_OFFSET) {
    movemacro(10,34);
    standout();
    printw("wacko");
    standend();
  } else {
    if (fabs(*decoff) < 0.01 && fabs(*decoff) > 0) {
      movemacro(10,30);
      standout();
      printw("%.2e ",*decoff);
      standend();
    } else {
      movemacro(10,34);
      printw(" %.2f ",*decoff);
    }
  }
  
  movemacro(11,14);
  addstr("AZIM");
  movemacro(11,30);
  addstr("ELEV");
  
  move(12,2);
  addstr("CMD");
  move(12,9);
#if DEBUG
  refresh();
#endif  
  if((az_sign<0)&&(az_cmd_d==0))addch('-');
  move(12,10);
  if (abs(az_cmd_d) > 399) {
    standout();
    printw("wac");
    standend();
  } else {
    printw("%03d",az_cmd_d);
  }
  move(12,15);
  if (az_cmd_m >= 60 || az_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&az_cmd_m,str);
    addch(str[0]);
    addch(str[1]);
  }
  move(12,18);
  if (az_cmd_si >= 60 || az_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&az_cmd_si,str);
    addch(str[0]);
    addch(str[1]);
    move(12,27);
  }
#if DEBUG
  refresh();
#endif  
  if(el_sign>=0)addch(' ');
  if(el_sign<0)addch('-');
  move(12,28);
  if (fabs(el_cmd_d) > 90) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_d,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(12,31);
  if (el_cmd_m >= 60 || el_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_m,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(12,34);
  if (el_cmd_si >= 60 || el_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_cmd_si,str);
    addch(str[0]);
    addch(str[1]);
  }
  move(13,2);
  addstr("ACTUAL");
  move(13,9);
  if((az_act_sign<0)&&(az_act_d==0))addch('-');
  move(13,10);
  printw("%03d",az_act_d);
  move(13,15);
  af(&az_act_m,str);
  addch(str[0]);
  addch(str[1]);
  move(13,18);
  af(&az_act_si,str);
  addch(str[0]);
  addch(str[1]);
#if DEBUG
  refresh();
#endif  
  if (el_act_d >= 100 || el_act_d <= -10) {
    move(13,27);
    standout();
    printw("wac");
    standend();
  } else {
    move(13,28);
    af(&el_act_d,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(13,31);
  if (el_cmd_m >= 60 || el_cmd_m < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_act_m,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(13,34);
  if (el_cmd_si >= 60 || el_cmd_si < 0) {
    standout();
    printw("wa");
    standend();
  } else {
    af(&el_act_si,str);
    addch(str[0]);
    addch(str[1]);
  }
#if DEBUG
  refresh();
#endif  
  move(14,2);
  addstr("ERROR");
  move(14,11);
#define LARGE_TRACKING_ERROR 2.0
  if (fabs(*az_error) > LARGE_TRACKING_ERROR) {
    standout();
  }
  printw("%7.1f\"",*az_error);
  standend();
#if DEBUG
  refresh();
#endif  
  move(14,28);
  if (fabs(*el_error) > LARGE_TRACKING_ERROR) {
    standout();
  }
  printw("%7.1f\"",*el_error);
  standend();
#if DEBUG
  refresh();
#endif  
  /*move(18,23);
    printw("(%.1f\")",*smoothed_tracking_error);
  */
  move(15,2);
  addstr("PMODELS(\")");
  move(15,12);
  if (fabs(*pmdaz) >= 1000000) {
    standout();
    printw("(wacko)");
    standend();
  } else {
    printw("(%6.0f)",*pmdaz);
  }
  move(15,32);
  if (fabs(*pmdel) >= 1000000) {
    standout();
    printw("(wacko)");
    standend();
  } else {
    printw("(%6.0f)",*pmdel);
  }
  move(16,2);
  addstr("OFFSETS(\")");
  move(16,12);
#if DEBUG
  refresh();
#endif  
  
  if (fabs(*azoff) > WACKO_OFFSET) {
    standout();
    printw(" wacko");
    standend();
  } else {
    if (fabs(*azoff) >= 20.) {
      standout();
    }
    printw("%6.0f",*azoff);
    standend();
  }
#if DEBUG
  refresh();
#endif  
  move(16,32);
  if (fabs(*eloff) > WACKO_OFFSET) {
    standout();
    printw(" wacko");
    standend();
  } else {
    if(fabs(*eloff) >= 20.) standout();
    printw("%6.0f",*eloff);
    standend();
  }
#if DEBUG
  refresh();
#endif  
  move(18,2);
  addstr("REFRACTION:");
  move(18,13);
#if DEBUG
  refresh();
#endif  
  switch (*radio_flag) {
  case 1:
    printw("%5.1f \" (radio)   ",*refraction);
    break;
  case 0:
    standout();
    printw("%5.1f \" (optical) ",*refraction);
    standend();
    break;
  default:
    printw(" wacko");
    break;
  }
#if DEBUG
  refresh();
#endif
/*
  move(2,42);
  strncpy(messgbuf,messg,79-42);
  wackoSource = 0;
  for (i=0; i<strlen(messgbuf); i++) {
    if (messgbuf[i] < 0) {
      printw("Wacko message");
      wackoSource = 1;
      break;
    }
  }

  if (wackoSource == 0) {
    addstr(messgbuf);
    addch('\n');
  }
*/
  move(10,42);
  addstr("SUB_Z:");
  move(10,49);
#define WACKO_CHOPPER_COUNTS 200000
  if (fabs(posMm[2]*Z_COUNTS_PER_MM) > WACKO_CHOPPER_COUNTS) {
    standout();
    printw(" wacko  cts");
    standend();
  } else {
    printw("%8.0f cts, %4.3f mm ",posMm[2] * Z_COUNTS_PER_MM, posMm[2]);
  }

#if DEBUG
  refresh();
#endif  
  move(8,42);
  addstr("SUB_X:");
  move(8,49);
  if (fabs(posMm[0]*X_COUNTS_PER_MM) > WACKO_CHOPPER_COUNTS) {
    standout();
    printw(" wacko  cts");
    standend();
  } else {
    printw("%8.0f cts, %4.3f mm ",posMm[0] * X_COUNTS_PER_MM, posMm[0]);
  }
  
#if DEBUG
  refresh();
#endif  
  move(9,42);
  addstr("SUB_Y:");
  move(9,49);
  if (fabs(posMm[1]*Y_COUNTS_PER_MM) > WACKO_CHOPPER_COUNTS) {
    standout();
    printw(" wacko  cts");
    standend();
  } else {
    printw("%8.0f cts, %4.3f mm ",posMm[1] * Y_COUNTS_PER_MM, posMm[1]);
  }
#if 0
  move(8,42);
  if (((*icount) % 60) == 0) {
    idleMinutes = checkObsconIdleTime();
  }
  printObsconIdleTime(idleMinutes);

  move(9,42);
  if (((*icount) % 60) == 0) {
    idlehMinutes = checkObsconhIdleTime();
  }
  printObsconhIdleTime(idlehMinutes);

  move(10,42);
  if (((*icount) % 60) == 0) {
    idlecMinutes = checkObsconcIdleTime();
  }
  printObsconcIdleTime(idlecMinutes);

#endif  
  move(2,42);
  rms=rm_read(*antennaNumber,"RM_PRESENT_SUN_SAFE_MINUTES_S",
	      &presentSunSafeMinutes);
  rms=rm_read(*antennaNumber,"RM_REQUIERD_SUN_SAFE_MINUTES_S",
	      &requiredSunSafeMinutes);
  switch(presentSunSafeMinutes) {
  case -1:
    printw("Safe from Sun                    ");
    break;
  case 0:
    standout();
    printw("In Sun Avoidance Zone            ");
    standend();
    break;
  default:
    printw("Sun Safe Minutes:    ");

    if (abs((int)presentSunSafeMinutes) >=10000) {
      printw("wacko, ");
    } else {
      printw("%d, ",(int)presentSunSafeMinutes);
    }
    if (abs((int)requiredSunSafeMinutes) >=10000) {
      printw("wacko required");
    } else {
      printw("%d required", (int)requiredSunSafeMinutes);
    }
    break;
  }
  
  move(3,42);
  rms=rm_read(*antennaNumber,"RM_SUN_AZ_DEG_F",&dummyFloat);
  sunaz=(double)dummyFloat;
  rms=rm_read(*antennaNumber,"RM_SUN_EL_DEG_F",&dummyFloat);
  sunel=(double)dummyFloat;
  sunaz=sunaz*radian;
  sunel=sunel*radian;
  sunD = sunDistance(az_act_d*radian, el_act_d*radian,sunaz,sunel);
  printw("Sun Dist: ");
  if (sunD < SUN_DISTANCE_LIMIT) {
    standout();
  }
  printw("%.1f deg",sunD);
  standend();
  move(5,42);
  printw("Sun Elev: ");
  printw("%+.1f deg",dummyFloat);
  move(4,42);
  printw("Sun RA/Dec: ");
  printw("%02dh%02dm / %c%02dd02m",
    (int)solarPosition.rightAscension,
    (int)(60*(solarPosition.rightAscension-floor(solarPosition.rightAscension))),
    (solarPosition.declination > 0? '+' : '-'),
    (int)(fabs(solarPosition.declination)),
	 (int)(60*(fabs(solarPosition.declination)-floor(fabs(solarPosition.declination)))));

/* display warning if rocker limits are beyond soft limits */
rms = rm_read(*antennaNumber,"RM_AZ_ROCKER_CW_LIMIT_F",	&azRockerCWLimit);
rms = rm_read(*antennaNumber,"RM_AZ_ROCKER_CCW_LIMIT_F",	&azRockerCCWLimit);
rms = rm_read(*antennaNumber, "RM_SCB_CW_LIMIT_F",	&cwLimit);
rms = rm_read(*antennaNumber, "RM_SCB_CCW_LIMIT_F",	&ccwLimit);
move(11,42);
if (cwLimit < azRockerCWLimit && *antennaNumber<=8) {
  standout();
  printw("Clockwise rocker limit needs service");
  standend();
} else if (ccwLimit > azRockerCCWLimit && *antennaNumber<=8) {
  standout();
  printw("CntrClock rocker limit needs service");
  standend();
} else {
#if 0
  if (((*icount) % 60) == 0) {
    idleaMinutes = checkAsiaaIdleTime();
  }
  printAsiaaIdleTime(idleaMinutes);
#endif
}

move(7,42);
addstr("SUB_TILT:");
move(7,52);
 if (fabs(posMm[3]*TILT_COUNTS_PER_ARCSEC) > WACKO_CHOPPER_COUNTS) {
   standout();
   printw(" wacko  cts");
   standend();
 } else {
   if (abs(posMm[3]) > 10000000) {
     standout();
     printw(" wacko  ");
     standend();
   } else {
     printw("%8.0f",posMm[3] * TILT_COUNTS_PER_ARCSEC);
   }
   printw(" cts,");
   if (abs(posMm[3]) > 10000000) {
     standout();
     printw(" wacko  ");
     standend();
   } else {
     printw(" %4.2f\" ",posMm[3]);
   }
 }

 rms = rm_read(*antennaNumber,"RM_SYNCDET2_STATS_V6_F",syncdetStats);
 /*
 syncdetStats[0] = rmsL;
 syncdetStats[1] = minOnL;
 syncdetStats[2] = maxOnL;
 syncdetStats[3] = rmsH;
 syncdetStats[4] = minOnH;
 syncdetStats[5] = maxOnH;
 */

move(19,2);
addstr("CONT1DET1:");
move(19,13);
 rms = rm_read(*antennaNumber, "RM_UNIX_TIME_L",&unixTime);
 if ((*cont1det1 > -32768.0) && (*cont1det1 < 32767.0)) {
   printw("%7.1fmV", (*cont1det1)*1000.);
   rms = rm_read(*antennaNumber, "RM_CONT1_DET1_POWER_MUWATT_F", &dummyFloat);
   rms = rm_read(*antennaNumber, "RM_SYNCDET2_TIMESTAMP_L",&syncdet2timestamp);
   if (syncdet2timestamp < (unixTime-300)) {
     printw(" ");
     standout();
     printw("syncdet2 stale");
     standend();
     printw("   ");
   } else {
     if (dummyFloat < 0.1) {
       standout();
     }
     printw(" %5.3fuW %+5.2fdBm",dummyFloat,10*log10(dummyFloat)-30.0);
     standend();
   }
 } else {
   standout();
   printw(" Wacko ");
   standend();
 }
 move(21,42);
 printw("rms=");
#define SYNCDET_RMS_STANDOUT 10.0
 if (fabs(syncdetStats[0]) > SYNCDET_RMS_STANDOUT) {
   standout();
 }
#define SYNCDET_RMS_WACKO 1000000
 if (fabs(syncdetStats[0]) >= SYNCDET_RMS_WACKO) {
   printw("wacko");
 } else {
   printw("%5.2f",syncdetStats[0]);
 }
 standend();

 if (fabs(syncdetStats[1]) > SYNCDET_RMS_WACKO) {
   printw("  min/max=wacko/");
 } else {
   printw("  min/max=%.0f/",syncdetStats[1]);
 }
 if (fabs(syncdetStats[2]) > SYNCDET_RMS_WACKO) {
   printw("wacko");
 } else {
   printw("%.0f",syncdetStats[2]);
 }

rms = rm_read(*antennaNumber,"RM_FEED_L",&feed);
rms = rm_read(*antennaNumber,"RM_FEEDOFFSET_A1_ARCSEC_D",&a1);
rms = rm_read(*antennaNumber,"RM_FEEDOFFSET_A2_ARCSEC_D",&a2);

  opticsPage(*icount,antlist,OPTICS_PAGE_CHECK_ONLY,&opticsFlags);

move(17,2);
  addstr("RxOFFSETS\"(");
  
 if (feed < 100 || feed > 999) {
   standout();
   addstr("wacko");
   standend();
 } else {
   if (opticsFlags.feedOffsetMismatch[*antennaNumber]) {
     standout();
   }
   printw("%3d GHz",feed);
   standend();
   printw("): ");
 }
 if (fabs(a1) < 999) {
   printw("A1=%.2f ",a1);
 } else {
   standout();
   addstr("A1=wacko");
   standend();
 }
 if (fabs(a2) < 999) {
   printw("A2=%.2f",a2);
 } else {
   standout();
   addstr("A2=wacko");
   standend();
 }

move(20,2);
addstr("CONT2DET1:");
move(20,13);
 if ((*cont2det1 > -32768.0) && (*cont2det1 < 32767.0)) {
   printw("%7.1fmV", (*cont2det1)*1000.);
   rms = rm_read(*antennaNumber, "RM_CONT2_DET1_MUWATT_F", &dummyFloat);
   if (syncdet2timestamp < (unixTime-300)) {
     printw(" ");
     printAge(unixTime,syncdet2timestamp);
     printw("        ");
     /*
     standout();
     printw("syncdet2 stale");
     standend();
     printw("   ");
     */
   } else {
     if (dummyFloat < 0.1) {
       standout();
     }
     printw(" %5.3fuW %+5.2fdBm",dummyFloat,10*log10(dummyFloat)-30.0);
     standend();
   }
 } else {
   printw(" Wacko ");
 }
 

 move(22,42);
 printw("rms=");
 if (fabs(syncdetStats[3]) > SYNCDET_RMS_STANDOUT) {
   standout();
 }
 if (fabs(syncdetStats[3]) >= SYNCDET_RMS_WACKO) {
   printw("wacko");
 } else {
   printw("%5.2f",syncdetStats[3]);
 }
 standend();
 printw("  min/max=");
 if (fabs(syncdetStats[4]) >= SYNCDET_RMS_WACKO) {
   printw("wacko/");
 } else {
   printw("%.0f/",syncdetStats[4]);
 }
 if (fabs(syncdetStats[5]) >= SYNCDET_RMS_WACKO) {
   printw("wacko");
 } else {
   printw("%.0f",syncdetStats[5]);
 }
 move(21,2);

 /*
 rms = rm_read(*antennaNumber, "RM_CONT_DET_MUWATT_V2_F", dummyFloat2);
 printw("v2f=%.3fuW ",dummyFloat2[0]);
 */
 addstr("SYNCDET_IF1:");
 move(21,14);
 if ((syncdet_channels[0] > SYNCDET_CHANNELS_WACKO_LOW) && (syncdet_channels[0] < SYNCDET_CHANNELS_WACKO_HIGH)) {
   printw("%6.3fuW", syncdet_channels[0]);
 } else {
  printw("  Wacko ");
 }
 rms = rm_read(*antennaNumber, "RM_CONT_DET_MUWATT_V2_F", dummyFloat2);
 printw("  v2f_IF1: %.3fuW ",dummyFloat2[0]);

 rms = rm_read(*antennaNumber, "RM_TILT_CORRECTION_FLAG_L",&tiltCorrFlag);
 rms = rm_read(*antennaNumber, "RM_TILT_VOLTS_V4_F",tiltVolts);
 rms = rm_read(*antennaNumber, "RM_TILT_AZOFF_ARCSEC_D", &tiltAzoff);
 rms = rm_read(*antennaNumber, "RM_TILT_ELOFF_ARCSEC_D", &tiltEloff);
 rms = rm_read(*antennaNumber, "RM_AFT_FOREWARD_TILT_UPPER_ARCSEC_D", 
				&tiltSX);
 rms = rm_read(*antennaNumber, "RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D", 
				&tiltSY);
 rms = rm_read(*antennaNumber, "RM_AFT_FOREWARD_TILT_UPPER_CELESTIAL_D", 
				&tiltGX);
 rms = rm_read(*antennaNumber, "RM_LEFT_RIGHT_TILT_UPPER_CELESTIAL_D", 
				&tiltGY);
 rms = rm_read(*antennaNumber, "RM_PMAZTILT_AZOFF_D", &pmaztiltAzoff);
 rms = rm_read(*antennaNumber, "RM_PMAZTILT_ELOFF_D", &pmaztiltEloff);
 move(13,42);
#ifdef LINUX
  if(colorFlag) {
  pair_content(pair,&foreground,&background);
  init_pair(1,COLOR_CYAN,background);
  attron(COLOR_PAIR(1));
  }
#endif
 printw("TILTX(V) = ");
 if (fabs(tiltVolts[0] ) >=1000) {
    printw("wacko");
 } else {
   printw("%+6.2f",tiltVolts[0]);
 }
 printw("  TILTY(V) = ");
 if (fabs(tiltVolts[1] ) >=1000) {
    printw("wacko");
 } else {
   printw("%+6.2f",tiltVolts[1]);
 }
 move(16,42);
 printw("TILTEL(\") = ");
 if (fabs(tiltEloff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltEloff);
 }
 printw("  TILTAZ(\") = ");
 if (fabs(tiltAzoff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltAzoff);
 }
 move(17,42);
 printw("pTLTEL(\")= ");
 if (fabs(pmaztiltEloff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",pmaztiltEloff);
 }
 printw("   pILTAZ(\")= ");
 if (fabs(pmaztiltAzoff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",pmaztiltAzoff);
 }

/*Difference between tiltmeter and model tilt offsets */
datiltAzoff=tiltAzoff-pmaztiltAzoff;
datiltEloff=tiltEloff-pmaztiltEloff;
move(18,42);
 printw("DtEloff(\")= ");
 if (fabs(datiltEloff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",datiltEloff);
 }
 printw("  DtAzoff(\")= ");
 if (fabs(datiltAzoff) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",datiltAzoff);
 }



 move(19,42);
 printw("TILTCORRECTION: %d",tiltCorrFlag);

 move(14,42);
 printw("TILTSX(\") = ");
 if (fabs(tiltSX) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltSX);
 }
 printw("  TILTSY(\") = ");
 if (fabs(tiltSY) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltSY);
 }

 move(15,42);
 printw("TILTGX(\") = ");
 if (fabs(tiltGX) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltGX);
 }
 printw("  TILTGY(\") = ");
 if (fabs(tiltGY) >= 1000) {
   printw("wacko");
 } else {
   printw("%+5.1f",tiltGY);
 }
 if(colorFlag) {
 standend();
 }

 move(22,2);
 addstr("SYNCDET_IF2:");
 move(22,14);
 if ((syncdet_channels[1] > SYNCDET_CHANNELS_WACKO_LOW) && (syncdet_channels[1] < SYNCDET_CHANNELS_WACKO_HIGH)) {
   printw("%6.3fuW", syncdet_channels[1]);
 } else {
   printw("  Wacko ");
 }
 rms = rm_read(*antennaNumber, "RM_CONT_DET_MUWATT_V2_F", dummyFloat2);
 printw("  v2f_IF2: %.3fuW ",dummyFloat2[1]);

 rms = rm_read(*antennaNumber, "RM_TILT_COEFFICIENTS_V6_D", tiltCoeff);
move(11, 42);
printw("HOME %3s CHOP %3s ",
	homeStateStrings[statusBits[P0]],
       XYZorChopStateStrings[statusBits[P3]]);

 if ((statusBits[P20] > 4)) {
   printw("XYZ wac");
 } else {
   printw("XYZ %3s",XYZorChopStateStrings[statusBits[P20]]);
 }
 /*
 box(stdscr, '|','-');
 */
 rms = dsm_read("hal9000","DSM_VVM1_LOCK_STATUS_S", &vvmlock1, &timestamp);
 rms = dsm_read("hal9000","DSM_VVM2_LOCK_STATUS_S", &vvmlock2, &timestamp);
 if (rms != 0) {
   /*   move(0,10);
   printw("dsm error = %d ",rms);
   */
 } else {
   if (unixTime-timestamp < 15 && unixTime-timestamp > -5) {
     move(0,10);
     printw(" VectorVoltMeter 1: ");
     if (vvmlock1==1) {
       printw("locked     ");
     } else {
       standout();
       printw("unlocked");
       standend();
       printw("   ");
     }
     printw("VectorVoltMeter 2: ");
     if (vvmlock2==1) {
       printw("locked ");
     } else {
       standout();
       printw("unlocked");
       standend();
       printw(" ");
     }
   }
 }
refresh();
}

void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign)
{
	double fmt;
	double absfx;

	if(*fx<0.) {
		absfx=-*fx;
		*dec_sign=-1;
		}
	if(*fx>=0.) {
		absfx=*fx;
		*dec_sign=1;
		}
    *fh = (int)absfx;
    fmt = (absfx - *fh) * 60.;
    *fm = (int) fmt;
    *fs = (fmt - *fm) * 60.;
    if (*fx < 0.) {
	*fh = -(*fh);
    }
}

void af(int *i,char s[2])
{
int j,k,l;
if(*i<0)*i=-*i;
j=48+*i;
if(*i<10) {s[0]='0';s[1]=(char)j;}
if(*i>=10){
k=*i%10;
l=(*i-k)/10;
j=48+k;
s[1]=j;
j=48+l;
s[0]=j;
}
}

void ComputeSolarRADec(double tjd_disp) {
  double T, argument; 
  double Lo;     /* geometric mean longitude, degrees */
  double L;      /* true longitude, degrees */
  double M;      /* mean anomaly, degrees */
  double Mrad;   /* mean anomaly, radians */
  double e;      /* eccentricity of Earth orbit */
  double C;      /* equation of the center, degrees */
  double nu;     /* true anomaly, radians */
  double Omega;  /* radians */
  double lambda; /* apparent longitude */
  double R;      /* radius vector */
  double epsilon0; /* mean obliquity of the ecliptic (degrees) */
  double epsilon;  /* true obliquity of the ecliptic (radians) */

  T = (tjd_disp - 2451545.0) / 36525.0;
  Lo = 280.46646 + 36000.76983*T + 0.0003032*T*T;
  M = 357.52911 + 35999.05029*T - 0.0001537*T*T;
#define DEG_TO_RAD (atan(1.0)/45.0)
#define RAD_TO_DEG (45.0/atan(1.0))
  Mrad = M * DEG_TO_RAD;
  e = 0.016708634 - 0.000042037*T - 0.0000001267*T*T;
  C = (1.914602 - 0.004817*T - 0.000014*T*T) * sin(Mrad) +
      (0.019993 - 0.000101*T) * sin(2*Mrad) + 0.000289*sin(3*Mrad);
  L = Lo + C;
  nu = DEG_TO_RAD*(M + C);
  R = 1.000001018 * (1-e*e) / (1 + e*cos(nu));
  Omega = DEG_TO_RAD*(125.04 - 1934.136*T);
  lambda = DEG_TO_RAD*(L - 0.00569 - 0.00478 * sin(Omega));  
  epsilon0 = (84381.448 - 46.8150*T - 0.00059*T*T + 0.001813*T*T*T) / 3600.;
  epsilon = (epsilon0 + 0.00256 * cos(Omega)) * DEG_TO_RAD;
#define RAD_TO_HRS (3.0/atan(1.0))
  solarPosition.rightAscension = RAD_TO_HRS*atan2(cos(epsilon)*sin(lambda), cos(lambda));
  if (solarPosition.rightAscension < 0) solarPosition.rightAscension += 24.0;
  argument = sin(epsilon) * sin(lambda);
  /* if (fabs(argument <= 1)) */ 
  solarPosition.declination = RAD_TO_DEG*asin(argument);
}



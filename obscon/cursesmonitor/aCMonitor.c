#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "dDSCursesMonitor.h" 
#define SLEEP_PERIOD 5
#define CRATES_PER_ARRAY 12
#include "monitor.h"
#include "upspage.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0

#define DHADT  (2.0 * M_PI / 86400.00)

void printVaultDoorStatus(char door,int delay);

struct ac_status {
  time_t last_read;  /* when did we last read the thing? */
 
  /********************************************************************/
  /* a negative value for any variable means either 1) no information */
  /* has been received from the AC for that monitor point, or 2) the  */
  /* information most recently received is not understood             */
  /********************************************************************/
 
  float digital_room_flow;        /* 0 to about 600 CFM */
  float digital_room_vent;        /* 0 to 100% open */
  float digital_room_duct_heater; /* 4mA=0watts to 20.0mA=maximum */
 
  float analog_room_flow;        /* 0 to about 600 CFM */
  float analog_room_vent;        /* 0 to 100% open */
  float analog_room_duct_heater; /* 4mA=0watts to 20.0mA=maximum */
 
  /* digital racks */
  /* array index is the digital rack number, 1-8 */
  char digital_rack_controlled[9];       /* 0=off, 1=on */
  char digital_rack_airflowswitch[9];    /* 0=off, 1=on */
  char digital_rack_smoke[9];            /* 0=no smoke, 1=smoke */
  float digital_rack_return_air_temp[9]; /* deg F */
  float digital_rack_vent_setting[9];    /* 0% to 100% open */
 
  /* analog rack */
  /* array index is the analog rack number, 1-13 */
  char analog_rack_controlled[14];       /* 0=off, 1=on */
  char analog_rack_airflowswitch[14];    /* 0=off, 1=on */
  char analog_rack_smoke[14];            /* 0=no smoke, 1=smoke */
  float analog_rack_return_air_temp[14]; /* deg F */
  float analog_rack_vent_setting[14];    /* 0% to 100% open */
 
  /* air filters; array index is filter number, 1-3 */
  char air_filter_dirty[4];  /* 0=clean, 1=dirty */
 
  /* supply side */
  char supply_fan_enable;   /* 0=off, 1=start */
  char supply_fan_started;  /* 0=false,1=true  (feedback to enable) */
  char supply_duct_smoke;   /* 0=no smoke, 1=smoke */
  float supply_fan_speed;   /* 4.0 to 20.0 mA */
  float supply_fan_flow;    /* 0 to 16000 CFM */
  float sf_duct_static_pres_1; /* inches of h2o */
  float sf_duct_static_pres_2; /* inches of h2o */
 
  /* return side */
  char return_fan_enable;   /* 0=off, 1=start */
  char return_fan_started;  /* 0=false,1=true  (feedback to enable) */
  char return_duct_smoke;   /* 0=no smoke, 1=smoke */
  float return_fan_speed;   /* 4.0 to 20.0 mA */
  float return_fan_flow;    /* 0 to 16000 CFM */
 
  /* compressors */
  char compressor_1_stage_A;   /* 0=off,  1=on  */
  char compressor_1_stage_B;   /* 0=off,  1=on  */
  char compressor_2_stage_A;   /* 0=off,  1=on  */
  char compressor_2_stage_B;   /* 0=off,  1=on  */
 
  /* ACU */
  float supply_air_temp;     /* deg F */
  float dx_leaving_air_temp; /* deg F */
 
  float mixed_air_temp;      /* deg F */
  float return_air_temp;     /* deg F */
  float outside_air_temp;    /* deg F */
 
  float supply_air_rel_hum;  /* 0.0 to 100.0% */
  float return_air_rel_hum;  /* 0.0 to 100.0% */
  float outside_air_rel_hum; /* 0.0 to 100.0% */
 
 
  /* miscellaneous */
  char humidifier;           /* 0=off, 1=on */
  float duct_heater_control; /* 4.0 to 20.0 mA */
  /* 4mA=0 watts, 20.0mA=maximum watts */
  float outside_air_vents;   /* (AC economizer?) 0-100% open */
};
 
void aCDisplay(int count)
{
  int curTime;
  static int firstCall = TRUE;
  int s, i;
  struct timespec tp;
  long powerpc_chip_temps[CRATES_PER_ARRAY+1];
  time_t timestamp, ppcTimestamp, rightnow;
  char door;
  int delay;
  char shutdown_message[100];
  float analogRoomTemp;
  float lowTempLimit,highTempLimit;
  static float hiloLowTempLimit,hiloHighTempLimit;
  short lowTempAlarmStatus=0;
  short highTempAlarmStatus=0;
  static int hiloTimestamp;
  static float hiloComputerRoomTemp;
  FILE *tempfd;
  static int idleMinutes = 0;
  struct ac_status acs;
  static dsm_structure aCStatus;

  if (firstCall) {
    s = dsm_structure_init(&aCStatus, "CORR_PACU_STATUS_X");
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "dsm_structure_init(&aCStatus, CORR_PACU_STATUS_X)");
      return;
    }
    firstCall = FALSE;
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
  s = dsm_read("corcon",
	       "CORR_PACU_STATUS_X", 
	       (char *)&aCStatus,
	       NULL);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }

  s = dsm_read("corcon",
	       "DSM_CORCON_HAL_CORR_SHUTDOWN_MESSAGE_C80", 
	       shutdown_message,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  s = dsm_read("corcon", 
	       "DSM_CORCON_HAL_CRATE_PPC_TEMPS_V12_L", 
	       powerpc_chip_temps,
	       &ppcTimestamp);
  if(s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read(THERMISTORS)");
    exit(1);
  }
  move(0,0);
  s = dsm_structure_get_element(&aCStatus, "TIME_LAST_READ_L", &acs.last_read);
  printw("Correlator Environment Monitor, last updated %s",
	 asctime(gmtime(&(acs.last_read))));
  move(1,0);
  printw("--------------------------------------------------------------------------------");
  move(2,0);
  printw("Digital Rack   |   1    2    3    4    5    6    7    8");
  move(3,0);
  s = dsm_structure_get_element(&aCStatus, "DIGITAL_RACK_RETURN_AIR_TEMP_V9_F", acs.digital_rack_return_air_temp);
  printw("   Temp. (F)   | %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f",
	 acs.digital_rack_return_air_temp[1],
	 acs.digital_rack_return_air_temp[2],
	 acs.digital_rack_return_air_temp[3],
	 acs.digital_rack_return_air_temp[4],
	 acs.digital_rack_return_air_temp[5],
	 acs.digital_rack_return_air_temp[6],
	 acs.digital_rack_return_air_temp[7],
	 acs.digital_rack_return_air_temp[8]);
  move(4,0);
  s = dsm_structure_get_element(&aCStatus, "DIGITAL_RACK_SMOKE_V9_B", acs.digital_rack_smoke);
  printw("   smoke       |   %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d",
	 acs.digital_rack_smoke[1],
	 acs.digital_rack_smoke[2],
	 acs.digital_rack_smoke[3],
	 acs.digital_rack_smoke[4],
	 acs.digital_rack_smoke[5],
	 acs.digital_rack_smoke[6],
	 acs.digital_rack_smoke[7],
	 acs.digital_rack_smoke[8]);
  move(5,0);
  s = dsm_structure_get_element(&aCStatus, "DIGITAL_RACK_VENT_SETTING_V9_F", acs.digital_rack_vent_setting);
  printw("   vent (%%)    | %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f",
	 acs.digital_rack_vent_setting[1],
	 acs.digital_rack_vent_setting[2],
	 acs.digital_rack_vent_setting[3],
	 acs.digital_rack_vent_setting[4],
	 acs.digital_rack_vent_setting[5],
	 acs.digital_rack_vent_setting[6],
	 acs.digital_rack_vent_setting[7],
	 acs.digital_rack_vent_setting[8]);
  move(6,0);
  printw("--------------------------------------------------------------------------------");
  move(7,0);
  printw("Analog Rack    |   1    2    3    4    5    6    7    8    9   10   11   12   13");
  move(8,0);
  s = dsm_structure_get_element(&aCStatus, "ANALOG_RACK_RETURN_AIR_TEMP_V14_F", acs.analog_rack_return_air_temp);
  printw("   Temp. (F)   | %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f",
	 acs.analog_rack_return_air_temp[1],
	 acs.analog_rack_return_air_temp[2],
	 acs.analog_rack_return_air_temp[3],
	 acs.analog_rack_return_air_temp[4],
	 acs.analog_rack_return_air_temp[5],
	 acs.analog_rack_return_air_temp[6],
	 acs.analog_rack_return_air_temp[7],
	 acs.analog_rack_return_air_temp[8],
	 acs.analog_rack_return_air_temp[9],
	 acs.analog_rack_return_air_temp[10],
	 acs.analog_rack_return_air_temp[11],
	 acs.analog_rack_return_air_temp[12],
	 acs.analog_rack_return_air_temp[13]);
  move(9,0);
  s = dsm_structure_get_element(&aCStatus, "ANALOG_RACK_SMOKE_V14_B", acs.analog_rack_smoke);
  printw("   smoke       |   %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d    %1d",
	 acs.analog_rack_smoke[1],
	 acs.analog_rack_smoke[2],
	 acs.analog_rack_smoke[3],
	 acs.analog_rack_smoke[4],
	 acs.analog_rack_smoke[5],
	 acs.analog_rack_smoke[6],
	 acs.analog_rack_smoke[7],
	 acs.analog_rack_smoke[8],
	 acs.analog_rack_smoke[9],
	 acs.analog_rack_smoke[10],
	 acs.analog_rack_smoke[11],
	 acs.analog_rack_smoke[12],
	 acs.analog_rack_smoke[13]);
  move(10,0);
  s = dsm_structure_get_element(&aCStatus, "ANALOG_RACK_VENT_SETTING_V14_F", acs.analog_rack_vent_setting);
  printw("   vent (%%)    | %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f  %3.0f",
	 acs.analog_rack_vent_setting[1],
	 acs.analog_rack_vent_setting[2],
	 acs.analog_rack_vent_setting[3],
	 acs.analog_rack_vent_setting[4],
	 acs.analog_rack_vent_setting[5],
	 acs.analog_rack_vent_setting[6],
	 acs.analog_rack_vent_setting[7],
	 acs.analog_rack_vent_setting[8],
	 acs.analog_rack_vent_setting[9],
	 acs.analog_rack_vent_setting[10],
	 acs.analog_rack_vent_setting[11],
	 acs.analog_rack_vent_setting[12],
	 acs.analog_rack_vent_setting[13]);
  move(11,0);
  printw("--------------------------------------------------------------------------------");
  move(12,0);
  s = dsm_structure_get_element(&aCStatus, "COMPRESSOR_1_STAGE_A_B", &acs.compressor_1_stage_A);
  printw("Compressor #1 stage A: ");
  if (acs.compressor_1_stage_A)
    printw("ON  ");
  else
    printw("OFF ");
  s = dsm_structure_get_element(&aCStatus, "COMPRESSOR_1_STAGE_B_B", &acs.compressor_1_stage_B);
  printw("stage B: ");
  if (acs.compressor_1_stage_B)
    printw("ON  ");
  else
    printw("OFF ");
  s = dsm_structure_get_element(&aCStatus, "COMPRESSOR_2_STAGE_A_B", &acs.compressor_2_stage_A);
  printw(" Compressor #2 stage A: ");
  if (acs.compressor_2_stage_A)
    printw("ON  ");
  else
    printw("OFF ");
  s = dsm_structure_get_element(&aCStatus, "COMPRESSOR_2_STAGE_B_B", &acs.compressor_2_stage_B);
  printw("stage B: ");
  if (acs.compressor_2_stage_B)
    printw("ON  ");
  else
    printw("OFF ");
  move(13,0);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_FAN_ENABLE_B", &acs.supply_fan_enable);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_FAN_STARTED_B", &acs.supply_fan_started);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_FAN_SPEED_F", &acs.supply_fan_speed);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_FAN_FLOW_F", &acs.supply_fan_flow);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_DUCT_SMOKE_B", &acs.supply_duct_smoke);
  s = dsm_structure_get_element(&aCStatus, "SF_DUCT_STATIC_PRES_1_F", &acs.sf_duct_static_pres_1);
  s = dsm_structure_get_element(&aCStatus, "SF_DUCT_STATIC_PRES_2_F", &acs.sf_duct_static_pres_2);
  printw("Supply Fan/Dct | en=%d  start=%d   spd=%4.1f mA  fl=%4.0f cfm  smk=%d  p1=%3.1f p2=%3.1f",
	 acs.supply_fan_enable,
	 acs.supply_fan_started,
	 acs.supply_fan_speed,
	 acs.supply_fan_flow,
	 acs.supply_duct_smoke,
	 acs.sf_duct_static_pres_1,
	 acs.sf_duct_static_pres_2);
  move(14,0);
  s = dsm_structure_get_element(&aCStatus, "RETURN_FAN_ENABLE_B", &acs.return_fan_enable);
  s = dsm_structure_get_element(&aCStatus, "RETURN_FAN_STARTED_B", &acs.return_fan_started);
  s = dsm_structure_get_element(&aCStatus, "RETURN_FAN_SPEED_F", &acs.return_fan_speed);
  s = dsm_structure_get_element(&aCStatus, "RETURN_FAN_FLOW_F", &acs.return_fan_flow);
  s = dsm_structure_get_element(&aCStatus, "RETURN_DUCT_SMOKE_B", &acs.supply_duct_smoke);
  printw("Return Fan/Dct | en=%d  start=%d   spd=%4.1f mA  fl=%4.0f cfm  smk=%d",
	 acs.return_fan_enable,
	 acs.return_fan_started,
	 acs.return_fan_speed,
	 acs.return_fan_flow,
	 acs.supply_duct_smoke);
  move(15,0);
  printw("--------------------------------------------------------------------------------");
  move(16,0);
  s = dsm_structure_get_element(&aCStatus, "SUPPLY_AIR_TEMP_F", &acs.supply_air_temp);
  s = dsm_structure_get_element(&aCStatus, "DX_LEAVING_AIR_TEMP_F", &acs.dx_leaving_air_temp);
  s = dsm_structure_get_element(&aCStatus, "MIXED_AIR_TEMP_F", &acs.mixed_air_temp);
  s = dsm_structure_get_element(&aCStatus, "RETURN_AIR_TEMP_F", &acs.return_air_temp);
  s = dsm_structure_get_element(&aCStatus, "OUTSIDE_AIR_TEMP_F", &acs.outside_air_temp);
  printw("air Temps. (F) | supl=%3.0f  dx=%3.0f  mixed=%3.0f  ret=%3.0f  outside=%0.1f",
	 acs.supply_air_temp,
	 acs.dx_leaving_air_temp,
	 acs.mixed_air_temp,
	 acs.return_air_temp,
	 acs.outside_air_temp);
  move(17,0);
  printw("********************************************************************************");
  move(18,0);
  printw("Corr. crate    |   1   2   3   4   5   6   7   8   9  10  11  12");
  move(19,0);
  printw("CPU Temp. (C)  |");
  for(i = 1; i <= 12; i++) {
    if(powerpc_chip_temps[i-1] == 99999)
      printw(" ---");
    else
      printw(" %3ld", powerpc_chip_temps[i-1]);
  }
  move(20,0);
  printw("last update %s", asctime(gmtime(&ppcTimestamp)));

  curTime = time((long *)0);
  move(20,40);
  printw("Vault doors: upper="); 
  s = dsm_read("colossus", 
	       "DSM_UPPER_VAULT_DOOR_STATUS_B", 
	       &door,
	       &ppcTimestamp);
  delay = curTime-ppcTimestamp;
  printVaultDoorStatus(door,delay);
  printw(" lower="); 
  s = dsm_read("colossus", 
	       "DSM_LOWER_VAULT_DOOR_STATUS_B", 
	       &door,
	       &ppcTimestamp);
  delay = curTime-ppcTimestamp;
  printVaultDoorStatus(door,delay);


  /* added by Todd */
  s = dsm_read("colossus",
	       "DSM_ANALOG_ROOM_TEMPERATURE_F", 
	       (char *)&analogRoomTemp,
	       &ppcTimestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  s = dsm_read("colossus",
	       "DSM_ANALOG_ROOM_HITEMPLIMIT_F", 
	       (char *)&highTempLimit,
	       &ppcTimestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  s = dsm_read("colossus",
	       "DSM_ANALOG_ROOM_LOTEMPLIMIT_F", 
	       (char *)&lowTempLimit,
	       &ppcTimestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  s = dsm_read("colossus",
	       "DSM_ANALOG_ROOM_LOTEMPALARM_S", 
	       (char *)&lowTempAlarmStatus,
	       &ppcTimestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  s = dsm_read("colossus",
	       "DSM_ANALOG_ROOM_HITEMPALARM_S", 
	       (char *)&highTempAlarmStatus,
	       &ppcTimestamp);
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_read");
    exit(1);
  }
  move(21,0); /* was 21,38 */
  printw("Equipment room temp. = ");
  if (analogRoomTemp > 200 || analogRoomTemp < -40) {
    printw("wacko");
  } else {
    if (highTempAlarmStatus!=0 || lowTempAlarmStatus!=0) {
      standout();
    }
    printw("%.1f C",analogRoomTemp);
    standend();
  }
  printw("   alarm levels: ");
  if (lowTempAlarmStatus) {
    standout();
  }
  if (fabs(lowTempLimit) < 40) {
    printw("<%.0f",lowTempLimit);
  } else {
    standout();
    printw("wac");
    standend();
  }
  standend();
  printw(" or ");
  if (highTempAlarmStatus) {
    standout();
  }
  printw(">");
  if (fabs(highTempLimit) > 9999) {
    printw("wack");
  } else {
    printw("%.0f",highTempLimit); 
  }
  standend();

  printw(" last update: ");
  clock_gettime(CLOCK_REALTIME,&tp);
  rightnow = tp.tv_sec;
  printAgeStandoutN(rightnow,ppcTimestamp,1);
  printw("ago");

  if ((count % 60) == 1) {
    move(22,0);
#define GLOBAL_FILE "/global/hiloComputerRoomTemperature"
    printw("Hilo computer room   = ");
    if ((tempfd = fopen(GLOBAL_FILE,"r")) != NULL) {
      /* example: 19.000000 Celsius at 1143128713 on ulua */
      fscanf(tempfd,"%f %*s %*s %d %*s %*s",&hiloComputerRoomTemp,
	     &hiloTimestamp);
      fscanf(tempfd,"%*s %f %d",&hiloHighTempLimit,&highTempAlarmStatus);
      fscanf(tempfd,"%*s %f %d",&hiloLowTempLimit,&lowTempAlarmStatus);
      fclose(tempfd);
    }
    if (hiloComputerRoomTemp > 200 || hiloComputerRoomTemp < -40) {
      printw("wacko");
    } else {
      if (hiloComputerRoomTemp >= hiloHighTempLimit) {
	standout();
      }
      printw("%.1f C   alarm levels: ",hiloComputerRoomTemp);
      if (lowTempAlarmStatus) {
	standout();
      }
      printw("<%1.0f",hiloLowTempLimit);
      standend();
      printw(" or ");
      if (highTempAlarmStatus) {
	standout();
      }
      printw(">%2.0f",hiloHighTempLimit);
      standend();
    }
  }
  move(22,57);
  printw("last update: ");
  clock_gettime(CLOCK_REALTIME,&tp);
  rightnow = tp.tv_sec;
#if 0
  printw(" %d ",hiloTimestamp);
#else
  printAgeStandoutN(rightnow,hiloTimestamp,1);
  printw("ago");
#endif
  move(23,0);
  printw("Type \"+\" to see the corr_monitor error log.");
  move(23,79);
  refresh();
  return;
}

void printVaultDoorStatus(char door,int delay) {
  if (delay > 60) {
    printw("stale ");
    return;
  }
  switch (door) {
  case 1: 
    printw("closed"); 
    break;
  case 0: 
    /*
    standout();
    */
    printw("open  ");
    standend();
    break;
  default:
    standout();
    printw("wacko ");
    standend();
    break;
  }
}

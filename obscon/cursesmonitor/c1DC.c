#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "rm.h"
#include "monitor.h"
#include "optics.h"
#include "c1DC.h"

#define PRINT_DSM_ERRORS  TRUE
#define FIBER_SOURCE 1
#define NOISE1_SOURCE 2
#define NOISE2_SOURCE 3
#define NUMBER_OF_SMA_ANTENNAS 8
extern int iFLOUnits;

void c1DC(int count, int pageMode, C1DC_FLAGS *flags) {
                      /* [rx] [antenna] [coefficients] */
  static float powerDetectorCalib[3][RM_ARRAY_SIZE][2];
  static dsm_structure statusStructure;
  int rx, ant, s, narg;
  int highlighted = 0;
  int doWeCare[11];
  static int firstTime = TRUE;
  short c1DCInputs[3][NUMBER_OF_SMA_ANTENNAS+1];
  short dummyShort;
  short opAlarms[3][NUMBER_OF_SMA_ANTENNAS+1];
  float voltage;
  float controlVoltages[3][NUMBER_OF_SMA_ANTENNAS+1];
  float controlVoltageRMSs[3][NUMBER_OF_SMA_ANTENNAS+1];
  int nAdjustments[3][9];
  FILE *fp;
  time_t system_time;
  char *ptr, line[100];
  float uWatt, dBM;
  float mW;

  /* the following values are stored in 
     instances/storage/1/configFiles/iFServerInfoNew.txt 
     and are read from there on the first time entering this page. 
  */
  float c1dc_if1_power[MAX_NUMBER_ANTENNAS+1] = {0,6.45,8.10,6.72,7.30,7.82,7.21,6.85,7.29,0,0};
  float c1dc_if2_power[MAX_NUMBER_ANTENNAS+1] = {0,6.47,7.50,6.68,7.38,7.40,8.00,7.20,7.40};

  time_t timestamp;
  time_t calVaneTime;
  short calWheelStatus;
  short opticsBoardPresent;

  if (firstTime==1) {
    s = dsm_structure_init(&statusStructure, "C1DC_STATUS_X");

    /* rx ant coeff */
    powerDetectorCalib[1][1][0] = -0.1972;
    powerDetectorCalib[1][1][1] = 1.6259;
    powerDetectorCalib[1][2][0] = -0.1611;
    powerDetectorCalib[1][2][1] = 1.3817;
    powerDetectorCalib[1][3][0] = -0.2064;
    powerDetectorCalib[1][3][1] = 1.8011;
    powerDetectorCalib[1][4][0] = -0.2103;
    powerDetectorCalib[1][4][1] = 1.7839;
    powerDetectorCalib[1][5][0] = -0.1535;
    powerDetectorCalib[1][5][1] = 1.5082;
    powerDetectorCalib[1][6][0] = -0.2265;
    powerDetectorCalib[1][6][1] = 1.711;
    powerDetectorCalib[1][7][0] = -0.2051;
    powerDetectorCalib[1][7][1] = 1.8353;
    powerDetectorCalib[1][8][0] = -0.1406;
    powerDetectorCalib[1][8][1] = 1.7879;
                   /* rx ant coeff */
    powerDetectorCalib[2][1][0] = -0.1928;
    powerDetectorCalib[2][1][1] = 1.7193;
    powerDetectorCalib[2][2][0] = -0.0584;
    powerDetectorCalib[2][2][1] = 1.5364;
    powerDetectorCalib[2][3][0] = -0.3929;
    powerDetectorCalib[2][3][1] = 1.7878;
    powerDetectorCalib[2][4][0] = -0.3466;
    powerDetectorCalib[2][4][1] = 1.5583;
    powerDetectorCalib[2][5][0] = -0.1946;
    powerDetectorCalib[2][5][1] = 1.195;
    powerDetectorCalib[2][6][0] = -0.6302;
    powerDetectorCalib[2][6][1] = 1.7227;
    powerDetectorCalib[2][7][0] = -0.1824;
    powerDetectorCalib[2][7][1] = 1.2301;
    powerDetectorCalib[2][8][0] = -0.4047;
    powerDetectorCalib[2][8][1] = 1.457;

    firstTime = 0;
    fp = fopen("/otherInstances/storage/1/configFiles/iFServerInfoNew.txt","r");
    if (fp != NULL) {
      do {
	ptr = fgets(line,sizeof(line),fp);
	if (ptr != NULL) {
	  if (present(ptr,"#")) continue;
	  narg = fscanf(fp,"%d",&ant);
	  if (narg < 1) continue;
	  if (ant < 1 || ant > NUMBER_OF_SMA_ANTENNAS) continue;
	  fscanf(fp,"%d %d %*f %f",&ant,&rx,&voltage);
	  if (rx == 1) {
	    c1dc_if1_power[ant] = voltage;
	  } else {
	    c1dc_if2_power[ant] = voltage;
	  }
	}
      } while (ptr!=NULL);
      fclose(fp);
    }
  }
  s = dsm_read("m5", "C1DC_STATUS_X", &statusStructure, &timestamp);
  if (pageMode == C1DC_PAGE_CHECK_ONLY) {
    getAntennaList(doWeCare);
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      flags->flags1[ant] = 0;
      flags->if1power[ant] = 0;
      flags->flags2[ant] = 0;
      flags->if2power[ant] = 0;
    }
    s = dsm_structure_get_element(&statusStructure,
		 "IF_POWER_V3_V9_F",
		 (char *)controlVoltages);
#define C1DC_IF_POWER_CRITERION (1.0)
    rx = 0;
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      if (doWeCare[ant] == 1) {
        rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
	if (fabs(controlVoltages[rx+1][ant] - c1dc_if1_power[ant]) > 
	    C1DC_IF_POWER_CRITERION && dummyShort==AMBIENT_IN) {
	  flags->flags1[ant] = flags->if1power[ant] = 1;
	}
      }
    }
    return;
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
  move(0,0);
  system_time = time(NULL);
  printw("            Correlator First Downconverter Status on %s",
	 asctime(gmtime(&system_time)));
  move(1,0);
  printw("Antenna      1");
  for (ant=2; ant<=NUMBER_OF_SMA_ANTENNAS; ant++) {
    switch (ant) {
    case CSO_ANTENNA_NUMBER:
      printw("      CSO");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("      JCMT");
      break;
    default:
      printw("        %d",ant);
    }
  }
  for (rx = 0; rx < 2; rx++) {
    move(2+rx+rx*10,0);
    printw("Receiver %d ", rx+1);
    if (rx == 0)
      printw("(Low Frequency)");
    else
      printw("(High Frequency)");
    move(3+rx+10*rx,0);
    printw("Input    ");
    s = dsm_structure_get_element(&statusStructure,
		 "INPUTS_V3_V9_S",
		 (char *)c1DCInputs);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "INPUTS_V3_V9_S");
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      switch (c1DCInputs[rx+1][ant]) {
      case FIBER_SOURCE:
	printw("  Fiber  ");
	break;
      case NOISE1_SOURCE:
	printw(" Noise 1 ");
	break;
      case NOISE2_SOURCE:
	printw(" Noise 2 ");
	break;
      default:
	printw(" Unknown ");
      }
    }
    move(4+rx+10*rx,0);
    printw("OptPowerV");
    s = dsm_structure_get_element(&statusStructure,
		 "OP_POWER_V3_V9_F",
		 (char *)controlVoltages);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "OP_POWER_V3_V9_F");
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      printw(" %7.4f ", controlVoltages[rx+1][ant]);
    }
    move(5+rx+10*rx,0);
    printw("OptPow");
#define VOLTS_PER_MILLIWATT 0.65
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      mW = controlVoltages[rx+1][ant]/VOLTS_PER_MILLIWATT;
      switch (iFLOUnits) {
      case IFLO_UNITS_VOLTS:
      case IFLO_UNITS_DBM:
	if (ant == 1) printw("dBm");
	printw(" %+7.3f ",10.0*log10(mW));
	break;
      default:
      case IFLO_UNITS_MILLIWATT:
	if (ant == 1) printw(" mW");
	printw(" %6.3f  ",mW);
	break;
      }
    }
    move(6+rx+10*rx,0);
    printw("Op Alarm ");
    s = dsm_structure_get_element(&statusStructure,
		 "OP_ALARM_V3_V9_S",
		 (char *)opAlarms);
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++)
      if (opAlarms[rx+1][ant])
	printw("ALARM ON ");
      else
	printw("    OK   ");
    move(7+rx+10*rx,0);
    printw("V  Atten ");
    s = dsm_structure_get_element(&statusStructure,
		 "IF_CNTR_VOLT_V3_V9_F",
		 (char *)controlVoltages);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "IF_CNTR_VOLT_V3_V9_F");
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      printw(" %7.4f ", controlVoltages[rx+1][ant]);
    }
    move(8+rx+10*rx,0);
    printw("N Adjust ");
    s = dsm_structure_get_element(&statusStructure,
		 "N_ADJUSTMENTS_V3_V9_L",
		 (char *)nAdjustments);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "IF_N_ADJUSTMENTS_V3_V9_L");
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      if ((nAdjustments[rx+1][ant] < 0) || (nAdjustments[rx+1][ant] >= 1000000))
	printw("  wacko ");
      else
	printw(" %7d ", nAdjustments[rx+1][ant]);
    }
    move(9+rx+10*rx,0);
    printw("IF Power ");
    s = dsm_structure_get_element(&statusStructure,
		 "IF_POWER_V3_V9_F",
		 (char *)controlVoltages);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS) {
	dsm_error_message(s, "IF_POWER_V3_V9_F");
      }
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
      printw(" ");
      if (fabs(controlVoltages[rx+1][ant] - c1dc_if1_power[ant]) > 
	  C1DC_IF_POWER_CRITERION && dummyShort==AMBIENT_IN) {
	standout();
	highlighted++;
      }
      if ((controlVoltages[rx+1][ant] <= -10.0e10) ||
	  (controlVoltages[rx+1][ant] >= 10.0e10)) {
	printw("  wacko ");
      } else {
	switch (iFLOUnits) {
	case IFLO_UNITS_DBM:
	  uWatt = powerDetectorCalib[rx+1][ant][1]*controlVoltages[rx+1][ant] +
	    powerDetectorCalib[rx+1][ant][0];
	  dBM = 10*log10(uWatt)-30;
	  printw("%4.1fdBm", dBM);
	  break;
	case IFLO_UNITS_MILLIWATT:
	  uWatt = powerDetectorCalib[rx+1][ant][1]*controlVoltages[rx+1][ant] +
	    powerDetectorCalib[rx+1][ant][0];
	  printw("%5.2fuW ", uWatt);
	  break;
	default:
	case IFLO_UNITS_VOLTS:
	  printw("%6.3fV ", controlVoltages[rx+1][ant]);
	  break;
	}
      }
      standend();
    }
    move(10+rx+10*rx,0);
    printw("IF RMS   ");
    s = dsm_structure_get_element(&statusStructure,
		 "IF_POWER_RMS_V3_V9_F",
		 (char *)controlVoltageRMSs);
    if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS) {
	dsm_error_message(s, "IF_POWER_RMS_V3_V9_F");
      }
    }
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      printw(" ");
      if (controlVoltageRMSs[rx+1][ant] > 0.5)
	standout();
      if ((controlVoltageRMSs[rx+1][ant] <= 10.0e10) ||
	  (controlVoltageRMSs[rx+1][ant] >= 10.0e10)) {
	printw("  wacko ");
      } else {
	switch (iFLOUnits) {
	case IFLO_UNITS_DBM:
	  uWatt = powerDetectorCalib[rx+1][ant][1]*controlVoltageRMSs[rx+1][ant] +
	    powerDetectorCalib[rx+1][ant][0];
	  dBM = 10*log10(uWatt)-30;
	  printw("%4.1fdBm", dBM);
	  break;
	case IFLO_UNITS_MILLIWATT:
	  uWatt = powerDetectorCalib[rx+1][ant][1]*controlVoltageRMSs[rx+1][ant] +
	    powerDetectorCalib[rx+1][ant][0];
	  printw("%5.2fuW ", uWatt);
	  break;
	default:
	case IFLO_UNITS_VOLTS:
	  printw("%6.3fV ", controlVoltageRMSs[rx+1][ant]);
	  break;
	}
      }
      standend();
    }
    move(11+rx+10*rx,0);
    printw("TargetIF ");
    for (ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
      if (rx == 0) {
	printw("  %.3fV ",c1dcTargetIF1[ant]);
      } else {
	printw("  %.3fV ",c1dcTargetIF2[ant]);
      }
    }
    /*
    move(11+rx+10*rx,0);
    printw("C2DC PS  ");
    */
    /*
    if (rx == 0) {
      if (highlighted > 0) {
	move(12+10*rx,0);
	printw(" When the hotload is in, if any of the 'IF Power' fields are highlighted, then");
	move(13+10*rx,0);
        printw(" you should run setIFLevels.\n");
      }
    }
    */
    if (rx==0) {
      move(12,0);
      printw("Cal Vane  ");
      for(ant = 1; ant <= NUMBER_OF_SMA_ANTENNAS; ant++) {
	if (deadAntennas[ant]) {
	  /*	  if (ant==1) {*/
	    printw(" -----   ");
	    /*	  }*/
	} else {
	  rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",&opticsBoardPresent);
	  if (opticsBoardPresent == 0) {
	    printw("NoBoard");
	    if (ant < NUMBER_OF_SMA_ANTENNAS) {
	      addstr("  ");
	    }
	  } else {
	    printw(" ");
            rm_read(ant,"RM_CALIBRATION_WHEEL_S",&calWheelStatus);
	    rm_read(ant,"RM_CALIBRATION_WHEEL_TIMESTAMP_L",&calVaneTime);
	    rm_read(ant,"RM_UNIX_TIME_L",&timestamp);
	    if (timestamp>(calVaneTime+CAL_VANE_STALE)) {
	      standout();
	      printw("stale");
	    } else {
	      printCalWheelStatus(calWheelStatus);
	    }
	    standend();
	    addstr("   ");
	  }
	}
      }
    }
  }
  move(23,18);
  printw(" --- Type \"#\" to change IF Power units ---");
  move(0,79);
  refresh();
}

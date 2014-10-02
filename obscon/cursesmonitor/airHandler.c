#define ANTENNA_NUMBER_WITH_FLUKE_HYDRA 6
#define DSM_WEATHER_HOST "colossus"
#include <stdio.h>
#include <curses.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <rm.h>
#include <dsm.h>
#include "monitor.h"
#include "commonLib.h"

extern dsm_structure upsStructure[MAX_NUMBER_ANTENNAS+1];
extern dsm_structure smaWeather, jcmtWeather, irtfWeather, ukirtWeather, cfhtWeather;
extern dsm_structure csoWeather, keckWeather, vlbaWeather ,subaruWeather, uh88Weather;
extern int airHandlerUnits;
extern int printAge(long, long);
extern char *toddtime(time_t *, char *str);
extern char *hsttime(time_t *, char *str);
extern void checkStatus(int status, char *string);
extern void checkDSMStatus(int status, char *string);
float celsiusToFahr2(float temperature);
int airHandlerPresent(int antennaNumber);
void printAirHandlerTemperatureUnits(void);
double celsiusToKelvin2(double c);
double kelvinToCelsius2(double c);
float findMedian(float *array, int elements);

void airHandler(int count, int *rm_list) 
{
  int rm_status,i,dsm_status,avoid;
  char acc[5];
  int ant, antennaNumber;
  long longvalue;
  short shortvalue;
  float floatvalue, average, setPointC[9], floatvalue2, insideMix, outsideMix;
  float postHeaterTemp,preHeaterTemp;
  static int firstTime = 1;
  long smaTimestamp,jcmtTimestamp,subaruTimestamp,ukirtTimestamp,cfhtTimestamp;
  long uh88Timestamp,keckTimestamp,vlbaTimestamp;
  float humidity[7];
  float temperature[7];
  int avgct;
  long rightnow, timevalue[11], tune6timevalue[11];
  time_t timestamp;
  time_t system_time;
  int doWeCare[11], tempctr, humidctr;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  float outsideHumidity, outsideTemperature,insideTemperatureC[9];
  float expectedHumidity;
  float antennaThermometers[20];
  float ambientLoadTemperature[11];

  if (firstTime == 1) {
    firstTime = 0;
  }
  if ((count % 20) == 1) {
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
  ant = 0;
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  for (i=1; i<11; i++) {
    rm_status = rm_read(i,"RM_AIR_HANDLER_TIMESTAMP_L",&timevalue[i]);
    rm_status = rm_read(i, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &tune6timevalue);
  }
  printw("Air blower status on %s UT = %s HST",timeString,timeString2);
  move(1,0);
  printw("Antenna             1      2       3       4       5       6       7       8");
  move(2,0);
  printw("InsideOutsideMix");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) {
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_INSIDE_AIR_DAMPER_F",
		    &floatvalue);
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_OUTSIDE_AIR_DAMPER_F",
		    &floatvalue2);
      if (floatvalue > 15 || floatvalue2 > 11 || floatvalue < 1 || floatvalue2 < 1) {
	printw("  wacko ");
      } else {
#define MAXIMUM_DAMPER_VOLTAGE 9.92
#define MINIMUM_DAMPER_VOLTAGE 1.92
#define DAMPER_RANGE (MAXIMUM_DAMPER_VOLTAGE-MINIMUM_DAMPER_VOLTAGE)
	insideMix  = (floatvalue-MINIMUM_DAMPER_VOLTAGE)*100/DAMPER_RANGE;
	outsideMix = 100-(floatvalue2-MINIMUM_DAMPER_VOLTAGE)*100/DAMPER_RANGE;
	if (outsideMix > 100) {
	  outsideMix = 100;
	}
	if (insideMix > 100) {
	  insideMix = 100;
	}
	if (insideMix < 0) {
	  insideMix = 0;
	}
	if (outsideMix < 0) {
	  outsideMix = 0;
	}
	if (outsideMix>=100) {
	  printw("%3.0f/%3.0f%%",insideMix,outsideMix);
	} else {
	  printw("%3.0f/%2.0f%% ",insideMix,outsideMix);
	}
      }
    }
  }

  move(3,0);
  printw("InsideAirDamper ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) {
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_INSIDE_AIR_DAMPER_F",
		    &floatvalue);
      if (floatvalue > 15) {
	printw("  wacko ");
      } else {
	if (floatvalue >= 10 || floatvalue < 0) {
	  printw(" %.3f ",floatvalue);
	} else {
	  printw("  %.3f ",floatvalue);
	}
      }
    }
  }

  move(4,0);
  printw("OutsideAirDamper");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) {
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_OUTSIDE_AIR_DAMPER_F", &floatvalue);
      if (floatvalue > 15 || floatvalue<0) {
	printw("  wacko ");
      } else {
	if (floatvalue >= 10 || floatvalue < 0) { 
	  printw(" %.3f ",floatvalue);
	} else {
	  printw("  %.3f ",floatvalue);
	}
      }
    }
  }
  move(14,0);
  printw("Rack temp      "); 
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_RACK_TEMPERATURE_F", &floatvalue);
      if (floatvalue >= 100 || floatvalue < -19) {
        printw("  wacko ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }//   printw("CabinSteel Left ");
//   rm_status = rm_read(5, "RM_ANTENNA_THERMOMETERS_V20_F", antennaThermometers);
//   printw("                                 ");
//   if (airHandlerUnits == 0) {
//     printw("%+.2f\n",antennaThermometers[4]);
//   } else {
//     printw("%+.2f\n",celsiusToFahr2(antennaThermometers[4]));
//   }
  move(15,0);
  printw("Enclosure temp ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_ENCLOSURE_TEMPERATURE_F", &floatvalue);
      if (floatvalue >= 100 || floatvalue < -19) {
        printw("  wacko ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }//   printw("CabinSteel Right");
//   printw("                                 ");
//   if (airHandlerUnits == 0) {
//     if (fabs(antennaThermometers[5]) > 100) {
//       printw("wacko");
//     } else {
//       printw("%+.2f\n",antennaThermometers[5]);
//     }
//   } else {
//     if (fabs(antennaThermometers[5]) > 100) {
//       printw("wacko");
//     } else {
//       printw("%+.2f\n",celsiusToFahr2(antennaThermometers[5]));
//     }
//   }

#if 0
  move(14,0);
  printw("CabinTemp Sock ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CABIN_TEMPERATURE_F", &floatvalue);
      insideTemperatureKelvin[antennaNumber] = celsiusToKelvin2(floatvalue);
#define CABIN_TEMP_WACKO_MAX 38.0
#define CABIN_TEMP_WACKO_MIN 0.01
      if (floatvalue > CABIN_TEMP_WACKO_MAX || floatvalue < CABIN_TEMP_WACKO_MIN) {
        printw("  ----- ");
      } else {
	if (airHandlerUnits == 0) {
          printw(" %+6.2f ",floatvalue);
	} else {
          printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }

  move(15,0);
  printw("CabinTempCalVane");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CABIN_TEMPERATURE2_F", &floatvalue);
      if (floatvalue > CABIN_TEMP_WACKO_MAX || floatvalue < CABIN_TEMP_WACKO_MIN) {
        printw("  ----- ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }

  move(16,0);
  printw("CabinTemp456Plat");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CABIN_TEMPERATURE3_F", &floatvalue);
      /*
      if (antennaNumber == ANTENNA_NUMBER_WITH_FLUKE_HYDRA) {
	insideTemperatureKelvin[antennaNumber] = celsiusToKelvin2(floatvalue);
	}
      */
      if (floatvalue > CABIN_TEMP_WACKO_MAX || floatvalue < CABIN_TEMP_WACKO_MIN) {
        printw("  ----- ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }
#endif
#if 0
  move(16,0);
  printw("Diff. Pressure  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_DIFFERENTIAL_PRESSURE_F", &floatvalue);
      if (floatvalue > 12 || floatvalue < -1) {
        printw("  ----- ");
      } else {
        printw(" %5.3f  ",floatvalue);
      }
    }
  }
#endif

  move(6,0);
  /*  printw("Temperature #1  ");*/
  printw("IntakeAir temp ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_INTAKE_TEMPERATURE_F", &floatvalue);
      if (floatvalue >= 100 || floatvalue < -19) {
        printw("  wacko ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }
  move(8,0);
  /* 
  printw("Temperature #2  ");
  */
  printw("PostHeaterTemp ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_SOCK_TEMPERATURE_F", &floatvalue);
      postHeaterTemp = floatvalue;
      if (floatvalue > 100 || floatvalue < -19) {
        printw("  ----- ");
      } else {
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }

  move(9,0);
  /*
  printw("Temperature #3  ");
  */
  printw("Pre-HeaterTemp ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber,
		"RM_AIR_HANDLER_PRE_HEATER_TEMPERATURE_F", &floatvalue);
      preHeaterTemp = floatvalue;
      if (floatvalue > 100 || floatvalue < -19) {
        printw("  wacko ");
      } else {
	if (fabs((double)floatvalue) < 10) {
	  printw(" ");
	}
	if (airHandlerUnits == 0) {
	  printw(" %+5.2f ",floatvalue);
	} else {
	  printw(" %+5.2f ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }

  move(7,0);
  printw("HeaterCmd/DeltaT");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!airHandlerPresent(antennaNumber)) {
      printw("  ----- ");
      continue; 
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {

      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_HEATER_ENABLED_S", &shortvalue);

      if(shortvalue <= 10 && shortvalue >= 0) {
        printw("%3d", shortvalue);
      } else {
	standout();
        printw("wac");
	standend();
      }
      printw("/");
#if 1
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_HEATER_DELTA_T_SEEN_S", &shortvalue);
      if (shortvalue == 1) {
	printw("on? ");
      } else if (shortvalue == 0) {
	printw("off?");
      } else {
	printw("wac?");
      }
#else
      if (postHeaterTemp > preHeaterTemp+2.0) {
	printw("on? ");
      } else {
	printw("off?");
      }
#endif
    }
  }

  move(5,0);
  printw("Servo output V  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else if (airHandlerPresent(antennaNumber)) {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_SERVO_CMD_TO_DAMPERS_F", &floatvalue);
      if (fabs(floatvalue) > 100) {
	printw("  wacko ");
      } else {
	printw(" %6.3f ",floatvalue);
      }
    } else {
      printw("  ----- ");
    }
  }
  move(16,0);
  printw("Diff Pressure   ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    rm_status = rm_read(antennaNumber,
	"RM_AIR_HANDLER_DIFFERENTIAL_PRESSURE_F", &floatvalue);
    if(floatvalue > 2 || floatvalue < 0) {
      printw(" wacko  ");
    } else {
      if(floatvalue > 0.5) {
        standout();
        printw(" %6.3f ",floatvalue);
        standend();
      } else {
        printw(" %6.3f ",floatvalue);
      }
    }
  }
  move(17,0);
  printw("RelativeHumidity");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else if (airHandlerPresent(antennaNumber)) {
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_SOCK_HUMIDITY_F", &floatvalue);
      if (fabs(floatvalue) > 150) {
	printw(" wacko  ");
      } else{
	printw(" %5.1f%% ",floatvalue);
	/*
	if (fabs(floatvalue) < 10) {
	  printw(" ");
	}
	*/
      }
    } else {
      printw("  ----- ");
    }
  }

  move(10,0);
  printw("ThermostatSetPnt");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else if (airHandlerPresent(antennaNumber)) {
      /* written by the program = airHandlerAuxWeather */
      /* can use the RPC command: air -a 6 -c setpoint 16 */
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_SET_POINT_F", &floatvalue);
      setPointC[antennaNumber] = floatvalue;
      if (setPointC[antennaNumber] < 10 || setPointC[antennaNumber] > 25) {
	standout();
      }
      if (airHandlerUnits == 0) {
	printw(" %+6.2f ",setPointC[antennaNumber]);
      } else {
	printw(" %+6.2f ",celsiusToFahr2(setPointC[antennaNumber]));
      }
      standend();
    } else {
      printw("  ----- ");
    }
  }

  move(19,0);
  printw("Latest Update   ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      printw(" ");
      printAge(rightnow,timevalue[antennaNumber]);
    }
  }


  move(13,0);
  printw("UPSAmbientTemp ");
  printAirHandlerTemperatureUnits();
  avoid = 0;
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antennaNumber == avoid) antennaNumber++;
    sprintf(acc,"acc%d",antennaNumber);
    if (antsAvailable[antennaNumber]) {
      dsm_status = call_dsm_read(acc, "UPS_DATA_X", 
				 (char *)&upsStructure[antennaNumber], 
				 &timestamp);
      dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					     "AMBIENT_TEMPERATURE_F", 
					     &floatvalue);
      checkDSMStatus(dsm_status,"dsm_get_element(AMBIENT_TEMPERATURE_F)");
    }
#define UPS_STALE_INTERVAL 120
    if (rightnow-timestamp > UPS_STALE_INTERVAL) {
      printw("  stale ");
    } else {
      if (floatvalue > 300 || floatvalue < 0) {
        printw("  wacko ");
      } else {
	if (airHandlerUnits == 0) {
	  printw("  %+3.0f   ",floatvalue);
	} else {
	  printw("  %+3.0f   ",celsiusToFahr2(floatvalue));
	}
      }
    }
  }

  move(12,0);
  printw("ThermostatFeedbk");
  /* there is no room for the units in this case */
  /*  printAirHandlerTemperatureUnits();*/
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    /*
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    */
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_AMBIENTLOAD_TEMPERATURE_F", &floatvalue);
      ambientLoadTemperature[antennaNumber] = floatvalue;
      rm_status = rm_read(antennaNumber, "RM_AIR_HANDLER_SERVO_CABIN_TEMP_F", &floatvalue);
      checkStatus(rm_status,"rm_read(RM_AIR_HANDLER_SERVO_CABIN_TEMP_F)");
      insideTemperatureC[antennaNumber] = floatvalue;
#define AMBIENT_TEMP_WACKO_MIN -5
#define AMBIENT_TEMP_WACKO_MAX 30
      if (floatvalue >= 100 || 
	  floatvalue <= -10) {
	printw("  wacko ");
      } else {
	if (floatvalue > AMBIENT_TEMP_WACKO_MAX || 
	    floatvalue < AMBIENT_TEMP_WACKO_MIN) {
	  standout();
	}
	if (airHandlerUnits == 0) {
	  printw(" %+6.2f ",floatvalue);
	} else {
	  printw(" %+6.2f ",celsiusToFahr2(floatvalue));
	}
	standend();
      }
    }
  }
  move(11,0);
  printw("FollowingError ");
  printAirHandlerTemperatureUnits();
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else {
      if (!airHandlerPresent(antennaNumber)) {
	printw("  ----- ");
      } else {
	floatvalue = -setPointC[antennaNumber] +
		insideTemperatureC[antennaNumber];
	if (fabs(floatvalue) > 100) {
	  printw("  wacko ");
	} else {
	  if (fabs(floatvalue) > 1) {
	    standout();
	  }
	  if (airHandlerUnits == 0) {
	    if (fabs(floatvalue) >= 10) {
	      printw(" %+5.2f ",floatvalue);
	    } else {
	      printw("  %+5.2f ",floatvalue);
	    }
	  } else {
	    if (fabs(floatvalue*1.8) >= 10) {
	      printw(" %+5.2f ",floatvalue*1.8);
	    } else {
	      printw("  %+5.2f ",floatvalue*1.8);
	    }
	  }
	  if (fabs(floatvalue) > 1) {
	    standend();
	  }
	}
      }
    }
  }

  move(20,0);             
  /*
  printw("            SMA    JCMT   Subaru   UKIRT    CFHT     UH88   Keck   VLBA  Average");
  */
  printw("            SMA    JCMT   Subaru   UKIRT    CFHT     UH88   Keck    VLBA  Median");
#define MAX_TEMP 30
#define MIN_TEMP -20
#define STALE_INTERVAL 600
  move(21,0);             
  printw("WeatherTemp");
  average = avgct = 0;
  rm_status = dsm_read("colossus","SMA_METEOROLOGY_X", &smaWeather, &smaTimestamp);
  rm_status = dsm_read("colossus","JCMT_METEOROLOGY_X", &jcmtWeather, &jcmtTimestamp);
  rm_status = dsm_read("colossus","SUBARU_METEOROLOGY_X", &subaruWeather, &subaruTimestamp);
  rm_status = dsm_read("colossus","UKIRT_METEOROLOGY_X", &ukirtWeather, &ukirtTimestamp);
  rm_status = dsm_read("colossus","CFHT_METEOROLOGY_X", &cfhtWeather, &cfhtTimestamp);
  rm_status = dsm_read("colossus","UH88_METEOROLOGY_X", &uh88Weather, &uh88Timestamp);
  rm_status = dsm_read("colossus","KECK_METEOROLOGY_X", &keckWeather, &keckTimestamp);
  rm_status = dsm_read("colossus","VLBA_METEOROLOGY_X", &vlbaWeather, &vlbaTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read()");
  }
  /*
  dsm_status = call_dsm_read(DSM_WEATHER_HOST, "DSM_SMA_TEMP_F", &floatvalue, &longvalue);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  tempctr=0;
  longvalue = smaTimestamp;
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw("%+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw("%+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&jcmtWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  /*  longvalue = jcmtTimestamp;*/
  rm_status = dsm_structure_get_element(&jcmtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      /* don't use JCMT in the median since meter is inside dome */
      /*  temperature[tempctr++] = floatvalue;*/
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      /*      average += floatvalue; avgct++;*/
    }
  }
  rm_status = dsm_structure_get_element(&subaruWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  /*  longvalue = subaruTimestamp;*/
  rm_status = dsm_structure_get_element(&subaruWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&ukirtWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  /*  longvalue = ukirtTimestamp;*/
  rm_status = dsm_structure_get_element(&ukirtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&cfhtWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  /*  longvalue = cfhtTimestamp;*/
  rm_status = dsm_structure_get_element(&cfhtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&uh88Weather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  /*  longvalue = uh88Timestamp;*/
  rm_status = dsm_structure_get_element(&uh88Weather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > 3600) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&keckWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  rm_status = dsm_structure_get_element(&keckWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > 3600) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      temperature[tempctr++] = floatvalue;
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&vlbaWeather,"TEMP_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TEMP_F)");
  }
  rm_status = dsm_structure_get_element(&vlbaWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale  ");
  } else {
    if (floatvalue > MAX_TEMP || floatvalue < MIN_TEMP) {
      printw(" wacko  ");
    } else {
      if (airHandlerUnits == 0) {
	if (fabs(floatvalue)<10) printw(" ");
	printw(" %+5.2f ",floatvalue);
      } else {
	if (fabs(celsiusToFahr2(floatvalue))<10) printw(" ");
	printw(" %+5.2f ",celsiusToFahr2(floatvalue));
      }
    }
  }
  average /= avgct++;
  if (airHandlerUnits == 0) {
    printw(" %+5.2f ",findMedian(temperature,tempctr));
  } else {
    printw(" %+5.2f ",celsiusToFahr2(findMedian(temperature,tempctr)));
  }
  /*  printw(" %+5.2f ",average);*/
  outsideTemperature = celsiusToKelvin2(average);

  move(22,0);             
  printw("RelHumidity");
  average = avgct = 0;
#define MAX_HUMID 110
#define MIN_HUMID 0
  /*
  dsm_status = call_dsm_read(DSM_WEATHER_HOST, "DSM_SMA_HUMIDITY_F", &floatvalue, &longvalue);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  humidctr = 0;
  /*  longvalue = smaTimestamp;*/
  rm_status = dsm_structure_get_element(&smaWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw(" wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (floatvalue >= 100) {
	printw("%5.0f%%",floatvalue);
      } else {
	printw("%5.1f%%",floatvalue);
      }
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&jcmtWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = jcmtTimestamp;*/
  rm_status = dsm_structure_get_element(&jcmtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw(" stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      /* don't use JCMT in the median since meter is inside dome */
      /*  humidity[humidctr++] = floatvalue;*/
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      /*      average += floatvalue; avgct++;*/
    }
  }
  rm_status = dsm_structure_get_element(&subaruWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = subaruTimestamp;*/
  rm_status = dsm_structure_get_element(&subaruWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&ukirtWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = ukirtTimestamp;*/
  rm_status = dsm_structure_get_element(&ukirtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&cfhtWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = cfhtTimestamp;*/
  rm_status = dsm_structure_get_element(&cfhtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&uh88Weather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = uh88Timestamp;*/
  rm_status = dsm_structure_get_element(&uh88Weather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > 3600) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&keckWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = keckTimestamp;*/
  rm_status = dsm_structure_get_element(&keckWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > 3600) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      humidity[humidctr++] = floatvalue;
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
      average += floatvalue; avgct++;
    }
  }
  rm_status = dsm_structure_get_element(&vlbaWeather,"HUMIDITY_F", &floatvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(HUMIDITY_F)");
  }
  /*  longvalue = vlbaTimestamp;*/
  rm_status = dsm_structure_get_element(&vlbaWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rightnow-longvalue > STALE_INTERVAL) {
    printw("  stale ");
  } else {
    if (floatvalue > MAX_HUMID || floatvalue < MIN_HUMID) {
      printw("  wacko ");
    } else {
      if (fabs(floatvalue)<100) printw(" ");
      printw(" %5.1f%%",floatvalue);
    }
  }
  average /= avgct++;
  printw(" %5.1f%%",findMedian(humidity,humidctr));
  /*  printw(" %5.1f%%",average);*/
  outsideHumidity = average;
  move(18,0);
  printw("ExpectedHumidity");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  ----- ");
    } else if (airHandlerPresent(antennaNumber)) {
#define HoverR (5.0878e+03)
      expectedHumidity = outsideHumidity*exp(HoverR*
      	(1/celsiusToKelvin2(insideTemperatureC[antennaNumber])-
	1/outsideTemperature));
      if (fabs(expectedHumidity) > 150) {
	printw(" wacko  ");
      } else {
	printw(" %5.1f%% ",expectedHumidity);
	/*
	if (fabs(expectedHumidity) < 10) {
	  printw(" ");
	}
	*/
      }
    } else {
      printw("  ----- ");
    }
  }

  move(23,10);
  if (airHandlerUnits == 0) {
    printw("press '+' to switch units to Fahrenheit");
  } else {
    printw("press '+' to switch units to Celsius   ");
  }
  move(23,62);
  printw("without JCMT,VLBA ");
  move (23,0);
  refresh();
}

float findMedian(float *array, int elements) {
  int i,j;
  float swap;

  /* sort into increasing order: array[0] will be < array[1]  */
  for (i=0; i<(elements-1); i++) {
    for (j=0; j<(elements-1); j++) {
      if (array[j]>array[j+1]) {
	swap = array[j];
	array[j] = array[j+1];
	array[j+1] = swap;
      }
    }
  }
  /*
  move(row,0);
  printw("%d: ",elements);
  for (i=0; i<elements; i++) printw("%.1f ",array[i]);
  */
  if ((elements % 2) == 0) {
    /* there are an even number of elements,  4 --> avg between 1&2,  6 --> avg between 2&3 */
    return(0.5*(array[elements/2]+array[(elements/2)-1]));
  } else {
    /* there are an odd number of elements,  3 --> 1,  5 --> 2 */
    return(array[(elements-1)/2]);
  }
}

void noBoard(int ant) {
  printw("No Board");
  if (ant < NUMANTS) {
    addstr(" ");
  }
}

float celsiusToFahr2(float temperature) {
  return(32+(temperature*9.0/5.0));
}

double kelvinToCelsius2(double c) {
  return(c-273.15);
}

double celsiusToKelvin2(double c) {
  return(c+273.15);
}

int airHandlerPresent(int antennaNumber) {
    return(1);
#if 0
    return(antennaNumber != 7);
    if (antennaNumber==6 || antennaNumber==4 || antennaNumber==5 || 
	antennaNumber==1 || antennaNumber==3 || antennaNumber==2) {
      return(1);
    } else {
      return(0);
    }
#endif
}

void printAirHandlerTemperatureUnits(void) {
  if (airHandlerUnits == 0) {
    printw("C");
  } else {
    printw("F");
  }
}

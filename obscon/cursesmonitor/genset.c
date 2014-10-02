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

#define ROACH_NUMBER 8
#define PRINT_DSM_ERRORS  TRUE
#define FIBER_SOURCE 1
#define NOISE1_SOURCE 2
#define NOISE2_SOURCE 3
#define NUMBER_OF_SMA_ANTENNAS 8
extern int iFLOUnits;
extern dsm_structure roachStructure;

void genset(int count) {
                      /* [rx] [antenna] [coefficients] */
  char elementName[100];
  short engineData, engineStarts, engineRuntime;
  short kilowatts, totalWattHours, dummyShort;
  static int firstTime = TRUE;
  int phase, s;
  float dummyFloat, frequency, powerFactor, kVA, kVAReactive, batteryVoltage;
  float oilPressure, oilTemperature, coolantTemperature;
  static dsm_structure statusStructure;
  time_t system_time;
  time_t timestamp;

  time_t Rsystem_time[8];
  time_t ROACHtimestamp;

//  long  ROACHtimestamp[8];
  float amb[8];
  float ppc[8];
  float fpga[8];
  float inlet[8];
  float outlet[8];
  		
  if (firstTime==1) {
    s = dsm_structure_init(&statusStructure, "DSM_IWATCH_DATA_X");
    firstTime = FALSE;
  }
  s = dsm_read("obscon", "DSM_IWATCH_DATA_X", &statusStructure, &timestamp);
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
  printw("GENSET STATUS & INTERIM CORRELATOR ROACH2 TEMPERATURES\non %s",
	 asctime(gmtime(&system_time)));
  move(3,0);
  s = dsm_structure_get_element(&statusStructure,"SERVER_TIMESTAMP_L",(char *)(&system_time));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "SERVER_TIMESTAMP_L");
	printw("Genset Server timestamp: %s", asctime(gmtime(&system_time)));
 move(4,0);
  printw("Genset data                    Phase A          Phase B          Phase C");
  move(5,0);
  printw("Voltage Line to Line         ");
  for (phase = 0; phase < 3; phase++) {
    sprintf(elementName, "VLL%d_S", phase+1);
    s = dsm_structure_get_element(&statusStructure, elementName, &dummyShort);
    if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
      dsm_error_message(s, elementName);
    printw("   %3d           ", dummyShort);
  }
  move(6,0);
  printw("Voltage Line to Neutral      ");
  for (phase = 0; phase < 3; phase++) {
    sprintf(elementName, "VLN%d_S", phase+1);
    s = dsm_structure_get_element(&statusStructure, elementName, &dummyShort);
    if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
      dsm_error_message(s, elementName);
    printw("   %3d           ", dummyShort);
  }
  move(7,0);
  printw("Current                      ");
  for (phase = 0; phase < 3; phase++) {
    sprintf(elementName, "CURRENTL%d_F", phase+1);
    s = dsm_structure_get_element(&statusStructure, elementName, (char *)(&dummyFloat));
    if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
      dsm_error_message(s, elementName);
    printw("   %3.0f           ", dummyFloat);
  }
  move(8,0);
  s = dsm_structure_get_element(&statusStructure, "FREQUENCY_F", (char *)(&frequency));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "FREQUENCY_F");
  s = dsm_structure_get_element(&statusStructure, "POWER_FACTOR_F", (char *)(&powerFactor));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "POWER_FACTOR_F");
  printw("Frequency %5.2f   Power Factor %7.4f", frequency, powerFactor);
  move(9,0);
  s = dsm_structure_get_element(&statusStructure, "KILOWATTS_S", (char *)(&kilowatts));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "KILOWATTS_S");
  s = dsm_structure_get_element(&statusStructure, "KVA_F", (char *)(&kVA));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "KVA_F");
  s = dsm_structure_get_element(&statusStructure, "KILOVARS_F", (char *)(&kVAReactive));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "KILOVARS_F");
  printw("Kilowatts %5d   KVA %5.2f           KVA Reactive %5.2f",
	 kilowatts, kVA, kVAReactive);
  move(10,0);
  s = dsm_structure_get_element(&statusStructure, "ENGINE_DATA_S", &engineData);
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "ENGINE_DATA_S");
  s = dsm_structure_get_element(&statusStructure, "ENGINE_STARTS_S", &engineStarts);
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "ENGINE_STARTS_S");
  s = dsm_structure_get_element(&statusStructure, "ENGINE_RUNTIME_S", &engineRuntime);
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
   dsm_error_message(s, "ENGINE_RUNTIME_S");
   s = dsm_structure_get_element(&statusStructure, "BATTERY_VOLTAGE_F", (char *)(&batteryVoltage));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "BATTERY_VOLTAGE_F");
  printw("Engine RPM %4d   #Starts %d          Battery Voltage %5.2f",
	 engineData, engineStarts, batteryVoltage);
  move(11,0);
  s = dsm_structure_get_element(&statusStructure, "OIL_PRESSURE_F", (char *)(&oilPressure));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "OIL_PRESSURE_F");
  s = dsm_structure_get_element(&statusStructure, "OIL_TEMPERATURE_F", (char *)(&oilTemperature));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "OIL_TEMPERATURE_F");
  s = dsm_structure_get_element(&statusStructure, "COOLANT_TEMPERATURE_F", (char *)(&coolantTemperature));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "COOLANT_TEMPERATURE_F");
  s = dsm_structure_get_element(&statusStructure, "TOTAL_WATT_HOURS_S", (char *)(&totalWattHours));
  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
    dsm_error_message(s, "TOTAL_WATT_HOURS_S");
  printw("Oil Pres. %5.2f  Coolant Temp. %5.2f  Total Mwatt-hours %4d",
	 oilPressure, coolantTemperature, totalWattHours);
// There is something wrong with these two values.
//   move(12,0);
//   printw("Oil Temp. %5.2f  Engine Runtime %4d", oilTemperature, engineRuntime);

// Added for ROACH2 data.
  	move(14,0);
  	Rsystem_time[2] = time(NULL);
  	s = dsm_read("obscon","ROACH2_TEMPS_X", &roachStructure, &ROACHtimestamp);
  	if (s != DSM_SUCCESS)
    	dsm_error_message(s,"dsm_read(ROACH2_TEMPS_X)");
	s = dsm_structure_get_element(&roachStructure, "TIMESTAMP_V8_L", (char *)(&Rsystem_time[0]));
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "TIMESTAMP_V8_L");
	s = dsm_structure_get_element(&roachStructure, "AMBIENT_TEMP_V8_F", &amb[0]);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "AMBIENT_TEMP_V8_F");
  	s = dsm_structure_get_element(&roachStructure, "PPC_TEMP_V8_F", &ppc[0]);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "PPC_TEMP_V8_F");
  	s = dsm_structure_get_element(&roachStructure, "FPGA_TEMP_V8_F", &fpga[0]);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "FPGA_TEMP_V8_F");
  	s = dsm_structure_get_element(&roachStructure, "INLET_TEMP_V8_F", &inlet[0]);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "INLET_TEMP_V8_F");
  	s = dsm_structure_get_element(&roachStructure, "OUTLET_TEMP_V8_F", &outlet[0]);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
  		dsm_error_message(s, "OUTLET_TEMP_V8_F");

	printw("ROACH2 Server timestamp at: %s", asctime(gmtime(&Rsystem_time[2])));
	printw("ROACH2 #   1      2      3      4      5      6      7      8\n");
	printw("ambient  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f\n", amb[0],amb[1],amb[2],amb[3],amb[4],amb[5],amb[6],amb[7]);
 	printw("ppc      %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f\n", ppc[0],ppc[1],ppc[2],ppc[3],ppc[4],ppc[5],ppc[6],ppc[7]);
 	printw("fpga     %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f\n", fpga[0],fpga[1],fpga[2],fpga[3],fpga[4],fpga[5],fpga[6],fpga[7]);
 	printw("inlet    %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f\n", inlet[0],inlet[1],inlet[2],inlet[3],inlet[4],inlet[5],inlet[6],inlet[7]);  
 	printw("outlet   %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f  %5.2f\n", outlet[0],outlet[1],outlet[2],outlet[3],outlet[4],outlet[5],outlet[6],outlet[7]);
// 	printw("timestamp   %d  %d  %d  %d  %d  %d  %d  %d\n", //Rsystem_time[0],Rsystem_time[1],Rsystem_time[2],Rsystem_time[3],Rsystem_time[4],Rsystem_time[5],Rsystem_time[6],Rsystem_time[7]);
 
	move(0,79);
	refresh();
}

#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "monitor.h"
#include "bdc.h" 

#define PRINT_DSM_ERRORS  TRUE
#define TRUE 1
#define FALSE 0

extern dsm_structure bdc812Structure;

void bdc812(int count) {
                      /* [rx] [antenna] [coefficients] */
  static int firstTime = TRUE;
//  static dsm_structure statusStructure;
  time_t time_read;
  time_t rawtime;

  signed int  deadAntennas[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
  float ar1_ctrl_readback[8]={0.0};
  float ar2_ctrl_readback[8]={0.0};
  float at20_ctrl_readback[8]={0.0};
  float at21_ctrl_readback[8]={0.0};
  float ar1_detector_readback[8]={0.0};
  float ar2_detector_readback[8]={0.0};
  float at20_detector_readback[8]={0.0};
  float at21_detector_readback[8]={0.0};
  float ar1_ctrl_nominal[8]={0.0};
  float ar2_ctrl_nominal[8]={0.0};
  float at20_ctrl_nominal[8]={0.0};
  float at21_ctrl_nominal[8]={0.0};
  float ar1_detector_target[8]={0.0};
  float ar2_detector_target[8]={0.0};
  float at20_detector_target[8]={0.0};
  float at21_detector_target[8]={0.0};
  float bdc_temp[2][7];
  char  bdc_do[8][16];
  unsigned char  bdc_di[8];
  int   i=0;
  int   s;
  signed int nDead, deadA[8]={-1,-1,-1,-1,-1,-1,-1,-1}, deadAnt;
  char  bdc_timestamp[15];
  FILE *deadListAnts;
  		
  deadListAnts= fopen("/global/configFiles/deadAntennas", "r");
  if (deadListAnts != NULL) {

    nDead = fscanf(deadListAnts, "%d %d %d %d %d %d %d %d",
                   &deadA[0], &deadA[1], &deadA[2], &deadA[3],
                   &deadA[4], &deadA[5], &deadA[6], &deadA[7]);
   fclose(deadListAnts);
      for (i = 0; i < 8; i++) {
        if(deadA[i] != 0) {deadAnt=deadA[i]-1;
                            deadAntennas[deadAnt]=1;
                           }
                                }
                      }

  if ((count % 60) == 1 || firstTime) {
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

  firstTime=FALSE;

  move(0,0);
  rawtime = time(NULL);
//  s = dsm_structure_get_element(&statusStructure,"SERVER_TIMESTAMP_L",(char *)(&rawtime));
//  if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS)
//    dsm_error_message(s, "SERVER_TIMESTAMP_L");
//	printw("Server timestamp: %s", asctime(gmtime(&rawtime)));

// Reading BDC data from DSM
//
//  	time_read = time(NULL);
        s = dsm_structure_init(&bdc812Structure, "BDC_X");
  	if (s != DSM_SUCCESS && PRINT_DSM_ERRORS) {
    	        dsm_error_message(s,"dsm_structure_init(BDC_X)");
                exit(-1);
                                                  }
  	s = dsm_read("obscon","BDC_X", &bdc812Structure, &time_read);
  	if (s != DSM_SUCCESS && PRINT_DSM_ERRORS) {
    	        dsm_error_message(s,"dsm_read(BDC_X)");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_TIMESTAMP_C15",&bdc_timestamp);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_TIMESTAMP_C15");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR1_CTRL_NOMINAL_V8_F", &ar1_ctrl_nominal);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR1_CTRL_NOMINAL_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR2_CTRL_NOMINAL_V8_F", &ar2_ctrl_nominal);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR2_CTRL_NOMINAL_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT20_CTRL_NOMINAL_V8_F", &at20_ctrl_nominal);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT20_CTRL_NOMINAL_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT21_CTRL_NOMINAL_V8_F", &at21_ctrl_nominal);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT21_CTRL_NOMINAL_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR1_CTRL_READBACK_V8_F", &ar1_ctrl_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR1_CTRL_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR2_CTRL_READBACK_V8_F", &ar2_ctrl_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR2_CTRL_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT20_CTRL_READBACK_V8_F", &at20_ctrl_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT20_CTRL_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT21_CTRL_READBACK_V8_F", &at21_ctrl_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT21_CTRL_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR1_DETECTOR_TARGET_V8_F", &ar1_detector_target);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR1_DETECTOR_TARGET_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR2_DETECTOR_TARGET_V8_F", &ar2_detector_target);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR2_DETECTOR_TARGET_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT20_DETECTOR_TARGET_V8_F", &at20_detector_target);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT20_DETECTOR_TARGET_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT21_DETECTOR_TARGET_V8_F", &at21_detector_target);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT21_DETECTOR_TARGET_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR1_DETECTOR_READBACK_V8_F", &ar1_detector_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR1_DETECTOR_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AR2_DETECTOR_READBACK_V8_F", &ar2_detector_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AR2_DETECTOR_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT20_DETECTOR_READBACK_V8_F", &at20_detector_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT20_DETECTOR_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_AT21_DETECTOR_READBACK_V8_F", &at21_detector_readback);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_AT21_DETECTOR_READBACK_V8_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_TEMP_V2_V7_F", &bdc_temp);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_TEMP_V2_V7_F");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_DO_V8_C16", &bdc_do);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_DO_V8_C16");
                exit(-1);
                                                  }
	s = dsm_structure_get_element(&bdc812Structure, "DSM_BDC_DI_V8_B", &bdc_di);
 	if ((s != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
  		dsm_error_message(s, "DSM_BDC_DI_V8_B");
                exit(-1);
                                                  }


    printw("BDC timestamp %s          BDC CHASSIS STATUS %s",bdc_timestamp,asctime(gmtime(&rawtime)));   
  move(1,0);
     printw("             AR5-1    AR5-2    AR5-3    AR5-4    12GHz-LO    Deck    Air-Intake");
     printw("\n");
     printw("BDC1 TMPS (C)");
     printw("%5.2f %8.2f %8.2f %8.2f %10.2f %9.2f %10.2f",bdc_temp[0][0],bdc_temp[0][1],bdc_temp[0][2],bdc_temp[0][3],bdc_temp[0][4],bdc_temp[0][5],bdc_temp[0][6]);
     printw("\n");
     printw("BDC2 TMPS (C)");
     printw("%5.2f %8.2f %8.2f %8.2f %10.2f %9.2f %10.2f",bdc_temp[1][0],bdc_temp[1][1],bdc_temp[1][2],bdc_temp[1][3],bdc_temp[1][4],bdc_temp[1][5],bdc_temp[1][6]);
        printw("\n");
        standend();
        printw("Clock Chassis   ");
     for (i=0; i<3; i++) {
      switch (i){
       case 0:
        if ((bdc_di[0]) == 0) {
          standout();
	  printw("+5V status   ");
          standend();
                            }
	else {  printw("+5V status   ");}
        break;
       case 1:
        if ((bdc_di[1]) == 0) {
          standout();
	  printw("     2.24 GHz status       ");
          standend();
                            }
	 else { printw("     2.24 GHz status       "); }
        break;
       case 2:
        if ((bdc_di[2]) == 0) {
          standout();
	  printw("Fan rotor status");
          standend();
                            }
	  else {printw("Fan rotor status");}
        break;
                     }
                            }
        printw("\n");
        standend();
     printw("LO Chassis      ");
     for (i=3; i<7; i++) {
      switch (i){
       case 3:
        if ((bdc_di[3]) == 0) {
          standout();
	  printw("+5V status   ");
          standend();
                            }
	else {  printw("+5V status   ");}
        break;
       case 4:
        if ((bdc_di[4]) == 0) {
          standout();
	  printw("LO1 status   ");
          standend();
                            }
	 else { printw("LO1 status   "); }
        break;
       case 5:
        if ((bdc_di[5]) == 0) {
          standout();
	  printw("LO2 status    ");
          standend();
                            }
	  else {printw("LO2 status    ");}
        break;
       case 6:
        if ((bdc_di[6]) == 0) {
          standout();
	  printw("Fan rotor status");
          standend();
                            }
	else {printw("Fan rotor status");}
        break;
                }
                         }
        move(6,0);
        printw("Antenna                    A1    A2    A3    A4    A5    A6    A7    A8\n");
     printw("VGA1   CONTROL  NOML (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
        else {
           if (ar1_ctrl_nominal[i] > 9.99) { ar1_ctrl_nominal[i] = 9.99;}
           printw("%6.2f",ar1_ctrl_nominal[i]);
             }
                           }
        printw("\n");

     printw("VGA1   CONTROL  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
        else              {
        if (ar1_ctrl_readback[i] > 9.99) {ar1_ctrl_readback[i] = 9.99;}
        if ((ar1_ctrl_readback[i] > CONTROL_MAX) || (ar1_ctrl_readback[i] < CONTROL_MIN )) {standout();}
        else {standend();}
        printw("%6.2f",ar1_ctrl_readback[i]);
                          }
                }
        printw("\n");
        standend();

     printw("VGA1  DETECTOR  TRGT (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
        else {
           if (ar1_detector_target[i] > 9.99) { ar1_detector_target[i] = 9.99;}
	   printw("%6.2f", ar1_detector_target[i]);
             }
                         }
        printw("\n");

     printw("VGA1  DETECTOR  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else    {
        if (ar1_detector_readback[i] > 9.99) {ar1_detector_readback[i] = 9.99;}
        if ((ar1_detector_readback[i] > DETECTOR_MAX) || (ar1_detector_readback[i] < DETECTOR_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", ar1_detector_readback[i]);
                          }
                         }
        printw("\n");
        standend();

     printw("VGA2   CONTROL  NOML (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
         if (ar2_ctrl_nominal[i] > 9.99) {ar2_ctrl_nominal[i] = 9.99;}
	 printw("%6.2f", ar2_ctrl_nominal[i]);
            }
                          }
        printw("\n");

     printw("VGA2   CONTROL  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (ar2_ctrl_readback[i] > 9.99) {ar2_ctrl_readback[i] = 9.99;}
        if ((ar2_ctrl_readback[i] > CONTROL_MAX) || (ar2_ctrl_readback[i] < CONTROL_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", ar2_ctrl_readback[i]);
            }
                          }
        printw("\n");
        standend();

     printw("VGA2  DETECTOR  TRGT (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
      else {
        if (ar2_detector_target[i] > 9.99) {ar2_detector_target[i] = 9.99;}
	printw("%6.2f", ar2_detector_target[i]);
           }
                          }
        printw("\n");

     printw("VGA2  DETECTOR  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (ar2_detector_readback[i] > 9.99) {ar2_detector_readback[i] = 9.99;}
        if ((ar2_detector_readback[i] > DETECTOR_MAX) || (ar2_detector_readback[i] < DETECTOR_MIN )) {
              if (strcmp(bdc_do[0],"8-10")==0) {standout();}
                                                                                                     }
        else {standend();}
	printw("%6.2f", ar2_detector_readback[i]);
            }
                          }
        printw("\n");
        standend();

     printw("ATTEN1 CONTROL  NOML (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (at20_ctrl_nominal[i] > 9.99) {at20_ctrl_nominal[i] = 9.99;}
	printw("%6.2f", at20_ctrl_nominal[i]);
            }
                          }
        printw("\n");

     printw("ATTEN1 CONTROL  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
      else {
        if (at20_ctrl_readback[i] > 9.99) {at20_ctrl_readback[i] = 9.99;}
        if ((at20_ctrl_readback[i] > CONTROL_MAX) || (at20_ctrl_readback[i] < CONTROL_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", at20_ctrl_readback[i]);
           }
                          }
        printw("\n");
        standend();

     printw("ATTEN1 DETECTOR TRGT (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (at20_detector_target[i] > 9.99) {at20_detector_target[i] = 9.99;}
	printw("%6.2f", at20_detector_target[i]);
            }
                          }
        printw("\n");

     printw("ATTEN1 DETECTOR RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (at20_detector_readback[i] > 9.99) {at20_detector_readback[i] = 9.99;}
        if ((at20_detector_readback[i] > DETECTOR_MAX) || (at20_detector_readback[i] < DETECTOR_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", at20_detector_readback[i]);
            }
                          }
        printw("\n");
        standend();

     printw("ATTEN2 CONTROL  NOML (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
      else {
        if (at21_ctrl_nominal[i] > 9.99) {at21_ctrl_nominal[i] = 9.99;}
	printw("%6.2f", at21_ctrl_nominal[i]);
           }
                          }
        printw("\n");

     printw("ATTEN2 CONTROL  RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (at21_ctrl_readback[i] > 9.99) {at21_ctrl_readback[i] = 9.99;}
        if ((at21_ctrl_readback[i] > CONTROL_MAX) || (at21_ctrl_readback[i] < CONTROL_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", at21_ctrl_readback[i]);
             }
                          }
        printw("\n");
        standend();

     printw("ATTEN2 DETECTOR TRGT (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
      else {
        if (at21_detector_target[i] > 9.99) {at21_detector_target[i] = 9.99;}
	printw("%6.2f", at21_detector_target[i]);
           }
                          }
        printw("\n");

     printw("ATTEN2 DETECTOR RDBK (V)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
        if (at21_detector_readback[i] > 9.99) {at21_detector_readback[i] = 9.99;}
        if ((at21_detector_readback[i] > DETECTOR_MAX) || (at21_detector_readback[i] < DETECTOR_MIN )) {standout();}
        else {standend();}
	printw("%6.2f", at21_detector_readback[i]);
             }
                          }
        printw("\n");
        standend();

     printw("SWITCH POSITION    (GHz)");
     for (i=0; i<8; i++) {
       if(deadAntennas[i] ==1){
            printw("------");
                    }
       else {
	printw("%6s", bdc_do[i]);
            }
                          }
        printw("\n");
        standend();
 
	move(0,79);
	refresh();
        return;
}

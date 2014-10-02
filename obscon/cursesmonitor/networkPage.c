#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include <dsm.h>
#include <string.h>
#include "monitor.h"

#define TUNE6_STALE_INTERVAL 10

enum {LO_BOARD_TYPE=1, MIXER_BOARD_TYPE, OPTICS_BOARD_TYPE, PALM_BOARD_TYPE,
      POWERPC_BOARD_TYPE, YIG_BOARD_TYPE};
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *, char *);

void networkPage(int count, int *rm_list) {
  short shortvalue;
  char tune6user[20];
  long timestamp;
  int gain,j,board,plotted[15][MAX_NUMBER_ANTENNAS+1], presentType;
  short actPLL[MAX_NUMBER_ANTENNAS+1];
  float freq;
  char dsmHost[NUMBER_OF_RECEIVERS];
  int rm_status,i,dsm_status;
  int ant, antennaNumber, rx;
  short shortarray[15];
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  short network[MAX_NUMBER_ANTENNAS+1][15];
  short subtype[MAX_NUMBER_ANTENNAS+1][15];
  long rightnow;
  short slaveAddress[MAX_NUMBER_ANTENNAS+1];
  short gunnTableType[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];
  char *rxlabel;

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
  ant = 0;
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("Rx Network   1        2        3        4        5        6        7        8");
  if (numberAntennas > 8) {
    printw("       CSO     JCMT");
  }
  move(1,0);
  printw("MixerPROM ");
  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----");
     if (antennaNumber < numberAntennas) {
       printw("  ");
     }
   } else {
     rm_status = rm_read(antennaNumber, "RM_MIXER_BOARD_EPROM_VERSION_S",&shortvalue);
     checkStatus(rm_status,"rm_read(RM_MIXER_BOARD_EPROM_VERSION_S)");
     if (shortvalue < 1 || shortvalue > 0xff) {
       if (antennaNumber != 8) {
	 printw(" ");
       }
       printw("wac/");
     } else {
       printw("0x%02x/",shortvalue);
     }
     rm_status = rm_read(antennaNumber, "RM_MIXER_BOARD_SRAM_VERSION_S",&shortvalue);
     checkStatus(rm_status,"rm_read(RM_MIXER_BOARD_SRAM_VERSION_S)");
     if (shortvalue < 1 || shortvalue > 0xff) {
       printw("wac");
     } else {
       printw("%02x",shortvalue);
       if (antennaNumber < 8) {
	 printw(" ");
       }
     }
     if (antennaNumber < 8) {
       printw(" ");
     }
   }
  }
  move(2,0);
  printw("Address      (presently active slave in the interactive thread is highlighted)");
  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
    rm_status = rm_read(antennaNumber, "RM_RX_MICROCONTROLLER_NETWORK_V15_S", network[antennaNumber]);
    rm_status = rm_read(antennaNumber, "RM_RX_MICROCONTROLLER_SUBTYPE_V15_S", subtype[antennaNumber]);
    rm_status = rm_read(antennaNumber, "RM_SLAVE_PLL_NO_S", &slaveAddress[antennaNumber]);
    /*
    printw(" slave=%d",slaveAddress[antennaNumber]);
    if (slaveAddress[antennaNumber] < 10) {
      printw(" ");
    }
    */
  }
  for (i=0; i<15; i++) {
    move(3+i,0);
    printw("    %2d    ",i);
    for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
      if (i == 0) {
	rm_status = rm_read(antennaNumber,"RM_RX_GUNN_TABLE_FORMAT_V8_S",gunnTableType[antennaNumber]);
      }
      if (slaveAddress[antennaNumber] == i) {
	standout();
      }
      if (!antsAvailable[antennaNumber]) {
        printw(" -----   ");
      } else {
	if (network[antennaNumber][i] > 8 || network[antennaNumber][i]<-1) {
          printw(" -----   ");
        } else if (network[antennaNumber][i] == 8) {
          printw("   -     ");
	} else {
	  switch(network[antennaNumber][i]) {
	  case YIG_BOARD_TYPE:
            rx = subtype[antennaNumber][i];
            rxlabel = getLoBoardTypeStringBrief(rx);
	    printw("Ph:%s",rxlabel); 
	    switch (gunnTableType[antennaNumber][rx]) {
	      /*
	    case 1:
	    case 2:
	      printw("ext ");
	      break;
	      */
	    default:
	    case 0:
	      printw("    ");
	      break;
	    }
	    if (strlen(rxlabel) < 2) { 
	      printw(" ");
	    }
	    break;
	  case LO_BOARD_TYPE:
            rx = subtype[antennaNumber][i];
            rxlabel = getLoBoardTypeStringBrief(rx);
	    printw("LO:%s",rxlabel); 
	    switch (gunnTableType[antennaNumber][rx]) {
	      /*
	    case 1:
	    case 2:
	      printw("ext ");
	      break;
	      */
	    default:
	    case 0:
	      printw("    ");
	      break;
	    }
	    if (strlen(rxlabel) < 2) { 
	      printw(" ");
	    }
	    /*
	    lo1address = i; 
	    lo2address = i; 
	    */
	    break;
	  case MIXER_BOARD_TYPE:
	    printw(" Mixer   "); 
	    break;
	  case OPTICS_BOARD_TYPE:
	    printw(" Optics  "); 
	    break;
	  case PALM_BOARD_TYPE:
	    printw("  Palm   "); 
	    break;
	  default:
            printw("   %d     ",network[antennaNumber][i]);
	  }
	}
      }
      standend();
    }
  }

  move(18,0);
  printw("User     ");
  for (ant=1; ant<=numberAntennas; ant++) { 
    sprintf(dsmHost,"acc%d",ant);
    dsm_status= dsm_read(dsmHost,"DSM_TUNE6_USERNAME_C20",tune6user,&timestamp);
    if (strlen(tune6user) > 8) {
      tune6user[8] = 0;
    }
    printw("%s",tune6user);
    for (i=strlen(tune6user); i<9; i++) {
      printw(" ");
    }
  }
#if 1
  j = 19;
  for (i=0; i<15; i++) {
    for (ant=1; ant<=numberAntennas; ant++) { 
      plotted[i][ant] = 0;
    }
  }
  for (board=0; board<5; board++) {
    move(j+board,0);
    for (ant=1; ant<=numberAntennas; ant++) { 
      for (i=0; i<15; i++) {
	if (!antsAvailable[ant]) {
	  printw("    -    ");
	} else {
	  unsigned short resets;
	  unsigned short loResets[NUMBER_OF_RECEIVERS];
	  if (plotted[i][ant] == 1) continue;
	  /* This code assumes that all boards will be at equivalent addresses
	   * in each antenna */
	  if (presentType != network[ant][i] && ant != 1) continue;
	  switch(network[ant][i]) {
	  case LO_BOARD_TYPE:
	    rm_status = rm_read(ant, "RM_LO_BOARD_RESETS_V8_S", loResets);
	    if (ant == 1) {
	      printw("    %2d  ",i);
	      presentType = network[ant][i];
	    } else {
	      move(j+board,8+(ant-1)*9);
	    }
	    if (subtype[ant][i] > 3) {
	      rm_status = rm_read(ant, "RM_SLAVE2_PLL_NO_S", &actPLL[ant]);
	      if (actPLL[ant] < 0) {
		printw(" rm off ");
	      } else {
		printw(" %6d ",loResets[subtype[ant][i]]);
	      }
	    } else {
	      rm_status = rm_read(ant, "RM_SLAVE1_PLL_NO_S", &actPLL[ant]);
	      if (actPLL[ant] != i) {
		printw("inactive");
	      } else {
		printw(" %6d ",loResets[subtype[ant][i]]);
	      }
	    }
	    plotted[i][ant] = 1;
	    i=15;
	    break;
	  case MIXER_BOARD_TYPE:
	    rm_status = rm_read(ant, "RM_MIXER_BOARD_RESETS_S", &resets);
	    if (ant == 1) {
	      printw("    %2d  ",i);
	      presentType = network[ant][i];
	    } else {
	      move(j+board,8+(ant-1)*9);
	    }
	    printw(" %6d ",resets);
	    plotted[i][ant] = 1;
	    i=15;
	    break;
	  case OPTICS_BOARD_TYPE:
	    rm_status = rm_read(ant, "RM_OPTICS_BOARD_RESETS_S", &resets);
	    if (ant == 1) {
	      printw("    %2d  ",i);
	      presentType = network[ant][i];
	    } else {
	      move(j+board,8+(ant-1)*9);
	    }
	    printw(" %6d ",resets);
	    plotted[i][ant] = 1;
	    i=15;
	    break;
	  } /* end switch */
	} /* end if */
      } /* end of loop over addresses */
    } /* end of loop over antennas */
    printw("\n");
  }
#endif
  move(20,0);
  printw("Re-");
  move(21,0);
  printw("sets");
  refresh();
}




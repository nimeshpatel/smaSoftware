#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "rm.h"
#include "monitor.h"

extern void checkStatus(int status, char *string);

void iFLODisplayPage4(int count, int *antlist)
{
  int ant, rms, channel, j;
  float values[MAX_NUMBER_ANTENNAS+1][16];

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
  for (ant = 1; ant <= numberAntennas; ant++) {
    for (j = 0; j < 16; j++) {
      values[ant][j] = -10000.0;
    }
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (antsAvailable[ant]) {
      rms = rm_read(ant, "RM_IFLO_THERMISTORS_V16_F", &values[ant][0]);
      checkStatus(rms,"rm_read(RM_IFLO_THERMISTORS_V16_F)");
    }
  }
  move(0,0);
  printw("                 Antenna IF/LO Status - Page 3: Thermistor Readings");
  move(1,0);
  printw("Antenna      1");
  for (ant=2; ant<=numberAntennas; ant++) {
    switch (ant) {
    case CSO_ANTENNA_NUMBER:
      printw("      CSO");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("     JCMT");
      break;
    default:
      printw("        %d",ant);
    }
  }
  move(3,0);
  printw("Ort Xmit 1");
  channel = 0;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat. ");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko ");
      else
	printw("%6.2fC", values[ant][channel]);
      printw("  ");
    }
  }
  move(4,0);
  printw("109/200 1 ");
  channel = 1;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(5,0);
  printw("Ort Xmit 2");
  channel = 2;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(6,0);
  printw("109/200 2 ");
  channel = 3;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(7,0);
  printw("Encl Top  ");
  channel = 4;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(8,0);
  printw("Encl Cntr ");
  channel = 5;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(9,0);
  printw("YIG 1     ");
  channel = 6;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(10,0);
  printw("200IF amp1");
  channel = 7;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(11,0);
  printw("YIG 2     ");
  channel = 8;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(12,0);
  printw("200IF amp2");
  channel = 9;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(13,0);
  printw("Plate Edge");
  channel = 10;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(14,0);
  printw("FO Rcvr   ");
  channel = 11;
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (values[ant][channel] == -100.0)
	printw("  Sat.");
      else if ((values[ant][channel] < 0.0) || (values[ant][channel] > 143.0))
	printw(" Wacko");
      else
	printw("%6.2f", values[ant][channel]);
      printw("   ");
    }
  }
  move(23,0);
  printw("                    --- Type \"+\" to cycle through pages ---");
  move(0,79);
  refresh();
}


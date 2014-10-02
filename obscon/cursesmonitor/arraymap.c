#define NUMANTS 8
#include <stdio.h>
#include <curses.h>
#include <rm.h>
#include "esma.h"
#ifdef LINUX
#include <sys/time.h>
#endif
#include "monitor.h"

int locatepad(int ant, short padid[9]);

void arraymap(int count, int *rm_list) {
  int ant;
  static short padid[9];
  char dummyByte;

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
    for(ant = 1; ant <= NUMANTS; ant++) {
      if(antsAvailable[ant]) {
	unsigned char statusBits[16];
	(void)rm_read(ant,"RM_PAD_ID_B",&dummyByte);
	padid[ant]=(short)dummyByte;
      }
    }
  }
  move (0,5);
  printw("TMT");
  move(0,51);
  printw("SMA Pad Layout");
  move(1,46);
  printw("(antennas are highlighted)");
  move(1,0);
  printw("             20");
  if ((ant=locatepad(20,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(2,0);
  printw("                              21");
  if ((ant=locatepad(21,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(5,0);
  printw(" 19");
  if ((ant=locatepad(19,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  move(7,0); 
  printw("                       15");
  if ((ant=locatepad(15,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(8,0);
  printw("                  24");
  if ((ant=locatepad(24,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(9,0);
  printw("                              16");
  if ((ant=locatepad(16,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  move(8,59);
  printw("22");
  if ((ant=locatepad(22,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("  5");
  if ((ant=locatepad(5,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(9,57);
  printw(".");
  move(10,0);
  printw("                                                      .                 6");
  if ((ant=locatepad(6,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(11,0);
  printw("                                                   .");
  move(12,0);
  printw("                               10");
  if ((ant=locatepad(10,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("             .        4");
  if ((ant=locatepad(4,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(13,0);
  printw("         18");
  if ((ant=locatepad(18,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("               23");
  if ((ant=locatepad(23,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("    11");
  if ((ant=locatepad(11,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("    .                 INNER");
  move(14,0);
  printw("                                            .                             1");
  if ((ant=locatepad(1,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(15,0);
  printw("                                       ._.                        RING");         
  move(16,0);
  printw("               14");
  if ((ant=locatepad(14,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("           9");
  if ((ant=locatepad(9,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("   /   \\                 3");
  if ((ant=locatepad(3,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(17,0);
  printw("                                      .   .                              2");
  if ((ant=locatepad(2,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(18,0);
  printw("                             8");
  if ((ant=locatepad(8,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("      \\_/  .");
  move(19,31);
  printw("12");
  if ((ant=locatepad(12,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  } else {
    printw("   ");
  }
  printw("          . . . . .  . . . .   7");
  if ((ant=locatepad(7,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(20,0);
  printw("              13");
  if ((ant=locatepad(13,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(21,0);
  printw("                      17");
  if ((ant=locatepad(17,padid))) {
    printw("(");
    standout();
    printw("%d",ant);
    standend();
    printw(")");
  }
  move(21,48); printw("Control building");
  move(23,10); printw("Puu Poli'ahu");
  if (numberAntennas == 8) {
    move(23,79);
  } else {
    move(22,64);
    if (antsAvailable[JCMT_ANTENNA_NUMBER]) {
      standout();
    }
    printw("JCMT");
    standend();
    move (23,95);
    if (antsAvailable[JCMT_ANTENNA_NUMBER]) {
      standout();
    }
    printw("CSO");
    standend();
    move(23,0);
  }
  refresh();
}

int locatepad(int pad, short padid[9]) {
  int i;
  for (i=1; i<=NUMANTS; i++) {
    if (padid[i] == pad) {
      return(i);
    }
  }
  return(0);
}

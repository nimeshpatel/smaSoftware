#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "dDSCursesMonitor.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0

extern antsAvailable[9];
extern int quit;
extern int verbose;
extern int opMessagePending;

int opMsg(int count, int bottom)
{
  static int savedLineCount;
  int lineCount = 0;
  int s, ant1, ant2, bslnCount;
  int doWeCare[11], nAntAve[9];
  float coh[11][11][2], antAve[9];
  float aveUSB, aveLSB;
  char string1[100], source[24];
  int i, j, lineOffset;
  char text[22][60], pPC[22][20];
  short severity[22], priority[22];
  long unixTime[22];
  time_t timestamp, curTime;

  if (bottom)
    lineOffset = 24;
  else {
    lineOffset = 0;
    opMessagePending = FALSE;
  }
  if (((count % 60) == 1) && (!bottom)) {
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
  curTime = time((long *)0);
  strcpy(string1, asctime(gmtime(&curTime)));
  string1[strlen(string1)-1] = (char)0;
  move(lineOffset+0,0);
  if (bottom)
    printw("                        Active Messages to the Operator                                 ");
  else
    printw("            Active Messages to the Operator at %s",
	   string1);
  for (i = 0; i < COLS; i++) {move(lineOffset+1,i); printw(" ");}
  s = dsm_read("colossus",
	       "DSM_OPMSG_PPC_V22_C20", 
	       (char *)pPC,
	       &timestamp);
  s = dsm_read("colossus",
	       "DSM_OPMSG_TEXT_V22_C60", 
	       (char *)text,
	       &timestamp);
  s = dsm_read("colossus",
	       "DSM_OPMSG_SEVERITY_V22_S", 
	       (char *)severity,
	       &timestamp);
  s = dsm_read("colossus",
	       "DSM_OPMSG_PRIORITY_V22_S", 
	       (char *)priority,
	       &timestamp);
  s = dsm_read("colossus",
	       "DSM_OPMSG_TIME_V22_L", 
	       (char *)unixTime,
	       &timestamp);
  for (i = 0; i < 22; i++) {
    pPC[i][7] = (char)0;
    if (strlen(text[i]) == 0) {
      for (j = 0; j < 59; j++)
	text[i][j] = ' ';
      text[i][59] = (char)0;
    }
  }
  for (i = 0; i < 22; i++) {
    char *cprintResult, timeString[10];

    move(lineOffset+i+2, 0);
    cprintResult = asctime(gmtime(&unixTime[i]));
    /*
    if (strlen(cprintResult) < 5) {
      printf("\n\n\n string = \"%s\", unixTime = %d\n",
	     cprintResult, unixTime[i]);
      exit(-1);
    }
    */
    for (j = 0; j < 5; j++)
      timeString[j] = cprintResult[11+j];
    timeString[5] = (char)0;
    if (priority[i] > 0) {
      int ii;

      for (ii = 0; ii < COLS; ii++) {move(lineOffset+i+2, ii);printw(" ");}
      move(lineOffset+i+2, 0);
      printw("%s %7s ", timeString, pPC[i]);
      lineCount++;
    } else if (!bottom)
      printw("                                                                                ");
    if ((severity[i] == 2) && (priority[i] > 0))
      printw("E ");
    else if ((severity[i] == 1) && (priority[i] > 0))
      printw("S ");
    else if ((severity[i] == 0) && (priority[i] > 0))
      printw("W ");
    else if (priority[i] > 0)
      printw("? ");
    if (priority[i] > 0)
      printw("%s", text[i]);
  }
  if ((priority[0] == 0) && (bottom))
    opMessagePending = FALSE;
  move(lineOffset+0,79);
  refresh();
  savedLineCount = lineCount;
  return (lineCount);
}

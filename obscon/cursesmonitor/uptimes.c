#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "monitor.h"
#define N_ANTENNAS 8
#define N_CRATES 12
#define N_ROACH2S 10

#define REBOOT_LOG "/global/logs/reboots"

time_t accTime_t[N_ANTENNAS+1], crateTime_t[N_CRATES+1], halTime_t,
  colossusTime_t, corconTime_t, m5Time_t, newddsTime_t, roach2Time_t[N_ROACH2S+1];
time_t system_time;

void getLine(FILE *inFile, char *line);

void fancyTimeString(time_t converted, char *string)
{
  double delta;
  int dd, hh, mm, ss;

  converted -= 36000;
  delta = difftime(system_time, converted);
  if (delta < 86400.0) {
    hh = (int)(delta / 3600.0);
    mm = (int)((delta - 3600.0*(double)hh) / 60.0);
    ss = (int)(delta - 3600.0*(double)hh - 60.0*(double)mm);
    sprintf(string, "  %02d:%02d:%02d     ", hh, mm, ss);
  } else {
    int days, hh, mm;

    days = (int)(delta / 86400.0);
    hh = (int)((delta - 86400.0*(double)days) / 3600.0);
    mm = (int)((delta - 86400.0*(double)days - 3600.0*(double)hh) / 60.0);
    if (days == 1)
      sprintf(string, "   1 day, %02d:%02d", hh, mm);
    else if (days > 10000)
      sprintf(string, "   Never booted");
    else
      sprintf(string, "%3d days, %02d:%02d", days, hh, mm);
  }
}

void uptimes(int count)
{
  int line;
  int static firstCall = TRUE;
  static struct stat rebootsStat, oldRebootsStat;
  char printString[20];

  if ((count % 60) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
    clear();
    move(1,1);
    refresh();
  }
  system_time = time(NULL);
  stat(REBOOT_LOG, &rebootsStat);
  if ((rebootsStat.st_mtime != oldRebootsStat.st_mtime) ||
      firstCall) {
    FILE *reboots;

    reboots = fopen(REBOOT_LOG, "r");
    if (!reboots) {
      fprintf(stderr, "Open error on %s\n",REBOOT_LOG);
      perror(REBOOT_LOG);
      exit(-1);
    }
    while (!feof(reboots)) {
      int day, hh, mm, ss, year, nRead;
      double delta;
      char name[20], weekDay[4], month[4], theLine[132], *utcPtr;
      struct tm scratchTime;
      time_t converted;

      getLine(reboots, theLine);
      utcPtr = strstr(theLine, "UTC");
      if (utcPtr) {
	utcPtr[0] = 'W';
	utcPtr[1] = 'E';
	utcPtr[2] = 'T';
      }
      nRead = sscanf(theLine, "%s %s %s %d %d:%d:%d WET %d\n",
		     name, weekDay, month, &day, &hh, &mm, &ss, &year);
      scratchTime.tm_sec = ss;
      scratchTime.tm_min = mm;
      scratchTime.tm_hour = hh;
      scratchTime.tm_mday = day;
      scratchTime.tm_mon = computeMonthFrom3CharString(month);

      scratchTime.tm_year = year-1900;
      converted = mktime(&scratchTime);
      if (!strcmp(name, "hal9000"))
	halTime_t = converted;
      else if (!strcmp(name, "colossus"))
	colossusTime_t = converted;
      else if (!strcmp(name, "corcon"))
	corconTime_t = converted;
      else if (!strcmp(name, "newdds"))
	newddsTime_t = converted;
      else if (!strcmp(name, "m5"))
	m5Time_t = converted;
      else if (name[0] == 'a') {
	int unit;

	sscanf(name, "acc%d", &unit);
	accTime_t[unit] = converted;
      } else if (name[0] == 'c') {
	int unit;

	sscanf(name, "crate%d", &unit);
	crateTime_t[unit] = converted;
      } else if (name[0] == 'r') {
	int unit;

	sscanf(name, "roach2-%d", &unit);
	roach2Time_t[unit] = converted;
      }
    }
    fclose(reboots);
    oldRebootsStat.st_mtime = rebootsStat.st_mtime;
    firstCall = FALSE;
  }
  move(0, 0);
  printw("        Time Since Last PowerPC Reboot as of %s",
	 asctime(gmtime(&system_time)));
  move(3, 0);
  fancyTimeString(halTime_t, printString);
  printw("hal9000   %s  ", printString);
  fancyTimeString(colossusTime_t, printString);
  printw("colossus %s  ", printString);
  move(4,0);
  fancyTimeString(corconTime_t, printString);
  printw("corcon    %s  ", printString);
  fancyTimeString(m5Time_t, printString);
  printw("m5       %s  ", printString);
  fancyTimeString(newddsTime_t, printString);
  printw("newdds      %s  ", printString);
  for (line = 1; line <= N_CRATES; line++) {
    move(line+5, 0);
    if (line <= N_ANTENNAS) {
      fancyTimeString(accTime_t[line], printString);
      printw("acc%2d   %s   ", line, printString);
    } else
      printw("                          ");
    fancyTimeString(crateTime_t[line], printString);
    if (line < 10)
      printw("crate%d   %s  ", line, printString);
    else
      printw("crate%d  %s  ", line, printString);
    if (line <= N_ROACH2S) {
      move(line+5, 53);
      fancyTimeString(roach2Time_t[line], printString);
      printw("roach2-%02d   %s", line, printString);
    }
  }
  move(23,79);
  refresh();
  return;
}




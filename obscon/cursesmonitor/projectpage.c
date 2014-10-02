#include <stdio.h>
#include <math.h>
#include <curses.h>
#include<string.h>
#ifdef LINUX
#include <sys/time.h>
#else
#include <time.h>
#endif
#include "rm.h"
#include "monitor.h"
#include "dsm.h"

#define DESC_STRING_SIZE 256
#define PI_STRING_SIZE 30


int notEmptyString(char *string);

/*
void checkForNulls(char *string) {
  char junk[DESC_STRING_SIZE];
  
  strcpy(junk, string);
  if (!strtok(junk, " \t"))
    string[0] = (char)0;
}
*/

int projectpage(int icount){
  
  FILE *fp1;
  static int idleMinutes;
  static int idleaMinutes;
  int hour;  
  struct timeval tv;
#ifndef LINUX
  struct timezone tz;
#endif
  int projectID,startTime,scriptPID=0;
  int lowestActiveCrate, activeCrate[14], crate;
  char projectPI[PI_STRING_SIZE],observer[PI_STRING_SIZE], dataFileName[80];
  char operatingLocation[DESC_STRING_SIZE],comment[DESC_STRING_SIZE];
  char scriptFileName[DESC_STRING_SIZE],description[DESC_STRING_SIZE];
  char antennasInArray[20];
  int y;
  FILE *fp;
  char line[100];
  int narg,junk;
  int dsm_status;
  time_t timestamp;
  struct tm *t;
  struct tm *utc;
  
  if((icount % 20) == 1) {
    initscr();
#ifdef LINUX
    clear();
#endif
    move(1,1);
    refresh();
  }
  
  fp1=fopen("/global/projects/antennasInArray","r");
  if(fp1==NULL) {
    printf("Could not open antenna file .\n");
    exit(-1);
  }
  fgets(antennasInArray,100,fp1);
  fclose(fp1);
  
  dsm_status=dsm_read("hal9000","DSM_AS_PROJECT_ID_L",&projectID,&timestamp);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_STARTTIME_L",&startTime,&timestamp);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_SCRIPT_PID_L",&scriptPID,&timestamp);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_PI_C30",projectPI,&timestamp);
  checkForNulls(projectPI);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_OBSERVER_C30",observer,&timestamp);
  checkForNulls(observer);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_COMMENT_C256",comment,&timestamp);
  checkForNulls(comment);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_DESCRIPTION_C256",
		       description,&timestamp);
  checkForNulls(description);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_OPERATINGLOCATION_C256",operatingLocation,&timestamp);
  checkForNulls(operatingLocation);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_COMMENT_C256",comment,&timestamp);
  checkForNulls(comment);
  dsm_status|=dsm_read("hal9000","DSM_AS_PROJECT_SCRIPT_FILENAME_C256",
		       scriptFileName,&timestamp);
  checkForNulls(scriptFileName);
  dsm_status |= dsm_read("m5",
			 "DSM_AS_FILE_NAME_C80", 
			 dataFileName,
			 &timestamp);
  if(dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_write()");
                exit(1);
  }
  lowestActiveCrate = getCrateList(&activeCrate[0]);
  
  /*
  move(0,10);
  printw("----Current project----");
  */
  move (0,2);
  if(projectID>0) {
    printw("Current Project ID: ");
    if (projectID <= 0 || projectID >= WACKO_PROJECT_ID_MAX) {
      printw("wacko");
    } else {
      printw("%d",projectID);
    }
    move(1,10);
    printw("Description: %s",description);
    move(2,10);
    if(startTime>0) {
      long now, age;
      char timeString[40];

      t = gmtime((time_t *)&startTime);
#ifdef LINUX
      gettimeofday(&tv,NULL);
#else
      gettimeofday(&tv,&tz);
#endif
      now = tv.tv_sec;
#ifdef LINUX
      utc = gmtime(&now);
#else
      *utc = *gmtime(&now);
#endif
      strcpy(timeString,asctime(t));
      timeString[strlen(timeString)-1] = (char)0;
      printw("Start time: %s, ", timeString);
      age = now-startTime;
      if (age < 60) {
	if (age < -1) {
	  printw("wacko");
	} else {
	  printw("%2d seconds ago", age);
	}
      } else if (age < 3600) {
	int minutes, seconds;

	minutes = age / 60;
	seconds = age - minutes*60;
	printw("%2d minutes, %2d seconds ago", minutes, seconds);
      } else if (age < 86400) {
	int hours, minutes, seconds;

	hours = age / 3600;
	minutes = (age - hours*3600) / 60;
	seconds = age - hours*3600 - minutes*60;
	if (hours == 1)
	  printw("%2d hour, %2d min, %2d sec ago", hours, minutes, seconds);
	else
	  printw("%2d hours, %2d min, %2d sec ago", hours, minutes, seconds);
      } else {
	int days, hours, minutes, seconds;

	days = age / 86400;
	hours = (age -days*3600) / 3600;
	minutes = (age - days*86400 - hours*3600) / 60;
	seconds = age - days*86400 - hours*3600 - minutes*60;
	if (days == 1)
	  printw("%2d day, %2d hrs, %2d min, %2d sec ago",
		 days, hours, minutes, seconds);
	else
	  printw("%2d days, %2d hrs, %2d min, %2d sec ago",
		 days, hours, minutes, seconds);
      }
    }
    move(3,10);
    printw("P.I.: %s ",projectPI);
    move(4,10);
    printw("Observers: %s ",observer);
    move(5,10);
    printw("Operating from: %s",operatingLocation);
    move(6,10);
    printw("Comment: %s",comment);
    move(7,10);
    if(projectID>0)
      printw("Antennas: %s",antennasInArray);
    
    
    move(8,10);
    printw("Correlator crates: ");
    for (crate = 1; crate <= 12; crate++) {
      if (activeCrate[crate])
	printw("%d ", crate);
    }
    move(9,10);
    printw("Observing script file: %s",scriptFileName);
    move(10,10);
    printw("Observing script PID: ");
    if (scriptPID >= 0 && scriptPID < 100000000) {
      printw("%d",scriptPID);
    } else {
      printw("wacko");
    }
    move(11,10);
    printw("Current MIR file %s", dataFileName);
  }/* if projectID > 0 */

  /*
  move(13,0);
  printw("Console");
  move(14,0);
  printw("idle times  ");

  move(13,12);
  dsm_status = dsm_read("colossus",
			 "DSM_OBSCON_IDLE_TIME_L", 
			 &idleMinutes,
			 &timestamp);
  printObsconIdleTime(idleMinutes);

  move(14,12);
  dsm_status = dsm_read("colossus",
			 "DSM_OBSCONH_IDLE_TIME_L", 
			 &idleMinutes,
			 &timestamp);
  printObsconhIdleTime(idleMinutes);

  move(15,12);
  dsm_status = dsm_read("colossus",
			 "DSM_OBSCONC_IDLE_TIME_L", 
			 &idleMinutes,
			 &timestamp);
  printObsconcIdleTime(idleMinutes);

  move(13,48);
  if (utc != NULL) {
    hour = utc->tm_hour+8;
    if (hour > 24) {
      hour -= 24;
    }
    printw("Taiwan local time: %02d:%02d",hour,utc->tm_min);
  }
  move(14,48);
  if (((icount) % 60) == 1) {
    idleaMinutes = checkAsiaaIdleTime();
  }
  printAsiaaIdleTime(idleaMinutes);

  move(15,48);
  dsm_status = dsm_read("colossus",
			 "DSM_OBSCONHP_IDLE_TIME_L", 
			 &idleMinutes,
			 &timestamp);
  printObsconhpIdleTime(idleMinutes);


    y = 16;
    if ((icount % 5) == 1) {
      fp = fopen("/global/projects/antennaUser","r");
      if (fp != NULL) {
	move(y,0);
	clrtobot();
	move(y,0);
	while (fgets(line,sizeof(line),fp) != NULL) {
	  narg = sscanf(line,"%d",&junk);
	  if (narg == 1 && junk>=0 && junk <= 8) {
	    if (y==16) {
	      printw("Other users: ");
	    }
	    printw("%s\n",line);
	    move(++y,13);
	  }
	}
	fclose(fp);
      }
    }
  */
    move(0,0);

  refresh();
  return 0;
}

#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "monitor.h"
#include "parseFITS.h"

#define MAX_TOKENS 10
#define OPOINT_DIR "/data/engineering/opoint/ant"

void LocateImages(int ant, IMAGE_DATA *image);
int tokenizeChar(char *input, char *tokenArray[MAX_TOKENS], char splitchar);
void printerror(int status);
unsigned char pixval(unsigned char imgval);
void drawImage(IMAGE_DATA *image);

void opointScreen(int count, int antennaNumber) {
  static IMAGE_DATA image;
  static int status = 0;
  char filename[FILE_NAME_LENGTH];

  if (count == 1) {
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
  printw("Antenna %d",antennaNumber);

  if ((count % 20) == 1) {
    /* search directory for latest file */
    LocateImages(antennaNumber,&image);
    /* open the file and read the header */
    sprintf(image.fullfilename,"%s%d/%s",OPOINT_DIR,antennaNumber,image.filename);
    status = parseFITS(&image);

    if (status != 0) {
      refresh();
      return;
    }
    move(4,0);
    printw("Latest image:\n");

    move(5,0);
    addstr(image.filename);

    move(6,0);
    printw("%02d:%02d:%02d UT\n",image.hour,image.minute,image.second);

    move(7,0);
    printw("%04d-%02d-%02d\n",image.year,image.month,image.day);

    move(8,0);
    /*  printw("Source:\n");*/

    move(9,0);
    printw("%s\n",image.source);

    move(11,0);
    printw("Vmag: %.1f",image.magnitude);

    move(12,0);
    printw("Max = %3d",image.max);

    move(13,0);
    printw("Avg = %3.0f",image.average);
    move(14,0);
    printw("Min = %3d",image.min);

    move(16,0);
    printw("Az=%+6.2f",image.azim);
    move(17,0);
    printw("Elev=%5.2f",image.elev);

    drawImage(&image);
  }
  move(23,0);
  refresh();
}

unsigned char pixval(unsigned char imgval) {
  unsigned short val = imgval;
  if (val >= 255) return('M');
  if (val > 250) return('Q');
  if (val > 245) return('N');
  if (val > 240) return('O');
  if (val > 235) return('@');
  if (val > 225) return('B');
  if (val > 220) return('R');
  if (val > 215) return('$');
  if (val > 210) return('#');
  if (val > 205) return('&');
  if (val > 200) return('E');
  if (val > 195) return('A');
  if (val > 190) return('Z');
  if (val > 185) return('G');
  if (val > 180) return('X');
  if (val > 175) return('9');
  if (val > 170) return('5');
  if (val > 160) return('3');
  if (val > 150) return('o');
  if (val > 145) return('e');
  if (val > 140) return('f');
  if (val > 135) return('1');
  if (val > 130) return('t');
  if (val > 125) return('s');
  if (val > 120) return('*');
  if (val > 115) return('l');
  if (val > 110) return('\\');
  if (val > 105) return('(');
  if (val >  90) return('|');
  if (val >  70) return('+');
  if (val >  60) return('=');
  if (val >  50) return(';');
  if (val >  40) return(':');
  if (val >  30) return('-');
  if (val >  20) return('.');
  return(' ');
}

void drawImage(IMAGE_DATA *image) {
  unsigned char brightness, imgval;
  unsigned short val;
  int x,y,u,v;
  int scale;

  scale = 4; /* image pixels per ASCII character (in the narrow direction) */

  for (y=0; y<24; y++) {
    x = 29;
    move(y,x);
    printw("|");
    v = (YPIX/2)+(y-12)*scale;
    for ( ; x<77; x++) {
      u = (XPIX/2)+(x-54)*scale;
      if (scale > 4) {
        val =  image->pix[u][v];
        val += image->pix[u+1][v];
        val += image->pix[u+1][v+1];
        val += image->pix[u][v+1];
        val += image->pix[u-1][v+1];
        val += image->pix[u-1][v];
        val += image->pix[u-1][v-1];
        val += image->pix[u][v-1];
        val += image->pix[u+1][v-1];
	imgval = val/ 9.0;
      } else {
        imgval = image->pix[u][v];
      }
      brightness = pixval(imgval);
      printw("%c",brightness);
    }
    printw("|");
    printw("\n");
  }
}

void LocateImages(int ant, IMAGE_DATA *image) {
 /*
    Set most recent directory to be the default
  */
  DIR *dirPtr;
  char filename[FILE_NAME_LENGTH];
  struct dirent *nextEnt;
  int day, time, antenna, year, month, hour, minute, second;
  double date, newestDate;
  char dir[FILE_NAME_LENGTH];
  char source[SOURCE_NAME_LENGTH];
  int hms,ymd;
  int tokens;
  char *token[MAX_TOKENS];

  sprintf(dir,"%s%d",OPOINT_DIR,ant);
  dirPtr = opendir(dir);
  if (dirPtr == NULL) {
    move(1,0);
    printw("dir not found\n");
    refresh();
    return;
  }
  newestDate = 0;
  move(1,0);
  printw("Scanning directory...\n");
  while ((nextEnt = readdir(dirPtr)) != NULL) {
#define DEBUG 0
    if (1) {
      move(2,0);
      printw("%s\n",nextEnt->d_name);
      refresh();
    }
    if (present(nextEnt->d_name, ".fits")) {
      /* split on '_' */
      strcpy(filename,nextEnt->d_name);
      tokens = tokenizeChar(nextEnt->d_name,token,'_');
      if (tokens > 2) {
        strcpy(source,token[0]);
        sscanf(token[1],"%d",&ymd);
        sscanf(token[2],"%d",&hms);
        year = ymd/10000;
        month = (ymd-10000*year)/100;
        day  =  (ymd-10000*year)-100*month;
        date = (double)ymd*1000000 + hms;
	hour = hms/10000;
        minute = (hms-10000*hour)/100;
        second = (hms-10000*hour)-100*minute;
	if (DEBUG) {
  	  move(17,0);
	  printw("ymd=%08d  year=%04d  month=%02d  day=%02d\n",ymd,year,
            month,day);
  	  move(18,0);
	  printw("hms=%06d    hour=%02d  minute=%02d  second=%02d\n",hms,hour,
	         minute,second);
          move(19,0);
	  printw("source=%s\n",source);
  	  move(20,0);
	}
        if (date > newestDate) {
	  newestDate = date;
          image->date = date;
	  strcpy(image->filename, filename);
          strcpy(image->source, source);
	  image->year = year; image->month = month; image->day = day;
  	  image->hour = hour; image->minute = minute; image->second = second;
	  if (DEBUG) {
            printw("newer\n");
            move(21,0);
    	    printw("ymd=%08d  year=%04d  month=%02d  day=%02d\n",
              ymd, image->year, image->month, image->day);
	    move(22,0);
	    printw("hms=%06d    hour=%02d  minute=%02d  second=%02d\n",
              hms, image->hour, image->minute, image->second);
	    move(23,0);
	    printw("source=%s\n",image->source);
          }
        } else {
	  if (DEBUG) {
	    printw("\n");
	  }
	}
      }
    }
  }
  /*
  if (strlen(image->filename) > 0) {
    image->filename[strlen(image->filename)-1] = 0;
  }
  */
  closedir(dirPtr);
  if (DEBUG) {
    move(3,0);
    printw("returning with %s\n",image->filename);
    refresh();
  }
  move(1,0);
  printw("resting\n");
  move(2,0);
#if 1
  printw("\n");
#endif
}

int tokenizeChar(char *input, char *tokenArray[MAX_TOKENS], char splitchar) {
  int i;
  int non_blanks = 0;
  int tokens = 0;
  char splitstring[2];

  splitstring[0] = splitchar;
  splitstring[1] = 0;
  if (strlen(input) > 0) {
    for (i=0; i<strlen(input); i++) {
      if (input[i] != splitchar) {
        non_blanks = 1; break;
      }
    }
    if (non_blanks == 0) return(0);
    tokenArray[tokens++] = strtok(input,splitstring);
    while ((tokenArray[tokens] = strtok(NULL,splitstring)) != NULL) {
      tokens++;
    }
  }
  return(tokens);
}


#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>

void ipcensus(int count, int page) {
  char string[100];
  char filename[50];
  char computer[25];
  FILE *fp;
  int i,ctr;

  if ((count % 120) == 1) {
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
  move(0,28);
  printw("iPCensus monitor for ");
  switch (page) {
  case 0:
    strcpy(filename,"/otherInstances/hal/1/logs/iPCensus.output");
    strcpy(computer, "hal9000");
    break;
  case 1:
    strcpy(filename,"/otherInstances/acc/1/logs/iPCensus.output");
    strcpy(computer, "acc1");
    break;
  case 2:
    strcpy(filename,"/otherInstances/acc/2/logs/iPCensus.output");
    strcpy(computer, "acc2");
    break;
  case 3:
    strcpy(filename,"/otherInstances/acc/3/logs/iPCensus.output");
    strcpy(computer, "acc3");
    break;
  case 4:
    strcpy(filename,"/otherInstances/acc/4/logs/iPCensus.output");
    strcpy(computer, "acc4");
    break;
  case 5:
    strcpy(filename,"/otherInstances/acc/5/logs/iPCensus.output");
    strcpy(computer, "acc5");
    break;
  case 6:
    strcpy(filename,"/otherInstances/acc/6/logs/iPCensus.output");
    strcpy(computer, "acc6");
    break;
  case 7:
    strcpy(filename,"/otherInstances/acc/7/logs/iPCensus.output");
    strcpy(computer, "acc7");
    break;
  case 8:
    strcpy(filename,"/otherInstances/acc/8/logs/iPCensus.output");
    strcpy(computer, "acc8");
    break;
  case 9:
    strcpy(computer, "colossus");
    strcpy(filename,"/otherInstances/monitor/1/logs/iPCensus.output");
    break;
  case 10:
    strcpy(filename,"/otherInstances/dds/1/logs/iPCensus.output");
    strcpy(computer, "newdds");
    break;
  case 11:
    strcpy(filename,"/otherInstances/corcon/1/logs/iPCensus.output");
    strcpy(computer, "corcon");
    break;
  case 12:
    strcpy(filename,"/otherInstances/storage/1/logs/iPCensus.output");
    strcpy(computer, "m5");
    break;
  case 13:
    strcpy(filename,"/otherInstances/dds/1/logs/iPCensus.output");
    strcpy(computer, "newdds");
    break;
  case 14:
    strcpy(filename,"/otherInstances/storage/1/logs/iPCensus.output");
    strcpy(computer, "m5");
    break;
  case 27:
    strcpy(filename,"/otherInstances/acc/9/logs/iPCensus.output");
    strcpy(computer, "acc9");
    break;
  case 28:
    strcpy(filename,"/otherInstances/acc/10/logs/iPCensus.output");
    strcpy(computer, "acc10");
    break;
  default:
    sprintf(filename,"/otherInstances/crate/%d/logs/iPCensus.output", page-14);
    sprintf(computer, "crate%d", page-14);
  }
  printw("%s",computer);
  fp = fopen(filename,"r");
  move(1,0);
  if (fp == NULL) {
    printw("could not open file = %s\n",filename);
    clrtobot();
  } else {
    /*
    printw("Opened file successfully\n");
    */
    i = 0;
    if ((count % 60) == 1) {
      ctr = 0;
      while (NULL != fgets(string,sizeof(string),fp)) {
	if (page == 13 || page == 14) {
	  /* newdds fills more than one page */
	  if (++ctr < 22) {
	    continue;
	  }
	}
	move(++i,0);
	printw("%s",string);
      }
      /*
      printw("Closed file successfully\n");
      */
    }
    fclose(fp);
  }
  move(23,0);
  printw("Type 1->A: accn, c{+-}: crate, C: colossus, d: newdds, ~: corcon, 0: hal, m: m5");
  refresh();
}




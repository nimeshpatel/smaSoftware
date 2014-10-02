#define MAX_XVALUES 600
#define N_ANTENNAS 8
#define MAX_TOKENS 15 /* in an RM var name */
enum {INT16=0, INT32, FLOATING, DOUBLE};
/*
layout of antenna panels:
  1 2 3 4
  5 6 7 8
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utsname.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include "smapopt.h"
#include "rm.h"
#include "tek_driv.h"
#include "stripchart.h"
#define INITIAL_SMAPOPT_STRING_ARGUMENT "000000000"

void cleanupTune(int sig, siginfo_t *extra, void *cruft);
int getNextValue(float *nextvalue, int antenna);
void title(char *rmvar, int index);
void xlabel(void);
int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *delimit);
void ylabel(int antenna);
void drawBorder(int antenna);
void init(void);
void refresh(void);
void drawAntennaLabel(int antenna);
void updateStripChart(int, int, int, int);
int rmvarindex = 0;
int rmvarindex2 = 0;
int pointsRead = 0;
int totalPointsRead = 0;
int autoscale = 1; /* default behavior if -n not given */
int noautoscale = 0;
int checkAutoScale(float value, int antenna);

int PIXELS_PER_SECOND = 1; 
#define XSPACE 12
#define YSPACE 24
int CHART_XWIDTH = 216;   /* ((DEF_CHART_XWIDTH-3*XSPACE)/4); */
                          /* (900-36)/4 = 216; 216/12 = 18 */
int CHART_YWIDTH = 288;
int CHART_XVALUES; 

int xvalues = 0;
static float yminimum[N_ANTENNAS+1];
static float ymax[N_ANTENNAS+1];
static float xmin = 0.0;
static float xmax = 216; /* assumes 1Hz update */
static float buf[N_ANTENNAS+1][MAX_XVALUES];
int vartype;
int arrayDim = 1;
int array2Dim = 0;
int chartXoffset[N_ANTENNAS+1];
int chartYoffset[N_ANTENNAS+1];

char rmvar[RM_NAME_LENGTH];
void setenvMessage(void);
int antennaNumber = 0; /* 1..8 */
int main(int argc, char *argv[]) {
  char rmvarCopy[RM_NAME_LENGTH];
  int help = 0;
  int rms,i;
  float nextvalue;
  char *ptr, *ptr2;
  int rm_list[RM_ARRAY_SIZE];
  smapoptContext optCon; 
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
  int DEBUG = 0;
  int rc;
  int typeToken, tokens, arrayToken, array2Token;
  int needToRefresh;
  char *token[MAX_TOKENS];
  char *rmvarArgument      = INITIAL_SMAPOPT_STRING_ARGUMENT;
  double yminimumGiven, ymaxGiven;
  int yminimumCmdline = 0;
  struct utsname unamebuf;
  int ymaxCmdline = 0;
  int printHelpAndQuit;
  struct sigaction action, oldAction;

  struct smapoptOption options[] = {
  {"noautoscale", 'n', SMAPOPT_ARG_NONE, &noautoscale, 'n',"autoscale after one screen width"},
  {"lower",'l', SMAPOPT_ARG_DOUBLE, &yminimumGiven, 'l', "Min value for y scale"},
  {"upper",'u', SMAPOPT_ARG_DOUBLE, &ymaxGiven, 'u', "Max value for y scale"},
  {"rmvar",'r', SMAPOPT_ARG_STRING, &rmvarArgument, 'r', "plot the specified RM variable (float,double,int, or short; default=RM_SYNCDET2_CHANNELS_V2_F)"},
  {"index",'i', SMAPOPT_ARG_INT, &rmvarindex, 'i', "which index of the RM array variable (default=0), e.g. to display the high-freq rx total power, use '-i 1'"},
  {"jindex",'j', SMAPOPT_ARG_INT, &rmvarindex2, 'j', "second index of the RM array variable (applicable to 2D arrays only, default=0)"},
  {"variable",'v', SMAPOPT_ARG_STRING, &rmvarArgument, 'v', "plot the specified RM variable (default=RM_SYNCDET2_CHANNELS_V2_F)"},
  {"xvalues",'x', SMAPOPT_ARG_INT, &xvalues, 'x', "how many 1Hz xvalues to show on one screen (up to 600), the default is 216, i.e. a 216-second sweep"},
  {"debug",'D', SMAPOPT_ARG_NONE, &DEBUG, 0, "print debugging messages"},
  SMAPOPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
  };
#define CHART_XOFFSET 100
#define CHART_YOFFSET  80
  if (DEBUG) {
    fprintf(stderr,"Entered stripchart8\n");
  }
  chartXoffset[1] = CHART_XOFFSET;
  chartXoffset[2] = CHART_XOFFSET+CHART_XWIDTH+XSPACE;
  chartXoffset[3] = CHART_XOFFSET+2*(CHART_XWIDTH+XSPACE);
  chartXoffset[4] = CHART_XOFFSET+3*(CHART_XWIDTH+XSPACE);
  chartXoffset[5] = CHART_XOFFSET;
  chartXoffset[6] = CHART_XOFFSET+CHART_XWIDTH+XSPACE;
  chartXoffset[7] = CHART_XOFFSET+2*(CHART_XWIDTH+XSPACE);
  chartXoffset[8] = CHART_XOFFSET+3*(CHART_XWIDTH+XSPACE);

  chartYoffset[5] = CHART_YOFFSET;
  chartYoffset[6] = CHART_YOFFSET;
  chartYoffset[7] = CHART_YOFFSET;
  chartYoffset[8] = CHART_YOFFSET;
  chartYoffset[1] = CHART_YOFFSET+CHART_YWIDTH+YSPACE;
  chartYoffset[2] = CHART_YOFFSET+CHART_YWIDTH+YSPACE;
  chartYoffset[3] = CHART_YOFFSET+CHART_YWIDTH+YSPACE;
  chartYoffset[4] = CHART_YOFFSET+CHART_YWIDTH+YSPACE;
  CHART_XVALUES = (CHART_XWIDTH/PIXELS_PER_SECOND);

  printHelpAndQuit = 0;
  for (i=0; i<argc; i++) {
    if (present(argv[i],"-h")) {
      printHelpAndQuit = 1;
    }
  }
  optCon = smapoptGetContext(argv[0], argc, argv, options, 0);
  if (DEBUG) {
    fprintf(stdout,"Finished smapoptGetContext()\n");
    fflush(stdout);
  }
  if (printHelpAndQuit == 1) {
    smapoptPrintHelp(optCon, stdout, 0);
    return(0);
  }

  while ((rc = smapoptGetNextOpt(optCon)) >= 0) {
    if (DEBUG) {
      fprintf(stderr,"Parsing %c\n",rc);
    }
    switch(rc) {
    case 'A':
      break;
    case 'l':
      yminimumCmdline = 1;
      break;
    case 'u':
      ymaxCmdline = 1;
      break;
    case 'h':
      smapoptPrintHelp(optCon, stdout, 0);
      return(0);
    }
  }
  for (antennaNumber=1; i<=N_ANTENNAS; i++) {
    yminimum[antennaNumber] = 1.0;
    ymax[antennaNumber] = 1.0; /*  3.0; */
  }
  if (strcmp(rmvarArgument,INITIAL_SMAPOPT_STRING_ARGUMENT)==0) {
    strcpy(rmvar,"RM_SYNCDET2_CHANNELS_V2_F");
  } else {
    for (i=0; i<strlen(rmvarArgument); i++) {
      rmvarArgument[i] = toupper(rmvarArgument[i]);
    }
    strcpy(rmvar,rmvarArgument);
  }
  fprintf(stderr,"Using variable = %s\n",rmvar);
  if (xvalues > MAX_XVALUES) {
    fprintf(stderr,"Too many x values selected (must be <= %d)\n",MAX_XVALUES);
    return(0);
  }
  ptr = getenv("TERM");
  if (ptr == NULL) {
    fprintf(stderr,"Sorry, but the terminal type is not defined.  You must run this command from an xterm window.\n");
    setenvMessage();
    return(0);
  }
  if (!present(ptr,"xterm")) {
    fprintf(stderr,"Sorry, but the terminal type is %s.  You must run this command from an xterm window.\n",ptr);
    setenvMessage();
    return(0);
  }
  if (help) {
    smapoptPrintHelp(optCon, stdout, 0);
    return(0);
  }
  strcpy(rmvarCopy,rmvar);
  tokens = tokenize(rmvarCopy,token,"_");
  typeToken = tokens-1;
  arrayToken = tokens-2;
  array2Token = tokens-3;
  if ((ptr=strstr(token[arrayToken],"V")) != NULL) {
    ptr++;
    /*
    if (isdigit(ptr[0])) {
      printf("This is an array variable of some sort.\n");
    }
    */
    if ((ptr2=strstr(token[array2Token],"V")) != NULL) {
      ptr2++;
      if (isdigit(ptr2[0])) {
	fprintf(stderr,"This is a two-dimensional array variable.\n");
	sscanf(ptr2,"%d",&array2Dim);
	if (rmvarindex2 >= array2Dim) {
	  fprintf(stderr,"The second dimension of the array variable is of size %d. Your index is too large!\n",array2Dim);
	  return(0);
	} else {
	  fprintf(stderr,"The second dimension of the array variable is of size %d. Using index %d\n",array2Dim,rmvarindex2);
	}
      }
    }
    if (isdigit(ptr[0])) {
      sscanf(ptr,"%d",&arrayDim);
      if (rmvarindex >= arrayDim) {
	if (array2Dim > 0) {
	  fprintf(stderr,"The first dimension of the array variable is of size %d. Your index is too large!\n",arrayDim);
	} else {
	  fprintf(stderr,"This is a one-dimensional array variable of size %d. Your index is too large!\n",arrayDim);
	}
	return(0);
      } else {
	if (array2Dim > 0) {
	  fprintf(stderr,"The first dimension of the array variable is of size %d. Using index %d\n",
		 arrayDim,rmvarindex);
	} else {
	  fprintf(stderr,"This is a one-dimensional array variable of size %d. Using index %d\n",
		 arrayDim,rmvarindex);
	}
      }
    }
  }
  if (present(token[typeToken],"F")) {
    fprintf(stderr,"This is a floating point variable\n");
    vartype = FLOATING;
  }
  if (present(token[typeToken],"D")) {
    fprintf(stderr,"This is a double precision variable\n");
    vartype = DOUBLE;
  }
  if (present(token[typeToken],"L")) {
    fprintf(stderr,"This is a 4-byte integer variable\n");
    vartype = INT32;
  }
  if (present(token[typeToken],"S")) {
    fprintf(stderr,"This is a 2-byte integer variable\n");
    vartype = INT16;
  }
  if (xvalues > 0) {
    CHART_XVALUES = xvalues;
    xmax = xvalues;
    PIXELS_PER_SECOND = CHART_XWIDTH/CHART_XVALUES;
  }
  if (noautoscale == 1) {
    autoscale = 0;
  }
  if (yminimumCmdline==1) {
    for (antennaNumber=1; antennaNumber<=N_ANTENNAS; antennaNumber++) {
      yminimum[antennaNumber] = yminimumGiven;
    }
    fprintf(stderr,"Using yminimum = %f\n",yminimum[1]);
  }
  if (ymaxCmdline==1) {
    for (antennaNumber=1; antennaNumber<=N_ANTENNAS; antennaNumber++) {
      ymax[antennaNumber] = ymaxGiven;
    }
    fprintf(stderr,"Using ymax = %f\n",ymax[1]);
  }
  uname(&unamebuf);

  antennaNumber = 1;
  rms = rm_open(rm_list);
  fprintf(stderr,"Will call rm_read using antenna number = %d\n",antennaNumber);
  fprintf(stderr,"Hit control-C to stop taking data\n");
  rms = getNextValue(&nextvalue,antennaNumber);
  fprintf(stderr,"First value from antenna %d (index %d) = %f\n",antennaNumber,
	  rmvarindex,nextvalue);

  /* install control c handler */
  action.sa_flags = 0;
  action.sa_handler = cleanupTune;
  sigemptyset(&action.sa_mask);
  if (sigaction(SIGINT,  &action, &oldAction) != 0) { 
    perror("sigaction(SIGINT):");
  } 
  
  init();
  usleep(100000);
  do {
    needToRefresh = 0;
    for (antennaNumber=1; antennaNumber <= 8; antennaNumber++) {
      rms = getNextValue(&nextvalue,antennaNumber);
      buf[antennaNumber][pointsRead] = nextvalue;
      needToRefresh |= checkAutoScale(nextvalue,antennaNumber);
    }
    if (needToRefresh) {
      refresh();
    }
    for (antennaNumber=1; antennaNumber <= 8; antennaNumber++) {
      updateStripChart(antennaNumber,pointsRead,totalPointsRead,needToRefresh);
    }
    totalPointsRead++;
    pointsRead++;
    fflush(stdout);
    usleep(500000);
    if (pointsRead >= CHART_XVALUES) {
      refresh();
      pointsRead = 0;
    }
  } while (1);
  return(pointsRead);
}

void updateStripChart(int antenna, int pointsRead, int totalPointsRead, int redraw) {
  int y,x;
  int i;
  int pixelsPerSecond;

  pixelsPerSecond = PIXELS_PER_SECOND;
  if (pointsRead > 0) {
    if (redraw) {
      x = chartXoffset[antenna] + (0)*pixelsPerSecond;
      y = floor(chartYoffset[antenna]+CHART_YWIDTH*(buf[antenna][pointsRead-1]-yminimum[antenna])/(ymax[antenna]-yminimum[antenna]));
      move(x,y);
      for (i=1; i<=pointsRead; i++) {
	x = chartXoffset[antenna] + i*pixelsPerSecond;
	y = floor(chartYoffset[antenna]+CHART_YWIDTH*(buf[antenna][i]-yminimum[antenna])/(ymax[antenna]-yminimum[antenna]));
	line(x,y);
      }
    } else {
      x = chartXoffset[antenna] + (pointsRead-1)*pixelsPerSecond;
      y = floor(chartYoffset[antenna]+CHART_YWIDTH*(buf[antenna][pointsRead-1]-yminimum[antenna])/(ymax[antenna]-yminimum[antenna]));
      move(x,y);
      x  = chartXoffset[antenna] + (pointsRead)*pixelsPerSecond;
      y = floor(chartYoffset[antenna]+CHART_YWIDTH*(buf[antenna][pointsRead]-yminimum[antenna])/(ymax[antenna]-yminimum[antenna]));
      line(x,y);
    }
  }
}

void init(void) {
  xterm();
  setup();
  refresh();
}

void refresh(void) {
  int i;
  clear();
  for (i=1; i<=N_ANTENNAS; i++) {
    drawBorder(i);
    drawAntennaLabel(i);
  }
  title(rmvar,rmvarindex);
  xlabel();
}

void drawAntennaLabel(int antenna) {
  char string[25];
  switch (antenna) {
  case 1: move(220,700); break;
  case 2: move(450,700); break;
  case 3: move(670,700); break;
  case 4: move(910,700); break;
  case 5: move(220,380); break;
  case 6: move(450,380); break;
  case 7: move(670,380); break;
  case 8: move(910,380); break;
  }
  sprintf(string,"Antenna %d",antenna);
  center(string);
}

void drawBorder(int antenna) {
  move(chartXoffset[antenna], chartYoffset[antenna]);
  line(chartXoffset[antenna]+CHART_XWIDTH,chartYoffset[antenna]);
  line(chartXoffset[antenna]+CHART_XWIDTH,chartYoffset[antenna]+CHART_YWIDTH);
  line(chartXoffset[antenna], chartYoffset[antenna]+CHART_YWIDTH);
  line(chartXoffset[antenna], chartYoffset[antenna]);
}

void checkReflectiveMemoryStatus(int status, char *string) {
    if (status != RM_SUCCESS) {
      rm_error_message(status,string);
      /*      SleepExit(1); */
    }
}

int call_rm_read(int ant, char *name, void *value) {
  char statusString[150];
  int rm_status;

  sprintf(statusString,"call_rm_read(%s)",name);
  rm_status = rm_read(ant,name,value);
  checkReflectiveMemoryStatus(rm_status,statusString);
  return(rm_status);
}

int present(char *search,char *token) {
  if (strstr(search,token) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

void title(char *rmvar, int index) {
  char rmvarIndex[RM_NAME_LENGTH+5];
  char header[30];

  sprintf(header,"stripchart8 is showing:");
  move(512,730);
  if (arrayDim == 1) {
    sprintf(rmvarIndex,"%s %s",header,rmvar);
  } else {
    if (array2Dim == 0) {
      sprintf(rmvarIndex,"%s %s[%d]", header,rmvar,rmvarindex);
    } else {
      sprintf(rmvarIndex,"%s %s[%d][%d]",header,
	      rmvar,rmvarindex,rmvarindex2);
    }
  }
  center(rmvarIndex);

}

void ylabel(int antenna) {
  char string[40];
  float i, yinc;

  /*
  move(CHART_XOFFSET-50,CHART_YOFFSET+CHART_YWIDTH/2);
  center("Volts");
  */

  yinc = (ymax[antenna]-yminimum[antenna])*0.33;
  for (i=yminimum[antenna]+yinc; i<(ymax[antenna]-0.5*yinc); i+=yinc) {
    move(chartXoffset[antenna]-50, 
	 chartYoffset[antenna]+(CHART_YWIDTH*(i-yminimum[antenna])/(ymax[antenna]-yminimum[antenna])));
    sprintf(string,"%.3f",i);
    center(string); 
  }
  move(chartXoffset[antenna]-50, chartYoffset[antenna]+CHART_YWIDTH);
  sprintf(string,"%.3f",ymax[antenna]);
  center(string); 
  move(chartXoffset[antenna]-50, chartYoffset[antenna]);
  sprintf(string,"%.3f",yminimum[antenna]);
  center(string); 
}

void xlabel(void) {
  char string[40];
  float xinc,i;
  int antenna;

  xinc = 60;
  antenna = 5;
  for (i=xmin; i<floor(xmax); i+=xinc) {
    move(chartXoffset[antenna]+CHART_XWIDTH*((i-xmin)/(xmax-xmin)), 
	 chartYoffset[antenna]-16);
    if (((int)floor(i) % 30) == 0) {
      sprintf(string,"%.0f",i);
      center(string); 
    }
  }
  move(chartXoffset[antenna]+CHART_XWIDTH/2, chartYoffset[antenna]-40);
  center("Time(seconds)");
}

int checkAutoScale(float value, int antenna) {
  static float historicalMin[N_ANTENNAS+1];
  static float historicalMax[N_ANTENNAS+1];
  static int firstTime = 1;
  int i;
#define HIGH_PAD 1.02
#define LOW_PAD 0.98
  int expanding = 0;

  if (firstTime == 1) {
    firstTime = 0;
    for (i=1; i<=N_ANTENNAS; i++) {
      historicalMin[i] = +9.9E9;
      historicalMax[i] = -9.9E9;
    }
  }
  if (autoscale) {
    if (pointsRead < 1) {
      historicalMin[antenna] = value;
      historicalMax[antenna] = value;
    }
    if (value > historicalMax[antenna]) {
      historicalMax[antenna] = value;
    }
    if (value < historicalMin[antenna]) {
      historicalMin[antenna] = value;
    }
    if (pointsRead>CHART_XVALUES || 1) {
      /* limits should follow entire history of the value */
      if (historicalMax[antenna] > 0) {
	if (value >= ymax[antenna] || pointsRead < 1) {
	  /* only expand limits if plot goes outside present range */
	  ymax[antenna] = historicalMax[antenna]*HIGH_PAD;
	  expanding = 1;
	}
      } else {
	/* all values are negative */
	if (value >= ymax[antenna] || pointsRead < 1) {
	  ymax[antenna] = historicalMax[antenna]*LOW_PAD;
	  expanding = 1;
	}
      }
      if (historicalMin[antenna] > 0) {
	/* all values are positive */
	if (value <= yminimum[antenna] || pointsRead < 1) {
	  yminimum[antenna] = historicalMin[antenna]*LOW_PAD;
	  expanding = 1;
	}
      } else {
	if (value <= yminimum[antenna] || pointsRead < 1) {
	  yminimum[antenna] = historicalMin[antenna]*HIGH_PAD;
	  expanding = 1;
	}
      }
    }
  } else {
    /* this section is defunct */
    if (value > ymax[antenna]) {
      if (value > 0) {
	ymax[antenna] = value*HIGH_PAD;
      } else {
	ymax[antenna] = value*LOW_PAD;
      }
      expanding = 1;
      /*
      printf("expanding ymax to %f\n",ymax);
      */
    }
    if (value < yminimum[antenna]) {
      if (yminimum[antenna] < 0) {
	yminimum[antenna] = value*HIGH_PAD;
      } else {
	yminimum[antenna] = value*LOW_PAD;
      }
      expanding = 1;
      /*
      printf("expanding yminimum to %f\n",yminimum[antenna]);
      */
    }
  }
  return(expanding);
}

int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *delimit) {
  int i;
  int non_blanks = 0;
  int tokens = 0;

  if (strlen(input) > 0) {
    for (i=0; i<strlen(input); i++) {
      if (input[i] != ' ') {
        non_blanks = 1; break;
      }
    }
    if (non_blanks == 0) return(0);
    tokenArray[tokens++] = strtok(input,delimit);
    while ((tokenArray[tokens] = strtok(NULL,delimit)) != NULL) {
      tokens++;
    }
  }
  return(tokens);
}

int getNextValue(float *nextvalue, int rmAntennaNumber) {
  int rms;
  short sv;
  int iv;
  float fv;
  double dv;
  static float *fva = NULL;
  static double *dva = NULL;
  static int *iva = NULL;
  static short *sva = NULL;

  if (arrayDim > 1) {
    switch (vartype) {
    case FLOATING:
      if (fva == NULL) {
	fva = malloc(sizeof(float)*arrayDim);
      }
      rms = call_rm_read(rmAntennaNumber,rmvar,fva);
      *nextvalue = fva[rmvarindex];
      break;
    case INT16:
      if (sva == NULL) {
	sva = malloc(sizeof(short)*arrayDim);
      }
      rms = call_rm_read(rmAntennaNumber,rmvar,sva);
      *nextvalue = sva[rmvarindex];
      break;
    case INT32:
      if (iva == NULL) {
	iva = malloc(sizeof(int)*arrayDim);
      }
      rms = call_rm_read(rmAntennaNumber,rmvar,iva);
      *nextvalue = iva[rmvarindex];
      break;
    case DOUBLE:
      if (dva == NULL) {
	dva = malloc(sizeof(double)*arrayDim);
      }
      rms = call_rm_read(rmAntennaNumber,rmvar,dva);
      *nextvalue = dva[rmvarindex];
      break;
    }
  } else {
    switch (vartype) {
    case FLOATING:
      rms = call_rm_read(rmAntennaNumber,rmvar,&fv);
      *nextvalue = fv;
      break;
    case INT16:
      rms = call_rm_read(rmAntennaNumber,rmvar,&sv);
      *nextvalue = sv;
      break;
    case INT32:
      rms = call_rm_read(rmAntennaNumber,rmvar,&iv);
      *nextvalue = iv;
      break;
    case DOUBLE:
      rms = call_rm_read(rmAntennaNumber,rmvar,&dv);
      *nextvalue = dv;
      break;
    }
  }
  if (rms != 0) {
    rm_error_message(rmAntennaNumber,"rm_read");
    return(0);
  }
  return(rms);
}

void cleanupTune(int sig, siginfo_t *extra, void *cruft) {
  cleanup();
  exit(0);
}

void setenvMessage(void) {
    fprintf(stderr,"If you think you are in an xterm, then you may have the TERM environmental variable set wrong (possibly due to a definition in your .cshrc file).  If so, then you should do 'setenv TERM xterm' and try running stripchart8 again.\n");
}

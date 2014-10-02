#define MAX_TOKENS 15 /* in an RM var name */
enum {INT16=0, INT32, FLOATING, DOUBLE};
#define CYCLES 999999
#define DEF_PIXELS_PER_SECOND 60
#define DEF_CHART_XVALUES 15
#define DEF_CHART_XWIDTH 900 /* (CHART_XVALUES*PIXELS_PER_SECOND) */
#define MAX_XVALUES 600
#define CHART_YWIDTH 600

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

int getNextValue(float *nextvalue);
void title(char *rmvar, int index);
void xlabel(void);
int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *delimit);
void ylabel(void);
void drawBorder(void);
void init(void);
void refresh(void);
void updateStripChart(float newValue);
void drawBufline(int ptr);
int rmvarindex = 0;
int rmvarindex2 = 0;
int pointsRead = 0;
int autoscale = 1; /* default behavior if -n not given */
int noautoscale = 0;
void checkAutoScale(float value);

int PIXELS_PER_SECOND = DEF_PIXELS_PER_SECOND;
int CHART_XVALUES = DEF_CHART_XVALUES;
int CHART_XWIDTH = DEF_CHART_XWIDTH;
int xvalues = 0;
static float yminimum = 1.0;
static float ymax = 3.0;
static float xmin = 0.0;
static float xmax = DEF_CHART_XVALUES; /* assumes 1Hz update */
static float buf[MAX_XVALUES];
int vartype;
int arrayDim = 1;
int array2Dim = 0;
int rmAntennaNumber = -1;

#ifdef USE_AS_SUBROUTINE
extern char rmvar[RM_NAME_LENGTH];
extern int antennaNumber; /* 1..8 */
extern int runningOnHal9000;
void cleanupControlC(int signal);
int keepgoing;

int stripchart(int antennaNumber, int rx, float bigBuffer[BIG_BUFFER_SIZE],
	       float bigBufferRx1[BIG_BUFFER_SIZE]) {
  float nextvalue;
  struct sigaction actionSIGINT, oldActionSIGINT;
  int rms;

  /* used only by the coldload command */

  xvalues = 45; /* how many seconds to display? */
  CHART_XVALUES = xvalues;
  xmax = xvalues;
  PIXELS_PER_SECOND = CHART_XWIDTH/CHART_XVALUES;

  if (runningOnHal9000==1) {
    rmAntennaNumber = antennaNumber;
  } else {
    rmAntennaNumber = 0;
  }
  rmvarindex = rx;
  arrayDim = 2;
  vartype = FLOATING;
  strcpy(rmvar,"RM_CONT_DET_MUWATT_V2_F");
  keepgoing = 1;
  init();
  /* install cntrl-C handler */
  actionSIGINT.sa_flags = 0;
  actionSIGINT.sa_handler = cleanupControlC;
  sigemptyset(&actionSIGINT.sa_mask);
  if (sigaction(SIGINT,  &actionSIGINT, &oldActionSIGINT) != 0) { 
    perror("sigaction:");
  } 
  do {
    /* assumes rm already open in calling program */
    rmvarindex = 0;
    rms = getNextValue(&nextvalue);
    bigBuffer[pointsRead] = nextvalue;
    if (rx == 0) {
      updateStripChart(nextvalue);
    }

    rmvarindex = 1;
    rms = getNextValue(&nextvalue);
    bigBufferRx1[pointsRead] = nextvalue;
    if (rx == 1) {
      updateStripChart(nextvalue);
    }
    pointsRead++;
    sleep(1);
  } while (keepgoing==1);
  cleanup();
  if (sigaction(SIGINT,  &oldActionSIGINT, &actionSIGINT) != 0) { 
    perror("sigaction:");
  } 
  return(pointsRead);
}

void cleanupControlC(int signal) {
  keepgoing = 0;
}

#else
char rmvar[RM_NAME_LENGTH];
int runningOnHal9000;
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
  int useAllAntennasInProject;
  int antennaArgument[ANT_LIST_SZ];
  int antennasGiven = 0;
  int DEBUG = 0;
  int rc;
  int typeToken, tokens, arrayToken, array2Token;
  char *token[MAX_TOKENS];
  char *rmvarArgument      = INITIAL_SMAPOPT_STRING_ARGUMENT;
  double yminimumGiven, ymaxGiven;
  int yminimumCmdline = 0;
  struct utsname unamebuf;
  int ymaxCmdline = 0;
  struct smapoptOption options[] = {
  {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, antennaArgument, 'a',"antenna argument (this supercedes -s)"},
  {"noautoscale", 'n', SMAPOPT_ARG_NONE, &noautoscale, 'n',"autoscale after one screen width"},
  {"lower",'l', SMAPOPT_ARG_DOUBLE, &yminimumGiven, 'l', "Min value for y scale"},
  {"upper",'u', SMAPOPT_ARG_DOUBLE, &ymaxGiven, 'u', "Max value for y scale"},
  {"rmvar",'r', SMAPOPT_ARG_STRING, &rmvarArgument, 'r', "plot the specified RM variable (float,double,int, or short; default=RM_SYNCDET2_CHANNELS_V2_F)"},
  {"index",'i', SMAPOPT_ARG_INT, &rmvarindex, 'i', "which index of the RM array variable (default=0)"},
  {"jindex",'j', SMAPOPT_ARG_INT, &rmvarindex2, 'j', "second index of the RM array variable (applicable to 2D arrays only, default=0)"},
  {"variable",'v', SMAPOPT_ARG_STRING, &rmvarArgument, 'v', "plot the specified RM variable (default=RM_SYNCDET2_CHANNELS_V2_F)"},
  {"xvalues",'x', SMAPOPT_ARG_INT, &xvalues, 'x', "how many 1Hz xvalues to show on one screen (up to 600)"},
  {"debug",'D', SMAPOPT_ARG_NONE, &DEBUG, 0, "print debugging messages"},
  SMAPOPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
  };
  char *hostptr;
  int useSmaPopt = 1;

  hostptr = getenv("HOST");
  if (hostptr != NULL) {
    if ((!present(hostptr,"hal") && !present(hostptr,"acc")) ||
        (present(hostptr,"acc0"))) {
      useSmaPopt = 0;
      printf("Will not use smapopt\n");
      /*
      tune6Flag = 1;
      if (present(hostptr,"acc0")) {
        strcpy(serverName,hostptr);
      }
      */
    }
  }
  if (useSmaPopt == 1) {
    optCon = smapoptGetContext(argv[0], argc, argv, options, 0);
    if (DEBUG) {
      fprintf(stdout,"Finished smapoptGetContext()\n");
      fflush(stdout);
    } 
    while ((rc = smapoptGetNextOpt(optCon)) >= 0) {
      switch(rc) {
      case 'a':
	antennasGiven = 1;
	break;
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
    if (help) {
      smapoptPrintHelp(optCon, stdout, 0);
      return(0);
    }
  }
  if (strcmp(rmvarArgument,INITIAL_SMAPOPT_STRING_ARGUMENT)==0) {
    strcpy(rmvar,"RM_SYNCDET2_CHANNELS_V2_F");
  } else {
    for (i=0; i<strlen(rmvarArgument); i++) {
      rmvarArgument[i] = toupper(rmvarArgument[i]);
    }
    strcpy(rmvar,rmvarArgument);
  }
  printf("Using variable = %s\n",rmvar);
  if (xvalues > MAX_XVALUES) {
    printf("Too many x values selected (must be <= %d)\n",MAX_XVALUES);
    return(0);
  }
  ptr = getenv("TERM");
  if (ptr == NULL) {
    printf("Sorry, but the terminal type is not defined.  You must run this command from an xterm window.\n");
    return(0);
  }
  if (!present(ptr,"xterm")) {
    printf("Sorry, but the terminal type is %s.  You must run this command from an xterm window.\n",ptr);
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
	printf("This is a two-dimensional array variable.\n");
	sscanf(ptr2,"%d",&array2Dim);
	if (rmvarindex2 >= array2Dim) {
	  printf("The second dimension of the array variable is of size %d. Your index is too large!\n",array2Dim);
	  return(0);
	} else {
	  printf("The second dimension of the array variable is of size %d. Using index %d\n",array2Dim,rmvarindex2);
	}
      }
    }
    if (isdigit(ptr[0])) {
      sscanf(ptr,"%d",&arrayDim);
      if (rmvarindex >= arrayDim) {
	if (array2Dim > 0) {
	  printf("The first dimension of the array variable is of size %d. Your index is too large!\n",arrayDim);
	} else {
	  printf("This is a one-dimensional array variable of size %d. Your index is too large!\n",arrayDim);
	}
	return(0);
      } else {
	if (array2Dim > 0) {
	  printf("The first dimension of the array variable is of size %d. Using index %d\n",
		 arrayDim,rmvarindex);
	} else {
	  printf("This is a one-dimensional array variable of size %d. Using index %d\n",
		 arrayDim,rmvarindex);
	}
      }
    }
  }
  if (present(token[typeToken],"F")) {
    printf("This is a floating point variable\n");
    vartype = FLOATING;
  }
  if (present(token[typeToken],"D")) {
    printf("This is a double precision variable\n");
    vartype = DOUBLE;
  }
  if (present(token[typeToken],"L")) {
    printf("This is a 4-byte integer variable\n");
    vartype = INT32;
  }
  if (present(token[typeToken],"S")) {
    printf("This is a 2-byte integer variable\n");
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
  if (DEBUG) {
    for (i=0; i<=8; i++) {
      fprintf(stderr,"antennaArgument[i] = %d\n",antennaArgument[i]);
    }
  }
  if (yminimumCmdline==1) {
    yminimum = yminimumGiven;
    printf("Using yminimum = %f\n",yminimum);
  }
  if (ymaxCmdline==1) {
    ymax = ymaxGiven;
    printf("Using ymax = %f\n",ymax);
  }
  useAllAntennasInProject = 0;
  uname(&unamebuf);
  if (strcmp(unamebuf.nodename,"hal9000") != 0) {
    runningOnHal9000 = 0;
    sscanf(unamebuf.nodename,"acc%d",&antennaNumber);
  } else {
    runningOnHal9000 = 1;
  }

  if (antennasGiven == 0) {
    if (runningOnHal9000==1) {
      printf("You must give an antenna number\n");
      return(0);
    }
  } else {
    if (DEBUG) {
      printf("We have seen the -a option\n");
    }
    if (runningOnHal9000==1) {
      for (i=1; i<=8; i++) {
	if (antennaArgument[i] == 1) {
	  antennaNumber = i;
	  break;
	}
      }
    } else {
      printf("It makes no sense to give -a when running from this program from an antenna computer\n");
      return(0);
    }
  }
  if (runningOnHal9000 && (antennaNumber<1 || antennaNumber >8)) {
    printf("Invalid antenna number = %d\n",antennaNumber);
    return(0);
  }
  if (runningOnHal9000==1) {
    rmAntennaNumber = antennaNumber;
  } else {
    rmAntennaNumber = 0;
  }
  rms = rm_open(rm_list);
  printf("Will call rm_read using antenna number = %d\n",rmAntennaNumber);
  printf("Hit control-C to stop taking data\n");
  rms = getNextValue(&nextvalue);
  printf ("First value from antenna %d (index %d) = %f\n",antennaNumber,
	  rmvarindex,nextvalue);
  checkAutoScale(nextvalue);
  init();
  usleep(100000);
  i= 0;
  do {
    rms = getNextValue(&nextvalue);
    pointsRead++;
    updateStripChart(nextvalue);
    sleep(1);
  } while (++i < BIG_BUFFER_SIZE);
  return(pointsRead);
}
#endif

void updateStripChart(float newValue) {
  /* stick latest value into the circular buffer */
  static int bufptr = 0;
  /* startPlot is where to start reading from the array for plotting */
  int startPlot;

  refresh();
  bufptr++;
  if (bufptr >= CHART_XVALUES) {
    bufptr = 0;
  }
  buf[bufptr] = newValue;
  checkAutoScale(newValue);
  xlabel();
  ylabel();
  title(rmvar,rmvarindex);
  startPlot = bufptr+1;
  if (pointsRead < CHART_XVALUES) {
    startPlot= 0;
  }
  if (startPlot >= CHART_XVALUES) {
    startPlot= 0;
  }
  drawBufline(startPlot);  
  cleanup();
}

void redraw(void) {
}

void drawBufline(int ptr) {
  /* plot circular buffer, starting at oldest point, which is ptr */
#define CHART_XOFFSET 100
#define CHART_YOFFSET 100
  int x; /* in units of pixels */
  int y,i;
  int pixelsPerSecond = PIXELS_PER_SECOND;
  int xpix; /* in units of number of values */
  int index;

  if (pointsRead<CHART_XVALUES) {
    xpix = pointsRead;
  } else {
    xpix = CHART_XVALUES;
  }

  for (i=0; i<xpix; i++) {
    x  = CHART_XOFFSET + i*pixelsPerSecond;
    index = ptr+i;
    if (index >= CHART_XVALUES) {
      index -= CHART_XVALUES;
    }
    y= floor(CHART_YOFFSET+CHART_YWIDTH*(buf[index]-yminimum)/(ymax-yminimum));
#if 0
    printf("extrema(%f,%f), Drawing to (%d,%d)\n",yminimum,ymax,x,y);
#endif
    if (i==0) {
      move(x,y);
    } else {
      line(x,y);
    }
  }
}

void init(void) {
  xterm();
  refresh();
}

void refresh(void) {
  setup();
  clear();
  drawBorder();
}

void drawBorder(void) {
  move(CHART_XOFFSET,CHART_YOFFSET);
  line(CHART_XOFFSET+CHART_XWIDTH, CHART_YOFFSET);
  line(CHART_XOFFSET+CHART_XWIDTH, CHART_YOFFSET+CHART_YWIDTH);
  line(CHART_XOFFSET, CHART_YOFFSET+CHART_YWIDTH);
  line(CHART_XOFFSET, CHART_YOFFSET);
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

  move(512,730);
  if (arrayDim == 1) {
    sprintf(rmvarIndex,"Antenna %d: %s",antennaNumber,
	    rmvar);
  } else {
    if (array2Dim == 0) {
      sprintf(rmvarIndex,"Antenna %d: %s[%d]",antennaNumber,
	      rmvar,rmvarindex);
    } else {
      sprintf(rmvarIndex,"Antenna %d: %s[%d][%d]",antennaNumber,
	      rmvar,rmvarindex,rmvarindex2);
    }
  }
  center(rmvarIndex);
}

void ylabel(void) {
  char string[40];
  float i, yinc;

  /*
  move(CHART_XOFFSET-50,CHART_YOFFSET+CHART_YWIDTH/2);
  center("Volts");
  */

  yinc = (ymax-yminimum)*0.33;
  for (i=yminimum+yinc; i<(ymax-0.5*yinc); i+=yinc) {
    move(CHART_XOFFSET-50, CHART_YOFFSET+(CHART_YWIDTH*(i-yminimum)/(ymax-yminimum)));
    sprintf(string,"%.3f",i);
    center(string); 
  }
  move(CHART_XOFFSET-50,CHART_YOFFSET+CHART_YWIDTH);
  sprintf(string,"%.3f",ymax);
  center(string); 
  move(CHART_XOFFSET-50,CHART_YOFFSET);
  sprintf(string,"%.3f",yminimum);
  center(string); 
}

void xlabel(void) {
  char string[40];
  float xinc,i;
  int divisor;

  if (xmax < 100) {
    divisor = 25;
  } else {
    divisor = 15;
  }
  xinc = 1+floor((xmax-xmin) / divisor);
  if (pointsRead > CHART_XVALUES) {
    xmin++;
    xmax++;
  }
  for (i=xmin; i<floor(xmax); i+=xinc) {
    move(CHART_XOFFSET+CHART_XWIDTH*((i-xmin)/(xmax-xmin)), CHART_YOFFSET-16);
    sprintf(string,"%.0f",i);
    center(string); 
  }
  move(CHART_XOFFSET+CHART_XWIDTH/2,CHART_YOFFSET-40);
  center("Time(seconds)");
}

void checkAutoScale(float value) {
  static float historicalMin = +9.9E9;
  static float historicalMax = -9.9E9;
#define HIGH_PAD 1.03
#define LOW_PAD 0.97

  if (autoscale) {
    if (pointsRead < 1) {
      historicalMin = value;
      historicalMax = value;
    }
    if (value > historicalMax) {
      historicalMax = value;
    }
    if (value < historicalMin) {
      historicalMin = value;
    }
    if (pointsRead>CHART_XVALUES || 1) {
      /* limits should follow entire history of the value */
      if (historicalMax > 0) {
	ymax = historicalMax*HIGH_PAD;
      } else {
	ymax = historicalMax*LOW_PAD;
      }
      if (historicalMin > 0) {
	yminimum = historicalMin*LOW_PAD;
      } else {
	yminimum = historicalMin*HIGH_PAD;
      }
    }
  } else {
    /* only expand limits if plot goes outside specified range */
    if (value > ymax) {
      if (value > 0) {
	ymax = value*HIGH_PAD;
      } else {
	ymax = value*LOW_PAD;
      }
      printf("expanding ymax to %f\n",ymax);
    }
    if (value < yminimum) {
      if (yminimum < 0) {
	yminimum = value*HIGH_PAD;
      } else {
	yminimum = value*LOW_PAD;
      }
      printf("expanding yminimum to %f\n",yminimum);
    }
  }
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

int getNextValue(float *nextvalue) {
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


#define SOLID 200
#define N_ANTENNAS 8
#define START_ANTENNA 1
/*
layout of antenna panels:
  1 2 3 4
  5 6 7 8
 */
#define LIMIT 100
#define CHART_XOFFSET 100
#define CHART_YOFFSET  80
#define XSPACE 12
#define YSPACE 74 /* 24 */
int CHART_XWIDTH = 216;   /* ((DEF_CHART_XWIDTH-3*XSPACE)/4); */
                          /* (900-36)/4 = 216; 216/12 = 18 */
int CHART_YWIDTH = 263; /* 288 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utsname.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include "smapopt.h"
#include "rm.h"
#include "ivplot.h"
#include "tek_driv.h"

void init8(void);
char *monthToString(int month);
void drawDatestring(int antenna, char *string);
void title8(int type);
void drawAntennaLabel(int antenna, int rx);
void refresh8(void);
int totalPoints;
float ymax = 3.0;
float ymin = 1.0;
float xmin = 0.0;
void drawOperatingPoint(float x);
float xmax = 3.0;
float zmin, zmax;
char buf[200];
char filename[200];
char newestFilename[200];

int runningOnHal9000;

int sweeps = 2;
int chartXoffset[N_ANTENNAS+1];
int chartYoffset[N_ANTENNAS+1];
int type = IVSWEEP;
int antennaNumber;
int DEBUG = 0;
int rmvarindex = 0;
char rmvar[RM_NAME_LENGTH];
int vbias = 0;

int main(int argc, char *argv[]) {
  int newestTime = 0;
  DIR *dirPtr;
  char searchDir[100];
  int help = 0;
  int i;
  float vset,vread,iread,mv,ua,power;
  char *ptr;
  smapoptContext optCon; 
  int statStatus;
  int rc, narg;
  FILE *fp;
  double yminimumGiven, ymaxGiven;
  int yminimumCmdline = 0;
  struct utsname unamebuf;
  struct dirent *nextEnt;
  struct stat messageStat;
  int ymaxCmdline = 0;
  char datestring[20];
  int year,month,day,hour,minute,second;
  int sweepsFound = 0;
  float operatingPoint;
  int pointsRead = 0;
  int receiver = 0;
  int rm_status;
  int rm_list[RM_ARRAY_SIZE];
  int rx;

  struct smapoptOption options[] = {
  {"file",'f', SMAPOPT_ARG_STRING, filename, 'f', "Filename to plot (Default = most recent in /instance/ivcurves"},
  {"lower",'l', SMAPOPT_ARG_DOUBLE, &yminimumGiven, 'l', "Min value for y scale"},
  {"upper",'u', SMAPOPT_ARG_DOUBLE, &ymaxGiven, 'u', "Max value for y scale"},
  {"receiver",'r', SMAPOPT_ARG_INT, &receiver, 'u', "Receiver band: 0=low-freq (Default), 1=high-freq"},
  {"vbias",'v', SMAPOPT_ARG_NONE, &vbias, 'v', "Plot the vbias value when the sweep was taken (default is to plot the present vbias)"},
  {"bfield",'b', SMAPOPT_ARG_NONE, &type, 0, "plot most recent Bfield sweep instead of I/V sweep"},
  {"debug",'D', SMAPOPT_ARG_NONE, &DEBUG, 0, "print debugging messages"},
  SMAPOPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
  };
  bzero(filename,sizeof(filename));
  optCon = smapoptGetContext(argv[0], argc, argv, options, 0); 
  /*			     SMAPOPT_CONTEXT_NOLOG);*/
  if (DEBUG) {
    fprintf(stdout,"Finished smapoptGetContext()\n");
    fflush(stdout);
  }
  while ((rc = smapoptGetNextOpt(optCon)) >= 0) {
    switch(rc) {
    case 'b':
      type = PBSWEEP;
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
  ptr = getenv("TERM");
  if (ptr == NULL) {
    printf("Sorry, but the terminal type is not defined.  You must run this command from an xterm window.\n");
    return(0);
  }
  if (!present(ptr,"xterm")) {
    printf("Sorry, but the terminal type is %s.  You must run this command from an xterm window.\n",ptr);
    return(0);
  }
  if (help) {
    smapoptPrintHelp(optCon, stdout, 0);
    return(0);
  }
  if (yminimumCmdline==1) {
    ymin = yminimumGiven;
    printf("Using yminimum = %f\n",ymin);
  }
  if (ymaxCmdline==1) {
    ymax = ymaxGiven;
    printf("Using ymax = %f\n",ymax);
  }
  /*
  if (receiver != 0) {
    printf("The '-r 1' option does not yet work.\n");
    return(0);
  }
  */
  uname(&unamebuf);
  if (strcmp(unamebuf.nodename,"hal9000") != 0) {
    printf("You must run this program on hal9000\n");
    return(0);
  } else {
    runningOnHal9000 = 1;
  }
  if (vbias == 0) {
    rm_status = rm_open(rm_list);
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

  init8();
  for (antennaNumber=START_ANTENNA; antennaNumber<=N_ANTENNAS; antennaNumber++) {
    /* figure out the latest filename */
    sweepsFound = 0;
    newestTime = 0;
    sprintf(searchDir,"/otherPowerPCs/acc/%d/ivcurves",antennaNumber);
    if (DEBUG) {
      printf("1) Searching %s\n",searchDir);
    }
    dirPtr = opendir(searchDir);
    if (dirPtr == NULL) {
      fprintf(stderr,"Failed to open directory\n");
    } else {
      if (DEBUG) {
	fprintf(stderr,"Successfully opened directory\n");
      }
    }
    while ((nextEnt = readdir(dirPtr)) != NULL) {
      if (((present(nextEnt->d_name, "ivcurve.") && type == IVSWEEP) ||
	   (present(nextEnt->d_name, "bfieldsweep.") && type==PBSWEEP)) &&
	  !present(nextEnt->d_name, ".ps")) {
	if (receiver == 0 && (present(nextEnt->d_name,"rx4") ||
			      present(nextEnt->d_name,"rx5") ||
			      present(nextEnt->d_name,"rx6") ||
			      present(nextEnt->d_name,"rx7"))) {
	    continue;
	}
	if (receiver == 1 && (present(nextEnt->d_name,"rx0") ||
			      present(nextEnt->d_name,"rx1") ||
			      present(nextEnt->d_name,"rx2") ||
			      present(nextEnt->d_name,"rx3"))) {
	    continue;
	}
	sprintf(filename,"%s/%s",searchDir,nextEnt->d_name);
	statStatus = stat(filename, &messageStat);
	if (statStatus != 0) {
	  perror("stat");
	  exit(0);
	}
#if 0
	printf("%s %d\n",nextEnt->d_name,messageStat.st_mtime);
#endif
	if (messageStat.st_mtime > newestTime) {
	  newestTime = messageStat.st_mtime;
	  sscanf(nextEnt->d_name,"ivcurve.acc%*d.rx%d.",&rx);
	  sprintf(newestFilename,"%s/%s",searchDir,nextEnt->d_name);
	  /*
	    printf("%d = %s\n",newestTime,nextEnt->d_name);
	  */
	}
	sweepsFound++;
      }
    }
    if (DEBUG) {
      printf("Found %d sweeps. Plotting file = %s\n",sweepsFound,newestFilename);
    }
    closedir(dirPtr);
    if ((totalPoints = computeLimits(type,newestFilename)) == 0) {
      move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+40);
      center("Found zero");
      move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+20);
      center("0 data points");
      continue;
    }

    if (type == IVSWEEP) {
      if (DEBUG) {
	printf("Found an IV sweep with %d data points: xrange=[%.3f,%.3f] yrange=[%.3f,%.3f] zrange=[%.3f,%.3f]\n",totalPoints,
	     xmin,xmax,ymin,ymax,zmin,zmax);
      }
    } 
    if (type == PBSWEEP) {
      if (DEBUG) {
	printf("Found a PB sweep with %d data points: xrange=[%.3f,%.3f] yrange=[%.3f,%.3f]\n",
	     totalPoints, xmin,xmax,ymin,ymax);
      }
    } 
    i = 0;
    fp = fopen(newestFilename,"r");
    pointsRead = 0;
    if (fp == NULL) {
      move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+80);
      center("Could not");
      move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+60);
      center("open file");
      continue;
    }
    if (type == IVSWEEP) {
      /* read the header line */
      ptr = fgets(buf,sizeof(buf),fp);
      /*         Vset
		 Vactual
		 Iactual    
		 V(mv)
		 I(ua)
		 Power 2 1.97 197. 248.24 2005-09-29_23h03m02 2.452 */
      sscanf(buf,"%*s %*s %*s %*s %*s %*s %*d %*f %*f %*f %s %f",
	     datestring,&operatingPoint);
      if (vbias == 0) {
	if (receiver == 0) {
	  rm_status = rm_read(antennaNumber,"RM_SIS_MIXER0_VOLTAGE_CALIB_F",
			      &operatingPoint);
	} else {
	  rm_status = rm_read(antennaNumber,"RM_SIS_MIXER1_VOLTAGE_CALIB_F",
			      &operatingPoint);
	}
      }
      sscanf(datestring,"%4d-%2d-%2d_%2dh%2dm%2d",&year,&month,&day,
	     &hour,&minute,&second);
      sprintf(datestring,"%4d%3s%02d_%02d:%02d",year,monthToString(month),
	      day,hour,minute);
      drawDatestring(antennaNumber,datestring);
      /*    printf("Read operating point = %f mV\n",operatingPoint);
       */
    }
    linetype(SOLID);
    drawAntennaLabel(antennaNumber,rx);
    xlabel(type,antennaNumber);
    while ((ptr = fgets(buf,sizeof(buf),fp)) != NULL) {
      narg = sscanf(buf,"%f %f %f %f %f %f",&vset,&vread,&iread,&mv,&ua,&power);
      if (narg == 6) {
	updatePlot(pointsRead,mv,ua,1);
	pointsRead++;
      }
      if (narg == 3) {
	updatePlot(pointsRead,vread,iread,1);
	pointsRead++;
      }
    }
    fclose(fp);
    linetype(1); /* 1=closely-spaced dashed; 2,3,4 are dashed */
    drawOperatingPoint(operatingPoint);
    if (type == IVSWEEP && sweeps == 2) {
      linetype(5);
      fp = fopen(newestFilename,"r");
      pointsRead = 0;
      if (fp == NULL) {
	move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+80);
	center("Could not");
	move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+60);
        center("open file");
	continue;
      }
      /* read the header line */
      ptr = fgets(buf,sizeof(buf),fp);
      while ((ptr = fgets(buf,sizeof(buf),fp)) != NULL) {
	narg = sscanf(buf,"%f %f %f %f %f %f",&vset,&vread,&iread,&mv,&ua,&power);
	if (narg == 6) {
	  updatePlot(pointsRead,mv,power,2);
	  pointsRead++;
	}
      }
      fclose(fp);
    }
  } /* end 'for' loop over antennas */
  if (vbias == 0) {
    rm_close();
  }
  cleanup();
  return(pointsRead);
}

int computeLimits(int type, char *afilename) {
  float vset,vread,iread,mv,ua,power;
  FILE *fp;
  int narg;
  char *ptr;
  int pointsRead = 0;

  fp = fopen(afilename,"r");
  if (fp == NULL) {
    move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+160);
    center("Could not");
    move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+140);
    center("open file.");
    move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+100);
    center("Check the");
    move(chartXoffset[antennaNumber]+CHART_XWIDTH/2,chartYoffset[antennaNumber]+80);
    center("permissions.");
    return(0);
  }
  if (type == IVSWEEP) {
    /* read the header line */
    ptr = fgets(buf,sizeof(buf),fp);
  }
  while ((ptr = fgets(buf,sizeof(buf),fp)) != NULL) {
    narg = sscanf(buf,"%f %f %f %f %f %f",&vset,&vread,&iread,&mv,&ua,&power);
    if (narg == 6) {
      /* then it is an IVSWEEP */
      if (pointsRead == 0) {
	xmax = xmin = mv;
	ymax = ymin = ua;
	zmax = zmin = power;
      } else {
	if (mv > xmax) {
	  xmax = mv;
	}
	if (mv < xmin) {
	  xmin = mv;
	}
	if (ua > ymax) {
	  ymax = ua;
	}
	if (ua < ymin) {
	  ymin = ua;
	}
	if (power > zmax) {
	  zmax = power;
	}
	if (power < zmin) {
	  zmin = power;
	}
      }
      pointsRead++;
    } else if (narg == 3) {
      /* then it is an PBSWEEP */
      if (pointsRead==0) {
	xmax = xmin = vread;
	ymax = ymin = iread;
      } else {
	if (vread > xmax) {
	  xmax = vread;
	}
	if (vread < xmin) {
	  xmin = vread;
	}
	if (iread > ymax) {
	  ymax = iread;
	}
	if (iread < ymin) {
	  ymin = iread;
	}
      }

      /* expand the plotting range */
      xmax = floor((xmax+90)/100)*100;
      xmin = floor(xmin/100)*100;
      pointsRead++;
    } else {
      if (DEBUG) {
	printf("saw %d arguments instead of 6 (I/V) or 3 (P/B)\n",narg);
      }
    }
  }
  if (DEBUG) {
    printf("ymax = %.3f, ymin = %.3f\n",ymax,ymin);
  }
  fclose(fp);
  return(pointsRead);
}

void drawOperatingPoint(float x) {
  int xp; /* in units of pixels */
  int yp;
  if (x >= xmin && x <= xmax) {
    xp = chartXoffset[antennaNumber] + CHART_XWIDTH*(x-xmin)/(xmax-xmin);
    yp = chartYoffset[antennaNumber];
    move(xp,yp);
    yp = CHART_YWIDTH + chartYoffset[antennaNumber];
    line(xp,yp);
  }
}

void updatePlot(int i, float x, float y, int plotnumber) {
  int xp; /* in units of pixels */
  int yp;

  xp = chartXoffset[antennaNumber] + CHART_XWIDTH*(x-xmin)/(xmax-xmin);
  if (plotnumber == 1) {
    yp = floor(chartYoffset[antennaNumber]+CHART_YWIDTH*(y-ymin)/(ymax-ymin));
  } else {
    yp = floor(chartYoffset[antennaNumber]+CHART_YWIDTH*(y-zmin)/(zmax-zmin));
  }
  if (i==0) {
    move(xp,yp);
  } else {
    line(xp,yp);
  }
}

void init8(void) {
  xterm();
  setup();
  refresh8();
}

void refresh8(void) {
  int type = IVSWEEP;

  clear();
  for (antennaNumber=START_ANTENNA; antennaNumber<=N_ANTENNAS; antennaNumber++) {
    drawBorder();
  }
  title8(type);
}

void drawDatestring(int antenna, char *string) {
  int upper = 695;
  int lower = 360;
  int offset = 20;
  switch (antenna) {
  case 1: move(220-offset,upper); break;
  case 2: move(450-offset,upper); break;
  case 3: move(680-offset,upper); break;
  case 4: move(910-offset,upper); break;
  case 5: move(220-offset,lower); break;
  case 6: move(450-offset,lower); break;
  case 7: move(680-offset,lower); break;
  case 8: move(910-offset,lower); break;
  }
  center(string);
}

void drawAntennaLabel(int antenna, int rx) {
  char string[25];
  int upper = 715;
  int lower = 380;
  int offset = -20;
  switch (antenna) {
  case 1: move(220+offset,upper); break;
  case 2: move(450+offset,upper); break;
  case 3: move(680+offset,upper); break;
  case 4: move(910+offset,upper); break;
  case 5: move(220+offset,lower); break;
  case 6: move(450+offset,lower); break;
  case 7: move(680+offset,lower); break;
  case 8: move(910+offset,lower); break;
  }
  switch (rx) {
  case 0:
    sprintf(string,"Antenna %d  230",antenna);
    break;
  case 2:
    sprintf(string,"Antenna %d  345",antenna);
    break;
  case 4:
    sprintf(string,"Antenna %d  400",antenna);
    break;
  case 6:
    sprintf(string,"Antenna %d  690",antenna);
    break;
  default:
    sprintf(string,"Antenna %d  rx%d",antenna,rx);
    break;
  }
  center(string);
}

void drawBorder(void) {
  move(chartXoffset[antennaNumber],chartYoffset[antennaNumber]);
  line(chartXoffset[antennaNumber]+CHART_XWIDTH, chartYoffset[antennaNumber]);
  line(chartXoffset[antennaNumber]+CHART_XWIDTH, chartYoffset[antennaNumber]+CHART_YWIDTH);
  line(chartXoffset[antennaNumber], chartYoffset[antennaNumber]+CHART_YWIDTH);
  line(chartXoffset[antennaNumber], chartYoffset[antennaNumber]);
}

int present(char *search,char *token) {
  if (strstr(search,token) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

void title8(int type) {
  char rmvarIndex[100];

  switch (type) {
  case IVSWEEP:
    if (vbias == 1) {
      sprintf(rmvarIndex,"Latest I/V (solid) & P/V sweep (dashed) with vbias at time of scan");
    } else {
      sprintf(rmvarIndex,"Latest I/V sweep (solid) and P/V (dashed) with present vbias marked");
    }
    break;
  case PBSWEEP:
    sprintf(rmvarIndex,"Latest P/B sweeps");
    break;
  }
  move(512,740);
  center(rmvarIndex);

  move(512,720);
  center(newestFilename);
}

void ylabel(int type, int sweeps) {
  char string[40];
  int j, ctr;
  float i, yinc, y,starti;
  int ytick, ztick;
  float zinc;
  float zrange,startz,z;

  move(chartXoffset[antennaNumber]-80,chartYoffset[antennaNumber]+CHART_YWIDTH*0.45);
  switch (type) {
  case IVSWEEP:
    center("Current(uA)");
    j = floor((ymax-ymin)*0.2);
    yinc = floor(2*(j/2));
    ctr = 0;
    for (i=floor(ymin+yinc); i<(ymax-0.5*yinc) && ++ctr<LIMIT; i+=yinc) {
      y = chartYoffset[antennaNumber]+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(chartXoffset[antennaNumber]-50, y);
      sprintf(string,"%.0f",i);
      center(string); 
    }
    move(chartXoffset[antennaNumber]+CHART_XWIDTH+65, chartYoffset[antennaNumber]+CHART_YWIDTH*0.45);
    center("Power(uW)");
    zrange = zmax-zmin;
    if (zrange > 5) {
      zinc = floor(zrange*0.2);
      startz = floor(zmin);
    } else if (zrange > 0.5) {
      zinc = floor(10*zrange*0.2)*0.1;
      startz = floor(zmin*10)*0.1;
    } else if (zrange > 0.05) {
      zinc = floor(100*zrange*0.2)*0.01;
      startz = floor(zmin*100)*0.01;
    } else {
      zinc = floor(1000*zrange*0.2)*0.001;
      startz = floor(zmin*1000)*0.001;
    }
    /*
    printf("\n\nzrange=%.3f, startz=%.3f, zinc=%.3f\n",zrange,startz,zinc);
    */
    ctr = 0;
    for (y=startz+zinc; y<zmax && ++ctr<LIMIT; y+=zinc) {
      z = chartYoffset[antennaNumber]+(CHART_YWIDTH*(y-zmin)/(zmax-zmin));
      /*      printf("y=%.2f, z=%.2f ",y,z); 
       */
      move(chartXoffset[antennaNumber]+CHART_XWIDTH+40, z);
      sprintf(string,"%.2f",y);
      center(string); 
    }
    break;
  case PBSWEEP:
    center("Power(V)");
#if 1
    if ((ymax-ymin) > 1) {
      yinc = floor(10*(ymax-ymin)*0.2)*0.1;
    } else if ((ymax-ymin) > 0.1) {
      yinc = floor(100*(ymax-ymin)*0.2)*0.01;
    } else {
      yinc = floor(1000*(ymax-ymin)*0.2)*0.001;
    }
#else
    yinc = 0.3;
    ymax = 3.0; ymin = 0.0;
#endif
    if (yinc <= 0) {
      printf("yinc = %f, ymin=%.2f, ymax=%.2f\n",yinc,ymin,ymax);
      return;
    }
    ctr = 0;
#if 1
    for (i=ymin; i<ymax && ++ctr<LIMIT; i+=yinc) {
#else
    for (i=0; i<3.0 && ++ctr<LIMIT; i+=yinc) {
#endif
      y = chartYoffset[antennaNumber]+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(chartXoffset[antennaNumber]-50, y);
      sprintf(string,"%.3f",i);
      center(string); 
    }
    break;
  }
  switch (type) {
  case IVSWEEP:
    ytick = 4;
    starti = floor(ymin+yinc);
    yinc = 2*(yinc/8);
    ctr = 0;
    for (i=floor(ymin+yinc); i<ymax && ++ctr<LIMIT; i+=yinc) {
      /* left side */
      y = chartYoffset[antennaNumber]+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(chartXoffset[antennaNumber],y);
      j = (int)floor((i-starti) / yinc+0.5);
      if ((j % ytick) == 0) {
	line(chartXoffset[antennaNumber]+12,y);
      } else {
	line(chartXoffset[antennaNumber]+6,y);
      }
      /* right side y-ticks */
      if (sweeps == 1) {
	move(chartXoffset[antennaNumber]+CHART_XWIDTH,y);
	if ((j % ytick) == 0) {
	  line(chartXoffset[antennaNumber]+CHART_XWIDTH-12,y);
	} else {
	  line(chartXoffset[antennaNumber]+CHART_XWIDTH-6,y);
	}
      }
    }
    if (sweeps == 2) {
      ctr = 0;
      zrange = zmax-zmin;
      if (zrange > 5) {
	zinc = floor(zrange*0.2);
	startz = floor(zmin);
      } else if (zrange > 0.5) {
	zinc = floor(10*zrange*0.2)*0.1;
	startz = floor(zmin*10)*0.1;
      } else if (zrange > 0.05) {
	zinc = floor(100*zrange*0.2)*0.01;
	startz  = floor(zmin*100)*0.01;
      } else { 
	zinc = floor(1000*zrange*0.2)*0.001;
	startz = floor(zmin*1000)*0.001;
      }
      ztick = 5;
      zinc /= ztick;
      for (y=startz; y<zmax && ++ctr<LIMIT; y+=zinc) {
	if (y > zmin) {
	  z = chartYoffset[antennaNumber]+(CHART_YWIDTH*(y-zmin)/(zmax-zmin));
	  move(chartXoffset[antennaNumber]+CHART_XWIDTH,z);
	  j = (int)floor(0.5+(y-startz)/zinc);
	  if ((j % ztick) == 0) {
	    line(chartXoffset[antennaNumber]+CHART_XWIDTH-12,z);
	  } else {
	    line(chartXoffset[antennaNumber]+CHART_XWIDTH-6,z);
	  }
	}
      }
    }
    break;
  case PBSWEEP:
    starti = ymin;
    yinc = yinc*0.16666666;
    if (yinc <= 0) {
      printf("yinc = %d\n",yinc);
      return;
    }
    j = 0;
    ctr= 0;
    ytick = 6;
    for (i=ymin; i<ymax && ++ctr<LIMIT; i+=yinc) {
      /* left side */
      y = chartYoffset[antennaNumber]+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(chartXoffset[antennaNumber],y);
      if ((j % ytick) == 0) {
	line(chartXoffset[antennaNumber]+12,y);
      } else {
	line(chartXoffset[antennaNumber]+6,y);
      }
      /* right side */
      move(chartXoffset[antennaNumber]+CHART_XWIDTH,y);
      if ((j % ytick) == 0) {
	line(chartXoffset[antennaNumber]+CHART_XWIDTH-12,y);
      } else {
	line(chartXoffset[antennaNumber]+CHART_XWIDTH-6,y);
      }
      j++;
    }
    break;
  }
}

void xlabel(int type, int antenna) {
  char string[40];
  float xinc,i,x;
  int ctr;
  int intxinc;
  int tick;

  ctr = 0;
  switch (type) {
  case IVSWEEP:
    xinc = 1.0; /* draw a label every this many mV */
    for (i=floor(xmin+xinc); i<xmax && ++ctr<LIMIT; i+=xinc) {
      x = chartXoffset[antenna]+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
      sprintf(string,"%.0f",i);
      move(x, chartYoffset[antenna]-16);
      center(string); 
    }
    break;
  case PBSWEEP:
    xinc = 50; /* label every 50uA */
    for (i=floor(xmin/100)*100; i<xmax && ++ctr<LIMIT; i+=xinc) {
      x = chartXoffset[antenna]+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
      move(x, chartYoffset[antenna]-16);
      sprintf(string,"%.0f",i);
      center(string); 
    }
    break;
  }
  ctr = 0;
  switch (type) {
  case IVSWEEP:
    xinc = 0.2; /* put a minor tick mark every this many mV */
    break;
  case PBSWEEP:
    xinc = 10; /* put a minor tick mark every 10uA */
    break;
  }
  intxinc = xinc;
  ctr = 0;
  /* tick marks */
  for (i=0.1*floor((xmin+xinc)*10); i<xmax && ++ctr<LIMIT; i+=xinc*0.5) {
    /* lower axis */
    x = chartXoffset[antenna]+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
    move(x,chartYoffset[antenna]);
    switch (type) {
    case IVSWEEP:
      tick = (int)floor(i*10+0.5);
      if (tick % 2 == 0) {
	if (tick % 10 == 0) {
	  line(x,chartYoffset[antenna]+12);
	} else {
	  line(x,chartYoffset[antenna]+6);
	}
      }
      /*
      if (antenna == 6 || antenna == 8) {
	sprintf(string,"tick=%d\n",tick);
	move(x,chartYoffset[antenna-4]+7.5*tick);
	center(string);
      }
      */
      break;
    case PBSWEEP:
      if ((((int)floor(i/10)*10) % intxinc) == 0) {
	line(x,chartYoffset[antenna]+12);
      } else {
	line(x,chartYoffset[antenna]+6);
      }
      break;
    }
    /* upper axis */
    move(x,chartYoffset[antenna]+CHART_YWIDTH);
    switch (type) {
    case IVSWEEP:
      if (tick % 2 == 0) {
	if (tick % 10 == 0) {
	  line(x,chartYoffset[antenna]+CHART_YWIDTH-12);
	} else {
	  line(x,chartYoffset[antenna]+CHART_YWIDTH-6);
	}
      }
      break;
    case PBSWEEP:
      if ((((int)floor(i/10)*10) % intxinc) == 0) {
	line(x,chartYoffset[antenna]+CHART_YWIDTH-12);
      } else {
	line(x,chartYoffset[antenna]+CHART_YWIDTH-6);
      }
      break;
    }
  }
  if (antenna >= 5) {
    move(chartXoffset[antenna]+CHART_XWIDTH/2, chartYoffset[antenna]-40);
    switch (type) {
    case IVSWEEP:
      center("SIS Bias (mV)");
      break;
    case PBSWEEP:
      center("B-Field (mA)");
      break;
    }
  }
}

char *monthToString(int month) {
  switch(month) {
  case 1: return("Jan");
  case 2: return("Feb");
  case 3: return("Mar");
  case 4: return("Apr");
  case 5: return("May");
  case 6: return("Jun");
  case 7: return("Jul");
  case 8: return("Aug");
  case 9: return("Sep");
  case 10: return("Oct");
  case 11: return("Nov");
  case 12: return("Dec");
    default: return(" ? ");
  }
}

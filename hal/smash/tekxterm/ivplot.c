#define INITIAL_SMAPOPT_STRING_ARGUMENT "0000000000"
#define LIMIT 100
#define CHART_XOFFSET 200
#define CHART_YOFFSET 100
#define CHART_XWIDTH 700 /* was 800 before adding second y label */
#define CHART_YWIDTH 600

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#if __linux__
#include <sys/utsname.h>
#include <sys/stat.h>
#else
#include <utsname.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include "smapopt.h"
#include "rm.h"
#include "ivplot.h"
#include "tek_driv.h"

int pointsRead = 0;

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

#if USE_AS_SUBROUTINE
int ivplot(int antennaNumber, int type, int sweeps) {
#else
int sweeps = 2;
int antennaNumber = 0; /* 1..8 */
int type = IVSWEEP;
int main(int argc, char *argv[]) {
#endif
  int newestTime = 0;
  DIR *dirPtr;
  char searchDir[100];
  int help = 0;
  int i;
  float vset,vread,iread,mv,ua,power;
  char *ptr;
  smapoptContext optCon; 
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
  int useAllAntennasInProject;
  int antennaArgument[ANT_LIST_SZ];
  int antennasGiven = 0;
  int statStatus;
  int DEBUG = 0;
  int vbias = 0;
  int rc, narg;
  FILE *fp;
  char *filenameArgument = INITIAL_SMAPOPT_STRING_ARGUMENT;
  double yminimumGiven, ymaxGiven;
  int yminimumCmdline = 0;
  struct utsname unamebuf;
  struct dirent *nextEnt;
  struct stat messageStat;
  int ymaxCmdline = 0;
  int sweepsFound = 0;
  float operatingPoint;

#if USE_AS_SUBROUTINE
  uname(&unamebuf);
  if (strcmp(unamebuf.nodename,"hal9000") != 0) {
    runningOnHal9000 = 0;
    sscanf(unamebuf.nodename,"acc%d",&antennaNumber);
  } else {
    runningOnHal9000 = 1;
  }
#else
  struct smapoptOption options[] = {
  {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, antennaArgument, 'a',"antenna argument (this supercedes -s)"},
  {"file",'f', SMAPOPT_ARG_STRING, &filenameArgument, 0, "Filename to plot (Default = most recent in /instance/ivcurves"},
  {"lower",'l', SMAPOPT_ARG_DOUBLE, &yminimumGiven, 'l', "Min value for y scale"},
  {"upper",'u', SMAPOPT_ARG_DOUBLE, &ymaxGiven, 'u', "Max value for y scale"},
  /*
  {"vbias",'v', SMAPOPT_ARG_NONE, &vbias, 'v', "Plot present vbias instead of the value when the sweep was taken"},
  */
  {"bfield",'b', SMAPOPT_ARG_NONE, &type, 0, "plot most recent Bfield sweep instead of I/V sweep"},
  {"debug",'D', SMAPOPT_ARG_NONE, &DEBUG, 0, "print debugging messages"},
  SMAPOPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
  };
  bzero(filename,sizeof(filename));
  optCon = smapoptGetContext(argv[0], argc, argv, options, 
			     SMAPOPT_CONTEXT_NOLOG);
  if (DEBUG) {
    fprintf(stdout,"Finished smapoptGetContext()\n");
    fflush(stdout);
  }
  while ((rc = smapoptGetNextOpt(optCon)) >= 0) {
    switch(rc) {
    case 'a':
      antennasGiven = 1;
      break;
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
  if (DEBUG) {
    for (i=0; i<=8; i++) {
      fprintf(stderr,"antennaArgument[i] = %d\n",antennaArgument[i]);
    }
  }
  if (yminimumCmdline==1) {
    ymin = yminimumGiven;
    printf("Using yminimum = %f\n",ymin);
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
  if (strcmp(filenameArgument,INITIAL_SMAPOPT_STRING_ARGUMENT) != 0) {
    if (present(filenameArgument,"/otherPower")) {
      strcpy(filename,filenameArgument);
      printf("read filename = %s.\n",filename);
    } else {
      sprintf(filename,"/otherPowerPCs/acc/%d/ivcurves/%s",antennaNumber,filenameArgument);
      printf("will use filename = %s\n",filename);
    }
  }


#endif
  if (runningOnHal9000 && (antennaNumber<1 || antennaNumber >8)) {
    printf("Invalid antenna number = %d\n",antennaNumber);
    return(0);
  }
  if (strlen(filename) < 1) {
    /* figure out the latest filename */
    if (runningOnHal9000==1) {
      sprintf(searchDir,"/otherPowerPCs/acc/%d/ivcurves",antennaNumber);
      printf("1) Searching %s\n",searchDir);
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
	    sprintf(newestFilename,"%s/%s",searchDir,nextEnt->d_name);
	    /*
	    printf("%d = %s\n",newestTime,nextEnt->d_name);
	    */
	  }
	  sweepsFound++;
	}
      } /* end 'while' */
    } else {  /* running on antenna computer */
      sprintf(searchDir,"/instance/ivcurves");
      dirPtr = opendir(searchDir);
      printf("2) Searching %s\n",searchDir);
      while ((nextEnt = readdir(dirPtr)) != NULL) {
        if (((present(nextEnt->d_name, "ivcurve.") && type == IVSWEEP) ||
	    (present(nextEnt->d_name, "bfieldsweep.") && type==PBSWEEP)) &&
	     !present(nextEnt->d_name, ".ps")) {
	  sprintf(filename,"%s/%s",searchDir,nextEnt->d_name);
	  statStatus = stat(filename, &messageStat);
	  if (statStatus != 0) {
	    perror("stat");
	    exit(0);
	  }
#if 0
	  printf("%s %ld\n",nextEnt->d_name,messageStat.st_mtime);
#endif
	  if (messageStat.st_mtime > newestTime) {
	    newestTime = messageStat.st_mtime;
	    sprintf(newestFilename,"%s/%s",searchDir,nextEnt->d_name);
	  }
	  sweepsFound++;
	}
      }
    }
    closedir(dirPtr);
    printf("Found %d sweeps. Plotting file = %s\n",sweepsFound,newestFilename);
  } else {
    printf("strlen(filename) = %d\n",strlen(filename));
    strcpy(newestFilename,filename);
    printf("Plotting file = %s\n",newestFilename);
  }
  if ((totalPoints = computeLimits(type,newestFilename)) == 0) {
    printf("Found 0 total points. Aborting.\n");
    return(0);
  }
  if (type == IVSWEEP) {
    printf("Found an IV sweep with %d data points: xrange=[%.3f,%.3f] yrange=[%.3f,%.3f] zrange=[%.3f,%.3f].\n",totalPoints,
	 xmin,xmax,ymin,ymax,zmin,zmax);
  } 
  if (type == PBSWEEP) {
    printf("Found a PB sweep with %d data points: xrange=[%.3f,%.3f] yrange=[%.3f,%.3f]\n",
	   totalPoints, xmin,xmax,ymin,ymax);
  } 
  init(antennaNumber,type,sweeps);
  /*
  usleep(100000);
  */
  i = 0;
  fp = fopen(newestFilename,"r");
  pointsRead = 0;
  if (fp == NULL) {
    printf("Could not open file = %s\n",newestFilename);
    return(pointsRead);
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
    sscanf(buf,"%*s %*s %*s %*s %*s %*s %*d %*f %*f %*f %*s %f",&operatingPoint);
    printf("\n\n                Operating point = %.3f mV\n",operatingPoint);
  }
  linetype(200); /* solid */
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
  linetype(2); /* 2 and 3 are dashed */
  drawOperatingPoint(operatingPoint);
  if (type == IVSWEEP && sweeps == 2) {
    linetype(5);
    fp = fopen(newestFilename,"r");
    pointsRead = 0;
    if (fp == NULL) {
      printf("Could not open file = %s\n",newestFilename);
      return(pointsRead);
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
  cleanup();
  return(pointsRead);
}

int computeLimits(int type, char *afilename) {
  float vset,vread,iread,mv,ua,power;
  FILE *fp;
  int narg;
  char *ptr;

  fp = fopen(afilename,"r");
  if (fp == NULL) {
    printf("Could not open file = %s\n",afilename);
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
      if (pointsRead==0) {
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
      xmax = floor((xmax+90)/100)*100;
      xmin = floor(xmin/100)*100;
      pointsRead++;
    } else {
      printf("saw %d arguments instead of 6 (I/V) or 3 (P/B)\n",narg);
    }
  }
  printf("ymax = %.3f, ymin = %.3f\n",ymax,ymin);
  fclose(fp);
  return(pointsRead);
}

void drawOperatingPoint(float x) {
  int xp; /* in units of pixels */
  int yp;
  /*  printf("About to draw operating point = %f mV\n",x);*/
  xp = CHART_XOFFSET + (float)CHART_XWIDTH*(x-xmin)/(xmax-xmin);
  yp = CHART_YOFFSET;
  move(xp,yp);
  yp = CHART_YWIDTH + CHART_YOFFSET;
  line(xp,yp);
}

void updatePlot(int i, float x, float y, int plotnumber) {
  int xp; /* in units of pixels */
  int yp;

  xp = CHART_XOFFSET + CHART_XWIDTH*(x-xmin)/(xmax-xmin);
  if (plotnumber == 1) {
    yp = floor(CHART_YOFFSET+CHART_YWIDTH*(y-ymin)/(ymax-ymin));
  } else {
    yp = floor(CHART_YOFFSET+CHART_YWIDTH*(y-zmin)/(zmax-zmin));
  }
  if (i==0) {
    move(xp,yp);
  } else {
    line(xp,yp);
  }
}

void init(int antennaNumber, int type, int sweeps) {
  xterm();
  refresh(antennaNumber,type,sweeps);
}

void refresh(int antennaNumber, int type, int sweeps) {
  setup();
  clear();
  drawBorder();
  xlabel(type,antennaNumber);
  ylabel(type,sweeps);
  title(antennaNumber,type);
}

void drawBorder(void) {
  move(CHART_XOFFSET,CHART_YOFFSET);
  line(CHART_XOFFSET+CHART_XWIDTH, CHART_YOFFSET);
  line(CHART_XOFFSET+CHART_XWIDTH, CHART_YOFFSET+CHART_YWIDTH);
  line(CHART_XOFFSET, CHART_YOFFSET+CHART_YWIDTH);
  line(CHART_XOFFSET, CHART_YOFFSET);
}

int present(char *search,char *token) {
  if (strstr(search,token) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

void title(int antennaNumber, int type) {
  char rmvarIndex[RM_NAME_LENGTH+5];

  switch (type) {
  case IVSWEEP:
    sprintf(rmvarIndex,"Antenna %d I/V sweep (solid) and P/V sweep (dashed)",antennaNumber);
    break;
  case PBSWEEP:
    sprintf(rmvarIndex,"Antenna %d P/B sweep",antennaNumber);
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

  move(CHART_XOFFSET-80,CHART_YOFFSET+CHART_YWIDTH*0.45);
  switch (type) {
  case IVSWEEP:
    center("Current(uA)");
    j = floor((ymax-ymin)*0.2);
    yinc = floor(2*(j/2));
    ctr = 0;
    for (i=floor(ymin+yinc); i<(ymax-0.5*yinc) && ++ctr<LIMIT; i+=yinc) {
      y = CHART_YOFFSET+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(CHART_XOFFSET-50, y);
      sprintf(string,"%.1f",i);
      center(string); 
    }
    move(CHART_XOFFSET+CHART_XWIDTH+65,CHART_YOFFSET+CHART_YWIDTH*0.45);
    center("Power(uW)");
    zmax = 1.03*zmax;
    if (zmin > 0) {
      zmin = 0;
    }
    zrange = (1.0*zmax)-zmin;
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
    printf("\n\nzrange=%.3f(%.3f to %.3f), startz=%.3f, zinc=%.3f\n",zrange,zmin,zmax,startz,zinc);
    */
    ctr = 0;
    for (y=startz+zinc; y<zmax && ++ctr<LIMIT; y+=zinc) {
      z = CHART_YOFFSET+(CHART_YWIDTH*(y-zmin)/(zmax-zmin));
      /*      printf("y=%.2f, z=%.2f ",y,z); 
       */
      move(CHART_XOFFSET+CHART_XWIDTH+40, z);
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
      printf("yinc = %d, ymin=%.2f, ymax=%.2f\n",yinc,ymin,ymax);
      return;
    }
    ctr = 0;
#if 1
    for (i=ymin; i<ymax && ++ctr<LIMIT; i+=yinc) {
#else
    for (i=0; i<3.0 && ++ctr<LIMIT; i+=yinc) {
#endif
      y = CHART_YOFFSET+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(CHART_XOFFSET-50, y);
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
      y = CHART_YOFFSET+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(CHART_XOFFSET,y);
      j = (int)floor((i-starti) / yinc+0.5);
      if ((j % ytick) == 0) {
	line(CHART_XOFFSET+12,y);
      } else {
	line(CHART_XOFFSET+6,y);
      }
      /* right side y-ticks */
      if (sweeps == 1) {
	move(CHART_XOFFSET+CHART_XWIDTH,y);
	if ((j % ytick) == 0) {
	  line(CHART_XOFFSET+CHART_XWIDTH-12,y);
	} else {
	  line(CHART_XOFFSET+CHART_XWIDTH-6,y);
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
	  z = CHART_YOFFSET+(CHART_YWIDTH*(y-zmin)/(zmax-zmin));
	  move(CHART_XOFFSET+CHART_XWIDTH,z);
	  j = (int)floor(0.5+(y-startz)/zinc);
	  if ((j % ztick) == 0) {
	    line(CHART_XOFFSET+CHART_XWIDTH-12,z);
	  } else {
	    line(CHART_XOFFSET+CHART_XWIDTH-6,z);
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
      y = CHART_YOFFSET+(CHART_YWIDTH*(i-ymin)/(ymax-ymin));
      move(CHART_XOFFSET,y);
      if ((j % ytick) == 0) {
	line(CHART_XOFFSET+12,y);
      } else {
	line(CHART_XOFFSET+6,y);
      }
      /* right side */
      move(CHART_XOFFSET+CHART_XWIDTH,y);
      if ((j % ytick) == 0) {
	line(CHART_XOFFSET+CHART_XWIDTH-12,y);
      } else {
	line(CHART_XOFFSET+CHART_XWIDTH-6,y);
      }
      j++;
    }
    break;
  }
}

void xlabel(int type, int antenna) {
  char string[40];
  float xinc,i,x;
  int divisor, ctr;
  int intxinc;

  switch (type) {
  case IVSWEEP:
    if (antenna == 7) {
      xinc = 0.8;
    } else {
      intxinc = floor(xmax-xmin+0.5);
      xinc = intxinc*0.1; /* label every 0.4mV for a typical 0..4mV sweep*/
      /* xinc = 0.4 */
      /*
      printf("Using xinc = %fmV in the I/V plot\n",xinc);
      */
    }
    for (i=floor(xmin+xinc); i<xmax && ++ctr<LIMIT; i+=xinc) {
      x = CHART_XOFFSET+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
      move(x, CHART_YOFFSET-16);
      sprintf(string,"%.1f",i);
      center(string); 
    }
    break;
  case PBSWEEP:
    xinc = 50; /* label every 50uA */
    for (i=floor(xmin/100)*100; i<xmax && ++ctr<LIMIT; i+=xinc) {
      x = CHART_XOFFSET+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
      move(x, CHART_YOFFSET-16);
      sprintf(string,"%.0f",i);
      center(string); 
    }
    break;
  }
  intxinc = xinc;
  ctr = 0;
  switch (type) {
  case IVSWEEP:
    xinc = 0.1; /* put a minor tick mark every 0.1mV */
    break;
  case PBSWEEP:
    xinc = 10; /* put a minor tick mark every 10uA */
    break;
  }
  ctr = 0;
  /* tick marks */
  for (i=0.1*floor((xmin+xinc)*10); i<xmax && ctr<LIMIT; i+=xinc) {
    /* lower axis */
    x = CHART_XOFFSET+CHART_XWIDTH*((i-xmin)/(xmax-xmin));
    move(x,CHART_YOFFSET);
    switch (type) {
    case IVSWEEP:
      if ((int)floor(i*10+0.5) % 4 == 0) {
	line(x,CHART_YOFFSET+12);
      } else {
	line(x,CHART_YOFFSET+6);
      }
      break;
    case PBSWEEP:
      if ((((int)floor(i/10)*10) % intxinc) == 0) {
	line(x,CHART_YOFFSET+12);
      } else {
	line(x,CHART_YOFFSET+6);
      }
      break;
    }
    /* upper axis */
    move(x,CHART_YOFFSET+CHART_YWIDTH);
    switch (type) {
    case IVSWEEP:
      if ((int)floor(i*10+0.5) % 4 == 0) {
	line(x,CHART_YOFFSET+CHART_YWIDTH-12);
      } else {
	line(x,CHART_YOFFSET+CHART_YWIDTH-6);
      }
      break;
    case PBSWEEP:
      if ((((int)floor(i/10)*10) % intxinc) == 0) {
	line(x,CHART_YOFFSET+CHART_YWIDTH-12);
      } else {
	line(x,CHART_YOFFSET+CHART_YWIDTH-6);
      }
      break;
    }
  }
  move(CHART_XOFFSET+CHART_XWIDTH/2,CHART_YOFFSET-40);
  switch (type) {
  case IVSWEEP:
    center("SIS Bias (mV)");
    break;
  case PBSWEEP:
    center("B-Field (mA)");
    break;
  }
}


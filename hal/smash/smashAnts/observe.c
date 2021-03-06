/***************************************************************
*
* observe.c
* 
* SMAsh command for tracking an astronomical source
* NAP 
* 1 March 2000
*
* 5 June 2001: added inputs for source specs and offsets
* This version is for the PowerPC
* Command line arguments are now parsed using the smapopt library.
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
* 22 oct 2003: added additional command line arguments for input
* of source name and coordinates- allowing non-catalog sources.
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <smapopt.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "rm.h"
#include "dsm.h"
#include "commonLib.h"
#include "scanFlags.h"
#include "statusServer.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: observe [options] --source <sourcename>\n"
                "[options] include:\n"
		"[--ra or -r <hh:mm:ss.sss> --dec or -d <+/-dd:mm:dd.ddd>\n"
		"--name_it or -n Name to use in the data file\n"
		"--type -t <main,flux,bandpass,gain,ipoint,planet,holography>\n"
		"--epoch -e <1950./2000.>\n"
		"--velocity or -v <velocity km/s (only vlsr for now)>]\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "                  (default: all online antennas)\n");
        exit(exitcode);
}

void ignoreResults(sSStatus *status) {
  if (status == NULL) {
    fprintf(stderr, "Null pointer returned by statusServer\n");
    exit(-1);
  }
}

int main(int argc, char *argv[])  
{
  char c, *source, Source[34], command_n[30], *nameIt, fileSource[34], oldSource[34], command_one[30];
  char *type;
  int includesAProjectAntenna = 0;
  int lowestAntennaInProject;
  short i, pmac_command_flag=0; 
  short  sourcelength;
  int gotsource=0,gottype=0,rm_status,antlist[RM_ARRAY_SIZE];
  int gotpmra=0,gotpmdec=0, gotnameit = 0;
  int gotra=0,gotdec=0,gotepoch=0,gotvel=0;
  int newSourceFlag=0;
  int dsm_status;
  int sourceType=0;
  int gotantenna=0;
  smapoptContext optCon;	
  int antenna;
  int antennaArray[SMAPOPT_MAX_ANTENNAS+1],
    antennas[SMAPOPT_MAX_ANTENNAS+1];
  int numberOfAnts=0;
  int iant;
  int tracktimestamp,timestamp;
  int trackStatus=0;
  int flagBad = 0;
  double ra=0.0,dec=0.0,velocity=0.0,epoch=2000.;
  double rAOff = _NAN;
  double decOff = _NAN;
  double pmra=0.0,pmdec=0.0;
  time_t timeStamp;
  flagChange list;
  CLIENT *cl = NULL;
  
  struct  smapoptOption optionsTable[] = {
    {"antenna",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a',
     "a comma-separated list of antennas"},
    {"source",'s',SMAPOPT_ARG_STRING,&source,'s',
     "source name."},
    {"name_it",'n',SMAPOPT_ARG_STRING,&nameIt,'n',
     "use this name in data file."},
    {"ra",'r',SMAPOPT_ARG_TIME,&ra,'r',
     "RA in hours  (decimal or HH:MM:SS.SSS format"},
    {"RAOff",'R',SMAPOPT_ARG_DOUBLE,&rAOff,'R',
     "RA offset (arcsec)"},
    {"dec",'d',SMAPOPT_ARG_DEC,&dec,'d',
     "DEC in degrees (decimal or +-DD:MM:SS.SSS"},
    {"DecOff",'D',SMAPOPT_ARG_DOUBLE,&decOff,'D',
     "Dec offset (arcsec)"},
    {"velocity",'v',SMAPOPT_ARG_DOUBLE,&velocity,'v',
     "source velocity in km/s, VLSR"},
    {"epoch",'e',SMAPOPT_ARG_DOUBLE,&epoch,'e',
     "Epoch of input coordinates 1950 or 2000."},
    {"pmra",'p',SMAPOPT_ARG_DOUBLE,&pmra,'p',
     "Proper motion along RA in mas/yr."},
    {"pmdec",'q',SMAPOPT_ARG_DOUBLE,&pmdec,'q',
     "Proper motion along DEC in mas/yr."},
    {"type",'t',SMAPOPT_ARG_STRING,&type,'t',
     "Source type- optional, default: main,
		main, flux, bandpass, gain, ipoint, planet, holography" },
    SMAPOPT_AUTOHELP	
    {NULL,0,0,NULL,0}
  };
  
  if (argc < 2)
    usage(-1,"Insufficient number of arguments","At least source- name required.");
 
  optCon = smapoptGetContext("observe", argc, argv, optionsTable,0);
  
  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    switch(c) {
    case 'h':  /* Help was requested */
      usage(0,NULL,NULL);
      break;
      
    case 'a':  /* At least one antenna was explicitly specified */
      gotantenna = 1;
      break;
      
    case 's':  /* The source was specified */
      gotsource = 1;
      break;
      
    case 'n':  /* The source's slisd was specified - use this for storage */
      gotnameit = 1;
      break;
      
    case 't':  /* The source type was specified */
      gottype = 1;
      break;
      
    case 'r':  /* The Right Ascension was specified */
      gotra = 1;
      break;
      
    case 'd':  /* The declination was specified */
      gotdec = 1;
      break;
      
    case 'p':  /* The proper motion in RA was specified */
      gotpmra = 1;
      break;
      
    case 'q':  /* The proper motion in Dec. was specified */
      gotpmdec = 1;
      break;
      
    case 'e':  /* The coordinate epoch was specified */
      gotepoch = 1;
      break;
      
    case 'v':  /* The LSR velocity was specified */
      gotvel = 1;
      break;
      
    }
  }
  
  if (gotsource != 1)
    usage(-2,"No source specified","Source name is required .\n");
  
  /* check if source type is valid */
  if (gottype == 0)
    sourceType = 0;
  else {
    gottype = -1;
    if      (!strcmp(type, "main"))       {sourceType = 0x0;  gottype = 1;}
    else if (!strcmp(type, "flux"))       {sourceType = 0x1;  gottype = 1;}
    else if (!strcmp(type, "bandpass"))   {sourceType = 0x2;  gottype = 1;}
    else if (!strcmp(type, "gain"))       {sourceType = 0x4;  gottype = 1;}
    else if (!strcmp(type, "ipoint"))     {sourceType = 0x8;  gottype = 1;}
    else if (!strcmp(type, "planet"))     {sourceType = 0x10; gottype = 1;}
    else if (!strcmp(type, "holography")) {sourceType = 0x20; gottype = 1;}
  }
  
  if (gottype == -1)
    usage(-3, "Invalid source type.",
	  " Source type should be either of main,flux,bandpass,gain,ipoint,holography\n");
  
  if ((gotpmra == 1) || (gotpmdec == 1)) {
    if (!((gotpmra == 1) && (gotpmdec == 1))) {
      usage(-5,"Insufficient arguments."," when giving proper
	motion values, both ra and dec pm are required.\n");
    }
  }
  
  if ((gotra == 1) || (gotdec == 1)) {
    if (!((gotra == 1) && (gotdec == 1) && (gotsource == 1))) {
      usage(-4,"Insufficient arguments.",
	    "if coordinates are specified, source-name, ra, dec, epoch and propermotions  are all required; 
	velocity is optional.\n");
    }
    else {
      newSourceFlag = 1;
      if (gotepoch == 0)
	epoch=2000.;
    }
  }
 
  if (c < -1) {
    fprintf(stderr, "%s: %s\n",
	    smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
	    smapoptStrerror(c));
  }
  smapoptFreeContext(optCon);
  lowestAntennaInProject = getAntennaList(NULL);
  /* initialize dsm */
  dsm_status = dsm_open();
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_open");
    exit(-1);
  }
  if (gotnameit)
    strcpy(fileSource, nameIt);
  else
    strcpy(fileSource, source);
  dsm_status= dsm_read("m5", "DSM_AS_SOURCE_C34", oldSource, &timeStamp);
  if (dsm_status != RM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(DSM_AS_SOURCE_C34)");
    exit(1);
  }
  
  if (gotantenna == 1) {
    numberOfAnts = 0;
    for (iant = 1; iant < ANT_ARRAY_SZ; iant++) {
      if (antennaArray[iant] == 1) {
	antennas[numberOfAnts] = iant;
	numberOfAnts++;
      }
    }
    if (numberOfAnts < 1) {
      printf("No antennas specified.  Nothing done.\n");
      exit(1);
    }
    if (antennaArray[lowestAntennaInProject])
      includesAProjectAntenna = 1;
    else
      includesAProjectAntenna = 0;
  } else {
    getAntennaList(antennaArray);
    includesAProjectAntenna = 1;
  }

  if (strcmp(oldSource, fileSource) && includesAProjectAntenna) {
    /* The source has changed - make statusServer flag the scan bad */
    flagBad = 1;
    list.action = SS_SPOIL_SCAN;
    list.condition = SFLAG_SOURCE_MISMATCH;
    if (!(cl = clnt_create("hal9000", STATUSSERVERPROG, STATUSSERVERVERS, "tcp"))) {
      clnt_pcreateerror("client create to statusServer");
    } else {
      ignoreResults(changeflagging_1(&list, cl));
    }
  }

  /* initializing ref. mem. */
  rm_status = rm_open(antlist);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    exit(1);
  }

  sourcelength=strlen(source);
  strcpy(Source, source);
  for (i = 0; i < 30; i++)
    command_n[i] = command_one[i] = 0x0;
  command_n[0] = 'n';/*0x6e;*/ /* send 'n' command */
  command_n[1] = 0x0;
  command_one[0] = '1';
  pmac_command_flag = 0;

  dsm_status = dsm_write_notify("M5ANDMONITOR", "DSM_AS_SOURCE_TYPE_L", &sourceType);
  if (dsm_status != RM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_write()");
    exit(1);
  }

  for (iant = 1; iant <= ANT_ARRAY_SZ; iant++) {
    if (antennaArray[iant] == 1) {
      antenna = iant;
      
      /* check if track is running on this antenna */
      
      rm_status = rm_read(antenna, "RM_UNIX_TIME_L", &timestamp);
      rm_status = rm_read(antenna, "RM_TRACK_TIMESTAMP_L", &tracktimestamp);
      if ((abs(tracktimestamp-timestamp) > 3L) && (antenna < 9)) {
	trackStatus = 0;
	printf("Track is not running on antenna %d.\n",antenna);
      }
      if ((abs(tracktimestamp-timestamp) <= 3L) || (antenna > 8))
	trackStatus = 1;

      if (trackStatus == 1) {
	rm_status = rm_write(antenna, "RM_CMD_SOURCE_FLAG_L",
			     &newSourceFlag);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write()");
	  exit(1);
        }
	if (newSourceFlag == 1) {
	  rm_status=rm_write(antenna, "RM_CMD_SOURCE_C34",
			     &Source);
	  if (rm_status != RM_SUCCESS) {
	    rm_error_message(rm_status, "rm_write()");
	    exit(1);
	  }
	  rm_status = rm_write(antenna, "RM_CMD_RA_HOURS_D", &ra);
	  rm_status = rm_write(antenna, "RM_CMD_DEC_DEG_D", &dec);
	  rm_status = rm_write(antenna, "RM_CMD_EPOCH_YEAR_D", &epoch);
	  rm_status = rm_write(antenna, "RM_CMD_PMRA_MASPYEAR_D", &pmra);
	  rm_status = rm_write(antenna, "RM_CMD_PMDEC_MASPYEAR_D", &pmdec);
	  if (gotvel == 0)
	    velocity = 0.0;
	  rm_status = rm_write(antenna, "RM_CMD_SVEL_KMPS_D", &velocity);
	} /* if new source flag =1 */

	if ((!isnan(rAOff)) && (!isnan(decOff))) {
	  rm_status = rm_write(antenna, "RM_RAOFF_ARCSEC_D",&rAOff);
	  if (rm_status != RM_SUCCESS) {
	    rm_error_message(rm_status, "rm_write()");
	    exit(1);
	  }
	  rm_status = rm_write(antenna, "RM_DECOFF_ARCSEC_D",&decOff);
	  if (rm_status != RM_SUCCESS) {
	    rm_error_message(rm_status, "rm_write()");
	    exit(1);
	  }
	}
	rm_status = rm_write(antenna, "RM_SMARTS_SOURCE_LENGTH_S", &sourcelength);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status,"rm_write()");
	  exit(1);
        }

	rm_status = rm_write(antenna, "RM_SMARTS_SOURCE_C34", Source);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write()");
	  exit(1);
        }

	rm_status = rm_write(antenna, "RM_SMASH_TRACK_COMMAND_C30", command_n);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write()");
	  exit(1);
        }

	rm_status = rm_write_notify(antenna, "RM_SMARTS_PMAC_COMMAND_FLAG_S",
				    &pmac_command_flag);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write_notify()");
	  exit(1);
        }

	rm_status = rm_write(antenna,"RM_OBSTYPE_L",&sourceType);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write()");
	  exit(1);
	}
      } /* if track is running */
    } /* if antenna is online */
  } /* for all antennas */
	
  if (flagBad) {
    /* The source has changed - make statusServer flag the scan bad */
    for (iant = 1; iant <= 8; iant++) {
      int j, count;
      char testName[34];
	    
      if (antennaArray[iant]) {
	rm_status = rm_read(iant, "RM_SOURCE_C34", testName);
	for (j = strlen(testName)-1; j > 0; j--)
	  if (isspace(testName[j]))
	    testName[j] = (char)0;
	  else
	    break;
	/*    	      printf("%d \"%s\" \"%s\"\n", iant, testName, fileSource); */
	count = 0;
	while ((strcmp(testName, fileSource)) && (count > 10)) {
	  usleep(100000);
	  count++;
	  rm_status=rm_read(iant, "RM_SOURCE_C34", testName);
	  for (j = strlen(testName)-1; j > 0; j--)
	    if (isspace(testName[j]))
	      testName[j] = (char)0;
	    else
	      break;
	}
      }
    }
    ignoreResults(changeflagging_1(&list, cl));
  }
  if (antennaArray[lowestAntennaInProject]) {
    /*
      Here we update the source name that is used in the interferometer data
      file ONLY if the new source name was given to the lowest numbered antenna
      in the array.   This allows different sources to be given to antennas not
      in the project list, for example antennas that are doing optical pointing.
    */
    dsm_status = dsm_write_notify("STORAGE", "DSM_AS_SOURCE_C34", fileSource);
    if (dsm_status != RM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(DSM_AS_SOURCE_C34)");
      exit(1);
    }
  }
return(0);
}

/***************************************************************
*
* project.c
* 
* SMAsh command for creating a project definition
* NAP 
* 2 January 2001
* modified on 14 June 2001 for logging the command
* and for specifying which antennas are to be included
* in the interferometer.
*
* 8 April 2003
* Lots of changes.
* SMAPopt compliant, hence scriptable.
* Added several new command-line switches.
*
* 3 march 2005, added receivers options followign taco's suggestions.
*
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <smapopt.h>
#include <time.h>
#include <netdb.h>
#include <socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <rpc/rpc.h>
#include <errno.h>
#include "ModbusTCP.h"
#include "rm.h"
#include "dsm.h"
#include "smapopt.h"
#include "commonLib.h"
#include "tune.h"
#include "dataDirectoryCodes.h"
#include "bandwidthDoubler.h"

#define PI_STRING_SIZE 30
#define DESC_STRING_SIZE 256
#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))
#define CRATES_ARRAY_SZ (sizeof(crateArray)/sizeof(crateArray[0]))

#define TRUE  (1)
#define FALSE (0)

int antennasInArray[11];

extern void sendHUPToCorrelator();

void usage(int exitcode, char *error, char *addl) {
  if (error) fprintf(stderr,"\n%s: %s\n\n", error, addl);
  fprintf(stderr, "project --help for usage\n");
  exit(exitcode);
}

int fd;
void updateProjectDetails(void);

void main(int argc, char**argv)  
{
  FILE *fp_projectID,*fpAntennaFile,*fpCrateFile,*fpArrayStatusFile;
  FILE *fpYIGFile;
  char c;
  int antennaArray[SMAPOPT_MAX_ANTENNAS+1],antennas[SMAPOPT_MAX_ANTENNAS+1];
  char projectPI[RM_ARRAY_SIZE * PI_STRING_SIZE],
    projectDescription[RM_ARRAY_SIZE * DESC_STRING_SIZE];
  char dsmProjectPI[30],dsmProjectDescription[256], scriptFilename[256];
  char receiversUsed[10];
  int numberOfReceivers = 0;
  int receiverSpecified = 0;
  char *_projectPI;
  char *_projectDescription;
  char *_scriptFilename;
  char *_comment,*_observer,*_operatingLocation;
  char *_receiversUsed;
  char comment[256],observer[30],operatingLocation[256];
  int iant,numberOfAnts=0;
  int occupiedPads[32];
  short antennaStatus;
  int dsm_status,rm_status,antlist[RM_ARRAY_SIZE];
  int projectID[RM_ARRAY_SIZE],currentProjectID;
  int i,status;
  char *_doubleBandwidth;
  int doubleBandwidth = FALSE;
  int fullPolarization = FALSE;
  int singleBandwidth = FALSE;
  int ignoreBandwidth = FALSE;
  int errorInSwitch;
  struct timeval tv;
  struct timezone tz;
  int seconds;
  int scriptPID, scriptPIDFlag=0, scriptFilenameFlag=0;
  int receiversFlag=0;
  int cratesFlag=0,numberOfCrates=0,icrate,crateArray[SMAPOPT_MAX_CRATES+1];
  int lowReceiver=1,highReceiver=2;
  FILE *fpReceiversFile;
  
  int corrDataType = DD_PRIMING;
  
  smapoptContext optCon;
  
  struct  smapoptOption optionsTable[] = {
    {"antennas",         'a', SMAPOPT_ARG_ANTENNAS,  antennaArray,        'a',
     "a comma-separated list of antennas"},
    {"bandwidthDoubler", 'B', SMAPOPT_ARG_STRING,    &_doubleBandwidth,   'B',
     "Force bandwidth doubler mode for \"l\" or \"h\""},
    {"comment",          'c', SMAPOPT_ARG_STRING,    &_comment,           'c',
     "Comment (e.g., phone number where observer can be reached)"},
    /*
    {"Crates",           'C', SMAPOPT_ARG_CRATES,    crateArray,          'C',
     "a comma-separated list of correlator crates"},
    */
    {"description",      'd', SMAPOPT_ARG_STRING,    &_projectDescription,'d',
     "brief description of project"},
    {"DataType",         'D', SMAPOPT_ARG_DATA_TYPE, &corrDataType,       'D',
     "garbage, science, priming (default), engineering, pointing, baseline, flux"},
    {"filename",         'f', SMAPOPT_ARG_STRING,    &_scriptFilename,    'f',
     "Filename of observing script (with full path)"},
    {"idprocess",        'i', SMAPOPT_ARG_INT,       &scriptPID,          'i',
     "PID of observing script"},
    {"IgnoreBandwidth",  'I', SMAPOPT_ARG_NONE,      &ignoreBandwidth,    'I',
     "Do nothing to the bandwidth doubler hardware"},
    {"location",         'l', SMAPOPT_ARG_STRING,    &_operatingLocation, 'l',
     "operating location (Summit, Hilo, Cambridge, Taipei)"},
    {"observer",         'o', SMAPOPT_ARG_STRING,    &_observer,          'o',
     "Name of collaborator/co-observer (enclose in double quotes if giving full name)"},
    {"pi",               'p', SMAPOPT_ARG_STRING,    &_projectPI,         'p',
     "Name of PI (enclose in double quotes if giving full name)"},
    /*
    {"fullPolarization", 'P', SMAPOPT_ARG_NONE,      &fullPolarization,   'P',
     "Full Polarization Mode"},
    */
    {"revise",           'r', SMAPOPT_ARG_NONE,      0,                   'r',
     "revise existing project (using additional input parameters)"},
    {"Receivers",        'R', SMAPOPT_ARG_STRING,    &_receiversUsed,     'R',
     "Receivers used in the project (l or h or l,h or h,l)"},
    {"status",           's', SMAPOPT_ARG_INT,       &status,             's',
     "status: integer 1: interferometry, 2: engineering, 
			0: idle, -1: lockout"},
    {"singleBandwidth",  'S', SMAPOPT_ARG_NONE,      &singleBandwidth,    'S',
     "Force single bandwidth mode"},
    {"verbose",          'v', SMAPOPT_ARG_NONE,      0,                   'v',
     "verbose flag (for some mostly useless feedback)"},
    SMAPOPT_AUTOHELP
    {NULL,'\0',0,NULL,0},
    "This command initiates an observing run on the SMA. \n
		--help gives a detailed usage.\n
		\n
		For interactive inputs of required arguments simply type
project and hit return.\n
		Some examples:\n
		To start a science track, the required arguments are: -a (list of
antennas), -p (PI), -o (Observer), -l (location), -d (description), -s (status: 1) ,
-R(l/h/lh receiver selection- this is important for science tracks-
otherwise the correlator will not start integrating)

project -a 2..7 -s 1 -p \"I. M. Astronomer\" -o \"U. R. Observer\" -l \"Summit\" 
-d \"Probing Dark Energy\" -Rl

Use the -r switch to revise the current project. Thus, for example, 
if we want to remove antennas 3 and 5 from the current project, issue:

project -r -a 3,5 -s 0

Later, if you want to add back antenna 3 to the project:

project -r -a 3 -s 1


		"
  };
  
  int antennaFlag=0, statusFlag=0, reviseFlag=0;
  int verboseFlag=0,corrDataFlag=0;
  int piFlag=0,observerFlag=0,commentFlag=0;
  int locationFlag=0,descriptionFlag=0;
  int antennaLockOutStatus, antennaGiven,jant;
  int dsmProjectID ;
  short dummyShort;
  time_t timestamp;
  char projectCommandString[1024];
  char receiverArgument[10],antennaArgument[22];
  char directoryArgument[20];
  
  /* END of variables declarations */

  for (i = 0; i <= SMAPOPT_MAX_ANTENNAS; i++)
    antennas[i] = 0;
  optCon = smapoptGetContext("projectCommand", argc, argv, optionsTable,
			     SMAPOPT_CONTEXT_EXPLAIN);
  
  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    
    switch(c) {
      
    case 'a' :
      antennaFlag=1;
      break;

    case 'B':
      if ((_doubleBandwidth[0] == 'l') || (_doubleBandwidth[0] == 'L'))
	doubleBandwidth = 1;
      else if ((_doubleBandwidth[0] == 'h') || (_doubleBandwidth[0] == 'H'))
	doubleBandwidth = 2;
      else {
	fprintf(stderr, "bandwidthDoubler (-B) must be given an argument of l(ow) or h(igh)\n");
	exit(-1);
      }
    case 'c':
      commentFlag=1;
      break;

    case 'C' :
      cratesFlag=1;
      fprintf(stderr, "The -C option is no longer supported - restartCorrelator handles this now\n");
      break;

    case 'd':
      descriptionFlag=1;
      break;
      
    case 'D':
      corrDataFlag=1;
      break;
      
    case 'f':
      scriptFilenameFlag=1;
      break;
      
    case 'i':
      scriptPIDFlag=1;
      break;
      
    case 'l':
      locationFlag=1;
      break;
      
    case 'o':
      observerFlag=1;
      break;
      
    case 'p':
      piFlag=1;
      break;
      /*
    case 'P':
      fullPolarization = TRUE;
      break;
      */
    case 'r':
      reviseFlag=1;
      break;
      
    case 'R':
      receiversFlag=1;
      break;
      
    case 's':
      statusFlag=1;
      break;

    case 'v':
      verboseFlag=1;
      break;
      
    }
  }
  smapoptFreeContext(optCon);
  /* if no arguments given, enter interactive mode for required arguments */
  
  /* debug here */
  
  if (argc < 2) {
    printf("No arguments given. Entering interactive mode for
required arguments...\n");
    printf("If you need to just revise some parameters, please
ctrl-C now and reissue the project command with the -r switch (see help
on the input parameters by typing: project --help).\n");
    printf("Or hit return to continue...\n");
    getchar();
    
    printf("Enter P.I.'s name: ");
    fgets(projectPI,30,stdin);
    projectPI[strlen(projectPI)-1]='\0';
    
    printf("Enter observer's name: ");
    fgets(observer,30,stdin);
    observer[strlen(observer)-1]='\0';
    
    printf("Enter observer's location (operating from): ");
    fgets(operatingLocation,256,stdin);
    operatingLocation[strlen(operatingLocation)-1]='\0';
    
    printf("Enter description of project (max 256 chars): ");
    fgets(projectDescription,256,stdin);
    projectDescription[strlen(projectDescription)-1]='\0';
    
    printf("Enter antenna-list without the -a (e.g. 1,2,3 or 1..7 or  1..3,6): ");
    fgets(antennaArgument,22,stdin);
    antennaArgument[strlen(antennaArgument)-1]='\0';
    
    printf("Enter receiver selection (l or h or l,h): ");
    fgets(receiverArgument,10,stdin);
    receiverArgument[strlen(receiverArgument)-1]='\0';

    printf("Enter the directory underwhich the data should be stored (baseline, flux, pointing, science, engineering, garbage or priming): ");
    fgets(directoryArgument,12, stdin);
    directoryArgument[strlen(directoryArgument)-1] = '\0';
    
    sprintf(projectCommandString,"project -p \"%s\" -o \"%s\" -d \"%s\" -l \"%s\" -a %s -s 1 -R %s -D %s",projectPI,observer,projectDescription,operatingLocation,antennaArgument,receiverArgument, directoryArgument);
    printf("\n%s\n",projectCommandString);
    system(projectCommandString);
    exit(0);
  }
  
  /* check whether the input switches are in correct combinations:
     if revise flag is set- 
     any of these are OK in any combination:
     pi, observer, location, description, comment.
     but, if antenna flag is given, status should also be set.
     if revise flag is not set- then it must be a new project, and the following
     are required to be set:
     antenna AND status,
     pi, location and description.
     Comment and observer are optional..
  */
  
  errorInSwitch=0;
  
  if((antennaFlag==1)||(statusFlag==1)) {
    if(!((antennaFlag==1)&&(statusFlag==1)))  errorInSwitch=1;
  }
  
  if(reviseFlag==0) {
    if(!(antennaFlag&&statusFlag&&piFlag&&locationFlag&&descriptionFlag)) 
      errorInSwitch=1;
  } 
  
  if(errorInSwitch==1) {
    printf("Some required input parameters are missing. Please type project --help
	for correct usage and reissue the project command.\n");
    exit(-1);
  }

  /* initializing ref. mem. and dsm */
  rm_status=rm_open(antlist);
  if(rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    exit(1);
  }

  /* Check that no two antennas claim to be on the same pad */
  for (iant = 0; iant < 32; iant++)
    occupiedPads[iant] = -1;
  for (iant = 1; iant <= 8; iant++) {
    char pad;

    rm_status=rm_read(iant,"RM_PAD_ID_B", &pad);
    if ((pad < 0) || (pad > 31)) {
      fprintf(stderr, "Warning - antenna %d claims to be on nonexistant pad #%d\n",
	      iant, pad);
    } else {
      if (occupiedPads[pad] != -1) {
	fprintf(stderr, "\n\n\nWARNING - antennas %d and %d both claim to be on pad %d\n\n\n",
		iant, occupiedPads[pad], pad);
      } else {
	occupiedPads[pad] = iant;
      }
    }
  }

  dsm_status = dsm_open();
  if(dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_open");
    exit(-1);
  }
  
  dsm_status = dsm_read("hcn","DSM_AS_PROJECT_ID_L",&dsmProjectID,&timestamp);
  

  if((dsmProjectID !=0) && (reviseFlag!=1)) {
    printf("There is an ongoing project. Please issue the -r switch to revise
some of the project parameters, or end the existing project by the endProject
command first.\n");
    exit(-1);
  }

  if(antennaFlag==1) {
    numberOfAnts=0;
    for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
      if(antennaArray[iant]==1) {
	antennas[numberOfAnts]=iant;
	numberOfAnts++;
      }
    }
  }
  
  
  if(cratesFlag==1) {
    fpCrateFile= fopen("/global/projects/cratesInArray","w");
    numberOfCrates=0;
    for(icrate=1;icrate<CRATES_ARRAY_SZ;icrate++) {
      if(crateArray[icrate]==1) {
	fprintf(fpCrateFile,"%d ",icrate);
	numberOfCrates++;
      }
    }
    fclose(fpCrateFile);
  }
  
  if (fullPolarization) {
    if (receiversFlag == 1) {
      fprintf(stderr, "-P and -R cannot be used together\n");
      exit(-1);
    }
    fpReceiversFile= fopen("/global/projects/receiversInArray","w");
    fprintf(fpReceiversFile, "1 2\n");
    fclose(fpReceiversFile);
  } else if(receiversFlag==1) {
    fpReceiversFile= fopen("/global/projects/receiversInArray","w");
    strcpy(receiversUsed,_receiversUsed);
    if(!(strcmp(receiversUsed,"l"))) {
      fprintf(fpReceiversFile,"%d",lowReceiver);
      numberOfReceivers = 1;
      receiverSpecified = 1;
    }
    else if(!(strcmp(receiversUsed,"h"))) {
      fprintf(fpReceiversFile,"%d ",highReceiver);
      numberOfReceivers = 1;
      receiverSpecified = 2;
    }
    else if(!(strcmp(receiversUsed,"l,h"))) {
      fprintf(fpReceiversFile,"%d %d ",lowReceiver,highReceiver);
      numberOfReceivers = 2;
    }
    else if(!(strcmp(receiversUsed,"h,l"))) {
      fprintf(fpReceiversFile,"%d %d ",lowReceiver,highReceiver);
      numberOfReceivers = 2;
    } else
      usage(-2,"Incorrect receivers argument","expecting l or h or l,h or h,l\n");
    fclose(fpReceiversFile);
  }

  if (ignoreBandwidth && singleBandwidth) {
    fprintf(stderr, "You cannot specify both IgnoreBandwidth (-I) and singleBandwidth (-S)\n");
    exit(-1);
  }
  if (ignoreBandwidth && doubleBandwidth) {
    fprintf(stderr, "You cannot specify both IgnoreBandwidth (-I) and doubleBandwidth (-S)\n");
    exit(-1);
  }
  if (singleBandwidth && doubleBandwidth) {
    fprintf(stderr, "You cannot specify both single bandwidth and double bandwidth\n");
    exit(-1);
  }
  if (reviseFlag == 1) {
    if (singleBandwidth || doubleBandwidth) {
      fprintf(stderr, "You cannot specify singleBandwidth or doubleBandwidth with -r\n");
      exit(-1);
    }
  }
  if((antennaFlag==1) && (reviseFlag!=1)) {
    
#if 0
    /* check if any of the requested antennas are used for engineering tests 
       or have been locked-out */
    antennaLockOutStatus=0;	
    for(iant=0;iant<numberOfAnts;iant++) {
      rm_status=rm_read(antennas[iant],"RM_ANTENNA_STATUS_S", &antennaStatus);
      
      
      if (antennaStatus==-1) {
	printf("Antenna %d is locked out!\n",antennas[iant]);
	fprintf(stderr,"Antenna %d is locked out!\n",antennas[iant]);
	antennaLockOutStatus=-1;	
      }
      if (antennaStatus==4) {
	printf("Antenna %d is busy with an engineering test.\n",antennas[iant]);
	fprintf(stderr,"Antenna %d is busy with an engineering test.\n",antennas[iant]);
		antennaLockOutStatus=4;	
      }
    }
    if((antennaLockOutStatus==-1)&&(antennaLockOutStatus==4)) {
      printf("Some of the requested antennas were not available. 
	Please re-issue the project command with a revised list of antennas.\n");
      fprintf(stderr,
	      "Some of the requested antennas were not available. 
	       Please re-issue the project command with a revised list of antennas.\n");
      exit(-1);
    }
#endif
    
    fpAntennaFile= fopen("/global/projects/antennasInArray","w");
    for(iant=0;iant<numberOfAnts;iant++) {
      fprintf(fpAntennaFile,"%d ",antennas[iant]);
      dummyShort=(short)status;
      rm_status=rm_write(antennas[iant],"RM_ANTENNA_STATUS_S", &dummyShort);
    }
    fclose(fpAntennaFile);
    
    fpArrayStatusFile= fopen("/global/projects/arrayStatus","w");
    fprintf(fpArrayStatusFile,"%d",status);
    fclose(fpArrayStatusFile);	
    
    /* update the project ID if this is a new interferometry project*/
    if(status==1) {
      
      fp_projectID = fopen("/global/projects/project.id","r+");
      if(fp_projectID==NULL) {
	fprintf(stderr,"Failed to open the project.id file.\n");
	exit(-1);
      }
      
      fscanf(fp_projectID,"%d",&currentProjectID);
      for(i=0;i<RM_ARRAY_SIZE;i++) projectID[i] = currentProjectID + 1;
      rewind(fp_projectID);
      fprintf(fp_projectID,"%d",projectID[0]);
      if(verboseFlag==1) printf("projectID=%d\n",projectID[0]);
      fclose(fp_projectID);
      /*
	rm_status|=rm_write(RM_ANT_ALL,"RM_PROJECT_ID_L", &projectID);
      */
      dsm_status = dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_ID_L",&projectID);
      if(dsm_status != DSM_SUCCESS) {
	dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) projectID");
	exit(1);
      }
      dsm_status = dsm_write("colossus","DSM_PROJECT_ID_L",&projectID);
      if(dsm_status != DSM_SUCCESS) {
	dsm_error_message(dsm_status,"dsm_write(colossus) projectID");
	exit(1);
      }
      
      /* write the starting time of the project in dsm for
	 keeping track of how stale this project becomes if it
	 is not ended by the endProject command by a careless observer */
      
      gettimeofday(&tv,&tz);
      seconds = tv.tv_sec;
      dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_STARTTIME_L",&seconds);
      if(dsm_status != DSM_SUCCESS) {
	dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) startTime");
	exit(1);
      }
      
    } /* if status =1 (interferometry) */ 
    
    /* create the killYIG files for Taco- to keep the YIGs from shutting OFF
       for better temperature stability */
    
    fpYIGFile= fopen("/otherPowerPCs/acc/1/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/1/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/2/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/2/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/3/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/3/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/4/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/4/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/5/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/5/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/6/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/6/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/7/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/7/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/8/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/8/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/9/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/9/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/10/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/acc/10/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/storage/1/configFiles/killYIG1","w");
    fclose(fpYIGFile);
    fpYIGFile= fopen("/otherPowerPCs/storage/1/configFiles/killYIG2","w");
    fclose(fpYIGFile);
    
    
  } /* if this is a new project */
  
  /*
    printf("reviseFlag=%d antennaFlag=%d status=%d numberOfAnts=%d\n",
    reviseFlag,antennaFlag,status,numberOfAnts);
  */
  
  if ((reviseFlag==1)&&(antennaFlag==1)) {
    if(status==1) {
      
      for(iant=0;iant<numberOfAnts;iant++) {
	dummyShort=(short)status;
	rm_status=rm_write(antennas[iant],"RM_ANTENNA_STATUS_S", &dummyShort);
      }
      
      fpAntennaFile= fopen("/global/projects/antennasInArray","w");
      for(iant=1;iant<=8;iant++) {
	rm_read(iant,"RM_ANTENNA_STATUS_S", &antennaStatus);
	if(antennaStatus==1)
	  fprintf(fpAntennaFile,"%d ",iant);
      }
      fclose(fpAntennaFile);
      
    }/* if status is set to 1 (interferometry)*/
    
    else {
      
      fpAntennaFile= fopen("/global/projects/antennasInArray","w");
      
      /*
	For antennas 1 to 10,
	check current status in RM.
	if currentstatus is 1 and this is not one of the given antennas,
	write this antenna number into the file. 
      */
      for(iant=1;iant<=10;iant++) {
	i=0;
	antennaGiven=0;
	while(antlist[i] != RM_ANT_LIST_END)
	  if(iant == antlist[i++]) {
	    rm_read(iant,"RM_ANTENNA_STATUS_S", &antennaStatus);
	    for(jant=0;jant<numberOfAnts;jant++) {
	      if(iant==antennas[jant]) antennaGiven=1;
	    }
	    if((antennaStatus==1)&&(antennaGiven==0)) {
	      fprintf(fpAntennaFile,"%d ",iant);
	    }
	  }
      }
      fclose(fpAntennaFile);
      
      
      /*
	For all given antennas, write given status in RM.
      */
      
      for(jant=0;jant<numberOfAnts;jant++) {
	dummyShort=(short)status;
	rm_status=rm_write(antennas[jant],
			   "RM_ANTENNA_STATUS_S", &dummyShort);
      }
      
    }/* if status is other than for interferometry */
  }
  
  
  if(piFlag==1) {
    strcpy(dsmProjectPI,_projectPI);
    if(verboseFlag==1) printf("project pi = %s\n",_projectPI);
    for(i=0;i<RM_ARRAY_SIZE;i++) strcpy(projectPI +i*PI_STRING_SIZE,_projectPI);
    /*	rm_status=rm_write(RM_ANT_ALL,"RM_PROJECT_PI_C30", projectPI);
     */
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_PI_C30", dsmProjectPI);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) projectPI");
      exit(1);
    }
    
  }
  
  if(descriptionFlag==1) {
    strcpy(dsmProjectDescription,_projectDescription);
    if(verboseFlag==1) printf("project description = %s\n",_projectDescription);
    for(i=0;i<RM_ARRAY_SIZE;i++) {
      strcpy(projectDescription+i*DESC_STRING_SIZE,_projectDescription);
    }
    /*	rm_status|=rm_write(RM_ANT_ALL,"RM_PROJECT_DESCRIPTION_C256",
	projectDescription);
    */
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_DESCRIPTION_C256",
			 dsmProjectDescription);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) projectDescription");
      exit(1);
    }
  }
  
  if(observerFlag==1) {
    strcpy(observer,_observer);
    if(verboseFlag==1) printf("observer = %s\n",_observer);
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_OBSERVER_C30",observer);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) observer");
      exit(1);
    }
  }

  if(locationFlag==1) {
    strcpy(operatingLocation,_operatingLocation);
    if (verboseFlag==1) printf("location = %s\n",_operatingLocation);
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_OPERATINGLOCATION_C256",
			 operatingLocation);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) operatingLocation");
      exit(1);
    }
  }
  
  if(commentFlag==1) {
    strcpy(comment,_comment);
    if(verboseFlag==1) printf("comment = %s\n",_comment);
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_COMMENT_C256",comment);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) comment");
      exit(1);
    }
  }
  
  if(scriptFilenameFlag==1) {
    strcpy(scriptFilename,_scriptFilename);
    if(verboseFlag==1) printf("script filename= %s\n",_scriptFilename);
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_SCRIPT_FILENAME_C256",scriptFilename);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) scriptFilename");
      exit(1);
    }
  } 
  
  if(scriptPIDFlag==1) {
    if(verboseFlag==1) printf("script PID= %d\n",scriptPID);
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_SCRIPT_PID_L",&scriptPID);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) scriptPID");
      exit(1);
    }
  } else {
    scriptPID=0;
    dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_SCRIPT_PID_L",&scriptPID);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(M5ANDMONITOR) scriptPID = 0");
      exit(1);
    }
  }
  
  if (reviseFlag != 1) {
    short shortZero = 0;
    
    if (verboseFlag == 1)
      printf("correlator Data Type Flag= %d\n",corrDataType);
    dsm_status=dsm_write("STORAGE","DSM_AS_CORRDATATYPE_L",&corrDataType);
    if(dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status,"dsm_write(STORAGE) datatype");
      exit(1);
    }
    dsm_status = dsm_write("DATACATCHER", "DSM_AS_POLAR_MODE_S", (char *)&shortZero);
    if (dsm_status != DSM_SUCCESS)
      dsm_error_message(dsm_status, "dsm_write(DATACATCHER) DSM_AS_POLAR_MODE_S)");
    dsm_status = dsm_write("OBS", "DSM_OBS_POLAR_MODE_S", (char *)&shortZero);
    if (dsm_status != DSM_SUCCESS)
      dsm_error_message(dsm_status, "dsm_write(OBS) DSM_OBS_POLAR_MODE_S)");
  }
  
  
  /* send HUP to correlator software to inform change of antennas in array */
  
  if((reviseFlag!=1)&&(status==1)) sendHUPToCorrelator();
  
  updateProjectDetails();

  if (!ignoreBandwidth && (reviseFlag != 1)) {
    /* Configure Bandwidth doubler assembly */
    int ant;
    configureBWD configuration;

    configuration.input[BWD_AUX] = BWD_IGN;
    if (!doubleBandwidth && !singleBandwidth) {
      if (fullPolarization)
	numberOfReceivers = 2;
      if (numberOfReceivers == 1) {
	doubleBandwidth = receiverSpecified;
      } else if (numberOfReceivers == 2) {
	singleBandwidth = TRUE;
      } else {
	fprintf(stderr, "Cannot determine which receiver(s) is/are to be used.\n");
	exit(-1);
      }
    }
    if (doubleBandwidth) {
      FILE *marker;
      
      marker = fopen("/global/configFiles/doubleBandwidth", "w");
      if (marker == NULL) {
	perror("creating double bandwidth marker file");
	exit(-1);
      } else
	fclose(marker);
      if (doubleBandwidth == 1) {
	configuration.input[BWD_LOWER] = BWD_LRL;
	configuration.input[BWD_UPPER] = BWD_LRH;
      } else {
	configuration.input[BWD_LOWER] = BWD_HRL;
	configuration.input[BWD_UPPER] = BWD_HRH;
      }
    } else {
      unlink("/global/configFiles/doubleBandwidth");
      configuration.input[BWD_LOWER] = BWD_LRL;
      configuration.input[BWD_UPPER] = BWD_HRL;
    }
    if (fullPolarization) {
      FILE *marker;
      
      marker = fopen("/global/configFiles/fullPolarization", "w");
      if (marker == NULL) {
	perror("creating polarization marker file");
	exit(-1);
      } else
	fclose(marker);
    } else {
      unlink("/global/configFiles/fullPolarization");
    }
    /*
      Send the new bandwidth doubler assembly configuration to tune6 on
      all antenna computers.
    */
    for (ant = 1; ant <= 8; ant++)
      if (antennasInArray[ant]) {
	char accName[10];
	CLIENT *cl;

	sprintf(accName, "acc%d", ant);
	if (!(cl = clnt_create(accName, TUNEPROG, TUNEVERS, "tcp"))) {
	  clnt_pcreateerror(accName);
	  return(-1);
	}
	configurebwd_1(&configuration, cl);
	clnt_destroy(cl);
      }
  }
}

void updateProjectDetails(void) {
  int i, dsm_status;
  int cratesInArray[14];
  int receiversInArray[3];
  short cratesInArrayShort[14];
  short antennasInArrayShort[11];
  short receiversInArrayShort[3];
  
  getCrateList(cratesInArray);
  getAntennaList(antennasInArray);
  getReceiverList(receiversInArray);
  for (i=0;i<13; i++) {
    cratesInArrayShort[i] = cratesInArray[i];
  }
  for (i=0;i<11; i++)
    antennasInArrayShort[i] = antennasInArray[i];
  for (i=0;i<3; i++)
    receiversInArrayShort[i] = receiversInArray[i];
  dsm_status = dsm_write("HALANDMONITOR","DSM_HAL_HAL_PROJECT_RECEIVERS_V3_S",
			 receiversInArrayShort);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status,"dsm_write(PROJECT_RECEIVERS) comment");
  dsm_status = dsm_write("HALANDMONITOR","DSM_HAL_HAL_PROJECT_ANTENNAS_V11_S",
			 antennasInArrayShort);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status,"dsm_write(PROJECT_ANTENNAS) comment");
  dsm_status = dsm_write("HALANDMONITOR","DSM_HAL_HAL_PROJECT_CRATES_V13_S",
			 cratesInArrayShort);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status,"dsm_write(PROJECT_CRATES) comment");
}

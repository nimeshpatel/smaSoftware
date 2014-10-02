/***************************************************************
*
* endProject.c
* 
* SMAsh command for ending a project
* NAP 
* 05 April 2001
* 
* 09 June 2003; modified following changes in project.c
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
#include <netdb.h>
#include <socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <errno.h>
#include "ModbusTCP.h"
#include "rm.h"
#include "dsm.h"
#include "commonLib.h"
#include "dataDirectoryCodes.h"

#define PI_STRING_SIZE 30
#define DESC_STRING_SIZE 256

int fd;
void updateProjectDetails(void);
extern void sendHUPToCorrelator();

void main()  
{
	FILE *fpAntennaFile;
	FILE *fpReceiversFile, *fpCrateFile;
	int dsm_status,rm_status,antlist[RM_ARRAY_SIZE];
	int i;
	short antennaStatus[11];
	short c1Source = 0;
	char comment[256],scriptFilename[256],operatingLocation[256],
		observer[30],dsmProjectDescription[256],dsmProjectPI[30];
	int projectStartTime=0;
	int scriptPID=0;
	int projectID=0;
	char zeroAntennas[20];
	int corrDataType=0;


/* Before doing anything, first prompt user to confirm if 
   we should proceed with endProject */

	printf("\n");
        printf("*****************************************************************************\n");
	printf("***                                                                       ***\n");
	printf("***    Please remember to execute /application/realTimeScripts/endObs.pl  ***\n");
	printf("***    before executing endProject at the end of every track.             ***\n");
	printf("***                                                                       ***\n");
        printf("*****************************************************************************\n");
	printf("\n");
	printf("Are you sure you want to issue endProject?\n");
	printf("Hit return to continue with endProject or Ctrl-C to abort.\n");
	getchar();
	

        /* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }
	dsm_status = dsm_open();
        if(dsm_status != DSM_SUCCESS) {
          dsm_error_message(dsm_status, "dsm_open");
          exit(-1);
        }

	
	/*
	for(i=0;i<PI_STRING_SIZE-1;i++) _projectPI[i]=' ';
	_projectPI[PI_STRING_SIZE-1]='\0';
	for(i=0;i<RM_ARRAY_SIZE;i++) strcpy(projectPI +i*PI_STRING_SIZE,_projectPI);
	*/

	for(i=0;i<11;i++) antennaStatus[i]=0;


	/* release all antennas */
        rm_status=rm_write(RM_ANT_ALL,"RM_ANTENNA_STATUS_S", &antennaStatus);

	fpAntennaFile= fopen("/global/projects/antennasInArray","w");
	for(i=0;i<19;i++) zeroAntennas[i]=' ';
	zeroAntennas[19]='\0';
	fprintf(fpAntennaFile,"%s",zeroAntennas);
	fclose(fpAntennaFile);
	fpReceiversFile= fopen("/global/projects/receiversInArray","w");
	for(i=0;i<19;i++) zeroAntennas[i]=' ';
	zeroAntennas[19]='\0';
	fprintf(fpReceiversFile,"%s",zeroAntennas);
	fclose(fpReceiversFile);
	fpCrateFile= fopen("/global/projects/cratesInArray","w");
	for(i=0;i<19;i++) zeroAntennas[i]=' ';
	zeroAntennas[19]='\0';
	fprintf(fpCrateFile,"%s",zeroAntennas);
	fclose(fpCrateFile);

/* now resetting all project related variables in dsm to
blanks/zeroes */


	for(i=0;i<DESC_STRING_SIZE-2;i++) {
	comment[i]=' ';
	scriptFilename[i]=' ';
	operatingLocation[i]=' ';
	dsmProjectDescription[i]=' ';
	}
	comment[DESC_STRING_SIZE-2]='\0';
	scriptFilename[DESC_STRING_SIZE-2]='\0';
	operatingLocation[DESC_STRING_SIZE-2]='\0';
	dsmProjectDescription[DESC_STRING_SIZE-2]='\0';


	for(i=0;i<PI_STRING_SIZE-2;i++) {
	observer[i]=' ';
	dsmProjectPI[i]=' ';
	}
	observer[PI_STRING_SIZE-2]='\0';
	dsmProjectPI[PI_STRING_SIZE-2]='\0';

	dsm_status=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_SCRIPT_PID_L",&scriptPID);
	dsm_status|=dsm_write("m5","DSM_AS_C1_SOURCE_S",&c1Source);
	dsm_status|=dsm_write("hcn","DSM_AS_C1_SOURCE_S",&c1Source);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_ID_L",&projectID);
	dsm_status|=dsm_write("colossus","DSM_PROJECT_ID_L",&projectID);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_STARTTIME_L",&projectStartTime);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_OBSERVER_C30",observer);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_PI_C30",dsmProjectPI);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_DESCRIPTION_C256",dsmProjectDescription);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_OPERATINGLOCATION_C256",operatingLocation);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_COMMENT_C256",comment);
	dsm_status|=dsm_write("M5ANDMONITOR","DSM_AS_PROJECT_SCRIPT_FILENAME_C256",scriptFilename);
	dsm_status|=dsm_write("m5","DSM_AS_CORRDATATYPE_L",&corrDataType);
	dsm_status|=dsm_write("hcn","DSM_AS_CORRDATATYPE_L",&corrDataType);
	if(dsm_status != DSM_SUCCESS) {
	  dsm_error_message(dsm_status,"dsm_write()");
	  exit(1);
	}
	
	

/* send HUP to correlator software to inform change of antennas in array
*/
        sendHUPToCorrelator();

	SMAshLogPrintfTimestamped("endProject.\n");

	system("cp /application/configFiles/statusServer.txt.default /application/configFiles/statusServer.txt");
	system("/global/bin/killdaemon hal9000 statusServer HUP");


/*
	system("/application/bin/smashLogExtract.pl");
*/

	updateProjectDetails();

	system("/application/bin/newFile -D garb");
	system("/global/bin/killdaemon hal9000 setLO restart");
	system("/global/bin/killdaemon m5 c1DCIFServer restart");
	system("/global/bin/killdaemon crate int_server QUIT");
	system("/global/bin/killdaemon newdds dDSServer QUIT");
	system("rm /global/configFiles/fullPolarization");
	system("configureCorrelatorLOs");
}

void updateProjectDetails(void) {
  short shortZero = 0;
  int i, dsm_status;
  int cratesInArray[14];
  int antennasInArray[11];
  int directoryCode = DD_GARBAGE;
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
  for (i=0;i<11; i++) {
    antennasInArrayShort[i] = antennasInArray[i];
  }
  for (i=0;i<3; i++) {
    receiversInArrayShort[i] = receiversInArray[i];
  }
  dsm_status = dsm_write("hal9000","DSM_HAL_HAL_PROJECT_RECEIVERS_V3_S",
			 receiversInArrayShort);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_write(PROJECT_RECEIVERS) comment");
  }
  dsm_status = dsm_write("hal9000","DSM_HAL_HAL_PROJECT_ANTENNAS_V11_S",
			 antennasInArrayShort);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_write(PROJECT_ANTENNAS) comment");
  }
  dsm_status = dsm_write("hal9000","DSM_HAL_HAL_PROJECT_CRATES_V13_S",
			 cratesInArrayShort);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_write(PROJECT_CRATES) comment");
  }
  dsm_status = dsm_write("DATACATCHER", "DSM_AS_POLAR_MODE_S", (char *)&shortZero);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status, "DSM_AS_POLAR_MODE_S)");
  dsm_status = dsm_write("OBS", "DSM_OBS_POLAR_MODE_S", (char *)&shortZero);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status, "DSM_OBS_POLAR_MODE_S)");
  dsm_status = dsm_write("m5", "DSM_AS_CORRDATATYPE_L", (char *)&directoryCode);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status, "DSM_AS_CORRDATATYPE_L");
  dsm_status = dsm_write("hcn", "DSM_AS_CORRDATATYPE_L", (char *)&directoryCode);
  if (dsm_status != DSM_SUCCESS)
    dsm_error_message(dsm_status, "DSM_AS_CORRDATATYPE_L");
}



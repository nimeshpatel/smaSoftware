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
#include "rm.h"
#include "smapopt.h"

#define PI_STRING_SIZE 30
#define DESC_STRING_SIZE 256
#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))

extern void sendHUPToCorrelator();

void main(int argc, char**argv)  
{
	FILE *fp_projectID,*fpAntennaFile;
	char c;
	int antennaArray[SMAPOPT_MAX_ANTENNAS+1],antennas[SMAPOPT_MAX_ANTENNAS+1];
	char projectPI[RM_ARRAY_SIZE * PI_STRING_SIZE],
		projectDescription[RM_ARRAY_SIZE * DESC_STRING_SIZE];
	char *_projectPI;
	char *_projectDescription;
	char *_comment,*_observer,*_operatingLocation;
	int iant,numberOfAnts;	
	short antennaStatus;
	int rm_status,antlist[RM_ARRAY_SIZE];
	int projectID[RM_ARRAY_SIZE],currentProjectID;
	int i,status;
	int errorInSwitch;
	
	smapoptContext optCon;
	
	struct  smapoptOption optionsTable[] = {
                {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a',
                "a comma-separated list of antennas"},
                {"status",'s',SMAPOPT_ARG_INT,&status,'s',
                "status: integer 1: interferometry, 2: holography, 3: pointing, 
			4: engineering, 0: idle, -1: lockout"},
                {"pi",'p',SMAPOPT_ARG_STRING,&_projectPI,'p',
                "Name of PI (enclose in double quotes if giving full name)"},
                {"observer",'o',SMAPOPT_ARG_STRING,&_observer,'c',
                "Name of collaborator/co-observer (enclose in double quotes if giving full name)"},
                {"location",'l',SMAPOPT_ARG_STRING,&_operatingLocation,'l',
                "operating location (Summit, Hilo, Cambridge, Taipei)"},
                {"description",'d',SMAPOPT_ARG_STRING,&_projectDescription,'d',
                "brief description of project"},
                {"comment",'c',SMAPOPT_ARG_STRING,&_comment,'c',
                "Comment (e.g., phone number where observer can be reached)"},
                {"revise",'r',SMAPOPT_ARG_NONE,0,'r',
                "revise existing project (using additional input parameters)"},
                SMAPOPT_AUTOHELP
                {NULL,'\0',0,NULL,0},
                "This command initiates an observing run on the SMA. \n
		--help gives a detailed usage.\n"
        };
/*
		-r (optional) - to revise existing project- will not generate \n
				a new project id.\n
		-a a comma separated list of antennas to include in the project\n
			or to revise the status\n
		-s status - an integer specifying how the antenna is to be used\n
			in the project: -1: lockout, 0: idle, 1: inteferometry\n
			2: holography, 3: pointing, 4: engineering.\n"
*/

	int antennaFlag=0, statusFlag=0, reviseFlag=0;
	int piFlag=0,observerFlag=0,commentFlag=0;
	int locationFlag=0,descriptionFlag=0;
	int antennaLockOutStatus, antennaGiven,jant;
	short dummyShort;


	/* END of variables declarations */

	optCon = smapoptGetContext("projectCommand", argc, argv, optionsTable,
                        SMAPOPT_CONTEXT_EXPLAIN);

        while ((c = smapoptGetNextOpt(optCon)) >= 0) {

                switch(c) {

                case 'a' :
                antennaFlag=1;
                break;

                case 'r':
                reviseFlag=1;
                break;

                case 's':
                statusFlag=1;
                break;

		case 'p':
		piFlag=1;
		break;

		case 'o':
		observerFlag=1;
		break;

		case 'l':
		locationFlag=1;
		break;

		case 'd':
		descriptionFlag=1;
		break;

		case 'c':
		commentFlag=1;
		break;

                }
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

	if(antennaFlag==1) {
	if(statusFlag!=1) errorInSwitch=1;
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



        /* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

	if((antennaFlag==1)&&(reviseFlag!=1)) {
	numberOfAnts=0;
	for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
                if(antennaArray[iant]==1) {
                antennas[numberOfAnts]=iant;
                numberOfAnts++;
                }
        }
	
	/* check if any of the requested antennas are used for engineering tests 
		or have been locked-out */
	antennaLockOutStatus=0;	
	for(iant=0;iant<numberOfAnts;iant++) {
	rm_status=rm_read(antennas[iant],"RM_ANTENNA_STATUS_S", &antennaStatus);

printf("antenna[%d] status = %d\n",iant,antennaStatus);

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

	fpAntennaFile= fopen("/global/projects/antennasInArray.test","w");
		for(iant=0;iant<numberOfAnts;iant++) {
		fprintf(fpAntennaFile,"%d ",antennas[iant]);
/***
		rm_status=rm_write(antennas[iant],"RM_ANTENNA_STATUS_S", &status);
***/
        	}
	fclose(fpAntennaFile);
	} /* if this is a new project */





	/* update the project ID */
	if((reviseFlag!=1)&&(statusFlag==1)&&(status==1)) {

		fp_projectID = fopen("/application/bin/project.id.test","r+");
		if(fp_projectID==NULL) {
		fprintf(stderr,"Failed to open the project.id file.\n");
		exit(-1);
		}

	fscanf(fp_projectID,"%d",&currentProjectID);
	printf("Read currentproject id as=%d\n",currentProjectID);
	for(i=0;i<RM_ARRAY_SIZE;i++) projectID[i] = currentProjectID + 1;
	rewind(fp_projectID);
	fprintf(fp_projectID,"%d",projectID[0]);
	printf("projectid=%d",projectID[0]);
	fclose(fp_projectID);
	} 




	if ((reviseFlag==1)&&(antennaFlag==1)) {
			numberOfAnts=0;
			for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
                	if(antennaArray[iant]==1) {
	                antennas[numberOfAnts]=iant;
       		         numberOfAnts++;
       		         }
        		}

	
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
		printf("Some of the requested antennas were not available.\n"); 
		fprintf(stderr,"Some of the requested antennas were not available.\n"); 
		}

		/* resetting those antennas which are currently in the project
		for interferometry, to zeroes */
		/* this should also be done in endProject command */
		for(iant=1;iant<=8;iant++) {
		rm_status=rm_read(iant,"RM_ANTENNA_STATUS_S", &antennaStatus);
			if(antennaStatus==1) {
			antennaStatus=0;
/***
			rm_status=rm_write(iant,"RM_ANTENNA_STATUS_S", &antennaStatus);
***/
			}
		}
	

		if((reviseFlag!=1)&&(status==1)) {

		   fpAntennaFile= fopen("/global/projects/antennasInArray.test","w");
		   for(iant=0;iant<numberOfAnts;iant++) {
		   fprintf(fpAntennaFile,"%d ",antennas[iant]);
/***
		   rm_status=rm_write(antennas[iant],"RM_ANTENNA_STATUS_S", &status);
***/
        	   }
		   fclose(fpAntennaFile);
		}/* if status is set to 1 (interferometry)*/

		else {

		   fpAntennaFile= fopen("/global/projects/antennasInArray.test","w");

		      for(iant=1;iant<=8;iant++) {
		      antennaGiven=0;
		        for(iant=0;iant<numberOfAnts;iant++) {
		        rm_status=rm_read(iant,"RM_ANTENNA_STATUS_S", &antennaStatus);
			   for(jant=0;jant<numberOfAnts;jant++) {
			   if(iant==antennas[jant]) antennaGiven=1;
			   }
			if((antennaStatus==1)&&(antennaGiven==0)) {
		   	fprintf(fpAntennaFile,"%d ",iant);
			}
			if(antennaGiven==1) {
			dummyShort=(short)status;
/***
			rm_status=rm_write(iant,"RM_ANTENNA_STATUS_S", &dummyShort);
***/
			}
		    }
		 }
		   fclose(fpAntennaFile);
		}/* if status is other than for interferometry */
	}
	
		
	if(piFlag==1) printf("project pi = %s\n",_projectPI);
	if(descriptionFlag==1) {
	printf("project description = %s\n",_projectDescription);
	}
	if(observerFlag==1) {
	printf("observer = %s\n",_observer);
	}
	if(locationFlag==1) printf("location = %s\n",_operatingLocation);
	if(commentFlag==1) printf("comment = %s\n",_comment);

/*
	for(i=0;i<RM_ARRAY_SIZE;i++) strcpy(projectPI +i*PI_STRING_SIZE,_projectPI);
	rm_status=rm_write(RM_ANT_ALL,"RM_PROJECT_PI_C30", projectPI);

	for(i=0;i<RM_ARRAY_SIZE;i++) strcpy(projectDescription+i*DESC_STRING_SIZE,_projectDescription);
	rm_status|=rm_write(RM_ANT_ALL,"RM_PROJECT_DESCRIPTION_C256",
				projectDescription);

	rm_status|=rm_write(RM_ANT_ALL,"RM_PROJECT_ID_L", &projectID);
*/


/* send HUP to correlator software to inform change of antennas in array */
	
/*
	if(reviseFlag!=1) sendHUPToCorrelator();
*/

}

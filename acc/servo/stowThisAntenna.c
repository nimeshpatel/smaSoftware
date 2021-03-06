/***************************************************************
*
* stowThisAntenna.c
*
* adapted from Nimesh's az.c
* 
* RWW 12/19/03
*
****************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include "rm.h"

#define OK     0
#define ERROR -1
#define RUNNING 1
#define AZ_NEG_LIM -148.
#define AZ_POS_LIM 358. 

command_n[30];

void main(int argc, char *argv[])
{

    char c,*degrees,messg[100],command_n[30];
    short  pmac_command_flag=0;
    short error_flag=RUNNING;
    int gotaz=0,rm_status,antlist[RM_ARRAY_SIZE];
    float az,actual_az,actual_el;
    smapoptContext optCon;
    int i,antenna;
    int gotantenna=0,antennaArray[]={0,0,0,0,0,0,0,0,0};
    int trackStatus=0,iant;
    int tracktimestamp,timestamp;
    short antennaStatus = 0;

    /* initializing ref. mem. */
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }
    antenna = 0;

    /* check if track is running on this antenna */

    rm_status=rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
    rm_status=rm_read(antenna,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
    if(abs(tracktimestamp-timestamp)>3L) {
        trackStatus=0;
        printf("Track is not running on this antenna.\n");
        exit(1);
    }



    command_n[0]='T'; /* send 'T' command */
    az = 10;
    el = 45;
    rm_status=rm_read(antenna, "RM_ACTUAL_AZ_DEG_F",&actual_az);
    rm_status|=rm_read(antenna,"RM_ACTUAL_EL_DEG_F",&actual_el);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_read()");
        exit(1);
    }


    rm_status=rm_write(antenna, "RM_SMARTS_AZ_DEG_F",&az);
    rm_status|=rm_write(antenna, "RM_SMARTS_EL_DEG_F",&actual_el);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_write()");
        exit(1);
    }


    rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
                       &command_n);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_write()");
        exit(1);
    }


    pmac_command_flag=0;

    rm_status=rm_write_notify(antenna,"RM_SMARTS_PMAC_COMMAND_FLAG_S",
                              &pmac_command_flag);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_write_notify()");
        exit(1);
    }


    error_flag=RUNNING;

    sleep(3);

    rm_status=rm_read(antenna,
                      "RM_SMARTS_COMMAND_STATUS_S",&error_flag);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_read()");
        exit(1);
    }

    if(error_flag==ERROR)
    {
        printf("Error from track:\n");
        rm_status=rm_read(antenna,
                          "RM_TRACK_MESSAGE_C100",messg);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read()");
            exit(1);
        }
        printf("%s\n",messg);
    }

    /*
            if(error_flag==OK) printf("antenna has reached the source.\n");
    */
}

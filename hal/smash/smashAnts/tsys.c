#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rm.h"
#include "smapopt.h"
#include "smashAnts.h"

#define NUMBER_ANTENNAS 10

static int intTime = 4;
static int debug = 0, measureTrx = 0;
static int quiet = 0;
static int eSMATest = 0;

void usage(void) {

  printf("Usage: tsys [options] \n"
	  "[options] include:\n"
	  "  -d or --debug		print debugging info\n"
	  "  -h or --help		this help\n"
	  "  -i or --integration	time in seconds (default = %d)\n",
	  intTime);
  printf("  -q or --quiet		do not print results\n");
  printf("  -r or --receiverTemp	also compute the receiver temperature "
	 "by\n			using the heated load\n"
	 "  -a<n> or --antenna <n> (n is the antenna number)\n"
         "                  (default: all antennas)\n");
  exit(0);
}
#if 0
int rxList[3] = {0, 1, 1};
#endif

int main(int argc, char *argv[]) {
  char c;
  smapoptContext optCon;
  int gotantenna=0;
  int antennaArray[SMAPOPT_MAX_ANTENNAS+1];
  int rm_status, antlist[RM_ARRAY_SIZE];


  struct  smapoptOption optionsTable[] = {
    {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
    {"debug",'d',SMAPOPT_ARG_NONE,&debug, 0},
    {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
    {"integration",'i',SMAPOPT_ARG_INT,&intTime, 0},
    {"quiet",'q',SMAPOPT_ARG_NONE,&quiet, 0},
    {"receiverTemp",'r',SMAPOPT_ARG_NONE,&measureTrx, 0},
    {NULL,0,0,NULL,0}
  };

  optCon = smapoptGetContext("tsys", argc, argv, optionsTable,0);
  
  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    switch(c) {
    case 'h':
      usage();
      break;
    case 'a':
      gotantenna=1;
      break;
    }
  }
  if (c < -1) {
    printf("%s: %s\n",
	    smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
	    smapoptStrerror(c));
  }
  smapoptFreeContext(optCon);
#if 0
  printf("calling tsys with antennaArray = %d, rxList %d, intTime %d,\n"
    "measureTrx %d, print %d debug %d\n", (gotantenna)? (int)antennaArray: 0,
    0, intTime, measureTrx, !quiet, debug);
#endif
  /*
    K L U D G E   A L E R T   ! ! ! !

    The following code handles the Tsys measurements on the JCMT and CSO.
    It is here for the moment while I (Taco) debug the interaction with
    the CSO and JCMT.   When it works, it should be cleaned up, and
    merged into the tsys library call.
  */
  if (1 || eSMATest) {
    if (!gotantenna)
      getAntennaList(&antennaArray[0]);
    if (antennaArray[9] || antennaArray[10]) {
      short pmac_command_flag=0;
      int ant, i;
      char command_n[30];

      for(i = 0; i < 30; i++) {
        command_n[i] = 0x0;
      };

      rm_status = rm_open(antlist);
      if (rm_status != RM_SUCCESS) {
	rm_error_message(rm_status,"rm_open()");
	exit(1);
      }
      for (ant = 9; ant <= 10; ant++) {
	printf("Sending ambient in command to antenna %d\n", ant);
	command_n[0] = 'a';
        rm_status = rm_write(ant, "RM_SMASH_TRACK_COMMAND_C30", &command_n);
        if (rm_status != RM_SUCCESS) {
	   rm_error_message(rm_status,"rm_write(1)");
	   exit(1);
        }
	pmac_command_flag = 0;
        rm_status = rm_write_notify(ant,"RM_SMARTS_PMAC_COMMAND_FLAG_S", &pmac_command_flag);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write_notify(1)");
	  exit(1);
        }
      }
    }
  }
  /*   E N D   O F   K L U D G E   */
  if (!eSMATest)
    tsys((gotantenna)? antennaArray: 0, 0, intTime, measureTrx, !quiet, debug);
  /*
    K L U D G E   A L E R T   ! ! ! !

    The following code handles the Tsys measurements on the JCMT and CSO.
    It is here for the moment while I (Taco) debug the interaction with
    the CSO and JCMT.   When it works, it should be cleaned up, and
    merged into the tsys library call.
  */
  if (1 || eSMATest) {
    if (!gotantenna)
      getAntennaList(&antennaArray[0]);
    if (antennaArray[9] || antennaArray[10]) {
      short pmac_command_flag=0;
      int ant, i;
      char command_n[30];

      if (!eSMATest) {
	rm_status = rm_open(antlist);
	if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status,"rm_open()");
	  exit(1);
	}
      }
      for(i = 0; i < 30; i++) {
        command_n[i] = 0x0;
      };
      sleep(10);
      for (ant = 9; ant <= 10; ant++) {
	printf("Sending ambient out command to antenna %d\n", ant);
	command_n[0] = 's';
        rm_status = rm_write(ant, "RM_SMASH_TRACK_COMMAND_C30", &command_n);
        if (rm_status != RM_SUCCESS) {
	   rm_error_message(rm_status,"rm_write(2)");
	   exit(1);
        }
	pmac_command_flag = 0;
        rm_status = rm_write_notify(ant,"RM_SMARTS_PMAC_COMMAND_FLAG_S", &pmac_command_flag);
        if (rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status, "rm_write_notify(2)");
	  exit(1);
        }
      }
    }
  }
  /*   E N D   O F   K L U D G E   */
  return(0);
}


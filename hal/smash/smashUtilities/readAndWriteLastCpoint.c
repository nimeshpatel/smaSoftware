#define INITIAL_SMAPOPT_STRING_ARGUMENT "00000000"
#include <stdio.h>
#include <string.h>
#include "rm.h"
#include "dsm.h"
#include "smapopt.h"
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
static int requestedAntennas[ANT_LIST_SZ];
void usage(char *name);
int parseReceiverName(char *dummyString);

void usage(char *name) {
  printf("Usage: %s -h -a <antennaList> -u <l or h>\n",name);
}

int main(int argc, char  *argv[]) {
  smapoptContext optCon; 
  int stat,rms;
  int nolog = 0;
  long lastcpoint[11];
  long newcpoint[11];
  long timestamp;
  int antennas;
  short lastrx[11];
  short newrx[11];
  long unixtime;
  int i;
  int antlist[RM_ARRAY_SIZE];
  int rx = 0;
  char rxstring[10];
  char *updateString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *cp, c;
  char dummyString[10];

  struct smapoptOption options[] = {
    {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',"antenna argument (comma or space delimited)"},
    {"help", 'h', SMAPOPT_ARG_NONE, &usage, 'h', "print the usage"},
    {"log", 'l', SMAPOPT_ARG_NONE, &nolog, 'l', 
       "Do not write the command to the SMAshLog"},
    {"updateRx", 'u', SMAPOPT_ARG_STRING, &updateString, 'u', 
       "which receiver is being used to update the offsets: l or h"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 }
  };

  bzero(rxstring,sizeof(rxstring));
  if((cp = strrchr(argv[0], '/')) == NULL) {
    cp = argv[0];
  } else {
    cp++;
  }
  for (i=0; i<argc; i++) {
    if (strstr(argv[i],"-l")!=NULL ||
	strstr(argv[i],"--log")!=NULL) {
      nolog = 1;
    }
  }
  if (nolog==1) {
    /*    printf("Will not log to the SMAshLog\n");*/
    optCon = smapoptGetContext(cp, argc, argv, options, SMAPOPT_CONTEXT_NOLOG);
  } else {
    optCon = smapoptGetContext(cp, argc, argv, options, 0);
  }

  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(-1);
    case 'a':
      break;
    case 'u':
      strcpy(rxstring,updateString);
      break;
    }
  }
  smapoptFreeContext(optCon);
  /*  printf("read rxString = %s\n",rxstring);*/
  antennas = 0;
  for (i=0; i<ANT_LIST_SZ; i++) {
    if (requestedAntennas[i]) {
      antennas++;
    }
  }

  rms = rm_open(antlist);
  if(rms != RM_SUCCESS) {
    rm_error_message(rms,"rm_open()");
    exit(1);
  }
  stat = dsm_open();
  if (stat) {
    dsm_error_message(stat,"dsm_open");
    exit(-8);
  }
  if (strstr(rxstring,"h") != NULL) {
    rx = 1;
  }
  stat = dsm_read("hal9000", "DSM_HAL_HAL_LAST_CPOINT_V11_L", lastcpoint, &timestamp);
  /*
  for (i=1; i<=8; i++) {
    printf("%d ",lastcpoint[i]);
  }
  */

  stat = dsm_read("hal9000", "DSM_HAL_HAL_LAST_CPOINT_RX_V11_S", lastrx, &timestamp);
  unixtime = time((long *)0);
  for (i=1; i<=ANT_LIST_SZ; i++) {
    if (requestedAntennas[i]) {
      if (rx == 0) {
	rms = rm_read(i, "RM_ACTIVE_LOW_RECEIVER_C10", &dummyString);
	dummyString[2] = (char)0;
	newrx[i] = parseReceiverName(dummyString);
      } else if (rx == 1) {
	rms = rm_read(i, "RM_ACTIVE_HIGH_RECEIVER_C10", &dummyString);
	dummyString[2] = (char)0;
	newrx[i] = parseReceiverName(dummyString);
      } else {
	newrx[i] = -1;
      }
      newcpoint[i] = unixtime;
    } else {
      newrx[i] = lastrx[i];
      newcpoint[i] = lastcpoint[i];
    }
  }
  stat = dsm_write("hal9000","DSM_HAL_HAL_LAST_CPOINT_RX_V11_S",newrx);
  stat = dsm_write("hal9000","DSM_HAL_HAL_LAST_CPOINT_V11_L",newcpoint);
  return(dsm_close());
}

int parseReceiverName(char *dummyString1) {
 if (!strcmp(dummyString1, "A1")) return(0);
 if (!strcmp(dummyString1, "A2")) return(1);
 if (!strcmp(dummyString1, "B1")) return(2);
 if (!strcmp(dummyString1, "B2")) return(3);
 if (!strcmp(dummyString1, "C")) return(4);
 if (!strcmp(dummyString1, "D")) return(5);
 if (!strcmp(dummyString1, "E")) return(6);
 if (!strcmp(dummyString1, "F")) return(7);
 return(-1);
}


#define INITIAL_SMAPOPT_STRING_ARGUMENT "00000000"
#define DEBUG 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dsm.h"
#include "smapopt.h"
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)

int present(char *search,char *token);
void usage(char *name);

void usage(char *name) {
  printf("Usage: %s -h -v <dsmVariableName> -n <newvalue> -i <index(ForArrays)> -a <antennaList(ForArrays)> -m <machineName: default=hal9000>\n",name);
  printf("This program works with integers, floats, doubles and their arrays, as\n");
  printf("well as single-byte (B) and multi-byte character variables.\n");
  printf("Use -l to prevent the command from being logged to the SMAshLog\n");
}

static int requestedAntennas[ANT_LIST_SZ];

int main(int argc, char  *argv[]) {
  int stat,i,j;
  smapoptContext optCon; 
  char var[DSM_NAME_LENGTH];
  char temp[DSM_NAME_LENGTH];
  char *value, *ptr;
  time_t timestamp;
  int index, antennas;
  int newvaluegiven = 0;
  float dummyFloat;
  short dummyShort;
  char dummyChar;
  long dummyLong;
  double dummyDouble;
  long dualrxpoint[8];
  int size;
  char *cp, c;
  char *varString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *newvalueString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *machine =  INITIAL_SMAPOPT_STRING_ARGUMENT;
  char dummyString[100];
  char machineName[50];
  int nolog = 0;

  struct smapoptOption options[] = {
    {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',"antenna argument (comma or space delimited)"},
    {"help", 'h', SMAPOPT_ARG_NONE, &usage, 'h', "print the usage"},
    {"index", 'i', SMAPOPT_ARG_INT, &index, 'i', 
       "DSM variable to modify (case insensitive)"},
    {"log", 'l', SMAPOPT_ARG_NONE, &nolog, 'l', 
       "Do not write the command to the SMAshLog"},
    {"machine", 'm', SMAPOPT_ARG_STRING, &machine, 'm', 
       "The name of the machine on which to write into the DSM variable"},
    {"newvalue", 'n', SMAPOPT_ARG_STRING, &newvalueString, 'n', 
       "The new value to stuff into the DSM variable"},
    {"variable", 'v', SMAPOPT_ARG_STRING, &varString, 'v', 
       "DSM variable to modify (case insensitive)"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 }
  };

  strcpy(machineName,"hal9000");
  index = -1;
  if((cp = strrchr(argv[0], '/')) == NULL) {
    cp = argv[0];
  } else {
    cp++;
  }
  for (i=0; i<argc; i++) {
    if (present(argv[i],"-l")||present(argv[i],"--log")) {
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
    case 'v':
      for (j=0; j<strlen(varString); j++) {
	var[j] = toupper(varString[j]);
      }
      var[j] = 0;
      break;
    case 'm':
      strcpy(machineName,machine);
      break;
    case 'n':
      newvaluegiven = 1;
      value = newvalueString;
      break;
    case 'a':
      break;
    case 'i':
      break;
    }
  }
  smapoptFreeContext(optCon);
#if 0
  for (i=1; i<argc; i++) {
    if (present(argv[i],"-v")) {
      if (++i < argc) {
	continue;
      } else {
	usage(argv[0]);
	exit(-2);
      }
    }
    if (present(argv[i],"-a")) {
      if (++i < argc) {
	sscanf(argv[i],"%d",&antenna);
	index = antenna - 1;
	continue;
      } else {
	usage(argv[0]);
	exit(-2);
      }
    }
    if (present(argv[i],"-i")) {
      if (++i < argc) {
	sscanf(argv[i],"%d",&index);
	continue;
      } else {
	usage(argv[0]);
	exit(-2);
      }
    }
    if (present(argv[i],"-n")) {
      if (++i < argc) {
	value = argv[i];
	newvaluegiven = 1;
	continue;
      } else {
	usage(argv[0]);
	exit(-3);
      }
    }
  }
#endif
  if (DEBUG) fprintf(stderr,"Finished parsing command-line\n");
  antennas = 0;
  for (i=0; i<ANT_LIST_SZ; i++) {
    if (requestedAntennas[i]) {
      antennas++;
    }
  }
  if (strlen(var) == 0) {
    printf("You must supply a DSM variable name via -v\n");
    usage(argv[0]);
    exit(-4);
  }
  if (newvaluegiven == 0) {
    printf("You must supply a new value via -n\n");
    usage(argv[0]);
    exit(-5);
  }
  if (!present(var,"dsm_") && !present(var,"DSM_")) {
    if (DEBUG) fprintf(stderr,"prepending DSM_\n");
    strcpy(temp,var);
    sprintf(var,"DSM_%s",temp);
    if (DEBUG) fprintf(stderr,"finished prepending DSM_\n");
  }
  if (DEBUG) fprintf(stderr,"checking type of %s\n",var);
  bzero(dummyString,sizeof(dummyString));
  if (var[strlen(var)-2] == '_') {
    if (var[strlen(var)-1] == 'F') {
      sscanf(value,"%f",&dummyFloat);
      value = &dummyFloat;
    } else if (var[strlen(var)-1] == 'D') {
      sscanf(value,"%lf",&dummyDouble);
      value = &dummyDouble;
    } else if (var[strlen(var)-1] == 'L') {
      sscanf(value,"%ld",&dummyLong);
      value = &dummyLong;
    } else if (var[strlen(var)-1] == 'S') {
      sscanf(value,"%hd",&dummyShort);
      value = &dummyShort;
    } else if (var[strlen(var)-1] == 'B') {
      sscanf(value,"%hd",&dummyShort);
      dummyChar = dummyShort;
      value = &dummyChar;
    } else {
      printf("Unrecognized variable type\n");
      exit(-6);
    }
  } else if ((var[strlen(var)-2] == 'C' && var[strlen(var)-3] == '_') ||
             (var[strlen(var)-3] == 'C' && var[strlen(var)-4] == '_')) {
    sscanf(value,"%s",dummyString);
  } else {
    printf("You must provide the variable type (normally at the end of the name)\n");
    exit(-7);
  }
  stat = dsm_open();
  if (stat) {
    dsm_error_message(stat,"dsm_open");
    exit(-8);
  }
  if (index >= 0 || antennas > 0) { 
    ptr = dualrxpoint;
    stat = dsm_read(machineName,var,ptr,&timestamp);
    if (stat == 0) {
      if (DEBUG) {
	printf("Read succeeded\n");
      }
    } else {
      dsm_error_message(stat,"dsm_read");
      exit(-9);
    }
    if (antennas > 0) {
      if (DEBUG) {
	printf("Writing value for antennas: ");
      }
      for (i=1; i<ANT_LIST_SZ; i++) {
	if (requestedAntennas[i]) {
	  dualrxpoint[i-1] = dummyLong;
	  if (DEBUG) {
	    printf("%d ",i);
	  }
	}
      }
      if (DEBUG) {
	printf("on %s\n",machineName);
      }
    } else {
      if (DEBUG) {
	printf("Writing value to array index %d on %s\n",index,machineName);
      }
      dualrxpoint[index] = dummyLong;
    }
    stat = dsm_write(machineName,var,dualrxpoint);
  } else {
    do {
      if ((ptr = strstr(var,"_V")) != NULL) {
	if (isdigit(*(ptr+2))) {
	  sscanf(ptr+2,"%d",&size);
	  if (index < 0) {
	    printf("Sorry, you must specify -a or -i if it is an array variable (V%d)\n",size);
	    exit(-10);
	  }
	}
      }
    } while (ptr != NULL);
    if (strlen(dummyString) > 0) {
      printf("Writing value to string variable on %s\n",machineName);
      stat = dsm_write(machineName,var,dummyString);
    } else {
      printf("Writing value to scalar variable on %s\n",machineName);
      stat = dsm_write(machineName,var,value);
    }
  }
  if (stat == 0) {
    if (DEBUG) {
      printf("Write succeeded\n");
    }
  } else {
    dsm_error_message(stat,"dsm_write");
  }
  dsm_close();
  return(stat);
}

int present(char *search,char *token) {
  if (strstr(search,token) == NULL) {
    return(0);
  } else {
    return(1);
  }
}


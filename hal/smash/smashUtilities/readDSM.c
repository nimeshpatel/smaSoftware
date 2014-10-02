/* written by Todd Hunter */
enum {INT16=0, INT32, FLOATING, DOUBLE, STRING, STRUCTURE, INT8};
#define MAX_TOKENS 15 /* in an RM var name */
#define INITIAL_SMAPOPT_STRING_ARGUMENT "00000000"
#define MAX_STRING_LENGTH 1000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dsm.h"
#include "smapopt.h"
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)

int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *delimit);
int present(char *search,char *token);
int getNextValue(float *nextvalue, char *var, int vartype, char *machineName,
		 int rmvarindex, int rmvarindex2, int arrayDim, int array2Dim,
		 time_t *timestamp);
void usage(char *name);

void usage(char *name) {
  printf("Usage: %s -h -t <dsmVariableName> -v <dsmVariableName> -i <index(ForArrays)> -a <antennaList(ForArrays)> -m <machineName: default=hal9000> -s <structureMember>\n",name);
  printf("This program only works with numeric variables (int, float, double) and their\n");
  printf("arrays, as well as character arrays less than 100 bytes long\n");
  printf("Use -l to prevent the command from being logged to the SMAshLog\n");
  printf("Use -d for debugging messages\n");
  printf("Use -v to get the value of the variable\n");
  printf("Use -t to get the Unix timestamp of the variable\n");
  printf("Use -T to get the age of the Unix timestamp of the variable\n");
  printf("If -v is a structure, then you must include -s to select the member variable\n");
}

static int requestedAntennas[ANT_LIST_SZ];
int DEBUG = 0;
int isStructure = 0;
dsm_structure dsmStructure;

int main(int argc, char  *argv[]) {
  time_t unixtime;
  int sawMachine = 0;
  int rmvarindex = -1;
  int rmvarindex2 = -1;
  int arrayDim = 1;
  int array2Dim = 1;
  int printAll = 0;
  int printAll2 = 0;
  int vartype = -1;
  int stat,i,j;
  smapoptContext optCon; 
  char var[DSM_NAME_LENGTH];
  char svar[DSM_NAME_LENGTH];
  int typeToken, tokens, arrayToken, array2Token;
  char *token[MAX_TOKENS];
  char rmvarCopy[DSM_NAME_LENGTH];
  char temp[DSM_NAME_LENGTH];
  float value;
  char string[MAX_STRING_LENGTH];
  char *ptr, *ptr2;
  time_t timestamp = 0;
  int readTimestamp = 0;
  int index, antennas;
  float dummyFloat;
  short dummyShort;
  long dummyLong;
  double dummyDouble;
  long dualrxpoint[8];
  double doublearray[8];
  float floatarray[8];
  int size;
  char *cp, c;
  char *varString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *svarString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *tvarString = INITIAL_SMAPOPT_STRING_ARGUMENT;
  char *machine =  INITIAL_SMAPOPT_STRING_ARGUMENT;
  char machineName[50];
  int helpFlag = 0;
  int nolog = 0;

  struct smapoptOption options[] = {
    {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',"antenna argument (comma or space delimited)"},
    {"debug", 'd', SMAPOPT_ARG_NONE, &DEBUG, 'd', "print debug messages"},
    {"help", 'h', SMAPOPT_ARG_NONE, &helpFlag, 'h', "print the usage"},
    {"index",'i', SMAPOPT_ARG_INT, &rmvarindex, 'i', "which index of the DSM array variable (default=0)"},
    {"jindex",'j', SMAPOPT_ARG_INT, &rmvarindex2, 'j', "second index of the DSM array variable (applicable to 2D arrays only, default=0)"},
    {"log", 'l', SMAPOPT_ARG_NONE, &nolog, 'l', 
       "Do not write the command to the SMAshLog"},
    {"machine", 'm', SMAPOPT_ARG_STRING, &machine, 'm', 
       "Machine name to contact"},
    {"timestamp", 't', SMAPOPT_ARG_STRING, &tvarString, 't', 
       "DSM variable to read the timestamp of (case insensitive)"},
    {"Timestamp", 'T', SMAPOPT_ARG_STRING, &tvarString, 'T', 
       "DSM variable to compute the age of (case insensitive)"},
    {"variable", 'v', SMAPOPT_ARG_STRING, &varString, 'v', 
       "DSM variable to read the value of (case insensitive)"},
    {"svariable", 's', SMAPOPT_ARG_STRING, &svarString, 's', 
       "DSM structure member to read the value of (case insensitive)"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 }
  };

  strcpy(machineName,"hal9000");
  bzero(var,sizeof(var));
  bzero(svar,sizeof(svar));
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
      if (DEBUG) {
	printf("Parsed -v into %s\n",var);
      }
      break;
    case 't':
      for (j=0; j<strlen(tvarString); j++) {
	var[j] = toupper(tvarString[j]);
      }
      readTimestamp = 1;
      var[j] = 0;
      if (DEBUG) {
	printf("Parsed -t into %s\n",var);
      }
      break;
    case 'T':
      for (j=0; j<strlen(tvarString); j++) {
	var[j] = toupper(tvarString[j]);
      }
      readTimestamp = 2;
      var[j] = 0;
      if (DEBUG) {
	printf("Parsed -T into %s\n",var);
      }
      break;
    case 's':
      for (j=0; j<strlen(svarString); j++) {
	svar[j] = toupper(svarString[j]);
      }
      svar[j] = 0;
      if (DEBUG) {
	printf("Parsed -s into %s\n",svar);
      }
      break;
    case 'm':
      sawMachine = 1;
      strcpy(machineName,machine);
      break;
    case 'a':
      break;
    case 'i':
      break;
    }
  }
  smapoptFreeContext(optCon);
  if (DEBUG) {
    fprintf(stderr,"Finished parsing command-line\n");
  }
  if (sawMachine == 0) {
    if (DEBUG) {
      printf("Did not see -m, so I will assume you want to use hal9000.\n");
    }
  }
  antennas = 0;
  for (i=0; i<ANT_LIST_SZ; i++) {
    if (requestedAntennas[i]) {
      antennas++;
    }
  }
  if (strlen(var) == 0) {
    printf("You must supply a DSM variable name via -v or -t or -T\n");
    usage(argv[0]);
    exit(-4);
  }
  /*
  if (!present(var,"dsm_") && !present(var,"DSM_")) {
    if (DEBUG) fprintf(stderr,"prepending DSM_\n");
    strcpy(temp,var);
    sprintf(var,"DSM_%s",temp);
    if (DEBUG) fprintf(stderr,"finished prepending DSM_\n");
  }
  */
  if (DEBUG) fprintf(stderr,"checking type of %s\n",var);
  if ((var[strlen(var)-2] == '_') || 
      (var[strlen(var)-3] == '_' && var[strlen(var)-2] == 'C') ||
      (var[strlen(var)-4] == '_' && var[strlen(var)-3] == 'C') ||
      (var[strlen(var)-5] == '_' && var[strlen(var)-4] == 'C')) {
    if (var[strlen(var)-1] == 'F') {
      if (DEBUG) {
	printf("It's a float\n");
      }
    } else if (var[strlen(var)-1] == 'D') {
      if (DEBUG) {
	printf("It's a double\n");
      }
    } else if (var[strlen(var)-1] == 'L') {
      if (DEBUG) {
	printf("It's a long int\n");
      }
    } else if (var[strlen(var)-1] == 'S') {
      if (DEBUG) {
	printf("It's a short int\n");
      }
    } else if (var[strlen(var)-1] == 'B') {
      if (DEBUG) {
	printf("It's a character (byte)\n");
      }
    } else if (var[strlen(var)-1] == 'X') {
      if (DEBUG) {
	printf("It's a structure\n");
      }
      if (strlen(svar) == 0) {
	printf("You must supply the name of a variable within the structure with -s\n");
	usage(argv[0]);
	exit(-4);
      }
      isStructure = 1;
      if (DEBUG) fprintf(stderr,"checking type of structure member = %s\n",svar);
      if ((svar[strlen(svar)-2] == '_') || 
	  (svar[strlen(svar)-3] == '_' && svar[strlen(svar)-2] == 'C') ||
	  (svar[strlen(svar)-4] == '_' && svar[strlen(svar)-3] == 'C') ||
	  (svar[strlen(svar)-5] == '_' && svar[strlen(svar)-4] == 'C')) {
	if (svar[strlen(svar)-1] == 'F') {
	  if (DEBUG) {
	    printf("It's a float\n");
	  }
	} else if (svar[strlen(svar)-1] == 'D') {
	  if (DEBUG) {
	    printf("It's a double\n");
	  }
	} else if (svar[strlen(svar)-1] == 'L') {
	  if (DEBUG) {
	    printf("It's a long int\n");
	  }
	} else if (svar[strlen(svar)-1] == 'S') {
	  if (DEBUG) {
	    printf("It's a short int\n");
	  }
	} else if (svar[strlen(svar)-1] == 'X') {
	  if (DEBUG) {
	    printf("It's a structure\n");
	  }
	  printf("Structure within a structure is not supported.\n");
	  exit(-4);
	} else if (svar[strlen(svar)-2] == 'C') {
	  if (DEBUG) {
	    printf("It's a char array (less than 10 bytes long)\n");
	  }
	} else if (svar[strlen(svar)-3] == 'C') {
	  if (DEBUG) {
	    printf("It's a char array (from 10-99 bytes long)\n");
	  }
	} else if (svar[strlen(svar)-4] == 'C') {
	  if (DEBUG) {
	    printf("It's a char array (from 100-999 bytes long)\n");
	  }
	} else {
	  printf("unrecognized variable type\n");
	  exit(-6);
	}
      }
    } else if (var[strlen(var)-2] == 'C') {
      if (DEBUG) {
	printf("It's a char array (less than 10 bytes long)\n");
      }
    } else if (var[strlen(var)-3] == 'C') {
      if (DEBUG) {
	printf("It's a char array (from 10-99 bytes long)\n");
      }
    } else if (var[strlen(var)-4] == 'C') {
      if (DEBUG) {
	printf("It's a char array (from 100-999 bytes long)\n");
      }
    } else {
      printf("unrecognized variable type\n");
      exit(-6);
    }
  } else {
    printf("You must provide the variable type (normally at the end of the name)\n");
    exit(-7);
  }
  stat = dsm_open();
  if (stat) {
    dsm_error_message(stat,"dsm_open");
    exit(-8);
  }
  if (DEBUG){ 
    printf("DSM opened successfully\n");
  }
  if (readTimestamp == 2) {
    unixtime = time((long *)0);
  }
  if (antennas > 0) { 
    switch (var[strlen(var)-1]) {
    case 'F': case 'f':
      ptr = (char *)floatarray;
      stat = dsm_read(machineName, var, ptr, &timestamp);
      if (stat) {
	dsm_error_message(stat,"dsm_read");
	exit(-8);
      }
      if (readTimestamp == 1) {
	printf("%d\n",timestamp);
      } else if (readTimestamp == 2) {
	printf("%d\n",unixtime-timestamp);
      } else {
	printf("%f\n",*((float *)ptr));
      }
      break;
    case 'D': case 'd':
      ptr = (char *)doublearray;
      stat = dsm_read(machineName, var, ptr, &timestamp);
      if (stat) {
	dsm_error_message(stat,"dsm_read");
	exit(-8);
      }
      if (readTimestamp == 1) {
	printf("%d\n",timestamp);
      } else if (readTimestamp == 2) {
	printf("%d\n",unixtime-timestamp);
      } else {
	printf("%f\n",*((double *)ptr));
      }
      break;
    case 'L': case 'l':
      ptr = (char *)dualrxpoint;
      stat = dsm_read(machineName, var, ptr, &timestamp);
      if (stat) {
	dsm_error_message(stat,"dsm_read");
	exit(-8);
      }
      if (readTimestamp == 1) {
	printf("%d\n",timestamp);
      } else if (readTimestamp == 2) {
	printf("%d\n",unixtime-timestamp);
      } else {
	printf("%d\n",*((long *)ptr));
      }
      break;
    case 'S': case 's':
      ptr = (char *)dualrxpoint;
      stat = dsm_read(machineName, var, ptr, &timestamp);
      if (stat) {
	dsm_error_message(stat,"dsm_read");
	exit(-8);
      }
      if (readTimestamp == 1) {
	printf("%d\n",timestamp);
      } else if (readTimestamp == 2) {
	printf("%d\n",unixtime-timestamp);
      } else {
	printf("%d\n",*((short *)ptr));
      }
      break;
    case 'B': case 'b':
      ptr = (char *)dualrxpoint;
      stat = dsm_read(machineName, var, ptr, &timestamp);
      if (stat) {
	dsm_error_message(stat,"dsm_read");
	exit(-8);
      }
      if (readTimestamp == 1) {
	printf("%d\n",timestamp);
      } else if (readTimestamp == 2) {
	printf("%d\n",unixtime-timestamp);
      } else {
	printf("%d\n",*((char *)ptr));
      }
      break;
    default:
      printf("Unrecognized variable type\n");
      break;
    }
  } else {
    /* not an antenna-based variable */
    if (DEBUG) {
      fprintf(stderr,"This is not an antenna-based variable\n");
    }
    if (isStructure) {
      stat = dsm_structure_init(&dsmStructure,var);
      dsm_read(machineName, var, &dsmStructure, &timestamp);
      strcpy(var,svar);
      if (DEBUG) {
	fprintf(stderr,"Copied svar=%s to var=%s\n",svar,var);
      }
    }
    strcpy(rmvarCopy,var);
    if (DEBUG) {
      fprintf(stderr,"Copied %s to %s\n",var,rmvarCopy);
    }
    tokens = tokenize(rmvarCopy,token,"_");
    if (DEBUG) {
      fprintf(stderr,"found %d tokens\n",tokens);
    }
    typeToken = tokens-1;
    arrayToken = tokens-2;
    array2Token = tokens-3;
    if ((ptr = strstr(token[arrayToken],"V")) != NULL) {
      /* it is an array variable */
      if (DEBUG) {
	fprintf(stderr,"About to increment ptr.\n");
      }
      ptr++;
      if (DEBUG) {
	if (isdigit(ptr[0])) {
	  printf("This is an array variable of some sort.\n");
	} else {
	  printf("This is not an array variable. It simply has a _V in the name.\n");
	}
      }
      if ((ptr2=strstr(token[array2Token],"V")) != NULL) {
	ptr2++;
	if (isdigit(ptr2[0])) {
	  if (DEBUG) {
	    printf("This is a two-dimensional array variable.\n");
	  }
	  sscanf(ptr2,"%d",&array2Dim);
	  if (rmvarindex2 >= array2Dim) {
	    printf("The second dimension of the array variable is of size %d. Your index is too large!\n",array2Dim);
	    return(0);
	  } else {
	    if (rmvarindex2 < 0) {
	      printf("You must give a valid array index with -j\n");
	      printf("Otherwise I print all of the values\n");
	    } else {
	      printf("The second dimension of the array variable is of size %d. Using index %d.\n",array2Dim,rmvarindex2);
	    }
	  }
	}
      }
      if (DEBUG) {
	fprintf(stderr,"checked for array variable\n");
      }
      if (isdigit(ptr[0])) {
	sscanf(ptr,"%d",&arrayDim);
	if (rmvarindex >= arrayDim) {
	  if (DEBUG) {
	    if (array2Dim > 1) {
	      printf("The first dimension of the array variable is of size %d. Your index is too large!\n",arrayDim);
	    } else {
	      printf("This is a one-dimensional array variable of size %d. Your index is too large!\n",arrayDim);
	    }
	  }
	  return(0);
	} else {
	  if (array2Dim > 1) {
	    if (rmvarindex < 0) {
	      printf("You must give a valid array index with -i\n");
	      printf("Otherwise I print all of the values\n");
	    } else {
	      printf("The first dimension of the array variable is of size %d. Using index %d.\n",
		     arrayDim,rmvarindex);
	    }
	  } else {
	    if (rmvarindex < 0) {
	      printf("You must give a valid array index with -i\n");
	      printf("Otherwise I print all of the values\n");
	    } else {
	      if (DEBUG) {
		printf("This is a one-dimensional array variable of size %d. Using index %d.\n",
		       arrayDim,rmvarindex);
	      }
	    }
	  }
	}
      }
    }
    if (DEBUG) {
      fprintf(stderr,"Setting vartype.\n");
    }
    if (present(token[typeToken],"F")) {
      if (DEBUG) {
	printf("This is a floating point variable\n");
      }
      vartype = FLOATING;
    }
    if (present(token[typeToken],"X")) {
      if (DEBUG) {
	printf("This is a structure\n");
      }
      printf("Structure within a structure is not supported (yet)\n");
    }
    if (present(token[typeToken],"D")) {
      if (DEBUG) {
	printf("This is a double precision variable\n");
      }
      vartype = DOUBLE;
    }
    if (present(token[typeToken],"L")) {
      if (DEBUG) {
	printf("This is a 4-byte integer variable\n");
      }
      vartype = INT32;
    }
    if (present(token[typeToken],"S")) {
      if (DEBUG) {
	printf("This is a 2-byte integer variable\n");
      }
      vartype = INT16;
    }
    if (present(token[typeToken],"B")) {
      if (DEBUG) {
	printf("This is a 1-byte integer variable\n");
      }
      vartype = INT8;
    }
    if (present(token[typeToken],"C")) {
      if (DEBUG) {
	printf("This is a string variable\n");
      }
      vartype = STRING;
    }
    if (vartype == -1) {
      printf("unknown variable type\n");
      return(-1);
    }
    if (DEBUG) {
      fprintf(stderr,"Reading from scalar variable on %s\n",machineName);
    }
    
    if (arrayDim > 1) {
      if (DEBUG) {
	printf("This is an array\n");
      }
      if (rmvarindex < 0) {
	printAll = 1;
	if (DEBUG) {
	  printf("Will print all %d values.\n",arrayDim);
	}
      }
      if (rmvarindex2 < 0 && array2Dim > 1) {
	printAll2 = 1;
	if (DEBUG) {
	  printf("Will print all %d*%d=%d values.\n",arrayDim,array2Dim,arrayDim*array2Dim);
	}
      }
      if (printAll) {
	for (rmvarindex = 0; rmvarindex < arrayDim; rmvarindex++) {
	  if (printAll2) {
	    for (rmvarindex2 = 0; rmvarindex2 < array2Dim; rmvarindex2++) {
	      if (vartype == STRING) {
		printf("Reading string arrays not yet implemented\n");
		exit(9);
	      } else {
		stat = getNextValue(&value,var,vartype,machineName,rmvarindex, 
				    rmvarindex2,
				    arrayDim, array2Dim, &timestamp);
	      }
	      if (stat != 0) {
		return(stat);
	      } else {
		if (DEBUG) {
		  printf("getNextValue(%d,%d) returned with %d\n",rmvarindex,rmvarindex2,stat);
		}
	      }
	      switch (vartype) {
	      case FLOATING:
	      case DOUBLE:
	      case INT32:
	      case INT16:
	      case INT8:
		if (readTimestamp == 1) {
		  printf("%d ",timestamp);
		} else if (readTimestamp == 2) {
		  printf("%d\n",unixtime-timestamp);
		} else {
		  printf("%f ",value);
		}
		break;
	      case STRING:
		if (readTimestamp == 1) {
		  printf("%d ",timestamp);
		} else if (readTimestamp == 2) {
		  printf("%d\n",unixtime-timestamp);
		} else {
		  printf("%s ",string);
		}
		break;
	      }
	    }
	    printf("\n");
	  } else {
	    if (vartype == STRING) {
	      printf("Reading string arrays not yet implemented\n");
	      exit(10);
	    } else {
	      stat = getNextValue(&value,var,vartype,machineName,rmvarindex, 
				  rmvarindex2,
				  arrayDim, array2Dim, &timestamp);
	    }
	    if (stat != 0) {
	      return(stat);
	    } else {
	      if (DEBUG) {
		printf("getNextValue(%d) returned with %d\n",rmvarindex,stat);
	      }
	    }
	    switch (vartype) {
	    case FLOATING:
	    case DOUBLE:
	    case INT32:
	    case INT16:
	    case INT8:
	      if (readTimestamp == 1) {
		printf("%d ",timestamp);
	      } else if (readTimestamp == 2) {
		printf("%d\n",unixtime-timestamp);
	      } else {
		printf("%f ",value);
	      }
	      break;
	    case STRING:
	      if (readTimestamp == 1) {
		printf("%d ",timestamp);
	      } else if (readTimestamp == 2) {
		printf("%d\n",unixtime-timestamp);
	      } else {
		printf("%s\n",string);
	      }
	      break;
	    }
	  }
	}
	printf("\n");
      } else {
	if (vartype == STRING) {
	  printf("Reading string arrays not yet implemented\n");
	  exit(11);
	} else {
	  stat = getNextValue(&value,var,vartype,machineName,rmvarindex, rmvarindex2,
			      arrayDim, array2Dim, &timestamp);
	  if (stat != 0) {
	    return(stat);
	  } else {
	    if (DEBUG) {
	      printf("getNextValue returned with %d\n",stat);
	    }
	  }
	}
	switch (vartype) {
	case DOUBLE:
	case INT32:
	case INT16:
	case INT8:
	case FLOATING:
	  if (readTimestamp == 1) {
	    printf("%d\n",timestamp);
	  } else if (readTimestamp == 2) {
	    printf("%d\n",unixtime-timestamp);
	  } else {
	    printf("%f\n",value);
	  }
	  break;
	case STRING:
	  if (readTimestamp == 1) {
	    printf("%d\n",timestamp);
	  } else if (readTimestamp == 2) {
	    printf("%d\n",unixtime-timestamp);
	  } else {
	    printf("%s\n",string);
	  }
	  break;
	  break;
	}
      }
    } else {
      /* it is not an array variable */
      if (DEBUG) {
	printf("Switch upon vartype\n");
      }
      switch (vartype) {
      case FLOATING:
	if (DEBUG) {
	  fprintf(stderr,"Floating\n");
	}
	ptr = (char *)floatarray;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%f\n",*((float *)ptr));
	}
	break;
      case DOUBLE:
	if (DEBUG) {
	  fprintf(stderr,"double\n");
	}
	ptr = (char *)doublearray;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%f\n",*((double *)ptr));
	}
	break;
      case INT32:
	if (DEBUG) {
	  fprintf(stderr,"int32\n");
	}
	ptr = (char *)dualrxpoint;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%d\n",*((long *)ptr));
	}
	break;
      case INT16:
	if (DEBUG) {
	  fprintf(stderr,"int16, readTimestamp = %d\n",readTimestamp);
	}
	ptr = (char *)dualrxpoint;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%hd\n",*((short *)ptr));
	}
	break;
      case INT8:
	if (DEBUG) {
	  fprintf(stderr,"int8, readTimestamp = %d\n",readTimestamp);
	}
	ptr = (char *)dualrxpoint;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%hd\n",*((char *)ptr));
	}
	break;
      case STRING:
      case 'S': case 's':
	if (DEBUG) {
	  fprintf(stderr,"About to dsm_read(%s,%s)\n",machineName,var);
	}
	ptr = string;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (readTimestamp == 1) {
	  printf("%d\n",timestamp);
	} else if (readTimestamp == 2) {
	  printf("%d\n",unixtime-timestamp);
	} else {
	  printf("%s\n",ptr);
	}
	break;
      default:
	if (DEBUG) {
	  printf("default = %d\n",vartype);
	}
	ptr = string;
	if (isStructure) {
	  stat = dsm_structure_get_element(&dsmStructure,var,ptr);
	} else {
	  stat = dsm_read(machineName, var, ptr, &timestamp);
	}
	if (stat) {
	  dsm_error_message(stat,"dsm_read");
	  exit(-8);
	}
	if (var[strlen(var)-2] == 'C' || var[strlen(var)-3] == 'C' || var[strlen(var)-4] == 'C') {
	  if (readTimestamp == 1) {
	    printf("%d\n",timestamp);
	  } else if (readTimestamp == 2) {
	    printf("%d\n",unixtime-timestamp);
	  } else {
	    printf("%s\n",ptr);
	  }
	}
	break;
      }
    }
  }
  if (stat == 0) {
    if (DEBUG) {
      printf("Read succeeded\n");
    }
  } else {
    dsm_error_message(stat,"dsm_read");
    exit(-9);
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

int getNextValue(float *nextvalue, char *var, int vartype, char *machineName,
		 int rmvarindex, int rmvarindex2, int arrayDim, int array2Dim,
		 time_t *timestamp) {
  int rms;
  short sv;
  int iv;
  float fv;
  double dv;
  char cv;
  int i;
  char message[60];
  static float *fva = NULL;
  static double *dva = NULL;
  static int *iva = NULL;
  static short *sva = NULL;
  static char *cva = NULL;

  if (array2Dim > 1) {
    switch (vartype) {
    case FLOATING:
      if (fva == NULL) {
	fva = calloc(sizeof(float)*arrayDim,array2Dim);
	if (fva == NULL) {
	  printf("error on calloc fva\n"); 
	  exit(0);
	}
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, fva);
      } else {
	rms = dsm_read(machineName, var, fva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"FLOATING: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = fva[rmvarindex2*array2Dim+rmvarindex];
      break;
    case INT32:
      if (iva == NULL) {
	iva = calloc(sizeof(long)*arrayDim,array2Dim);
	if (iva == NULL) {
	  printf("error on calloc iva\n"); 
	  exit(0);
	}
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, iva);
      } else {
	rms = dsm_read(machineName, var, iva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT32: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = iva[rmvarindex*arrayDim+rmvarindex2];
      break;
    case INT16:
      if (sva == NULL) {
	sva = calloc(sizeof(short)*arrayDim, array2Dim);
	if (sva == NULL) {
	  printf("error on calloc sva\n"); 
	  exit(0);
	}
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, sva);
      } else {
	rms = dsm_read(machineName, var, sva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT16: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = sva[rmvarindex*arrayDim+rmvarindex2];
      break;
    case INT8:
      if (cva == NULL) {
	cva = calloc(sizeof(short)*arrayDim, array2Dim);
	if (cva == NULL) {
	  printf("error on calloc cva\n"); 
	  exit(0);
	}
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, cva);
      } else {
	rms = dsm_read(machineName, var, cva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT8: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = cva[rmvarindex*arrayDim + rmvarindex2];
      break;
    case DOUBLE:
      if (dva == NULL) {
	dva = calloc(sizeof(double)*arrayDim,array2Dim);
	if (dva == NULL) {
	  printf("error on calloc dva\n"); 
	  exit(0);
	}
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, dva);
      } else {
	rms = dsm_read(machineName, var, dva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"DOUBLE: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = dva[rmvarindex*arrayDim+rmvarindex2];
      break;
    }
  } else if (arrayDim > 1) {
    switch (vartype) {
    case FLOATING:
      if (fva == NULL) {
	fva = malloc(sizeof(float)*arrayDim);
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, fva);
      } else {
	rms = dsm_read(machineName, var, fva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"FLOATING: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = fva[rmvarindex];
      break;
    case INT16:
      if (sva == NULL) {
	sva = malloc(sizeof(short)*arrayDim);
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, sva);
      } else {
	rms = dsm_read(machineName, var, sva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT16: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = sva[rmvarindex];
      break;
    case INT8:
      if (cva == NULL) {
	cva = malloc(sizeof(short)*arrayDim);
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, cva);
      } else {
	rms = dsm_read(machineName, var, cva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT16: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = cva[rmvarindex];
      break;
    case INT32:
      if (iva == NULL) {
	iva = malloc(sizeof(int)*arrayDim);
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, iva);
      } else {
	rms = dsm_read(machineName, var, iva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"INT32: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = iva[rmvarindex];
      break;
    case DOUBLE:
      if (dva == NULL) {
	dva = malloc(sizeof(double)*arrayDim);
      }
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, dva);
      } else {
	rms = dsm_read(machineName, var, dva, timestamp);
      }
      if (rms != 0) {
	sprintf(message,"DOUBLE: dsm_read(%s)",var);
	dsm_error_message(rms,message);
	return(0);
      }
      *nextvalue = dva[rmvarindex];
      break;
    }
  } else {
    if (DEBUG) {
      printf("This is not an array\n");
    }
    /* it is not an array */
    switch (vartype) {
    case FLOATING:
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, &fv);
      } else {
	rms = dsm_read(machineName, var, &fv, timestamp);
      }
      *nextvalue = fv;
      break;
    case INT8:
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, &cv);
      } else {
	rms = dsm_read(machineName, var, &cv, timestamp);
      }
      *nextvalue = cv;
      break;
    case INT16:
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, &sv);
      } else {
	rms = dsm_read(machineName, var, &sv, timestamp);
      }
      *nextvalue = sv;
      break;
    case INT32:
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, &iv);
      } else {
	rms = dsm_read(machineName, var, &iv, timestamp);
      }
      *nextvalue = iv;
      break;
    case DOUBLE:
      if (isStructure) {
	rms = dsm_structure_get_element(&dsmStructure, var, &dv);
      } else {
	rms = dsm_read(machineName, var, &dv, timestamp);
      }
      *nextvalue = dv;
      break;
    }
  }
  if (rms != 0) {
    sprintf(message,"dsm_read(%s)",var);
    dsm_error_message(rms,message);
    printf("rms = %d\n",rms);
  }
  return(rms);
} /* end of main() */

int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *delimit) {
  int i;
  int non_blanks = 0;
  int tokens = 0;

  if (strlen(input) > 0) {
    for (i=0; i<strlen(input); i++) {
      if (input[i] != ' ') {
        non_blanks = 1; break;
      }
    }
    if (non_blanks == 0) return(0);
    tokenArray[tokens++] = strtok(input,delimit);
    while ((tokenArray[tokens] = strtok(NULL,delimit)) != NULL) {
      tokens++;
    }
  }
  return(tokens);
}
















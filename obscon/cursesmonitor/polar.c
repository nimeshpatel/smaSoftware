#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "rm.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0

#define ERROR -1
#define OK 0
#define R 0
#define L 1
#define H 2
#define V 3
#define OUT 0
#define IN 1
#define SOURCE 0
#define CAL 1
#define PAUSE 0
#define INTEGRATE 1

extern int firstDSMCall;
extern dsm_structure crateStatusStructure[13];

int nAntennasInProject;

typedef struct antStep {
  short polar;
  short hotload;
  char  source[100];
} antStep;

typedef struct crateStep {
  int integrating;
} crateStep;

typedef struct step {
  short *ant;        /* Points to an array of antenna states */
  short hotload;     /* Cal vane state                       */
  short source;      /* 0 = source, else calibator n         */
} step;

typedef struct stepHolder {
  char *next;
  step step;
} stepHolder;

typedef struct pattern {
  char *next;       /* Pointer to next pattern */
  char *last;       /* Pointer to previous pattern */
  char *name;       /* Points to array holding the name of the pattern */
  /* char *source; */    /* Points to name of the source */
  /* int nCalibrators; */ /* number of calibrators */
  /* char *cal[100]; */  /* Points to calibrator name pointers */
  int nAntennas;    /* Number of antennas in the pattern */
  int nSteps;       /* Number of steps in the pattern */
  step *step;       /* Points to an array of steps forming the pattern */
} pattern;

pattern *patternRoot = NULL;
pattern *pPtr;
stepHolder *stepRoot = NULL;

void getLine(FILE *theFile, char *line)
{
  int ptr = 0;
  int next = (int)'n';

  while ((!feof(theFile)) && (next != (int)'\n')) {
    next = getc(theFile);
    if ((!feof(theFile)) && (next != (int)'\n'))
      line[ptr++] = (char)next;
  }
  line[ptr] = (char)0;
}

void cleanUp(void)
{
  int i;
  step *sPtr, *sLastPtr;
  pattern *ptr, *lastPtr;

  ptr = patternRoot;
  while (ptr != NULL) {
    lastPtr = ptr;
    ptr = (pattern *)ptr->next;
    free(lastPtr->name);
    for (i = 0; i < lastPtr->nSteps; i++) {
      free(lastPtr->step[i].ant);
    }
    free(lastPtr->step);
    free(lastPtr);
  }
  patternRoot = NULL;
}

int readPattern(char *fileName, int number)
{
  int nAntSeen, startSeen, endSeen;
  int lineNumber = 1;
  char fullName[100], inLine[132], name[100],  *class;
  pattern *thisPattern;
  FILE *theFile;

  nAntSeen = startSeen = endSeen = FALSE;
  sprintf(fullName, "/otherInstances/hal/1/configFiles/%s.txt", fileName);
  theFile = fopen(fullName, "r");
  if (theFile == NULL) {
    return(ERROR);
  }
  thisPattern = (pattern *)malloc(sizeof(pattern));
  if (thisPattern == NULL) {
    perror("malloc error on thisPattern");
    return(ERROR);
  }
  thisPattern->nSteps = 0;
  thisPattern->next = thisPattern->last = NULL;
  while ((!feof(theFile)) && (!endSeen)) {
    int i;

    getLine(theFile, inLine);
    dprintf("\"%s\"\n", inLine);
    class = strtok(inLine, " \t,");
    if (class == NULL) {
      fprintf(stderr, "Error at line %d in \"%s\": \"%s\"\n",
	      lineNumber, fileName, inLine);
      fclose(theFile);
      return(ERROR);
    }
    for (i = 0; i < strlen(class); i++)
      class[i] = toupper(class[i]);
    if (startSeen) {
      if ((!strcmp(class, "L")) || (!strcmp(class, "R")) ||
	  (!strcmp(class, "H")) || (!strcmp(class, "V"))) {
	int ant = 0;
	stepHolder *thisStep;

	/* Get a new storage element to temporarily hold the step */
	thisStep = (stepHolder *)malloc(sizeof(stepHolder));
	if (thisStep == NULL) {
	  perror("thisSate");
	  exit(ERROR);
	}
	thisStep->next = NULL;
	thisStep->step.ant = (short *)malloc(thisPattern->nAntennas * sizeof(short));
	if (thisStep->step.ant == NULL) {
	  perror("thisStep->step.ant");
	  exit(ERROR);
	}
	/* First decode antenna steps */
	while (ant < thisPattern->nAntennas) {
	  if (!strcmp(class, "L"))
	    thisStep->step.ant[ant] = L;
	  else if (!strcmp(class, "R"))
	    thisStep->step.ant[ant] = R;
	  else if (!strcmp(class, "V"))
	    thisStep->step.ant[ant] = V;
	  else if (!strcmp(class, "H"))
	    thisStep->step.ant[ant] = H;
	  else {
	    fprintf(stderr, "ERROR in pattern file - ant %d step \"%s\"\n",
		    ant+1, class);
	    fclose(theFile);
	    return(ERROR);
	  }
	  class = strtok(NULL, " \t,");
	  if ((class == NULL) && (ant < thisPattern->nAntennas - 1)) {
	    fprintf(stderr, "No step specified for ant %d (of %d) in pattern file\n",
		    ant+1, thisPattern->nAntennas);
	    fclose(theFile);
	    return(ERROR);
	  } else if (class != NULL)
	    for (i = 0; i < strlen(class); i++)
	      class[i] = toupper(class[i]);
	  ant++;
	}
	/* OK, now I've got a new state - stick it on the linked list */
	if (stepRoot == NULL)
	  stepRoot = thisStep;
	else {
	  stepHolder *ptr, *lastPtr;

	  ptr = stepRoot;
	  while (ptr != NULL) {
	    lastPtr = ptr;
	    ptr = (stepHolder *)ptr->next;
	  }
	  lastPtr->next = (char *)thisStep;
	}
	thisPattern->nSteps++;
      } else if (!strcmp(class, "END")) {
	if (!nAntSeen) {
	  fprintf(stderr,
		  "In file \"%s\" END seen before number of antennas - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	/* Malloc an array to hold the steps */
	thisPattern->step = (step *)malloc(thisPattern->nSteps * sizeof(step));
	if (thisPattern->step == NULL) {
	  perror("thisPattern->step");
	  exit(ERROR);
	}
	/* Copy the linked list of steps into the step array */
	{
	  int i = 0;
	  stepHolder *ptr, *curr;

	  ptr = stepRoot;
	  while (ptr != NULL) {
	    curr = ptr;
	    ptr = (stepHolder *)ptr->next;
	    bcopy((char *)&(curr->step),
		  (char *)&(thisPattern->step[i++]), sizeof(step));
	    free(curr);
	  }
	  stepRoot = NULL;
	}
	endSeen = TRUE;
      }
    } /* if (startSeen) */
    else {
      if (!strcmp(class, "START")) {
	if (!nAntSeen) {
	  fprintf(stderr,
		  "In file \"%s\" START seen before number of antennas - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	startSeen = TRUE;
      } else if (!strcmp(class, "ANTENNAS")) {
	char *antennas;
	
	antennas = strtok(NULL, " \t,");
	if (antennas == NULL) {
	  fprintf(stderr,
		  "In file \"%s\" no text given on line with ANTENNAS token\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	sscanf(antennas, "%d", &(thisPattern->nAntennas));
	if ((thisPattern->nAntennas < 2) ||
	    (thisPattern->nAntennas > 10)) {
	  fprintf(stderr,
		  "In file \"%s\" illegal number of antennas specified: %d\n",
		  fileName, thisPattern->nAntennas);
	  return(ERROR);
	}
	nAntSeen = TRUE;
      }
    }
    if (class != NULL)
      dprintf("class: \"%s\"\n", class);
    lineNumber++;
  }
  sprintf(name, "pattern%d", number);
  thisPattern->name = (char *)malloc(strlen(name)+1);
  if (thisPattern->name == NULL) {
    perror("malloc of thisPattern->name");
    exit(ERROR);
  }
  strcpy(thisPattern->name, name);
  if (patternRoot == NULL) {
    patternRoot = thisPattern;
  } else {
    int found = FALSE;
    pattern *ptr, *lastPtr;

    ptr = patternRoot;
    while ((ptr != NULL) && (!found)) {
      if (!strcmp(thisPattern->name, ptr->name)) {
	int i;
	pattern *next, *last;
	
	free(ptr->name);
	next = (pattern *)ptr->next;
	last = (pattern *)ptr->last;
	free(ptr);
	thisPattern->last = (char *)last;
	thisPattern->next = (char *)next;
	found = TRUE;
      } else {
	lastPtr = ptr;
	ptr = (pattern *)ptr->next;
      }
    }
    if (!found) {
      lastPtr->next = (char *)thisPattern;
      thisPattern->last = (char *)lastPtr;
    }
  }
  fclose(theFile);
  return(OK);
}

/*
int readPatternOld(char *fileName)
{
  int nameSeen, nAntSeen, startSeen, endSeen, sourceSeen, calSeen;
  int lineNumber = 1;
  char fullName[100], inLine[132], *class;
  pattern *thisPattern;
  FILE *theFile;

  nameSeen = nAntSeen = startSeen = endSeen = sourceSeen = calSeen = FALSE;
  sprintf(fullName, "/otherInstances/hal/1/configFiles/%s.txt", fileName);
  theFile = fopen(fullName, "r");
  if (theFile == NULL) {
    return(ERROR);
  }
  thisPattern = (pattern *)malloc(sizeof(pattern));
  if (thisPattern == NULL) {
    perror("malloc error on thisPattern");
    return(ERROR);
  }
  thisPattern->nCalibrators = thisPattern->nSteps = 0;
  thisPattern->next = thisPattern->last = NULL;
  while ((!feof(theFile)) && (!endSeen)) {
    int i;

    getLine(theFile, inLine);
    class = strtok(inLine, " \t,");
    if (class == NULL) {
      fprintf(stderr, "Error at line %s in \"%s\": \"%s\"\n",
	      lineNumber, fileName, inLine);
      fclose(theFile);
      return(ERROR);
    }
    for (i = 0; i < strlen(class); i++)
      class[i] = toupper(class[i]);
    if (startSeen) {
      if ((!strcmp(class, "L")) || (!strcmp(class, "R")) ||
	  (!strcmp(class, "V")) || (!strcmp(class, "H")) ) {
	int ant = 0;
	stepHolder *thisStep;

	thisStep = (stepHolder *)malloc(sizeof(stepHolder));
	if (thisStep == NULL) {
	  perror("thisSate");
	  exit(ERROR);
	}
	thisStep->next = NULL;
	thisStep->step.ant = (short *)malloc(thisPattern->nAntennas * sizeof(short));
	if (thisStep->step.ant == NULL) {
	  perror("thisStep->step.ant");
	  exit(ERROR);
	}
	while (ant < thisPattern->nAntennas) {
	  if (!strcmp(class, "L"))
	    thisStep->step.ant[ant++] = L;
	  else if (!strcmp(class, "R"))
	    thisStep->step.ant[ant++] = R;
	  else if (!strcmp(class, "V"))
	    thisStep->step.ant[ant++] = V;
	  else if (!strcmp(class, "H"))
	    thisStep->step.ant[ant++] = H;
	  else {
	    fprintf(stderr, "ERROR in pattern file - ant %d step \"%s\"\n",
		    ant+1, class);
	    fclose(theFile);
	    return(ERROR);
	  }
	  class = strtok(NULL, " \t,");
	  if (class == NULL) {
	    fprintf(stderr, "No step specified for ant %d in pattern file\n",
		    ant+1);
	    fclose(theFile);
	    return(ERROR);
	  }
	  for (i = 0; i < strlen(class); i++)
	    class[i] = toupper(class[i]);
	}
	if (class == NULL) {
	  fprintf(stderr, "No state specified for ambient load in pattern file\n");
	  fclose(theFile);
	  return(ERROR);
	}
	if (!strcmp(class, "OUT"))
	  thisStep->step.hotload = OUT;
	else if (!strcmp(class, "IN"))
	  thisStep->step.hotload = IN;
	else {
	  fprintf(stderr, "Illegal state \"%s\" specified for Cal. vane\n",
		  class);
	  fclose(theFile);
	  return(ERROR);
	}
	class = strtok(NULL, " \t,");
	if (class == NULL) {
	  fprintf(stderr, "No state specified for source/cal in pattern file\n");
	  fclose(theFile);
	  return(ERROR);
	}
	for (i = 0; i < strlen(class); i++)
	  class[i] = toupper(class[i]);
	if (!strcmp(class, "SOURCE"))
	  thisStep->step.source = SOURCE;
	else if (!strcmp(class, "CAL"))
	  thisStep->step.source = CAL;
	else {
	  fprintf(stderr, "Illegal state \"%s\" specified for source/cal\n",
		  class);
	  fclose(theFile);
	  return(ERROR);
	}
	if (stepRoot == NULL)
	  stepRoot = thisStep;
	else {
	  stepHolder *ptr, *lastPtr;

	  ptr = stepRoot;
	  while (ptr != NULL) {
	    lastPtr = ptr;
	    ptr = (stepHolder *)ptr->next;
	  }
	  lastPtr->next = (char *)thisStep;
	}
	thisPattern->nSteps++;
      }
      if (!strcmp(class, "END")) {
	if (!nameSeen) {
	  fprintf(stderr,
		  "In file \"%s\" END seen before pattern name - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!nAntSeen) {
	  fprintf(stderr,
		  "In file \"%s\" END seen before number of antennas - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!sourceSeen) {
	  fprintf(stderr,
		  "In file \"%s\" END seen before SOURCE - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!calSeen) {
	  fprintf(stderr,
		  "In file \"%s\" END seen before any CAL - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	thisPattern->step = (step *)malloc(thisPattern->nSteps * sizeof(step));
	if (thisPattern->step == NULL) {
	  perror("thisPattern->step");
	  exit(ERROR);
	}
	{
	  int i = 0;
	  stepHolder *ptr, *curr;

	  ptr = stepRoot;
	  while (ptr != NULL) {
	    curr = ptr;
	    ptr = (stepHolder *)ptr->next;
	    bcopy((char *)&(curr->step),
		  (char *)&(thisPattern->step[i++]), sizeof(step));
	    free(curr);
	  }
	  stepRoot = NULL;
	}
	endSeen = TRUE;
      }
    }
    else {
      if (!strcmp(class, "START")) {
	if (!nameSeen) {
	  fprintf(stderr,
		  "In file \"%s\" START seen before pattern name - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!nAntSeen) {
	  fprintf(stderr,
		  "In file \"%s\" START seen before number of antennas - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!sourceSeen) {
	  fprintf(stderr,
		  "In file \"%s\" START seen before SOURCE - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	} else if (!calSeen) {
	  fprintf(stderr,
		  "In file \"%s\" START seen before any CAL - abort\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	startSeen = TRUE;
      } else if (!strcmp(class, "NAME")) {
	char *name;

	name = strtok(NULL, " \t,");
	if (name == NULL) {
	  fprintf(stderr,
		  "In file \"%s\" no name given on line with NAME token\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	thisPattern->name = (char *)malloc(strlen(name)+1);
	if (thisPattern->name == NULL) {
	  perror("malloc of thisPattern->name");
	  exit(ERROR);
	}
	strcpy(thisPattern->name, name);
	nameSeen = TRUE;
      } else if (!strcmp(class, "SOURCE")) {
	char *source;

	source = strtok(NULL, " \t,");
	if (source == NULL) {
	  fprintf(stderr,
		  "In file \"%s\" no name given on line with SOURCE token\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	thisPattern->source = (char *)malloc(strlen(source)+1);
	if (thisPattern->source == NULL) {
	  perror("malloc of thisPattern->source");
	  exit(ERROR);
	}
	strcpy(thisPattern->source, source);
	sourceSeen = TRUE;
      } else if (!strcmp(class, "CAL")) {
	char *cal;

	cal = strtok(NULL, " \t,");
	if (cal == NULL) {
	  fprintf(stderr,
		  "In file \"%s\" no name given on line with CAL token\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	thisPattern->cal[thisPattern->nCalibrators] = (char *)malloc(strlen(cal)+1);
	if (thisPattern->cal[thisPattern->nCalibrators] == NULL) {
	  perror("malloc of thisPattern->cal[thisPattern->nCalibrators]");
	  exit(ERROR);
	}
	strcpy(thisPattern->cal[thisPattern->nCalibrators++], cal);
	calSeen = TRUE;
      } else if (!strcmp(class, "ANTENNAS")) {
	char *antennas;
	
	antennas = strtok(NULL, " \t,");
	if (antennas == NULL) {
	  fprintf(stderr,
		  "In file \"%s\" no text given on line with ANTENNAS token\n",
		  fileName);
	  fclose(theFile);
	  return(ERROR);
	}
	sscanf(antennas, "%d", &(thisPattern->nAntennas));
	if ((thisPattern->nAntennas < 2) ||
	    (thisPattern->nAntennas > 10)) {
	  fprintf(stderr,
		  "In file \"%s\" illegal number of antennas specified: %d\n",
		  fileName, thisPattern->nAntennas);
	  return(ERROR);
	}
	nAntSeen = TRUE;
      }
    }
    dprintf("class: \"%s\"\n", class);
    lineNumber++;
  }
  if (patternRoot == NULL) {
    patternRoot = thisPattern;
  } else {
    int found = FALSE;
    pattern *ptr, *lastPtr;

    ptr = patternRoot;
    while ((ptr != NULL) && (!found)) {
      if (!strcmp(thisPattern->name, ptr->name)) {
	int i;
	pattern *next, *last;
	
	printf("We've found a pattern with the same name - replace it\n");
	for (i = 0; i < ptr->nCalibrators; i++)
	  free(ptr->cal[i]);
	free(ptr->name);
	free(ptr->source);
	next = (pattern *)ptr->next;
	last = (pattern *)ptr->last;
	free(ptr);
	thisPattern->last = (char *)last;
	thisPattern->next = (char *)next;
	found = TRUE;
      } else {
	lastPtr = ptr;
	ptr = (pattern *)ptr->next;
      }
    }
    if (!found) {
      lastPtr->next = (char *)thisPattern;
      thisPattern->last = (char *)lastPtr;
    }
  }
  fclose(theFile);
  return(OK);
}
*/

void polar(int *count)
{
  static short lastPattern;
  short shortPattern, cycle, shortStep, nPatterns;
  int s, i, j, lowestAntenna;
  int doWeCare[11], doWeCrate[14];
  char patternName[100];
  time_t curTime, timestamp;
  short pendingPattern;
  int ss;

  if ((*count % 60) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
    move(1,1);
    refresh();
  }
  i = 0;
  s = 1;
  while (i == 0) {
    sprintf(patternName, "polarPattern_%d", s);
    i = readPattern(patternName, s);
    s++;
  }
  lowestAntenna = getAntennaList(doWeCare);
  nAntennasInProject = 0;
  for (j = 1; j < 11; j++)
    if (doWeCare[j])
      nAntennasInProject++;
  getCrateList(doWeCrate);
  nPatterns = s - 1;
  move(0,0);
  curTime = time((long *)0);
  s = dsm_read("hal9000",
	       "DSM_HAL_HAL_POLAR_PATTERN_S", 
	       (char *)&shortPattern,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_HAL_HAL_POLAR_PATTERN_S");
  }
  if (shortPattern != lastPattern) {
    *count = 0;
    lastPattern = shortPattern;
  }
  s = dsm_read("hal9000",
	       "DSM_HAL_HAL_CYCLE_NUMBER_S", 
	       (char *)&cycle,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_HAL_HAL_CYCLE_NUMBER_S");
  }
  s = dsm_read("hal9000",
	       "DSM_HAL_HAL_POLAR_STEP_S", 
	       (char *)&shortStep,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_HAL_HAL_POLAR_STEP_S");
  }
  printw("Polarization - Pattern %2d  Cycle %3d  Step %2d  %s",
	 shortPattern, cycle, shortStep,
	 asctime(gmtime(&curTime)));
  if ((shortPattern > 0) && (shortPattern <= nPatterns)) {
    int patternFound;
    
    sprintf(patternName, "pattern%d", shortPattern);
    pPtr = patternRoot;
    patternFound = FALSE;
    while ((pPtr != NULL) && (!patternFound)) {
      if (!strcmp(patternName, pPtr->name))
	patternFound = TRUE;
      else
	pPtr = (pattern *)pPtr->next;
    }
    if (!patternFound) {
      fprintf(stderr, "Pattern \"%s\" unknown - aborting\n",
	      patternName);
      exit(ERROR);
    }
    if (pPtr->nAntennas != nAntennasInProject) {
      move(1,0);
      standout();
      printw("RUNNING A %d ANTENNA PATTERN WITH %d ANTENNAS IN PROJECT", pPtr->nAntennas, nAntennasInProject);
      standend();
    }
    {
      int lSThh, lSTmm;
      float lSTHours, lSTss;
      double rA;

      s=rm_read(lowestAntenna, "RM_LST_HOURS_F", &lSTHours);
      lSThh = (int)lSTHours;
      lSTmm = (int)(60.0*(lSTHours-(float)lSThh));
      lSTss = 3600.0*(lSTHours-(float)lSThh -(float)(lSTmm)/60.0);
      move(1,54);
      printw("LST ");
      printw("%02d:%02d:%02d", lSThh, lSTmm, (int)lSTss);
      s=rm_read(lowestAntenna, "RM_RA_APP_HR_D", &rA);
      printw("  HA %5.3f", lSTHours - rA);
    }
    move (2,0);
    printw("Step ");
    for (i = 1; i < 11; i++)
      if (doWeCare[i])
	printw(" %d", i);
    printw("               ");
    for (i = 0; (i < pPtr->nSteps) && (i < 20); i++) {
      int scrollOffset;

      if (shortStep > 19)
	scrollOffset = shortStep-19;
      else
	scrollOffset = 0;
      move(3+i, 0);
      if ((i+scrollOffset) == shortStep)
	printw("->");
      else
	printw("  ");
      printw("%2d ", (i+scrollOffset));
      if ((i+scrollOffset) == shortStep)
	standout();
      for (j = 0; j < pPtr->nAntennas; j++)
	if (pPtr->step[(i+scrollOffset)].ant[j] == L)
	  printw(" L");
	else if (pPtr->step[(i+scrollOffset)].ant[j] == R)
	  printw(" R");
	else if (pPtr->step[(i+scrollOffset)].ant[j] == V)
	  printw(" V");
	else if (pPtr->step[(i+scrollOffset)].ant[j] == H)
	  printw(" H");
	else
	  printw(" ?");
      standend();
      printw("      ");
      /*
      if (pPtr->step[(i+scrollOffset)].source == SOURCE) {
	int ii;

	printw("  %s", pPtr->source);
	ii = strlen(pPtr->source);
	while (ii++ < 11)
	  printw(" ");
      } else {
	int ii;

	printw("  %s", pPtr->cal[0]);
 	ii = strlen(pPtr->cal[0]);
	while (ii++ < 11)
	  printw(" ");
      }
      */
    }
  } else if (shortPattern == 0) {
    move(12,10);
    printw("polar program idle");
    *count = 0;
  } else if (shortPattern == -1) {
    move(12,10);
    printw("abort after current step");
    *count = 0;
  } else if (shortPattern == -2) {
    printw("aborting polar program");
    *count = 0;
  }
  move(2,40);
  printw("Antennas in array:");
  s = 0;
  for (i = 1; i < 11; i++)
    if (doWeCare[i]) {
      short cal;
      int rms;
      float azError, elError, trackingError;

      move(4+(s++), 40);
      printw("%d ", i);
      rms = rm_read(i, "RM_CALIBRATION_WHEEL_S", &cal);
      if (cal == 0)
	printw("Sky   ");
      else if (cal == 1)
	printw("Vane  ");
      else
	printw("Vane? ");
      rms = rm_read(i, "RM_AZ_TRACKING_ERROR_F",
		  &azError);
      rms = rm_read(i, "RM_EL_TRACKING_ERROR_F",
		  &elError);
      trackingError = sqrt(azError*azError + elError*elError);
      if (trackingError < 10.0) {
	printw("On Source                  ");
      } else {
	printw("Off Source, error %4.1f\"", trackingError);
      }
    }
  move(13,40);
  printw("Crates in array:");
  s = 0;
  for (i = 1; i < 7; i++)
    if (doWeCrate[i] || doWeCrate[i+6]) {
      short mode;
      int ss;
      double scanTime, scanLength;
      char crateName[10];
      
      move(15+(s++), 40);
      if (doWeCrate[i]) {
	sprintf(crateName, "crate%d", i);
	ss = dsm_read(crateName,
		      "CRATE_TO_HAL_X", 
		      &crateStatusStructure[i],
		      &timestamp);
	if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	  dsm_error_message(s, "dsm_read - CRATE_TO_HAL_X");
	ss = dsm_structure_get_element(&crateStatusStructure[i],
				       "MODE_S", 
				       (char *)&mode);
	if (ss != DSM_SUCCESS) {
	  if (PRINT_DSM_ERRORS)
	    dsm_error_message(s, "dsm_structure_get_element - MODE_S");
	}
	printw("%2d ", i);
	switch (mode) {
	case -1:
	  printw("paused    ");
	  break;
	case 0:
	  printw("Autocorr. ");
	  break;
	case 1:
	  printw(" ");
	  ss = dsm_structure_get_element(&crateStatusStructure[i],
					 "SCAN_TIME_D", 
					 (char *)&scanTime);
	  if (ss != DSM_SUCCESS) {
	    if (PRINT_DSM_ERRORS)
	      dsm_error_message(ss, "dsm_structure_get_element - SCAN_TIME_D");
	  }
	  ss = dsm_structure_get_element(&crateStatusStructure[i],
					 "SCAN_LENGTH_D", 
					 (char *)&scanLength);
	  if (ss != DSM_SUCCESS) {
	    if (PRINT_DSM_ERRORS)
	      dsm_error_message(ss, "dsm_structure_get_element - SCAN_LENGTH_D");
	  }
	  printw("%4.1f/%4.1f", scanTime, scanLength);
	  break;
	default:
	  printw("Unknown   ");
	}
      } else
	printw("              ");
      if (doWeCrate[i+6]) {
	sprintf(crateName, "crate%d", i+6);
	ss = dsm_read(crateName,
		      "CRATE_TO_HAL_X", 
		      &crateStatusStructure[i+6],
		      &timestamp);
	if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	  dsm_error_message(s, "dsm_read - CRATE_TO_HAL_X");
	ss = dsm_structure_get_element(&crateStatusStructure[i+6],
				       "MODE_S", 
				       (char *)&mode);
	if (ss != DSM_SUCCESS) {
	  if (PRINT_DSM_ERRORS)
	    dsm_error_message(s, "dsm_structure_get_element - MODE_S");
	}
	printw("  %2d ", i+6);
	switch (mode) {
	case -1:
	  printw("paused    ");
	  break;
	case 0:
	  printw("Autocorr. ");
	  break;
	case 1:
	  printw(" ");
	  ss = dsm_structure_get_element(&crateStatusStructure[i+6],
					 "SCAN_TIME_D", 
					 (char *)&scanTime);
	  if (ss != DSM_SUCCESS) {
	    if (PRINT_DSM_ERRORS)
	      dsm_error_message(ss, "dsm_structure_get_element - SCAN_TIME_D");
	  }
	  ss = dsm_structure_get_element(&crateStatusStructure[i+6],
					 "SCAN_LENGTH_D", 
					 (char *)&scanLength);
	  if (ss != DSM_SUCCESS) {
	    if (PRINT_DSM_ERRORS)
	      dsm_error_message(ss, "dsm_structure_get_element - SCAN_LENGTH_D");
	  }
	  printw("%4.1f/%4.1f", scanTime, scanLength);
	  break;
	default:
	  printw("Unknown   ");
	}
      }
    }
  move(22,40);
  ss = dsm_read("hal9000",
		"DSM_HAL_HAL_POLAR_PATTERN_REQUEST_S", 
		(char *)&pendingPattern,
		&timestamp);
  printw("Pending pattern = %hd\n",pendingPattern);
  cleanUp();
  move(23,79);
  refresh();
  return;
}

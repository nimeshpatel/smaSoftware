#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "rm.h"
#include "antennaWait.h"
#include "astrophys.h"
#include "smapopt.h"

static char *getAntennaWaitUnits(int units);
static void printAntennaWaitUnits(int units);
static double dishDiameter(int ant);
static double radiansToArcSeconds(double rad);
static double ComputeAcquireLimit(double beamFraction, double frequencyGHz, int ant);

int antennaWait(int rm_antlist[RM_ARRAY_SIZE], 
		int units,
                double *acquireLimitCriterion, 
		double frequencyGHz,
		char sourceName[SOURCENAMELENGTH],
                int verbose) {
  float azTrackingError, elTrackingError;
  char source[SOURCENAMELENGTH];
  char sourceWithSpaces[SOURCENAMELENGTH];
  int keepWaiting = 0;
  int commanded[RM_ARRAY_SIZE];
  int iter,i,j,ant;
  int arrived[RM_ARRAY_SIZE];
  double acquireLimitArcSeconds[RM_ARRAY_SIZE];
  int debug = 0;

  if (debug) {
    printAntennaWaitUnits(units);
  }
  switch (units) {
  case ANTENNA_WAIT_BEAM_FRACTION_INDIVIDUAL:
    for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      acquireLimitArcSeconds[ant] = 
       radiansToArcSeconds(ComputeAcquireLimit(acquireLimitCriterion[ant],frequencyGHz,ant));
    }
    break;
  case ANTENNA_WAIT_DEFAULT: /* continue on with next case */
  case ANTENNA_WAIT_BEAM_FRACTION_COMMON:
    for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      acquireLimitArcSeconds[ant] = 
       radiansToArcSeconds(ComputeAcquireLimit(acquireLimitCriterion[0],frequencyGHz,ant));
    }
    break;
  case ANTENNA_WAIT_ARC_SECONDS_INDIVIDUAL:
    for (i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      acquireLimitArcSeconds[ant] = acquireLimitCriterion[ant];
    }
    break;
  case ANTENNA_WAIT_ARC_SECONDS_COMMON:
    for (i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      acquireLimitArcSeconds[ant] = acquireLimitCriterion[0];
    }
    break;
  }
  for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
    arrived[ant] = 0;
    commanded[ant] = 0;
  }
  iter = 0;
  if (verbose) {
    fprintf(stderr,"Antennas to wait on: ");
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      fprintf(stderr," %d", ant);
    }
    fprintf(stderr,"\n");
    fprintf(stderr,"Acquire limits = ");
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      fprintf(stderr,"%.2f ",acquireLimitArcSeconds[ant]);
    }
    fprintf(stderr,"arc sec\n");
  }
  do {
    keepWaiting = 0;
    if (iter>0) {
      sleep(1);    
      if (verbose) {
	fprintf(stderr,"Still waiting on antennas: ");
	for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
	  if (arrived[ant] == 0) {
	    fprintf(stderr,"%d ",ant);
	  }
	}
	fprintf(stderr,"\n");
      }
    }
    iter++;
    for(i = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
      rm_read(ant,"RM_AZ_TRACKING_ERROR_F", &azTrackingError);
      rm_read(ant,"RM_EL_TRACKING_ERROR_F", &elTrackingError);
      rm_read(ant,"RM_SOURCE_C34", sourceWithSpaces);
      sscanf(sourceWithSpaces,"%s",source);
#define COMMANDED_COUNT 3
      if (strcmp(sourceName,INITIAL_SMAPOPT_STRING_ARGUMENT) != 0) {
	if (verbose && commanded[ant] < COMMANDED_COUNT) {
	  fprintf(stderr,"Checking to see if antenna %d has been sent to %s (it is on %s)\n",ant,sourceName,source);
	}
        if (strcmp(source,sourceName) != 0) {
	  if (verbose) {
	    fprintf(stderr,"Antenna %d has not yet been commanded to move to %s\n",ant,sourceName);
	  }
	  keepWaiting++;
	  continue;
	} else {
	  /* have we been commanded for a few seconds? */
	  commanded[ant]++;
	  if (commanded[ant] <= COMMANDED_COUNT) {
	    keepWaiting++;
	    continue;
	  }
	}
      } else {
	/* no source name has been specified */
        if (iter==1 && verbose) {
	  fprintf(stderr,"Antenna %d shows %s in RM_SOURCE_C34\n",ant,source);
	}
      }
      if (iter==1 && verbose) {
	fprintf(stderr,"Antenna %d initial error: Azim: %+f arcsec, Elev: %+f arcsec\n",
          ant, azTrackingError, elTrackingError);
      }
      if (sqrt(pow(azTrackingError,2)+pow(elTrackingError,2)) < 
          acquireLimitArcSeconds[ant]) {
	if (verbose && arrived[ant] == 0) {
  	  fprintf(stderr,"Antenna %d has arrived\n",ant);
	}
	commanded[ant]++;
	/* Thus, if -s was given, it won't wait any longer here, otherwise */
	/* it will wait, e.g. in the case that a large azoff was given and */
	/* the source name did not change. */
	if (commanded[ant] > COMMANDED_COUNT) {
	  arrived[ant] = 1;
	} else {
	  keepWaiting++;
	}
      } else {
	if (verbose && arrived[ant] == 1) {
  	  fprintf(stderr,"Antenna %d has lost acquisition\n",ant);
	}
	commanded[ant] = 0;
        arrived[ant] = 0;
	keepWaiting++;
      }
      if (verbose && iter>1 && arrived[ant]==0) {
	fprintf(stderr,"Antenna %d error: Azim: %+f arcsec, Elev: %+f arcsec\n",
          ant, azTrackingError, elTrackingError);
      }
    }
  } while (keepWaiting!=0);
  if (verbose || 1) {
    if (strcmp(sourceName,INITIAL_SMAPOPT_STRING_ARGUMENT) != 0) {
      fprintf(stderr,"All antennas have arrived on %s\n",sourceName);
    } else {
      fprintf(stderr,"All antennas have arrived\n");
    }
  }
  return(0);
}

static double radiansToArcSeconds(double rad) {
  return(rad*206264.8);
}

static double ComputeAcquireLimit(double beamFraction, double frequencyGHz, int ant) {
  double lim;
  lim = beamFraction*1.22*SPEED_OF_LIGHT/(frequencyGHz*(1.0E+9)*dishDiameter(ant));
  /*  fprintf(stderr,"Antenna %d limit = %f\n",ant,lim);*/
  return(lim);
}

static double dishDiameter(int ant) {
  switch (ant) {
  case 9:  /* CSO */
    return(10.4);
  case 10: /* JCMT */
    return(15);
  default:
    return(6.0);
  }
}

static void printAntennaWaitUnits(int units) {
  printf("%s\n",getAntennaWaitUnits(units));
}

static char *getAntennaWaitUnits(int units) {
  switch (units) {
  case ANTENNA_WAIT_DEFAULT:
    return("ANTENNA_WAIT_DEFAULT");
    break;
  case ANTENNA_WAIT_BEAM_FRACTION_INDIVIDUAL:
    return("ANTENNA_WAIT_BEAM_FRACTION_INDIVIDUAL");
    break;
  case ANTENNA_WAIT_BEAM_FRACTION_COMMON:
    return("ANTENNA_WAIT_BEAM_FRACTION_COMMON");
    break;
  case ANTENNA_WAIT_ARC_SECONDS_INDIVIDUAL:
    return("ANTENNA_WAIT_ARC_SECONDS_INDIVIDUAL");
    break;
  case ANTENNA_WAIT_ARC_SECONDS_COMMON:
    return("ANTENNA_WAIT_ARC_SECONDS_COMMON");
    break;
  default:
    return("Unknown");
  }
}

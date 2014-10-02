#define INITIAL_SMAPOPT_STRING_ARGUMENT "000000000"
#define SOURCENAMELENGTH 35
extern int antennaWait(int rm_antlist[RM_ARRAY_SIZE], 
		int acquireLimitUnits,
                double *acquireLimitCriterion,
		double frequencyGHz,
 	        char sourceName[SOURCENAMELENGTH],
                int verbose);

/* acquireLimitUnits must be one of the following */ 
enum {ANTENNA_WAIT_DEFAULT=0, 
      ANTENNA_WAIT_ARC_SECONDS_COMMON, 
      ANTENNA_WAIT_ARC_SECONDS_INDIVIDUAL, 
      ANTENNA_WAIT_BEAM_FRACTION_COMMON,
      ANTENNA_WAIT_BEAM_FRACTION_INDIVIDUAL
};


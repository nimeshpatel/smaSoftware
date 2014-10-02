enum { ALLAN_VARIANCE_PAGE_PRINT = 0,
       ALLAN_VARIANCE_PAGE_CHECK_ONLY
};
typedef struct {
  int highFreqFlags[MAX_NUMBER_ANTENNAS+1]; /* this is true if any of the two below is true */
  int highFreqShortTimescale[MAX_NUMBER_ANTENNAS+1];
  int highFreqLongTimescale[MAX_NUMBER_ANTENNAS+1];

  int lowFreqFlags[MAX_NUMBER_ANTENNAS+1]; /* this is true if any of the two below is true */
  int lowFreqShortTimescale[MAX_NUMBER_ANTENNAS+1];
  int lowFreqLongTimescale[MAX_NUMBER_ANTENNAS+1];
} ALLAN_VARIANCE_FLAGS;
void allanVariancePage(int count, int *rm_list, int mode, ALLAN_VARIANCE_FLAGS *allanVarianceFlags);

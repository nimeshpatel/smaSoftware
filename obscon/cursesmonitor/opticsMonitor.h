extern int translateBandNumberToPort(int band);
extern int checkLastNightly(int *days);
extern void printLinearLoadStatus(short shortvalue);
extern void printLinearLoadStatusChar(int antenna);
extern int printLinearLoadStatusWordStandout(int antenna);
extern int printLinearLoadStatusWord(int antenna);
extern int printLinearLoadStatusWordSky(int antenna);

#define NIGHTLY_POINTING_DAY_LIMIT 3
#define GRID_STALE_INTERVAL 35

enum { OPTICS_PAGE_PRINT = 0,
       OPTICS_PAGE_CHECK_ONLY
};

typedef struct {
  int flags[NUMANTS+1]; /* this is true if any of the below is true */
  int gridRx[NUMANTS+1];
  int combinerRx[NUMANTS+1];
  int feedOffsetMismatch[NUMANTS+1];
  int wavePlate[NUMANTS+1];
} OPTICS_FLAGS;

extern void opticsPage(int count, int *rm_list, int pageMode, OPTICS_FLAGS *opticsFlags);

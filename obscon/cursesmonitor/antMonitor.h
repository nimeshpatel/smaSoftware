#define SOURCE_CHAR_LEN 34
enum {ANTENNA_PAGE_CHECK_ONLY = 0, ANTENNA_PAGE_PRINT};
typedef struct {
  int flags;
  int timeDifferential;
  int azBrakeManualRelease;
  int estopBypass;
  int wackoOffsets;
  int driveTimeoutDisabled;
} ANTENNA_FLAGS;
extern int antDisplay(int ant, int icount, int pageMode, ANTENNA_FLAGS *flags);

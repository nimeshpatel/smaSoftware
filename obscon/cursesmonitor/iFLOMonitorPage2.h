#define IFLO_PAGE_PRINT 0
#define IFLO_PAGE_CHECK_ONLY 1
extern int printVoltageAsThreeCharLogicState(float dFloat);
extern int doVoltageAsThreeCharLogicState(float dFloat, int print);
typedef struct {
  int flags1[MAX_NUMBER_ANTENNAS+1];
  int xmit1power[MAX_NUMBER_ANTENNAS+1];
  int xmit1OpAlarm[MAX_NUMBER_ANTENNAS+1];
  int xmit1TempAlarm[MAX_NUMBER_ANTENNAS+1];
  int quad1[MAX_NUMBER_ANTENNAS+1];
  int flags2[MAX_NUMBER_ANTENNAS+1];
  int xmit2power[MAX_NUMBER_ANTENNAS+1];
  int xmit2OpAlarm[MAX_NUMBER_ANTENNAS+1];
  int xmit2TempAlarm[MAX_NUMBER_ANTENNAS+1];
  int quad2[MAX_NUMBER_ANTENNAS+1];
  int mrgAlarm[MAX_NUMBER_ANTENNAS+1];
  int mrgRfPower[MAX_NUMBER_ANTENNAS+1];
  int power1_109_200[MAX_NUMBER_ANTENNAS+1];
  int power2_109_200[MAX_NUMBER_ANTENNAS+1];
} IFLO_FLAGS;
extern void iFLODisplayPage2(int count, int *antlist, int pageMode, IFLO_FLAGS *flags);

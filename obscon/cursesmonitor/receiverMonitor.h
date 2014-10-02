#define HEATEDLOAD_STALE_HOURS 20
#define IVSWEEP_HOURS_HIGHLIGHT 24
#define MIN_IFPOWER_MUWATT 0.10 /* 0.12 = -39dBm */
#define SIS0_MIXER_CURRENT_MIN 20
#define SIS0_MIXER_CURRENT_MAX 35
#define ASIAA_MIXER_CURRENT_MIN 20
#define ASIAA_MIXER_CURRENT_MAX 50
/* guesses */
#define JCMT_MIXER_CURRENT_MAX 20
#define CSO_MIXER_CURRENT_MAX 20
#define JCMT_MIXER_CURRENT_MIN 8
#define CSO_MIXER_CURRENT_MIN  8
#define LO_BOARD_ADC_SATURATED 5.0599  /* volts */

extern int findReceiverNumberLow(int ant);
extern float sis0MixerCurrentMin[MAX_NUMBER_ANTENNAS+1][4];
extern float sis0MixerCurrentMax[MAX_NUMBER_ANTENNAS+1][4];

extern float sis0MixerBiasMin[MAX_NUMBER_ANTENNAS+1][4];
extern float sis0MixerBiasMax[MAX_NUMBER_ANTENNAS+1][4];

#define HI_LO_FREQ_CUTOFF_GHZ 600.0
#define HI_LO_FREQ_CUTOFF_HZ (HI_LO_FREQ_CUTOFF_GHZ*1.0e9)
enum { RECEIVER_PAGE_PRINT = 0,
       RECEIVER_PAGE_CHECK_ONLY
};

typedef struct {
  int flags[MAX_NUMBER_ANTENNAS+1]; /* this is true if any of the below is true */
  int mixerBfield[MAX_NUMBER_ANTENNAS+1];
  int mixerCurrent[MAX_NUMBER_ANTENNAS+1];
  int mixerVoltage[MAX_NUMBER_ANTENNAS+1];
  int mixerSelect[MAX_NUMBER_ANTENNAS+1];
  int yigSelect[MAX_NUMBER_ANTENNAS+1];
  int gridSelect[MAX_NUMBER_ANTENNAS+1];
  int activePLL[MAX_NUMBER_ANTENNAS+1];
  int ivsweepAge[MAX_NUMBER_ANTENNAS+1];
  int ifpower[MAX_NUMBER_ANTENNAS+1];
} RECEIVER_FLAGS;

extern void receiverMonitor(int count, int *antlist, int pageMode, RECEIVER_FLAGS *flags);

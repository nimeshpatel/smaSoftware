extern int translateBandNumberToGHz(int band);
extern float sis1MixerCurrentMin[MAX_NUMBER_ANTENNAS+1][4];
extern float sis1MixerCurrentMax[MAX_NUMBER_ANTENNAS+1][4];
extern float sis1MixerVoltageMin[MAX_NUMBER_ANTENNAS+1][4];
extern float sis1MixerVoltageMax[MAX_NUMBER_ANTENNAS+1][4];
extern void receiverMonitorHighFreq(int count, int *antlist, int pageMode, RECEIVER_FLAGS *flags);

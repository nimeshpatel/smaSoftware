#define INTG_N_ANTENNAS 11
#define INTG_N_CHUNKS 5
#define INTG_SUCCESS 1
#define INTG_FAILURE 0
#define INGR_NOSUCH_COMMAND 1
#define INTG_BAD_CONFIG 2
#define INTG_NOT_IMPLEMENTED 3
#define INTG_NO_CBOARDS 4
#define INTG_SYSTEM_ERROR 4
#define INTG_CB_IOCTL 5
#define INTG_MEZZANINE_OPEN 6
#define INTG_MZ_IOCTL 7
#define INTG_NOT_INITIALIZED 8
#define INTG_DSP_TASK 9
#define INTG_ZERO_MEMORY 10
#define INTG_CONFIG_ERROR 11
#define INTG_DESCRAMBLE 12
#define INTG_BAD_CONF_FILE 13
#define INTG_RX_IOCTL 14
#define INTG_LOOP_RUNNING 15
#define INTG_LOOP_INACTIVE 16
#define INTG_CANT_START_TH 17

struct intgStatus {
	int status;
	int reason;
};
typedef struct intgStatus intgStatus;
bool_t xdr_intgStatus();


struct intgTiming {
	int status;
	int reason;
	double midPoint;
};
typedef struct intgTiming intgTiming;
bool_t xdr_intgTiming();

#define INTG__ONOFFSCAN 2
#define INTG__CROSSCORRELATE 1
#define INTG__AUTOCORRELATE 0

struct INTG__IF_Chunk_struct {
	int antenna;
	int RFreceiver;
	int rxNumber;
	int rXFD;
	int rxBoardHalf;
	int Chunk;
	double fifo_delay;
	double attenuation;
	char cbBoardInput;
	char cbChipInput;
};
typedef struct INTG__IF_Chunk_struct INTG__IF_Chunk_struct;
bool_t xdr_INTG__IF_Chunk_struct();


struct INTG__test_struct {
	INTG__IF_Chunk_struct Chunk1;
	INTG__IF_Chunk_struct Chunk2;
	int cBvMEId;
	int cbSlot;
	int num_integrations;
	double integrationTime;
	double scanTime;
	int chipsPerChunk;
	int switching;
	int FIFODelayEnable;
	int softDelayEnable;
	int antenna;
	int scanType;
	int restartDSPs;
	int prdsp_xbar_load;
	int verbose;
	int crate_number;
	char cr_configfiles[80];
	char crate_controller[80];
	char crate_name[80];
	char dds_computer[80];
	char display_server[80];
	char dsp_executable[80];
	char observatory[80];
	char storage_server[80];
};
typedef struct INTG__test_struct INTG__test_struct;
bool_t xdr_INTG__test_struct();


struct intgParameters {
	double time;
	double scanNumber;
	char sourceName[80];
};
typedef struct intgParameters intgParameters;
bool_t xdr_intgParameters();


struct intgCommand {
	int command;
};
typedef struct intgCommand intgCommand;
bool_t xdr_intgCommand();


struct intgCommand1IP {
	int command;
	int param;
};
typedef struct intgCommand1IP intgCommand1IP;
bool_t xdr_intgCommand1IP();


struct intgDumpFile {
	char fileName[80];
	int fileType;
};
typedef struct intgDumpFile intgDumpFile;
bool_t xdr_intgDumpFile();

#define DUMP_FILE 1
#define CLASS_FILE 2
#define INTG_INITIALIZE 1
#define INTG_INTEGRATE 2
#define INTG_DISPLAY_ON 3
#define INTG_DISPLAY_OFF 4
#define INTG_ABORT 5
#define INTG_REBOOT 6
#define INTG_DEBUG_ON 7
#define INTG_DEBUG_OFF 8
#define INTG_SHUTDOWN 9
#define INTG_START_LOOP 10
#define INTG_STOP_LOOP 11
#define INTG_POS_MAN 12
#define INTG_POS_AUTO 13
#define INTG_POS_ON 14
#define INTG_POS_OFF 15
#define INTG_DO_DIV 16
#define INTG_NO_DIV 17
#define INTG_NO_PLOT_LAGS 18
#define INTG_NO_PLOT_FFT 19
#define INTG_PLOT_BOTH 20
#define INTG_DO_STORE 21
#define INTG_DONT_STORE 22
#define INTG_DO_FUDGE_PHASE 23
#define INTG_DONT_FUDGE_PHASE 24
#define INTG_DO_IDL 25
#define INTG_DONT_IDL 26
#define INTG_SMASH_WAIT 27
#define INTG_IGNORE_DERS 28
#define INTG_DONT_IGNORE_DERS 29
#define INTG_C2DC_ROT_ON 30
#define INTG_C2DC_ROT_OFF 31
#define INTG_DO_EXTRA_IDL 32
#define INTG_DONT_EXTRA_IDL 33
#define INTG_5POINT_ON 34
#define INTG_5POINT_OFF 35
#define INTG_PAUSE_ON 36
#define INTG_PAUSE_OFF 37
#define INTG_DEC_COUNT 38
#define INTG_NOISE_DATA 39
#define INTG_NOT_NOISE 40
#define INTG_PAUSE_NEXT 41
#define INTG_SET_PHASES 1
#define INTG_WALSH_TOO_LATE 100

struct intgWalshPattern {
	struct {
		u_int step_len;
		int *step_val;
	} step;
};
typedef struct intgWalshPattern intgWalshPattern;
bool_t xdr_intgWalshPattern();


struct intgWalshPackage {
	struct {
		u_int pattern_len;
		intgWalshPattern *pattern_val;
	} pattern;
	int interleave;
	int walshCycleTime;
	int startYear;
	int startDay;
	int startHour;
	int startMin;
	int startSec;
	int startuSec;
	int syncError;
	int skipCycles;
};
typedef struct intgWalshPackage intgWalshPackage;
bool_t xdr_intgWalshPackage();


struct intgWalshResponse {
	int status;
	int failureCode;
	double startWalsh;
};
typedef struct intgWalshResponse intgWalshResponse;
bool_t xdr_intgWalshResponse();

#define INTG_ANTENNA_BAD 0
#define INTG_ANTENNA_GOOD 1

struct intgDERSMessage {
	int status[INTG_N_ANTENNAS];
	int online[INTG_N_ANTENNAS];
	long timeStamp[INTG_N_ANTENNAS];
};
typedef struct intgDERSMessage intgDERSMessage;
bool_t xdr_intgDERSMessage();


struct chunkDescription {
	int nChannels;
};
typedef struct chunkDescription chunkDescription;
bool_t xdr_chunkDescription();


struct blockDescription {
	chunkDescription chunk[4];
};
typedef struct blockDescription blockDescription;
bool_t xdr_blockDescription();


struct correlatorDescription {
	int nActiveCrates;
	int nBaselines;
	int nSidebands;
	blockDescription block[6];
};
typedef struct correlatorDescription correlatorDescription;
bool_t xdr_correlatorDescription();

#define INTG_READ_ATTENUATORS 1
#define INTG_OPTIMIZE_ATTENUATORS 2
#define INTG_ADD_ATTENUATION 3
#define INTG_SUBTRACT_ATTENUATION 4
#define INTG_SET_ATTENUATION 5

struct attenuationRequest {
	int command;
	int requestedAntenna;
	int requestedChunk;
	double newValue;
};
typedef struct attenuationRequest attenuationRequest;
bool_t xdr_attenuationRequest();


struct attenuation {
	double upperValues[INTG_N_CHUNKS];
	double lowerValues[INTG_N_CHUNKS];
};
typedef struct attenuation attenuation;
bool_t xdr_attenuation();


struct attenuationResults {
	int status[INTG_N_ANTENNAS];
	int exists[INTG_N_ANTENNAS];
	attenuation values[INTG_N_ANTENNAS];
};
typedef struct attenuationResults attenuationResults;
bool_t xdr_attenuationResults();


struct intgC2DCParameters {
	double reference_time;
	double sin_coefficient[INTG_N_ANTENNAS];
	double cos_coefficient[INTG_N_ANTENNAS];
	double const_coefficient[INTG_N_ANTENNAS];
};
typedef struct intgC2DCParameters intgC2DCParameters;
bool_t xdr_intgC2DCParameters();


struct intgChannelCounts {
	int channelCount;
	int chunk[INTG_N_CHUNKS];
};
typedef struct intgChannelCounts intgChannelCounts;
bool_t xdr_intgChannelCounts();


struct intgConfiguration {
	int chipsPerChunkRx1[INTG_N_CHUNKS];
	int chipsPerChunkRx2[INTG_N_CHUNKS];
};
typedef struct intgConfiguration intgConfiguration;
bool_t xdr_intgConfiguration();


#define INTGPROG ((u_long)0x20000105)
#define INTGVERS ((u_long)1)
#define INTGCONFIGURE ((u_long)1)
extern intgStatus *intgconfigure_1();
#define INTGSETPARAMS ((u_long)2)
extern intgStatus *intgsetparams_1();
#define INTGCOMMAND ((u_long)3)
extern intgStatus *intgcommand_1();
#define INTGNEWFILE ((u_long)4)
extern intgStatus *intgnewfile_1();
#define INTGATTENADJUST ((u_long)5)
extern attenuationResults *intgattenadjust_1();
#define INTGSYNCWALSH ((u_long)6)
extern intgWalshResponse *intgsyncwalsh_1();
#define INTGDERSMESSAGE ((u_long)7)
extern intgStatus *intgdersmessage_1();
#define INTGREPORTCONFIGURATION ((u_long)8)
extern correlatorDescription *intgreportconfiguration_1();
#define INTGC2DCRATES ((u_long)9)
extern intgTiming *intgc2dcrates_1();
#define INTGCOMMAND1IP ((u_long)10)
extern intgStatus *intgcommand1ip_1();
#define INTGSETCHANNELS ((u_long)11)
extern intgStatus *intgsetchannels_1();
#define INTGGETCONFIG ((u_long)12)
extern intgConfiguration *intggetconfig_1();


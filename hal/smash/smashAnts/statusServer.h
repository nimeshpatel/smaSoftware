#define SS_IGNORE 1
#define SS_DONT_IGNORE 2
#define SS_USE_FOR_COUNTING 3
#define SS_DONT_USE_FOR_COUNTING 4
#define SS_SET_ONLINE 5
#define SS_SET_ONLINE_GOOD 6
#define SS_SET_OFFLINE 7
#define SS_SET_OFFLINE_BAD 8
#define SS_SPOIL_SCAN 9
#define SS_SUCCESS 0
#define SS_FAILURE -1

struct requestCodes {
	double utsec;
	double interval;
};
typedef struct requestCodes requestCodes;
bool_t xdr_requestCodes();


struct antDataDef {
	int isvalid[2];
	int antennaStatus;
	int trackStatus;
	int rx[2];
	char sourceName[34];
	double ra_j2000;
	double dec_j2000;
	double hour_angle;
	double rightascension;
	double declination;
	double velocity;
	int velocity_type;
	int sol_sys_flag;
	double planet_size;
	double tjd;
	double utc;
	double dut;
	double lst;
	double actual_az;
	double actual_el;
	double azoff;
	double eloff;
	double az_tracking_error;
	double el_tracking_error;
	double delta_ra;
	double delta_dec;
	double tsys;
	double tsys_rx2;
	int project_id;
	int obstype;
	long utsec;
	double tiltx;
	double tilty;
	double pmdaz;
	double pmdel;
	double refraction;
	double chopper_x;
	double chopper_y;
	double chopper_z;
	double chopper_angle;
	double ambient_load_temperature;
	float N;
	float Tamb;
	float pressure;
	float humid;
	float windSpeed;
	float windDir;
};
typedef struct antDataDef antDataDef;
bool_t xdr_antDataDef();


struct chunkDef {
	double centerfreq;
};
typedef struct chunkDef chunkDef;
bool_t xdr_chunkDef();


struct blockDef {
	struct chunkDef chunk[4];
};
typedef struct blockDef blockDef;
bool_t xdr_blockDef();


struct sidebandDef {
	struct blockDef block[6];
};
typedef struct sidebandDef sidebandDef;
bool_t xdr_sidebandDef();


struct receiverDef {
	struct sidebandDef sideband[2];
};
typedef struct receiverDef receiverDef;
bool_t xdr_receiverDef();


struct frequenciesDef {
	struct receiverDef receiver[2];
};
typedef struct frequenciesDef frequenciesDef;
bool_t xdr_frequenciesDef();


struct lODataDef {
	int loStatus[2];
	int sideband[2];
	double skyFrequency[2];
	double frequencyOffset[2];
	double velocityOffset[2];
	double restFrequency[2];
	int pLLMultiple[2];
	int pLLSideband[2];
	double mRGFrequency[2];
	int vType;
	double vCatalog;
	double vEarth;
	double vNoEarth;
	double vRadial;
	struct frequenciesDef frequencies;
};
typedef struct lODataDef lODataDef;
bool_t xdr_lODataDef();


struct dDSDataDef {
	int ddsStatus;
	double ha;
	double ra;
	double dec;
	double lst;
	double trackingFrequency[2];
	double x[11];
	double y[11];
	double z[11];
	double u[11];
	double v[11];
	double w[11];
	double UT1MinusUTC;
	double dayFraction;
};
typedef struct dDSDataDef dDSDataDef;
bool_t xdr_dDSDataDef();


struct info {
	int antlist[11];
	struct antDataDef antavg[11];
	struct lODataDef loData;
	struct dDSDataDef DDSdata;
	float tau225;
};
typedef struct info info;
bool_t xdr_info();


struct elHistory {
	float ant1[300];
	float ant2[300];
	float ant3[300];
	float ant4[300];
	float ant5[300];
	float ant6[300];
	float ant7[300];
	float ant8[300];
};
typedef struct elHistory elHistory;
bool_t xdr_elHistory();


struct azHistory {
	float ant1[300];
	float ant2[300];
	float ant3[300];
	float ant4[300];
	float ant5[300];
	float ant6[300];
	float ant7[300];
	float ant8[300];
};
typedef struct azHistory azHistory;
bool_t xdr_azHistory();


struct tsysHistory {
	float ant1[300];
	float ant2[300];
	float ant3[300];
	float ant4[300];
	float ant5[300];
	float ant6[300];
	float ant7[300];
	float ant8[300];
	float ant1_rx2[300];
	float ant2_rx2[300];
	float ant3_rx2[300];
	float ant4_rx2[300];
	float ant5_rx2[300];
	float ant6_rx2[300];
	float ant7_rx2[300];
	float ant8_rx2[300];
};
typedef struct tsysHistory tsysHistory;
bool_t xdr_tsysHistory();


struct sourceName {
	char name[35];
};
typedef struct sourceName sourceName;
bool_t xdr_sourceName();


struct sourceHistory {
	struct sourceName ant1[300];
	struct sourceName ant2[300];
	struct sourceName ant3[300];
	struct sourceName ant4[300];
	struct sourceName ant5[300];
	struct sourceName ant6[300];
	struct sourceName ant7[300];
	struct sourceName ant8[300];
};
typedef struct sourceHistory sourceHistory;
bool_t xdr_sourceHistory();


struct history {
	u_int combiner[11];
	u_int m3Door[11];
	float ambientLoadTemperature[11];
	float temperature;
	float windSpeed;
	float windDirection;
	float humidity;
	float pressure;
	u_int weatherTimestamp;
	u_long time[300];
	u_int scanNo[300];
	double dTime[300];
	struct elHistory el;
	struct azHistory az;
	struct tsysHistory tsys;
	struct sourceHistory source;
};
typedef struct history history;
bool_t xdr_history();


struct historyRequest {
	int command;
};
typedef struct historyRequest historyRequest;
bool_t xdr_historyRequest();


struct ddsCoordInfo {
	double ra_disp;
	double dec_disp;
	double ra_cat_disp;
	double dec_cat_disp;
};
typedef struct ddsCoordInfo ddsCoordInfo;
bool_t xdr_ddsCoordInfo();


struct ccdHeader {
	char filename[100];
	char sourcename[34];
	char sptype[10];
	double tjd_disp;
	double lst_disp;
	double utc_disp;
	double ra_disp;
	double dec_disp;
	double ra_cat_disp;
	double dec_cat_disp;
	double az_disp;
	double el_disp;
	double azoff;
	double eloff;
	double az_actual;
	double el_actual;
	float magnitude;
	float pmdaz;
	float pmdel;
	float pressure;
	float temperature;
	float humidity;
	float windspeed;
	float winddirection;
	short bias;
	short gain;
	short integration;
	double tiltx;
	double tilty;
	int status;
};
typedef struct ccdHeader ccdHeader;
bool_t xdr_ccdHeader();


struct antennaQuery {
	int antennaNumber;
};
typedef struct antennaQuery antennaQuery;
bool_t xdr_antennaQuery();


struct sSStatus {
	int returnCode;
};
typedef struct sSStatus sSStatus;
bool_t xdr_sSStatus();


struct antennaList {
	int onLine;
	int bad;
	int ants[11];
};
typedef struct antennaList antennaList;
bool_t xdr_antennaList();


struct flagChange {
	int antennas[11];
	int global;
	int condition;
	float value;
	int action;
};
typedef struct flagChange flagChange;
bool_t xdr_flagChange();


#define STATUSSERVERPROG ((u_long)0x20001001)
#define STATUSSERVERVERS ((u_long)1)
#define STATUSREQUEST ((u_long)1)
extern info *statusrequest_1();
#define ANTHISTORY ((u_long)2)
extern history *anthistory_1();
#define CCDHEADER ((u_long)5)
extern ccdHeader *ccdheader_1();
#define DDSCOORDINFO ((u_long)6)
extern ddsCoordInfo *ddscoordinfo_1();
#define OFFLINE ((u_long)7)
extern sSStatus *offline_1();
#define CHANGEFLAGGING ((u_long)8)
extern sSStatus *changeflagging_1();


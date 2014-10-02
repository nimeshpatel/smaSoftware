#define N_ANTENNAS 11
#define N_RECEIVERS 2

const SS_IGNORE                = 1;
const SS_DONT_IGNORE           = 2;
const SS_USE_FOR_COUNTING      = 3;
const SS_DONT_USE_FOR_COUNTING = 4;
const SS_SET_ONLINE            = 5;
const SS_SET_ONLINE_GOOD       = 6;
const SS_SET_OFFLINE           = 7;
const SS_SET_OFFLINE_BAD       = 8;
const SS_SPOIL_SCAN            = 9;

const SS_SUCCESS  =  0;
const SS_FAILURE  = -1;

struct requestCodes {  /* Structure sent by dataCatcher to get header info     */
  double utsec;        /* The midpoint time of the scan which just completed   */
  double interval;     /* The duration of the scan, in seconds                 */
};                                                                             

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

struct chunkDef {
  double centerfreq;
};
struct blockDef {
  struct chunkDef chunk[4];
};
struct sidebandDef {
  struct blockDef block[6];
};
struct receiverDef {
  struct sidebandDef sideband[2];
};
struct frequenciesDef {
  struct receiverDef receiver[2];
};

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

struct dDSDataDef {
  int ddsStatus;
  double ha;
  double ra;
  double dec;
  double lst;
  double trackingFrequency[N_RECEIVERS];
  double x[N_ANTENNAS];
  double y[N_ANTENNAS];
  double z[N_ANTENNAS];
  double u[N_ANTENNAS];
  double v[N_ANTENNAS];
  double w[N_ANTENNAS];
  double UT1MinusUTC;
  double dayFraction;
}; 

struct info {
  int    antlist[N_ANTENNAS];
  struct antDataDef antavg[N_ANTENNAS];
  struct lODataDef loData;
  struct dDSDataDef DDSdata;
  float tau225;
};

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
 
struct sourceName {
  char name[35];
};
 
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
 
struct history {
  unsigned int combiner[11];
  unsigned int m3Door[11];
  float ambientLoadTemperature[11];
  float temperature;
  float windSpeed;
  float windDirection;
  float humidity;
  float pressure;
  unsigned int weatherTimestamp;
  unsigned long time[300];
  unsigned int scanNo[300];
  double dTime[300];
  struct elHistory el;
  struct azHistory az;
  struct tsysHistory tsys;
  struct sourceHistory source;
};

struct historyRequest {
  int command;
};

struct ddsCoordInfo {
  double ra_disp;
  double dec_disp;
  double ra_cat_disp;
  double dec_cat_disp;
};

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
 
struct antennaQuery {
  int antennaNumber;
};

struct sSStatus {
  int returnCode;
};

struct antennaList {
  int onLine;            /* set to 0 to flag antennas offline, 1 for online */
  int bad;               /* set to 1 to say data should be flagged as bad */
  int ants[N_ANTENNAS];  
};

struct flagChange {
  int antennas[11]; /* Which antennas should be effected                     */
  int global;
  int condition;    /* Which flag to modify                                  */
  float value;      /* Some conditions (like tracking error) need a value    */
  int action;
};

program STATUSSERVERPROG {
  version STATUSSERVERVERS {
    info           STATUSREQUEST(requestCodes) = 1;
    history        ANTHISTORY(historyRequest)  = 2;
    ccdHeader      CCDHEADER(antennaQuery)     = 5;
    ddsCoordInfo   DDSCOORDINFO(antennaQuery)  = 6;
    sSStatus       OFFLINE(antennaList)        = 7;
    sSStatus       CHANGEFLAGGING(flagChange)  = 8;
  } = 1;      /* Program version number */ 
} = 0x20001001; /* RPC Program number */

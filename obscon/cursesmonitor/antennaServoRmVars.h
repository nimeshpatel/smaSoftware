unsigned char padID;
unsigned char irigLock;
unsigned char palmCodeDate[10];
float padAzOffset;
float lowerLimit, upperLimit, cwLimit, ccwLimit;
unsigned char chkElCollisionLimit, azRockerBits;
int azTachDivisor, elTachDivisor;
float azIntegralGain, azProportionalGain, azDerivativeGain, azTorqueBias;
float elIntegralGain, elProportionalGain, elDerivativeGain;
float tachAzVel, tachElVel;
float azRockerCWLimit, azRockerCCWLimit;
float elMotCurrent, azMot1Current, azMot2Current;
float elMotTemp, azMot1Temp, azMot2Temp;
unsigned char scbEPROMVersion, scbSRAMVersion;
int scbRestarts, ttTimeoutCount, nakCount;
int scbFaultWord;
unsigned char scbStatus, fault, driveState, m3Cmd, m3State;
char azCmd, elCmd;
unsigned char azDrvState, elDrvState;
int unixTime, tracktimestamp, msecCmd, msecAccept, msec;
float sunAz, sunEl;
float az, el, azVel, elVel, cmdAz, cmdEl;
float encAz, encEl, limAz, limEl;
float azTrErrorArcSec, elTrErrorArcSec, totTrErrorArcSec, trErrorBoxtime;
unsigned char azEncType, elEncType;
int azEncoderOffset, elEncoderOffset;
short int azEncoderReversed, elEncoderReversed;
int elLimEncZero, azLimEncZero;

typedef struct rm_vars {
	void *a;
	char *name;
} RV;


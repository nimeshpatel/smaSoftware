/*     logTilts.c                                                           */
/*     First version Feb. 13, 2000                                          */
/*                                                                          */
/*   This is a very simple program which reads the tilt meter via A/Ds, and */
/* writes values into reflective memory.   Boxcar smmothing is done here,   */
/* as is the conversion from volts to arc seconds, including corrections    */
/* for nonorthogonality and rotation of the actual tilt meters relative to  */
/* the aft-forward and left-right directions.                               */
/*                                                                          */
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include "rm.h"
#include "stderrUtilities.h"

#define TRUE   (1)
#define FALSE  (0)
#define ERROR (-1)

int debug = FALSE;
#define dprintf if (debug) printf

#define DEGREES_TO_RADIANS (M_PI/180.0)

#define N_TILTS 4

#define AF_UPPER_CHANNEL (0)
#define LR_UPPER_CHANNEL (1)
#define AF_LOWER_CHANNEL (2)
#define LR_LOWER_CHANNEL (3)
#define AF_EXTRA_CHANNEL (16)
#define LR_EXTRA_CHANNEL (17)
#define TEMPERATURE_CHANNEL (18)

/* Default values for Config. file parameters: */

int deadTiltMeters = FALSE; /* Use to indicate that the tilt meters are nonfunctional */
int useXYCom = TRUE;        /* Use a Xycom board for A/D, otherwise use an IP         */
int readPatchPanel = FALSE; /* Read the XYcom patchpanel inputs                       */
int extraChannels = TRUE;   /* Read 4 extra A/D channels starting at channel 16       */
int readPadTilts = FALSE;   /* Read 13 extra XYCOM channels for pad tilt info.        */
double boxcarTime = 1.0;    /* Width on the averaging boxcar in seconds               */
double sampleRate = 10.0;   /* Sampling rate in Hz                                    */
double nominalV2A = 103.0;  /* Conversion factor for ideal component arcsec/volt      */
double aFUpperCor = 1.0;    /* Correction to NOMINAL_V2A for upper tilt A-F           */
double lRUpperCor = 1.0;    /* Correction to NOMINAL_V2A for upper tilt L-R           */
double aFLowerCor = 1.0;    /* Correction to NOMINAL_V2A for lower tilt A-F           */
double lRLowerCor = 1.0;    /* Correction to NOMINAL_V2A for lower tilt L-R           */
double aFExtraCor = 1.0;    /* Correction to NOMINAL_V2A for extra tilt A-F           */
double lRExtraCor = 1.0;    /* Correction to NOMINAL_V2A for extra tilt L-R           */
double aFUpperVOff = 0.0;   /* Offset for upper tilt A-F (in volts)                   */
double lRUpperVOff = 0.0;   /* Offset for upper tilt L-R (in volts)                   */
double aFLowerVOff = 0.0;   /* Offset for lower tilt A-F (in volts)                   */
double lRLowerVOff = 0.0;   /* Offset for lower tilt L-R (in volts)                   */
double aFExtraVOff = 0.0;   /* Offset for extra tilt A-F (in volts)                   */
double lRExtraVOff = 0.0;   /* Offset for extra tilt L-R (in volts)                   */
double aFUpperMOff = 0.0;   /* Mechanical offset for upper tilt A-F (in arcsec)       */
double lRUpperMOff = 0.0;   /* Mechanical offset for upper tilt L-R (in arcsec)       */
double aFLowerMOff = 0.0;   /* Mechanical offset for lower tilt A-F (in arcsec)       */
double lRLowerMOff = 0.0;   /* Mechanical offset for lower tilt L-R (in arcsec)       */
double aFExtraMOff = 0.0;   /* Mechanical offset for extra tilt A-F (in arcsec)       */
double lRExtraMOff = 0.0;   /* Mechanical offset for extra tilt L-R (in arcsec)       */
double minTiltRMS = 0.05;   /* Flag tilt meter bad if arcsec rms is below this        */
double maxTiltRMS = 2.0;    /* Flag tilt meter bad if arcsec rms is above this        */
double upperTransform[2][2] = {{1.0,0.0}, {0.0, 1.0}};
double lowerTransform[2][2] = {{1.0,0.0}, {0.0, 1.0}};
double extraTransform[2][2] = {{1.0,0.0}, {0.0, 1.0}};

char padID = 0;
double padZenithR = 0.0;     /* Magnitude of pad gravity term in arc seconds    */
double padZenithTheta = 0.0; /* Direction of pad gravity term in radians        */

/* End of default values */

int channelErrors[32];
/*
int antList[RM_ARRAY_SIZE];
*/
int nTilts = N_TILTS;
int fD[24]; /* File descriptors for the various ADCs */

int getLine(int fD, char *buffer, int *eOF)
{
  char inChar = (char)0;
  int count = 0;
  int sawComment = FALSE;
  int foundSomething = FALSE;

  buffer[0] = (char)0;
  while ((!(*eOF)) && (inChar != '\n') && (count < 100)) {
    int nChar;

    nChar = read(fD, &inChar, 1);
    if (nChar > 0) {
      foundSomething = TRUE;
      if (inChar == '#')
	sawComment = TRUE;
      if (!sawComment)
	buffer[count++] = inChar;
    } else {
      *eOF = TRUE;
    }
  }
  if (foundSomething) {
    if (count > 0)
      buffer[count-1] = (char)0;
    return(TRUE);
  } else
    return(FALSE);
}

void tokenCheck(char *line, char *token, int integer, int *value)
     /*
       Scan "line" for "token".   If found, read the value into
       "value" as an integer, if "integer == TRUE", otherwise
       as a double precision float.
     */
{
  if (strstr(line, token)) {
    if (integer) {
      sscanf(&((char *)strstr(line, token))[strlen(token)+1], "%d", value);
      dprintf("%s = %d\n", token, *value);
    } else {
      sscanf(&((char *)strstr(line, token))[strlen(token)+1], "%lf", (double *)value);
      dprintf("%s = %f\n", token, *((double *)value));
    }
  }
}

double readMeter(int channel)
     /*
       Read one of the ADC channels, return it as a float
       in volts.
     */
{
  short data;
  int nRead;
  float oneValue;

  if (deadTiltMeters)
    return(0.0);
  if (useXYCom) {
    nRead = read(fD[channel], (char *)(&data), 2);
    if (nRead != 2) {
      fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 2\n",
	      nRead, channel);
      channelErrors[channel]++;
      return(_NAN);
    } else
      return(((float)data)*10.0/32768.0);
  } else {
    *((int *)&oneValue) = channel;
    nRead = read(fD[0], (char *)(&oneValue), 4);
    if (nRead != 4) {
      fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 4 (%d)\n",
	      nRead, channel, channelErrors[channel]);
      channelErrors[channel]++;
      return(_NAN);
    } else
      return(-oneValue);
  }
}

void logTilts(void)
{
  short tiltFlags = 0;
  int eOF = FALSE;
  int maxBadCount = 60;
  int configFD, i, status;
  int lineNumber = 0;
  int gettingUpperTransform = FALSE;
  int gettingLowerTransform = FALSE;
  int gettingExtraTransform = FALSE;
  int transformLine;
  int sleepyTime, nBoxcarSamples;
  int loopCount = 0;
  int badCount[N_TILTS] = {0, 0, 0, 0};
  int tiltComplaintActive = FALSE;
  int tiltComplaintCount = 0;
  int tiltComplaintTime = 1320;
  double *boxcar[N_TILTS+2];
  double *boxcarC[N_TILTS+2];
  double tiltRMS[N_TILTS];
  float tiltRMSF[N_TILTS], tFloat, tFloatArray[4];
  char inLine[100];
  char errorMessage[MAX_OPMSG_SIZE+1];

  /* First parse the configuration file */
  configFD = open("/instance/configFiles/logTilts.txt", O_RDONLY);
  if (configFD < 0) {
    perror("logTilts.txt");
    exit(ERROR);
  }
  while (!eOF) {
    lineNumber++;
    if (getLine(configFD, &inLine[0], &eOF))
      if (strlen(inLine) > 0) {
	if (gettingUpperTransform) {
	  if (transformLine == 1) {
	    sscanf(inLine, "%lf %lf", &upperTransform[0][0], &upperTransform[0][1]);
	    transformLine++;
	  } else {
	    sscanf(inLine, "%lf %lf", &upperTransform[1][0], &upperTransform[1][1]);
	    dprintf("UPPER_TRANSFORM = {{%f, %f}, {%f, %f}}\n",
		   upperTransform[0][0],
		   upperTransform[0][1],
		   upperTransform[1][0],
		   upperTransform[1][1]);
	    gettingUpperTransform = FALSE;
	  }
	} else if (gettingLowerTransform) {
	  if (transformLine == 1) {
	    sscanf(inLine, "%lf %lf", &lowerTransform[0][0], &lowerTransform[0][1]);
	    transformLine++;
	  } else {
	    sscanf(inLine, "%lf %lf", &lowerTransform[1][0], &lowerTransform[1][1]);
	    dprintf("LOWER_TRANSFORM = {{%f, %f}, {%f, %f}}\n",
		   lowerTransform[0][0],
		   lowerTransform[0][1],
		   lowerTransform[1][0],
		   lowerTransform[1][1]);
	    gettingLowerTransform = FALSE;
	  }
	} else if (gettingExtraTransform) {
	  if (transformLine == 1) {
	    sscanf(inLine, "%lf %lf", &extraTransform[0][0], &extraTransform[0][1]);
	    transformLine++;
	  } else {
	    sscanf(inLine, "%lf %lf", &extraTransform[1][0], &extraTransform[1][1]);
	    dprintf("EXTRA_TRANSFORM = {{%f, %f}, {%f, %f}}\n",
		   extraTransform[0][0],
		   extraTransform[0][1],
		   extraTransform[1][0],
		   extraTransform[1][1]);
	    gettingExtraTransform = FALSE;
	  }
	} else {
	  if (strstr(inLine, "UPPER_TRANSFORM")) {
	    gettingUpperTransform = TRUE;
	    transformLine = 1;
	  } else if (strstr(inLine, "LOWER_TRANSFORM")) {
	    gettingLowerTransform = TRUE;
	    transformLine = 1;
	  } else if (strstr(inLine, "EXTRA_TRANSFORM")) {
	    gettingExtraTransform = TRUE;
	    transformLine = 1;
	  } else {
	    tokenCheck(inLine, "DEAD_TILT_METERS", TRUE, &deadTiltMeters);
	    tokenCheck(inLine, "USE_XYCOM", TRUE, &useXYCom);
	    tokenCheck(inLine, "EXTRA_CHANNELS", TRUE, &extraChannels);
	    tokenCheck(inLine, "READ_PATCH_PANEL", TRUE, &readPatchPanel);
	    tokenCheck(inLine, "READ_PAD_TILTS", TRUE, &readPadTilts);
	    tokenCheck(inLine, "BOXCAR_TIME", FALSE, (int *)&boxcarTime);
	    tokenCheck(inLine, "SAMPLE_RATE", FALSE, (int *)&sampleRate);
	    tokenCheck(inLine, "NOMINAL_V2A", FALSE, (int *)&nominalV2A);
	    tokenCheck(inLine, "AF_UPPER_COR", FALSE, (int *)&aFUpperCor);
	    tokenCheck(inLine, "LR_UPPER_COR", FALSE, (int *)&lRUpperCor);
	    tokenCheck(inLine, "AF_LOWER_COR", FALSE, (int *)&aFLowerCor);
	    tokenCheck(inLine, "AF_LOWER_COR", FALSE, (int *)&lRLowerCor);
	    tokenCheck(inLine, "AF_EXTRA_COR", FALSE, (int *)&aFExtraCor);
	    tokenCheck(inLine, "LR_EXTRA_COR", FALSE, (int *)&lRExtraCor);
	    tokenCheck(inLine, "AF_UPPER_V_OFF", FALSE, (int *)&aFUpperVOff);
	    tokenCheck(inLine, "LR_UPPER_V_OFF", FALSE, (int *)&lRUpperVOff);
	    tokenCheck(inLine, "AF_LOWER_V_OFF", FALSE, (int *)&aFLowerVOff);
	    tokenCheck(inLine, "LR_LOWER_V_OFF", FALSE, (int *)&lRLowerVOff);
	    tokenCheck(inLine, "AF_EXTRA_V_OFF", FALSE, (int *)&aFExtraVOff);
	    tokenCheck(inLine, "LR_EXTRA_V_OFF", FALSE, (int *)&lRExtraVOff);
	    tokenCheck(inLine, "AF_UPPER_M_OFF", FALSE, (int *)&aFUpperMOff);
	    tokenCheck(inLine, "LR_UPPER_M_OFF", FALSE, (int *)&lRUpperMOff);
	    tokenCheck(inLine, "AF_LOWER_M_OFF", FALSE, (int *)&aFLowerMOff);
	    tokenCheck(inLine, "LR_LOWER_M_OFF", FALSE, (int *)&lRLowerMOff);
	    tokenCheck(inLine, "AF_EXTRA_M_OFF", FALSE, (int *)&aFExtraMOff);
	    tokenCheck(inLine, "LR_EXTRA_M_OFF", FALSE, (int *)&lRExtraMOff);
	    tokenCheck(inLine, "MIN_TILT_RMS", FALSE, (int *)&minTiltRMS);
	    tokenCheck(inLine, "MAX_TILT_RMS", FALSE, (int *)&maxTiltRMS);
	  }
	}
      }
  }
  close(configFD);
  /*
    Done reading in the configuration file - now open the required A/D
    devices.
  */

  if (extraChannels)
    nTilts = 6;
  else
    nTilts = 4;
  if (useXYCom) {
    int i;
    char nodeName[40];

    /* Open the four real tiltmeter channels */
    for (i = 0; i < nTilts; i++) {
      sprintf(nodeName, "/dev/xVME564-%d", i);
      fD[i] = open(nodeName, O_RDONLY);
      if (fD[i] < 0) {
	perror(nodeName);
	exit(-1);
      }
    }
    if (readPatchPanel) {
      fD[4] = open("/dev/xVME564-4", O_RDONLY);
      fD[5] = open("/dev/xVME564-5", O_RDONLY);
    }
    /* Open the extra ADC channels, for thermometers, if needed */
    if (extraChannels) {
      for (i = 16; i <= 18; i++) {
	sprintf(nodeName, "/dev/xVME564-%d", i);
	fD[i] = open(nodeName, O_RDONLY);
	if (fD[i] < 0) {
	  perror(nodeName);
	  exit(-1);
	}
      }
    }
    if (readPadTilts)
      for (i = 19; i < 32; i++) {
	sprintf(nodeName, "/dev/xVME564-%d", i);
	fD[i] = open(nodeName, O_RDONLY);
	if (fD[i] < 0) {
	  perror(nodeName);
	  exit(-1);
	}
      }
  } else
    fD[0] = open("/dev/iPOptoAD16_6000", O_RDONLY);
  sleepyTime = (int) ((1.0e6 / sampleRate) + 0.5);
  if (boxcarTime > 0.0)
    nBoxcarSamples = sampleRate / boxcarTime;
  else
    nBoxcarSamples = 1;
  if (nBoxcarSamples < 1)
    nBoxcarSamples = 1;
  if (sampleRate > 0.0) {
    maxBadCount = (int)(sampleRate*60.0 + 0.5);
    tiltComplaintTime = (int)(sampleRate*66.0 +0.5);
  }
  for (i = 0; i < nTilts; i++) {
    boxcar[i] = (double *)malloc(nBoxcarSamples*sizeof(double));
    if (boxcar[i] == NULL) {
      perror("boxcar malloc");
      exit(-1);
    }
    boxcarC[i] = (double *)malloc(nBoxcarSamples*sizeof(double));
    if (boxcarC[i] == NULL) {
      perror("boxcarC malloc");
      exit(-1);
    }
  }
  /* Initialize Reflective memory driver */
  /*
  status = rm_open(antList);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_open()");
    exit(-1);
  }
  */
  status = rm_read(RM_ANT_0, "RM_PAD_ID_B", &padID); 
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_read(RM_PAD_ID_B)");
    exit(-1);
  }
  fprintf(stderr, "logTilts: I'm running on pad %d\n", padID);
  if (( padID > 0) && (padID < 25)) {
    FILE *gravityFile;

    gravityFile = fopen("/application/configFiles/padGravity.conf", "r");
    if (gravityFile == NULL) {
      sprintf(errorMessage, "Cannot open local gravity file\n");
      sendOpMessage(OPMSG_SEVERE, 11, 60, errorMessage);
    } else {
      int padFound = FALSE;

      while ((!padFound) && (!feof(gravityFile))) {
	int nRead, pad;
	int inLinePtr;
	double r, theta;
	char inChar, inLine[132];

	inLinePtr = 0;
	while ((!feof(gravityFile)) && ((inChar = fgetc(gravityFile)) != '\n'))
	  inLine[inLinePtr++] = inChar;
	inLine[inLinePtr] = (char)0;
	if (inLine[0] != '#') {
	  nRead = sscanf(inLine, "%d %lf %lf", &pad, &r, &theta);
	  if (nRead == 3) {
	    if (pad == padID) {
	      dprintf("Saw pad %d, r = %f, theta = %f\n", pad, r, theta);
	      padFound = TRUE;
	      padZenithR = r;
	      padZenithTheta = theta;
	      if (fabs(padZenithR > 100.0)) {
		sprintf(errorMessage, "Implausibly large pad zenith r (%f)\n",
			padZenithR);
		sendOpMessage(OPMSG_WARNING, 12, 60, errorMessage);
	      }
	      if (fabs(padZenithTheta > 2.0*M_PI)) {
		sprintf(errorMessage, "Unnormalized pad zenith theta (%f)\n",
			padZenithTheta);
		sendOpMessage(OPMSG_WARNING, 13, 60, errorMessage);
	      }
	      tFloat = padZenithR;
	      status = rm_write(RM_ANT_0, "RM_TILT_PAD_ZENITH_R_F", &tFloat);
	      if (status != RM_SUCCESS) {
		rm_error_message(status, "rm_write(RM_TILT_PAD_ZENITH_R_F)");
		exit(-1);
	      }
	      tFloat = padZenithTheta;
	      status = rm_write(RM_ANT_0, "RM_TILT_PAD_ZENITH_THETA_F", &tFloat);
	      if (status != RM_SUCCESS) {
		rm_error_message(status, "rm_write(RM_TILT_PAD_ZENITH_THETA_F)");
		exit(-1);
	      }
	    }
	  }
	}
      }
      fclose(gravityFile);
      if (!padFound) {
	sprintf(errorMessage, "No local gravity info found for pad %d\n", padID);
	sendOpMessage(OPMSG_SEVERE, 11, 60, errorMessage);
      }
    }
  }
  tFloat = boxcarTime;
  status = rm_write(RM_ANT_0, "RM_TILT_BOXCAR_TIME_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_BOXCAR_TIME_F)");
    exit(-1);
  }
  tFloat = sampleRate;
  status = rm_write(RM_ANT_0, "RM_TILT_SAMPLE_RATE_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_SAMPLE_RATE_F)");
    exit(-1);
  }
  tFloat = nominalV2A;
  status = rm_write(RM_ANT_0, "RM_TILT_NOMINAL_V2A_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_NOMINAL_V2A_F)");
    exit(-1);
  }
  tFloat = aFUpperCor;
  status = rm_write(RM_ANT_0, "RM_TILT_AF_UPPER_COR_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_AF_UPPER_COR_F)");
    exit(-1);
  }
  tFloat = lRUpperCor;
  status = rm_write(RM_ANT_0, "RM_TILT_LR_UPPER_COR_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_LR_UPPER_COR_F)");
    exit(-1);
  }
  tFloat = aFUpperVOff;
  status = rm_write(RM_ANT_0, "RM_TILT_AF_UPPER_V_OFF_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_AF_UPPER_V_OFF_F)");
    exit(-1);
  }
  tFloat = lRUpperVOff;
  status = rm_write(RM_ANT_0, "RM_TILT_LR_UPPER_V_OFF_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_LR_UPPER_V_OFF_F)");
    exit(-1);
  }
  tFloat = aFUpperMOff;
  status = rm_write(RM_ANT_0, "RM_TILT_AF_UPPER_M_OFF_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_AF_UPPER_M_OFF_F)");
    exit(-1);
  }
  tFloat = lRUpperMOff;
  status = rm_write(RM_ANT_0, "RM_TILT_LR_UPPER_M_OFF_F", &tFloat);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_LR_UPPER_M_OFF_F)");
    exit(-1);
  }
  tFloatArray[0] = upperTransform[0][0];
  tFloatArray[1] = upperTransform[0][1];
  tFloatArray[2] = upperTransform[1][0];
  tFloatArray[3] = upperTransform[1][1];
  status = rm_write(RM_ANT_0, "RM_TILT_UPPER_TRANSFORM_V4_F", tFloatArray);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_write(RM_TILT_UPPER_TRANSFORM_V4_F)");
    exit(-1);
  }
  while (TRUE) {
    long longvalue;
    int j;
    float rmArray[4], extraData[4], temperature, processedExtra[4], az;
    double rawTilts[4], processedStandard[4], processedStandardC[4];
    double aFUpperT, lRUpperT, aFLowerT, lRLowerT, aFExtraT, lRExtraT;
    double aFUpperC, lRUpperC, aFLowerC, lRLowerC;
    double aFUpper, lRUpper, aFLower, lRLower, aFExtra, lRExtra, average, averageC;

    loopCount++;
    for (j = 0; j < 4; j++) {
      rawTilts[j] = readMeter(j);
      rmArray[j] = (float)rawTilts[j];
    }
    status = rm_write(RM_ANT_0, "RM_TILT_VOLTS_V4_F", rmArray);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_TILT_VOLTS_V4_F)");
      exit(-1);
    }
    aFUpperT = (rawTilts[AF_UPPER_CHANNEL] + aFUpperVOff)*nominalV2A*aFUpperCor + aFUpperMOff;
    lRUpperT = (rawTilts[LR_UPPER_CHANNEL] + lRUpperVOff)*nominalV2A*lRUpperCor + lRUpperMOff;
    aFLowerT = (rawTilts[AF_LOWER_CHANNEL] + aFLowerVOff)*nominalV2A*aFLowerCor + aFLowerMOff;
    lRLowerT = (rawTilts[LR_LOWER_CHANNEL] + lRLowerVOff)*nominalV2A*lRLowerCor + lRLowerMOff;
    if (readPatchPanel) {
      float pp[2];

      pp[0] = readMeter(4);
      pp[1] = readMeter(5);
      status = rm_write(RM_ANT_0, "RM_PATCH_PANEL_AD_V2_F", pp);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_PATCH_PANEL_AD_V2_F)");
      }
    }
    if (extraChannels) {
      if (useXYCom) {
	extraData[0] = readMeter(AF_EXTRA_CHANNEL);
	extraData[1] = readMeter(LR_EXTRA_CHANNEL);
	extraData[2] = readMeter(TEMPERATURE_CHANNEL);
	extraData[3] = 0.0;
      } else {
	extraData[0] = readMeter(4);
	extraData[1] = readMeter(5);
	extraData[2] = readMeter(6);
	extraData[3] = readMeter(7);
      }
      status = rm_write(RM_ANT_0, "RM_TILT_XTRA_TESTPOINTS_V4_F", &extraData);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_TILT_XTRA_TESTPOINTS_V4_F)");
	exit(-1);
      }
      aFExtraT = (extraData[0] + aFExtraVOff)*nominalV2A*aFExtraCor + aFExtraMOff;
      lRExtraT = (extraData[1] + lRExtraVOff)*nominalV2A*lRExtraCor + lRExtraMOff;
      temperature = extraData[2] * 100.0;
    }
    if (readPadTilts) {
      float padTiltV13[13] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};

      for (j = 19; j < 32; j++)
	if (useXYCom)
	  if (channelErrors[j] < 10)
	    padTiltV13[j-19] = (float)readMeter(j);
	  else
	    padTiltV13[j-19] = _NAN;
	else
	  if ((channelErrors[j-15] < 10) && (j < 23))
	    padTiltV13[j-19] = (float)readMeter(j-15);
	  else
	    padTiltV13[j-19] = _NAN;
      status = rm_write(RM_ANT_0, "RM_PAD_TILT_V13_F", &padTiltV13);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_PAD_TILT_V13_F)");
	exit(-1);
      }
    }
    /* Now correct to the celestial zenith */

    status = rm_read(RM_ANT_0, "RM_ACTUAL_AZ_DEG_F", &az); 
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_read(RM_ACTUAL_AZ_DEG_F)");
      exit(-1);
    }
    aFUpper = aFUpperT - padZenithR*cos((az*DEGREES_TO_RADIANS)-padZenithTheta);
    lRUpper = lRUpperT - padZenithR*sin((az*DEGREES_TO_RADIANS)-padZenithTheta);
    aFLower = aFLowerT - padZenithR*cos((az*DEGREES_TO_RADIANS)-padZenithTheta);
    lRLower = lRLowerT - padZenithR*sin((az*DEGREES_TO_RADIANS)-padZenithTheta);
    /*
      Now apply rotation
    */
    aFUpperC = upperTransform[0][0]*aFUpper + upperTransform[0][1]*lRUpper;
    lRUpperC = upperTransform[1][0]*aFUpper + upperTransform[1][1]*lRUpper;
    aFLowerC = lowerTransform[0][0]*aFLower + lowerTransform[0][1]*lRLower;
    lRLowerC = lowerTransform[1][0]*aFLower + lowerTransform[1][1]*lRLower;
    aFExtra = extraTransform[0][0]*aFExtraT + extraTransform[0][1]*lRExtraT;
    lRExtra = extraTransform[1][0]*aFExtraT + extraTransform[1][1]*lRExtraT;


    j = 0;
    while (j < (nBoxcarSamples-1)) {
      for (i = 0; i < nTilts; i++) {
	boxcar[i][j] = boxcar[i][j+1];
	boxcarC[i][j] = boxcarC[i][j+1];
      }
      j++;
    }
    boxcar[0][j] = aFUpper;
    boxcar[1][j] = lRUpper;
    boxcar[2][j] = aFLower;
    boxcar[3][j] = lRLower;
    boxcarC[0][j] = aFUpperC;
    boxcarC[1][j] = lRUpperC;
    boxcarC[2][j] = aFLowerC;
    boxcarC[3][j] = lRLowerC;
    if (extraChannels) {
      boxcar[4][j] = aFExtra;
      boxcar[5][j] = lRExtra;
    }
    for (i = 0; i < nTilts; i++) {
      average = averageC = 0.0;
      for (j = 0; j < nBoxcarSamples; j++) {
	average += boxcar[i][j];
	averageC += boxcarC[i][j];
      }
      average /= (double)nBoxcarSamples;
      averageC /= (double)nBoxcarSamples;
      if (i < N_TILTS) {
	processedStandard[i] = average;
	processedStandardC[i] = averageC;
      } else
	processedExtra[i-N_TILTS] = average;
    }
    
    if (loopCount > 2*nBoxcarSamples) {
      for (i = 0; i < N_TILTS; i++) {
	tiltRMS[i] = 0.0;
	if (nBoxcarSamples > 1) {
	  for (j = 0; j < nBoxcarSamples; j++)
	    tiltRMS[i] += (boxcar[i][j]-processedStandard[i]) *
	      (boxcar[i][j]-processedStandard[i]);
	  tiltRMS[i] = sqrt(tiltRMS[i]/nBoxcarSamples);
	  tiltRMSF[i] = (float)tiltRMS[i];
	  if ((tiltRMS[i] < minTiltRMS) || (tiltRMS[i] > maxTiltRMS)) {
	    badCount[i]++;
	    dprintf("Tilt %d RMS (%f) out of bounds, badCount = %d (%d)\n",
		    i, tiltRMS[i], badCount[i], maxBadCount);
	    if (badCount[i] >= maxBadCount) {
	      tiltFlags |= 1 << i;
	    }
	  } else {
	    tiltFlags &= ~(1 << i);
	    badCount[i] = 0;
	  }
	  if ((!tiltComplaintActive) && (!deadTiltMeters) &&
	      (tiltRMS[i] < minTiltRMS) &&
	      (badCount[i] >= maxBadCount)) {
	    sprintf(errorMessage, "Tilt meter %d RMS %10.4e < %6.4f, will flag bad\n",
		    i, tiltRMS[i], minTiltRMS);
	    if (debug) {
	      dprintf("Sending opmsg \"%s\"\n", errorMessage);
	      for (j = 0; j < nBoxcarSamples; j++)
		dprintf("sample %d: %f\n", j, boxcar[i][j]);
	    }
	    fprintf(stderr, "Bad tilt (too low) %d: %6.4f:%6.5f\n",
		    i, processedStandard[i], tiltRMS[i]);
	    sendOpMessage(OPMSG_SEVERE, 11, 60, errorMessage);
	    tiltComplaintActive = TRUE;
	    tiltComplaintCount = tiltComplaintTime;
	  }
	  if ((!tiltComplaintActive) && (!deadTiltMeters) &&
	      (tiltRMS[i] > maxTiltRMS) &&
	      (badCount[i] >= maxBadCount)) {
	    sprintf(errorMessage, "Tilt meter %d RMS %6.4f > %6.4f, will flag bad\n",
		    i, tiltRMS[i], maxTiltRMS);
	    dprintf("Sending opmsg \"%s\"\n", errorMessage);
	    fprintf(stderr, "Bad tilt (too high) %d: %6.4f:%6.5f\n",
		    i, processedStandard[i], tiltRMS[i]);
	    sendOpMessage(OPMSG_SEVERE, 11, 60, errorMessage);
	    tiltComplaintActive = TRUE;
	    tiltComplaintCount = tiltComplaintTime;
	  }
	}
      }
      if (deadTiltMeters)
	tiltFlags = -1;
      status = rm_write(RM_ANT_0, "RM_TILT_FLAGS_S", &tiltFlags);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_TILT_FLAGS_S)");
	exit(-1);
      }
      status = rm_write(RM_ANT_0, "RM_TILT_RMS_V4_F", tiltRMSF);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_TILT_RMS_V4_F)");
	exit(-1);
      }
    }
    /*
    printf("aFU: %7.3f(%7.3f)  lRU: %7.3f(%7.3f)  aFL: %7.3f(%7.3f)  lRL: %7.3f(%7.3f)\n",
	   processedStandard[0], aFUpper, processedStandard[1], lRUpper,
	   processedStandard[2], aFLower, processedStandard[3], lRLower);
    */
    status = rm_write(RM_ANT_0, "RM_AFT_FOREWARD_TILT_UPPER_ARCSEC_D",
		      &processedStandard[0]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_AFT_FOREWARD_TILT_UPPER_ARCSEC_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D",
		      &processedStandard[1]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_AFT_FOREWARD_TILT_LOWER_ARCSEC_D",
		      &processedStandard[2]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_AFT_FOREWARD_TILT_LOWER_ARCSEC_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_LEFT_RIGHT_TILT_LOWER_ARCSEC_D",
		      &processedStandard[3]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_LEFT_RIGHT_TILT_LOWER_ARCSEC_D)");
      exit(-1);
    }
    if (((processedStandardC[0] != processedStandardC[0]) ||
	 (processedStandardC[1] != processedStandardC[1]) ||
	 (processedStandardC[2] != processedStandardC[2]) ||
	 (processedStandardC[3] != processedStandardC[3]))
	&& (loopCount > 2*nBoxcarSamples)) {
      sprintf(errorMessage, "NaN Celestial Tilt Value - turn tilts off and on");
      sendOpMessage(OPMSG_SEVERE, 11, 60, errorMessage);
    }
    status = rm_write(RM_ANT_0, "RM_AFT_FOREWARD_TILT_UPPER_CELESTIAL_D",
		      &processedStandardC[0]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_AFT_FOREWARD_TILT_UPPER_CELESTIAL_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_LEFT_RIGHT_TILT_UPPER_CELESTIAL_D",
		      &processedStandardC[1]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_LEFT_RIGHT_TILT_UPPER_CELESTIAL_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_AFT_FOREWARD_TILT_LOWER_CELESTIAL_D",
		      &processedStandardC[2]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_AFT_FOREWARD_TILT_LOWER_CELESTIAL_D)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_LEFT_RIGHT_TILT_LOWER_CELESTIAL_D",
		      &processedStandardC[3]);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(RM_LEFT_RIGHT_TILT_LOWER_CELESTIAL_D)");
      exit(-1);
    }
    if (extraChannels) {
      status = rm_write(RM_ANT_0, "RM_TILT_XTRA_TESTPOINTS_CALIBRATED_V4_F", &processedExtra);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write(RM_TILT_XTRA_TESTPOINTS_CALIBRATED_V4_F)");
	exit(-1);
      }
      status = rm_write(RM_ANT_0, "RM_TILT3_HI_XELEV_TEMPERATURE_F", &temperature);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write()");
	exit(-1);
      }
    }
    status = rm_read(RM_ANT_0, "RM_UNIX_TIME_L", &longvalue); 
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_read(UNIX_TIME_L)");
      exit(-1);
    }
    status = rm_write(RM_ANT_0, "RM_TILT_TIMESTAMP_L", &longvalue);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write(TILT_TIMESTAMP_L)");
      exit(-1);
    }
    if (tiltComplaintActive && (tiltComplaintCount-- <= 0))
      tiltComplaintActive = FALSE;
    usleep(sleepyTime);
  }
  fprintf(stderr, "Exiting logTilts (That should never happen!)\n");
}
/*
main(int argc, char **argv)
{
  int debugMessagesOn;

  if (argc > 1)
    debugMessagesOn = TRUE;
  else
    debugMessagesOn = FALSE;
  fclose(stdin);
  logTilts(debugMessagesOn);
}
*/

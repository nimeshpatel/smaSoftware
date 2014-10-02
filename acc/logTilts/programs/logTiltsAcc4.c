/*     logTilts.c                                                           */
/*     First version Feb. 13, 2000                                          */
/*                                                                          */
/*    This program merely reads the tilt meters, and either writes them to  */
/* Reflective Memory.   The raw A/D values are converted to volts before    */
/* they are written to RM.                                                  */
/*                                                                          */

#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>    /* needed to handle SIGHUP from the rc file */
#include <sys/types.h>
#include <fcntl.h>
#include "rm.h"

#define N_TILTS 4

int sigHUPReceived = 0;

void signalHandler(int signum) {
  if (signum == SIGHUP) {  
    fprintf(stderr,"logTilts received SIGHUP - will now proceed\n");
    sigHUPReceived = 1;
  }
  else { 
    fprintf(stderr,"logTilts received unexpected signal #%d\n",signum);
  }
}

int main(int argc, char **argv)
{
  int i, fD[N_TILTS+2], nRead, status;
  short data;
  unsigned loopCount = 0;
  int antList[RM_ARRAY_SIZE];
  int gotSecondAD, secondADFD;
  float fData[N_TILTS+2], fSecond, boxCar;
  char nodeName[80];
  int debugMessagesOn;
  double voltage;
  struct sigaction action, oldAction;
  long longvalue;
  float celsius;

  if (argc > 1)
    debugMessagesOn = 1;
  else
    debugMessagesOn = 0;

  boxCar = 0.0;
/*********************************************************************/   
/* set up to receive and wait for the SIGHUP signal from the rc file */
/*********************************************************************/   
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = signalHandler;
    sigaction(SIGHUP, &action, &oldAction);

  /* Open all A/D channels: */
  for (i = 0; i <= (N_TILTS+1); i++) {
    sprintf(nodeName, "/dev/xVME564-%d", i);
    fD[i] = open(nodeName, O_RDONLY);
    if (fD[i] < 0) {
      perror("Open");
      exit(-1);
    }
  }

  secondADFD = open("/dev/iPOptoAD16_2", O_RDONLY);
  if (secondADFD < 0)
    gotSecondAD = 0;
  else
    gotSecondAD = 1;

  /* Initialize Reflective memory driver */
  status = rm_open(antList);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_open()");
    exit(-1);
  }

  /* Loop forever read'n and write'n tilts */
  while (1) {
    loopCount += 1;
    if (!(loopCount % 10)) {
      for (i = 0; i <= (N_TILTS+1); i++) {
	nRead = read(fD[i], (char *)(&data), 2);
	if (nRead != 2)
	  fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 2\n",
		  nRead, i);
	else
	  fData[i] = ((float)data)*10.0/32768.0;
      }
    }
    if (gotSecondAD) {
      *((int *)&fSecond) = 0; /* Martina, set channel 0 or 1 */
      nRead = read(secondADFD, (char *)(&fSecond), 4);
      if (nRead != 4) {
	fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 2\n",
		nRead, i);
	gotSecondAD = 0;
	fSecond = 0.0;
      }
    } else
      fSecond = 0.0;
    boxCar  = (boxCar + fSecond) / 1.010101010101010101;
    if (!(loopCount % 10)) {
      if (debugMessagesOn)
	printf(
	       "Writing tilts: %f %f %f %f TP 1: %f TP 2: %f TP 3: %f (%f)\n",
	       fData[0], fData[1], fData[2], fData[3], fData[4], fData[5],
	       fSecond, boxCar/99.0);
      status = rm_write(RM_ANT_0, "RM_TILT_VOLTS_V4_F", fData);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write()");
	exit(-1);
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
      voltage = (double)fData[5];
      status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS_D", &voltage);
      if (status != RM_SUCCESS) { 
	rm_error_message(status, "rm_write()");
	exit(-1);
      } 
      voltage = (double)fData[5];
      status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS2_D", &voltage);
      if (status != RM_SUCCESS) { 
	rm_error_message(status, "rm_write()");
	exit(-1);
      } 
      voltage = (double)(boxCar/99.0);
      status = rm_write(RM_ANT_0, "RM_IF_LO_TOTAL_POWER_D", &voltage);
      if (status != RM_SUCCESS) { 
	rm_error_message(status, "rm_write()");
	exit(-1);
      } 
      celsius = fData[N_TILTS+2]*100;
      status = rm_write(RM_ANT_0, "RM_TILT3_HI_XELEV_TEMPERATURE_F", &celsius);
      if (status != RM_SUCCESS) {
	rm_error_message(status, "rm_write()");
	exit(-1);
      }
    }
    usleep(10000);
  }

  /* Should never get here; why do I bother? */

  /* Close A/D files */
  for (i = 0; i < (N_TILTS+2); i++)
    close(fD[i]);

  /* Shut down RM */
  status = rm_close();
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_close()");
    exit(-1);
  }
  exit(0);

}

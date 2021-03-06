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
/* #include "rm.h" */

#define N_TILTS 4

int sigHUPReceived = 0;
float fData[N_TILTS][100000];

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
  int i, ii, fD[N_TILTS], nRead, status;
  short data;
  char nodeName[80];
  int debugMessagesOn;
  int nSamples = 100;
  struct sigaction action, oldAction;

  if (argc > 1)
    sscanf(argv[1], "%d", &nSamples);

/*********************************************************************/   
/* set up to receive and wait for the SIGHUP signal from the rc file */
/*********************************************************************/   
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = signalHandler;
    sigaction(SIGHUP, &action, &oldAction);

  /* Open all A/D channels: */
  for (i = 0; i < N_TILTS; i++) {
    sprintf(nodeName, "/dev/xVME564-%d", i);
    fD[i] = open(nodeName, O_RDONLY);
    if (fD[i] < 0) {
      perror("Open");
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

  /* Loop forever read'n and write'n tilts */
  for (ii = 0; ii < nSamples; ii++) {
    /*
    printf("ii: %d\n", ii);
    */
    for (i = 0; i < N_TILTS; i++) {
      nRead = read(fD[i], (char *)(&data), 2);
      if (nRead != 2)
	fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 2\n",
		nRead, i);
      else
	fData[i][ii] = ((float)data)*10.0/32768.0;
    }
  }
  for (ii = 0; ii < nSamples; ii++) {
    printf("%d\t", ii);
    for (i = 0; i < N_TILTS; i++) {
      printf("%f ", fData[i][ii]);
    }
    printf("\n");
  }
  /* Should never get here; why do I bother? */

  /* Close A/D files */
  for (i = 0; i < N_TILTS; i++)
    close(fD[i]);

  /* Shut down RM */
  /* status = rm_close(); */
  /*
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_close()");
    exit(-1);
  }
  */
  exit(0);

}

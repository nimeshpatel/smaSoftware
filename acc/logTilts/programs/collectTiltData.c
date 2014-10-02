#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>

#define N_TILTS 4
#define N_SAMPLES 262145

short readings[N_SAMPLES][N_TILTS];
struct timespec times[N_SAMPLES][N_TILTS];

main()
{
  int i, j, fD[N_TILTS+3], nRead, status;
  short data;
  char nodeName[100];
  FILE *results;

  for (i = 0; i < N_TILTS; i++) {
    sprintf(nodeName, "/dev/xVME564-%d", i);
    fD[i] = open(nodeName, O_RDONLY);
    if (fD[i] < 0) {
      perror("Open");
      exit(-1);
    }
  }
  for (j = 0; j < N_SAMPLES; j++)
    for (i = 0; i < N_TILTS; i++) {
      nRead = read(fD[i], (char *)(&data), 2);
      if (nRead != 2)
	fprintf(stderr, "on channel %d got %d bytes instead of 2\n",
		i, nRead);
      else {
	clock_gettime(CLOCK_REALTIME, &times[j][i]);
	readings[j][i] = data;
      }
    }
  printf("Done collecting\n");fflush(stdout);
  results = fopen("/instance/logs/tilts.txt", "w");
  for (j = 0; j < N_SAMPLES; j++)
    for (i = 0; i < N_TILTS; i++) {
      double doubleTime;
      float fReading;

      fReading = ((float)readings[j][i])*10.0/32768.0;
      doubleTime = (double)(times[j][i].tv_sec) +
	((double)(times[j][i].tv_nsec))*1.0e-9;
      fprintf(results, "Sample %d\tChannel %d: %f\t%f\n",
	     j, i, fReading, doubleTime);
    }
}

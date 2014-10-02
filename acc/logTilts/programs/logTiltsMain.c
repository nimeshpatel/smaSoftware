/*     logTiltsMain.c                                                       */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include "rm.h"

#define TRUE   (1)
#define FALSE  (0)
#define ERROR (-1)

int antList[RM_ARRAY_SIZE];

main(int argc, char **argv)
{
  int status;

  fclose(stdin);
  status = rm_open(antList);
  if (status != RM_SUCCESS) {
    rm_error_message(status, "rm_open()");
    exit(-1);
  }
  logTilts();
}

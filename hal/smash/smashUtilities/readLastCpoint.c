#include <stdio.h>
#include "dsm.h"

int main(int argc, char  *argv[]) {
  int stat;
  long lastcpoint[11];
  long timestamp;
  short lastrx[11];
  int i;

  stat = dsm_open();
  if (stat) {
    dsm_error_message(stat,"dsm_open");
    exit(-8);
  }
  stat = dsm_read("hal9000", "DSM_HAL_HAL_LAST_CPOINT_V11_L", lastcpoint, &timestamp);
  for (i=1; i<=8; i++) {
    printf("%d ",lastcpoint[i]);
  }
  stat = dsm_read("hal9000", "DSM_HAL_HAL_LAST_CPOINT_RX_V11_S", lastrx, &timestamp);
  for (i=1; i<=8; i++) {
    printf("%d ",lastrx[i]);
  }
  printf("\n");
  return(dsm_close());
}


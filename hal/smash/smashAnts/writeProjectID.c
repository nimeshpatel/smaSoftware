#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <errno.h>
#include "dsm.h"
#include "commonLib.h"

void main(int argc, char**argv)  {
  FILE *fp_projectID;
  int projectID, timestamp;
  int dsm_status = dsm_status = dsm_open();
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_open");
    exit(-1);
  }
  fp_projectID = fopen("/global/projects/project.id","r+");
  if(fp_projectID==NULL) {
    fprintf(stderr,"Failed to open the project.id file.\n");
    exit(-1);
  }
  fscanf(fp_projectID,"%d",&projectID);
  dsm_status = dsm_write("m5","DSM_AS_PROJECT_ID_L",&projectID);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_write() projectID");
    exit(1);
  }
  fclose(fp_projectID);
}

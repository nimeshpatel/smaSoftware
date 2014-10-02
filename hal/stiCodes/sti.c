#include <stdio.h>
#include <string.h>
#include <rpc/rpc.h>
#include "sti.h"

commandStatus *result;
commandRequest rawCommandRequest;

int main(int argc, char *argv[]) {
  CLIENT *cl;         /* Client "handle"                    */
  char fileNamePrefix[80]={""};
  double exposure=0.0;

  if (!(cl = clnt_create(argv[1], STIPROG, STIVERS, "tcp"))) {
    clnt_pcreateerror(argv[1]);
    exit(-1);
  }
    exposure=atof(argv[2]);
    printf("%s %f %s \n",argv[1],exposure,argv[3]);
    if ((exposure<0.01)||(exposure>10.)) {
    printf("Exposure time should between 0.010 and 10.0 seconds.\n");
    exit(-2);
    }
    rawCommandRequest.exposureSec=exposure;
    strcpy(rawCommandRequest.fileNamePrefix,argv[3]);
    result=sticommandrequest_1(&rawCommandRequest,cl);
}

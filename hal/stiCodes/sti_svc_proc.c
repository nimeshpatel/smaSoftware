#include <stdio.h>
#include <rpc/rpc.h>
#include "sti.h"

extern int snapshot(double exposureSec, char *filenameprefix);

static commandStatus *commandstatus=NULL;

commandStatus *sticommandrequest_1(commandRequest *request, CLIENT *cl);
commandStatus *sticommandrequest_1_svc(commandRequest *request, struct svc_req *dummy);

commandStatus *sticommandrequest_1_svc(commandRequest *request, struct svc_req *dummy) {
  CLIENT *cl;

  return(sticommandrequest_1(request, cl));
}

commandStatus *sticommandrequest_1(commandRequest *request, CLIENT *cl) {
  int status;
  /* Create a structure in which to return the results */
  if(commandstatus == NULL) {
  commandstatus = (commandStatus *)malloc(sizeof(commandStatus));
  }
  printf("Got filename prefix: %s\n",request->fileNamePrefix);
  printf("Got exposure time: %f seconds\n",request->exposureSec);
  status=snapshot(request->exposureSec,request->fileNamePrefix);
  commandstatus->status=status;
  return((commandStatus *)commandstatus);
}

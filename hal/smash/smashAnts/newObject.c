/*
    newObject.c

    This function sends an RPC client request to the DDS server, telling it
  to request new coordinates from the coordinates server.   The client
  handle is created and destroyed each time this function is called, which
  increases the overhead somewhat, but should make the code more robust.
*/

#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "dDS.h"

#define OK     0
#define ERROR -1

/*
  statusCheck checks the status returned by the client call to the DDS
  server.
*/
void main()
{
newObject();
}
int statusCheck(dDSStatus *status)
{
  if (status == NULL) {
    fprintf(stderr,
"NULL pointer returned from client call to DDS server.   Server terminated?\n"
	    );
    return(ERROR);
  }
  if (status->status != DDS_SUCCESS) {
    fprintf(stderr, "DDS server returned error status, reason = %d\n",
      status->reason);
    return(ERROR);
  }
  return(OK);
}

int newObject(void)
{
  CLIENT *cl;
  dDSCommand command;
  int returnCode;

  /* First get client handle */

  if (!(cl = clnt_create("dds", DDSPROG, DDSVERS, "tcp"))) {
    clnt_pcreateerror("dds");
    return(ERROR);
  }

  command.antenna = DDS_ALL_ANTENNAS;
  command.receiver = DDS_ALL_RECEIVERS;
  command.command = DDS_GET_COORDS;
  returnCode = statusCheck(ddsrequest_1(&command, cl));

  clnt_destroy(cl);
  return(returnCode);
}

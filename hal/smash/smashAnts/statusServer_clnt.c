#include <rpc/rpc.h>
#include "statusServer.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

info *
statusrequest_1(argp, clnt)
	requestCodes *argp;
	CLIENT *clnt;
{
	static info res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, STATUSREQUEST, xdr_requestCodes, argp, xdr_info, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


history *
anthistory_1(argp, clnt)
	historyRequest *argp;
	CLIENT *clnt;
{
	static history res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, ANTHISTORY, xdr_historyRequest, argp, xdr_history, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


ccdHeader *
ccdheader_1(argp, clnt)
	antennaQuery *argp;
	CLIENT *clnt;
{
	static ccdHeader res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, CCDHEADER, xdr_antennaQuery, argp, xdr_ccdHeader, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


ddsCoordInfo *
ddscoordinfo_1(argp, clnt)
	antennaQuery *argp;
	CLIENT *clnt;
{
	static ddsCoordInfo res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSCOORDINFO, xdr_antennaQuery, argp, xdr_ddsCoordInfo, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


sSStatus *
offline_1(argp, clnt)
	antennaList *argp;
	CLIENT *clnt;
{
	static sSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, OFFLINE, xdr_antennaList, argp, xdr_sSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


sSStatus *
changeflagging_1(argp, clnt)
	flagChange *argp;
	CLIENT *clnt;
{
	static sSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, CHANGEFLAGGING, xdr_flagChange, argp, xdr_sSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


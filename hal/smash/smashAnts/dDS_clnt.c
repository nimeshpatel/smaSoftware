#include <rpc/rpc.h>
#include "dDS.h"

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

dDSStatus *
ddsrequest_1(argp, clnt)
	dDSCommand *argp;
	CLIENT *clnt;
{
	static dDSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSREQUEST, xdr_dDSCommand, argp, xdr_dDSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSStatus *
ddssource_1(argp, clnt)
	dDSSource *argp;
	CLIENT *clnt;
{
	static dDSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSSOURCE, xdr_dDSSource, argp, xdr_dDSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSFringeRates *
ddsrates_1(argp, clnt)
	dDSCommand *argp;
	CLIENT *clnt;
{
	static dDSFringeRates res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSRATES, xdr_dDSCommand, argp, xdr_dDSFringeRates, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSInfo *
ddsinfo_1(argp, clnt)
	dDSCommand *argp;
	CLIENT *clnt;
{
	static dDSInfo res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSINFO, xdr_dDSCommand, argp, xdr_dDSInfo, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSStatus *
ddssign_1(argp, clnt)
	dDSSignChange *argp;
	CLIENT *clnt;
{
	static dDSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSSIGN, xdr_dDSSignChange, argp, xdr_dDSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSStatus *
ddsfrequency_1(argp, clnt)
	dDSFrequency *argp;
	CLIENT *clnt;
{
	static dDSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSFREQUENCY, xdr_dDSFrequency, argp, xdr_dDSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSStatus *
ddssetbaselines_1(argp, clnt)
	dDSBaselines *argp;
	CLIENT *clnt;
{
	static dDSStatus res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSSETBASELINES, xdr_dDSBaselines, argp, xdr_dDSStatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


dDSDelayValues *
ddsgetdelay_1(argp, clnt)
	dDSDelayRequest *argp;
	CLIENT *clnt;
{
	static dDSDelayValues res;

	bzero((char *)&res, sizeof(res));
	if (clnt_call(clnt, DDSGETDELAY, xdr_dDSDelayRequest, argp, xdr_dDSDelayValues, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


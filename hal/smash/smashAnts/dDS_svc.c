#include <stdio.h>
#include <rpc/rpc.h>
#include "dDS.h"

static void ddsprog_1();

main()
{
	SVCXPRT *transp;

	(void)pmap_unset(DDSPROG, DDSVERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, DDSPROG, DDSVERS, ddsprog_1, IPPROTO_UDP)) {
		(void)fprintf(stderr, "unable to register (DDSPROG, DDSVERS, udp).\n");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create tcp service.\n");
		exit(1);
	}
	if (!svc_register(transp, DDSPROG, DDSVERS, ddsprog_1, IPPROTO_TCP)) {
		(void)fprintf(stderr, "unable to register (DDSPROG, DDSVERS, tcp).\n");
		exit(1);
	}
	svc_run();
	(void)fprintf(stderr, "svc_run returned\n");
	exit(1);
}

static void
ddsprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		dDSCommand ddsrequest_1_arg;
		dDSSource ddssource_1_arg;
		dDSCommand ddsrates_1_arg;
		dDSCommand ddsinfo_1_arg;
		dDSSignChange ddssign_1_arg;
		dDSFrequency ddsfrequency_1_arg;
		dDSBaselines ddssetbaselines_1_arg;
		dDSDelayRequest ddsgetdelay_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void)svc_sendreply(transp, xdr_void, (char *)NULL);
		return;

	case DDSREQUEST:
		xdr_argument = xdr_dDSCommand;
		xdr_result = xdr_dDSStatus;
		local = (char *(*)()) ddsrequest_1;
		break;

	case DDSSOURCE:
		xdr_argument = xdr_dDSSource;
		xdr_result = xdr_dDSStatus;
		local = (char *(*)()) ddssource_1;
		break;

	case DDSRATES:
		xdr_argument = xdr_dDSCommand;
		xdr_result = xdr_dDSFringeRates;
		local = (char *(*)()) ddsrates_1;
		break;

	case DDSINFO:
		xdr_argument = xdr_dDSCommand;
		xdr_result = xdr_dDSInfo;
		local = (char *(*)()) ddsinfo_1;
		break;

	case DDSSIGN:
		xdr_argument = xdr_dDSSignChange;
		xdr_result = xdr_dDSStatus;
		local = (char *(*)()) ddssign_1;
		break;

	case DDSFREQUENCY:
		xdr_argument = xdr_dDSFrequency;
		xdr_result = xdr_dDSStatus;
		local = (char *(*)()) ddsfrequency_1;
		break;

	case DDSSETBASELINES:
		xdr_argument = xdr_dDSBaselines;
		xdr_result = xdr_dDSStatus;
		local = (char *(*)()) ddssetbaselines_1;
		break;

	case DDSGETDELAY:
		xdr_argument = xdr_dDSDelayRequest;
		xdr_result = xdr_dDSDelayValues;
		local = (char *(*)()) ddsgetdelay_1;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}
	bzero((char *)&argument, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		return;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
}


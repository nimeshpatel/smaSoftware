#include <stdio.h>
#include <rpc/rpc.h>
#include "statusServer.h"

static void statusserverprog_1();

main()
{
	SVCXPRT *transp;

	(void)pmap_unset(STATUSSERVERPROG, STATUSSERVERVERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, STATUSSERVERPROG, STATUSSERVERVERS, statusserverprog_1, IPPROTO_UDP)) {
		(void)fprintf(stderr, "unable to register (STATUSSERVERPROG, STATUSSERVERVERS, udp).\n");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create tcp service.\n");
		exit(1);
	}
	if (!svc_register(transp, STATUSSERVERPROG, STATUSSERVERVERS, statusserverprog_1, IPPROTO_TCP)) {
		(void)fprintf(stderr, "unable to register (STATUSSERVERPROG, STATUSSERVERVERS, tcp).\n");
		exit(1);
	}
	svc_run();
	(void)fprintf(stderr, "svc_run returned\n");
	exit(1);
}

static void
statusserverprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		requestCodes statusrequest_1_arg;
		historyRequest anthistory_1_arg;
		antennaQuery ccdheader_1_arg;
		antennaQuery ddscoordinfo_1_arg;
		antennaList offline_1_arg;
		flagChange changeflagging_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void)svc_sendreply(transp, xdr_void, (char *)NULL);
		return;

	case STATUSREQUEST:
		xdr_argument = xdr_requestCodes;
		xdr_result = xdr_info;
		local = (char *(*)()) statusrequest_1;
		break;

	case ANTHISTORY:
		xdr_argument = xdr_historyRequest;
		xdr_result = xdr_history;
		local = (char *(*)()) anthistory_1;
		break;

	case CCDHEADER:
		xdr_argument = xdr_antennaQuery;
		xdr_result = xdr_ccdHeader;
		local = (char *(*)()) ccdheader_1;
		break;

	case DDSCOORDINFO:
		xdr_argument = xdr_antennaQuery;
		xdr_result = xdr_ddsCoordInfo;
		local = (char *(*)()) ddscoordinfo_1;
		break;

	case OFFLINE:
		xdr_argument = xdr_antennaList;
		xdr_result = xdr_sSStatus;
		local = (char *(*)()) offline_1;
		break;

	case CHANGEFLAGGING:
		xdr_argument = xdr_flagChange;
		xdr_result = xdr_sSStatus;
		local = (char *(*)()) changeflagging_1;
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


#include <stdio.h>
#include <rpc/rpc.h>
#include "tune.h"

static void tuneprog_1();

main()
{
	SVCXPRT *transp;

	(void)pmap_unset(TUNEPROG, TUNEVERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, TUNEPROG, TUNEVERS, tuneprog_1, IPPROTO_UDP)) {
		(void)fprintf(stderr, "unable to register (TUNEPROG, TUNEVERS, udp).\n");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create tcp service.\n");
		exit(1);
	}
	if (!svc_register(transp, TUNEPROG, TUNEVERS, tuneprog_1, IPPROTO_TCP)) {
		(void)fprintf(stderr, "unable to register (TUNEPROG, TUNEVERS, tcp).\n");
		exit(1);
	}
	svc_run();
	(void)fprintf(stderr, "svc_run returned\n");
	exit(1);
}

static void
tuneprog_1(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	union {
		loBoardStatusRequest loboardstatusrequest_1_arg;
		rxInfoRequest inforequest_1_arg;
		tuningRequest tunerequest_1_arg;
		opticsRequest opticsrequest_1_arg;
		ivCurveRequest ivrequest_1_arg;
		controlRequest controlrequest_1_arg;
		rawRequest rawrequest_1_arg;
		bfieldSweepRequest bfieldsweeprequest_1_arg;
		rawRequest rawrequest1_1_arg;
		rawRequest rawrequest2_1_arg;
		rawRequest rawrequest3_1_arg;
		rawRequest rawrequest4_1_arg;
		rawRequest rawrequest5_1_arg;
		rawRequest rawrequest6_1_arg;
		rawRequest rawrequest7_1_arg;
		rawRequest rawrequest8_1_arg;
		rawRequest rawrequest9_1_arg;
		rawRequest rawrequest10_1_arg;
		configureBWD configurebwd_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case LOBOARDSTATUSREQUEST:
		xdr_argument = xdr_loBoardStatusRequest;
		xdr_result = xdr_loBoardStatus;
		local = (char *(*)()) loboardstatusrequest_1;
		break;

	case INFOREQUEST:
		xdr_argument = xdr_rxInfoRequest;
		xdr_result = xdr_rxInfoReply;
		local = (char *(*)()) inforequest_1;
		break;

	case TUNEREQUEST:
		xdr_argument = xdr_tuningRequest;
		xdr_result = xdr_tuningResults;
		local = (char *(*)()) tunerequest_1;
		break;

	case OPTICSREQUEST:
		xdr_argument = xdr_opticsRequest;
		xdr_result = xdr_opticsStatus;
		local = (char *(*)()) opticsrequest_1;
		break;

	case IVREQUEST:
		xdr_argument = xdr_ivCurveRequest;
		xdr_result = xdr_ivCurveResults;
		local = (char *(*)()) ivrequest_1;
		break;

	case CONTROLREQUEST:
		xdr_argument = xdr_controlRequest;
		xdr_result = xdr_controlResults;
		local = (char *(*)()) controlrequest_1;
		break;

	case RAWREQUEST:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest_1;
		break;

	case BFIELDSWEEPREQUEST:
		xdr_argument = xdr_bfieldSweepRequest;
		xdr_result = xdr_bfieldSweepResults;
		local = (char *(*)()) bfieldsweeprequest_1;
		break;

	case RAWREQUEST1:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest1_1;
		break;

	case RAWREQUEST2:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest2_1;
		break;

	case RAWREQUEST3:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest3_1;
		break;

	case RAWREQUEST4:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest4_1;
		break;

	case RAWREQUEST5:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest5_1;
		break;

	case RAWREQUEST6:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest6_1;
		break;

	case RAWREQUEST7:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest7_1;
		break;

	case RAWREQUEST8:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest8_1;
		break;

	case RAWREQUEST9:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest9_1;
		break;

	case RAWREQUEST10:
		xdr_argument = xdr_rawRequest;
		xdr_result = xdr_rawResults;
		local = (char *(*)()) rawrequest10_1;
		break;

	case CONFIGUREBWD:
		xdr_argument = xdr_configureBWD;
		xdr_result = xdr_bWDResults;
		local = (char *(*)()) configurebwd_1;
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


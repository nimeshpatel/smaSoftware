#include <rpc/rpc.h>
#include "dDS.h"


bool_t
xdr_dDSStatus(xdrs, objp)
	XDR *xdrs;
	dDSStatus *objp;
{
	if (!xdr_int(xdrs, &objp->status)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->reason)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSBaselines(xdrs, objp)
	XDR *xdrs;
	dDSBaselines *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->X, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->Y, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->Z, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSSource(xdrs, objp)
	XDR *xdrs;
	dDSSource *objp;
{
	if (!xdr_double(xdrs, &objp->hourAngle)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->declination)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSFrequency(xdrs, objp)
	XDR *xdrs;
	dDSFrequency *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->frequency, DDS_N_RECEIVERS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->gunnMultiple, DDS_N_RECEIVERS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSFringeRates(xdrs, objp)
	XDR *xdrs;
	dDSFringeRates *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->rate1, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->rate2, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSCommand(xdrs, objp)
	XDR *xdrs;
	dDSCommand *objp;
{
	if (!xdr_int(xdrs, &objp->command)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->antenna)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->receiver)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->refFrequency)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->fringeRate1, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->fringeRate2, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->phase1, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->phase2, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSInfo(xdrs, objp)
	XDR *xdrs;
	dDSInfo *objp;
{
	if (!xdr_int(xdrs, &objp->validPosition)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->hardwareEnabled)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->frequency, DDS_N_RECEIVERS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->gunnMultiple, DDS_N_RECEIVERS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->hourAngle)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->declination)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->frequencySign)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->phaseSign)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS1Exists, DDS_N_ANTENNAS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS1Rate, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS1Phase, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS2Exists, DDS_N_ANTENNAS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS2Rate, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dDS2Phase, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->delayTracking)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->pattern, DDS_N_ANTENNAS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->delay, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->baseline, DDS_N_BASELINES, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSSignChange(xdrs, objp)
	XDR *xdrs;
	dDSSignChange *objp;
{
	if (!xdr_int(xdrs, &objp->frequencySign)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->phaseSign)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSDelayRequest(xdrs, objp)
	XDR *xdrs;
	dDSDelayRequest *objp;
{
	if (!xdr_int(xdrs, &objp->function)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSDelayValues(xdrs, objp)
	XDR *xdrs;
	dDSDelayValues *objp;
{
	if (!xdr_int(xdrs, &objp->status)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->antennaExists, DDS_N_ANTENNAS, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->delaySec, DDS_N_ANTENNAS, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	return (TRUE);
}



#include <rpc/rpc.h>
#include "statusServer.h"


bool_t
xdr_requestCodes(xdrs, objp)
	XDR *xdrs;
	requestCodes *objp;
{
	if (!xdr_double(xdrs, &objp->utsec)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->interval)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_antDataDef(xdrs, objp)
	XDR *xdrs;
	antDataDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->isvalid, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->antennaStatus)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->trackStatus)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->rx, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->sourceName, 34, sizeof(char), xdr_char)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ra_j2000)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec_j2000)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->hour_angle)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->rightascension)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->declination)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->velocity)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->velocity_type)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->sol_sys_flag)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->planet_size)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tjd)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->utc)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dut)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->lst)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->actual_az)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->actual_el)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->azoff)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->eloff)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->az_tracking_error)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->el_tracking_error)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->delta_ra)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->delta_dec)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tsys)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tsys_rx2)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->project_id)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->obstype)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->utsec)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tiltx)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tilty)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->pmdaz)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->pmdel)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->refraction)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->chopper_x)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->chopper_y)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->chopper_z)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->chopper_angle)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ambient_load_temperature)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->N)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->Tamb)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->pressure)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->humid)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->windSpeed)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->windDir)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_chunkDef(xdrs, objp)
	XDR *xdrs;
	chunkDef *objp;
{
	if (!xdr_double(xdrs, &objp->centerfreq)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_blockDef(xdrs, objp)
	XDR *xdrs;
	blockDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->chunk, 4, sizeof(chunkDef), xdr_chunkDef)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_sidebandDef(xdrs, objp)
	XDR *xdrs;
	sidebandDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->block, 6, sizeof(blockDef), xdr_blockDef)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_receiverDef(xdrs, objp)
	XDR *xdrs;
	receiverDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->sideband, 2, sizeof(sidebandDef), xdr_sidebandDef)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_frequenciesDef(xdrs, objp)
	XDR *xdrs;
	frequenciesDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->receiver, 2, sizeof(receiverDef), xdr_receiverDef)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_lODataDef(xdrs, objp)
	XDR *xdrs;
	lODataDef *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->loStatus, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->sideband, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->skyFrequency, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->frequencyOffset, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->velocityOffset, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->restFrequency, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->pLLMultiple, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->pLLSideband, 2, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->mRGFrequency, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->vType)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->vCatalog)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->vEarth)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->vNoEarth)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->vRadial)) {
		return (FALSE);
	}
	if (!xdr_frequenciesDef(xdrs, &objp->frequencies)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_dDSDataDef(xdrs, objp)
	XDR *xdrs;
	dDSDataDef *objp;
{
	if (!xdr_int(xdrs, &objp->ddsStatus)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ha)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ra)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->lst)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->trackingFrequency, 2, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->x, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->y, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->z, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->u, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->v, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->w, 11, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->UT1MinusUTC)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dayFraction)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_info(xdrs, objp)
	XDR *xdrs;
	info *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->antlist, 11, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->antavg, 11, sizeof(antDataDef), xdr_antDataDef)) {
		return (FALSE);
	}
	if (!xdr_lODataDef(xdrs, &objp->loData)) {
		return (FALSE);
	}
	if (!xdr_dDSDataDef(xdrs, &objp->DDSdata)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->tau225)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_elHistory(xdrs, objp)
	XDR *xdrs;
	elHistory *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->ant1, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant3, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant4, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant5, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant6, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant7, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant8, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_azHistory(xdrs, objp)
	XDR *xdrs;
	azHistory *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->ant1, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant3, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant4, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant5, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant6, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant7, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant8, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_tsysHistory(xdrs, objp)
	XDR *xdrs;
	tsysHistory *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->ant1, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant3, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant4, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant5, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant6, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant7, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant8, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant1_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant2_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant3_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant4_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant5_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant6_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant7_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant8_rx2, 300, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_sourceName(xdrs, objp)
	XDR *xdrs;
	sourceName *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->name, 35, sizeof(char), xdr_char)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_sourceHistory(xdrs, objp)
	XDR *xdrs;
	sourceHistory *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->ant1, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant2, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant3, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant4, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant5, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant6, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant7, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ant8, 300, sizeof(sourceName), xdr_sourceName)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_history(xdrs, objp)
	XDR *xdrs;
	history *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->combiner, 11, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->m3Door, 11, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ambientLoadTemperature, 11, sizeof(float), xdr_float)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->temperature)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->windSpeed)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->windDirection)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->humidity)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->pressure)) {
		return (FALSE);
	}
	if (!xdr_u_int(xdrs, &objp->weatherTimestamp)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->time, 300, sizeof(u_long), xdr_u_long)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->scanNo, 300, sizeof(u_int), xdr_u_int)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->dTime, 300, sizeof(double), xdr_double)) {
		return (FALSE);
	}
	if (!xdr_elHistory(xdrs, &objp->el)) {
		return (FALSE);
	}
	if (!xdr_azHistory(xdrs, &objp->az)) {
		return (FALSE);
	}
	if (!xdr_tsysHistory(xdrs, &objp->tsys)) {
		return (FALSE);
	}
	if (!xdr_sourceHistory(xdrs, &objp->source)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_historyRequest(xdrs, objp)
	XDR *xdrs;
	historyRequest *objp;
{
	if (!xdr_int(xdrs, &objp->command)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ddsCoordInfo(xdrs, objp)
	XDR *xdrs;
	ddsCoordInfo *objp;
{
	if (!xdr_double(xdrs, &objp->ra_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ra_cat_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec_cat_disp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_ccdHeader(xdrs, objp)
	XDR *xdrs;
	ccdHeader *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->filename, 100, sizeof(char), xdr_char)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->sourcename, 34, sizeof(char), xdr_char)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->sptype, 10, sizeof(char), xdr_char)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tjd_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->lst_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->utc_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ra_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->ra_cat_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->dec_cat_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->az_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->el_disp)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->azoff)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->eloff)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->az_actual)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->el_actual)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->magnitude)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->pmdaz)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->pmdel)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->pressure)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->temperature)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->humidity)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->windspeed)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->winddirection)) {
		return (FALSE);
	}
	if (!xdr_short(xdrs, &objp->bias)) {
		return (FALSE);
	}
	if (!xdr_short(xdrs, &objp->gain)) {
		return (FALSE);
	}
	if (!xdr_short(xdrs, &objp->integration)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tiltx)) {
		return (FALSE);
	}
	if (!xdr_double(xdrs, &objp->tilty)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->status)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_antennaQuery(xdrs, objp)
	XDR *xdrs;
	antennaQuery *objp;
{
	if (!xdr_int(xdrs, &objp->antennaNumber)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_sSStatus(xdrs, objp)
	XDR *xdrs;
	sSStatus *objp;
{
	if (!xdr_int(xdrs, &objp->returnCode)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_antennaList(xdrs, objp)
	XDR *xdrs;
	antennaList *objp;
{
	if (!xdr_int(xdrs, &objp->onLine)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->bad)) {
		return (FALSE);
	}
	if (!xdr_vector(xdrs, (char *)objp->ants, 11, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_flagChange(xdrs, objp)
	XDR *xdrs;
	flagChange *objp;
{
	if (!xdr_vector(xdrs, (char *)objp->antennas, 11, sizeof(int), xdr_int)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->global)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->condition)) {
		return (FALSE);
	}
	if (!xdr_float(xdrs, &objp->value)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->action)) {
		return (FALSE);
	}
	return (TRUE);
}



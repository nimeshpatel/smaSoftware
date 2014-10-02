/*
 * $Log: mkcontDetectorInfo.c,v $
 * Revision 1.4  2005/07/08 19:49:00  rwilson
 * change address of Acromag for antennas 9 and 10
 *
 * Revision 1.3  2004/06/30 17:42:26  rwilson
 * changes for testing in ant 3
 *
 * Revision 1.2  2003/03/20 14:15:19  rwilson
 * ContDetector works as driver with test setup
 *
 * Revision 1.1.1.1  2002/06/13 15:12:26  rwilson
 * Early non-working version
 *
 *
 * Driver for Acromag counter IP module as continuum detector integrator
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "contDetector.h"
#include "contDetectordrvr.h"

struct contDetectorInfo info = {
	(ACROMAG *)(VME_A16_VIRT_ADDRESS + ACROMAG_CARRIER_BASE +
		ACROMAG_MODULE * IP_MODULE_OFFSET),
	INT_VECTOR
};

int main(int argc, char *argv[]) {
    char *hp;

    if((hp = getenv("HOST")) == NULL) {
	fprintf(stderr,
		"No host environmental variable found, assuming acc1-9\n");
	hp = "acc1";
    }
    if(strcmp("acc9", hp) == 0) {
	info.cntr = (ACROMAG *)(VME_A16_VIRT_ADDRESS + 
	    ACC9_ACROMAG_CARRIER_BASE +
	    ACC9_ACROMAG_MODULE * IP_MODULE_OFFSET);
    } else if(strcmp("acc10", hp) == 0) {
	info.cntr = (ACROMAG *)(VME_A16_VIRT_ADDRESS + 
	    ACC10_ACROMAG_CARRIER_BASE +
	    ACC10_ACROMAG_MODULE * IP_MODULE_OFFSET);
    }
    write(1, &info, sizeof(info));
    return(0);
}

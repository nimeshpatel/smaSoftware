/*
 * $Log: contDetector.h,v $
 * Revision 1.5  2005/07/08 19:56:36  rwilson
 * Get rid of cambridge acc2
 *
 * Revision 1.4  2005/03/30 16:55:51  rwilson
 * change ioctl definitions
 *
 * Revision 1.3  2004/11/19 16:01:51  rwilson
 * if statements for CAMBRIDGE_ACC2
 *
 * Revision 1.2  2003/03/20 14:34:41  rwilson
 * driver works
 *
 * Revision 1.1.1.1  2002/06/13 15:12:26  rwilson
 * Early non-working version
 *
 *
 * Continuum detector device interface.
 */

/* ioctl codes for the continuum detector */
#define IOCTL_BASE	256

/* Turn debugging on or off.  */
#define CONTDET_CLRDEBUG	(IOCTL_BASE + 0)
#define CONTDET_SETDEBUG	(IOCTL_BASE + 1)

#define CONTDET_RESETCARD	(IOCTL_BASE + 2)


/* Set the mode and integration time */
#define CONTDET_SETMODE	(IOCTL_BASE + 3)
#define CONTDET_GETMODE	(IOCTL_BASE + 4)

/* Release the wait for an integration to complete */
#define CONTDET_RELEASEWAIT	(IOCTL_BASE + 5)

/* Defs for the IOCTL argument */

enum mode { UNDEF_MODE = 0, IND, CHOP, IND_MULTIREAD, TIMING_TEST};

typedef struct {
	enum mode mode;
	int intTime;
} contDetectorioc_t;

/* Structure for integration results. */

typedef struct {
	int lowRx, highRx;	/* Counts of the two V/Fs */
	int timer;		/* final reading of counter 6 */
} contDetector_result_t;

/* deltet.f -- translated by f2c (version 19940927).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* $Procedure      DELTET ( Delta ET, ET - UTC ) */
/* Subroutine */ int deltet_(epoch, eptype, delta, eptype_len)
doublereal *epoch;
char *eptype;
doublereal *delta;
ftnlen eptype_len;
{
    /* Initialized data */

    static char missed[20*5+1] = "DELTET/DELTA_T_A, # DELTET/K, #         DE\
LTET/EB, #        DELTET/M, #         DELTET/DELTA_AT, #  ";

    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Builtin functions */
    integer s_cmp();
    double d_nint(), sin();

    /* Local variables */
    static char type[4];
    static integer i;
    static doublereal k, m[2];
    static integer n;
    static doublereal dleap[400]	/* was [2][200] */;
    extern /* Subroutine */ int chkin_();
    static integer nleap;
    extern /* Subroutine */ int ucase_(), errch_();
    static doublereal leaps, ettai;
    static logical found[5];
    static doublereal ea, eb, ma, et;
    extern /* Subroutine */ int sigerr_(), chkout_(), setmsg_();
    extern logical return_();
    extern /* Subroutine */ int rtpool_();
    static doublereal dta, aet;

/* $ Abstract */

/*     Return the value of Delta ET (ET-UTC) for an input epoch. */

/* $ Required_Reading */

/*     TIME */
/*     KERNEL */

/* $ Keywords */

/*     TIME */

/* $ Declarations */
/* $ Brief_I/O */

/*      VARIABLE  I/O  DESCRIPTION */
/*      --------  ---  -------------------------------------------------- 
*/
/*      EPOCH      I   Input epoch (seconds past J2000). */
/*      EPTYPE     I   Type of input epoch ('UTC' or 'ET'). */
/*      DELTA      O   Delta ET (ET-UTC) at input epoch. */

/* $ Detailed_Input */

/*      EPOCH       is the epoch at which Delta ET is to be computed. */
/*                  This may be either UTC or ephemeris seconds past */
/*                  J2000, as specified by EPTYPE. */

/*      EPTYPE      indicates the type of input epoch. It may be either */
/*                  of the following: */

/*                     'UTC'    input is UTC seconds past J2000. */
/*                     'ET'     input is ephemeris seconds past J2000. */


/* $ Detailed_Output */

/*      DELTA       is the value of */

/*                     Delta ET = ET - UTC */

/*                  at the input epoch. This is added to UTC to give */
/*                  ET, or subtracted from ET to give UTC. The routine */
/*                  is reversible: that is, given the following calls, */

/*                     CALL DELTET ( UTC,      'UTC', DEL1 ) */
/*                     CALL DELTET ( UTC+DEL1, 'ET',  DEL2 ) */

/*                  the expression */

/*                     ( DEL1 .EQ. DEL2 ) */

/*                  is always true. */

/* $ Parameters */

/*     None. */

/* $ Exceptions */

/*     1) If the input epoch is not recognized, the error */
/*        SPICE(INVALIDEPOCH) is signalled. */

/*     2) If the variables necessary for the computation of DELTA */
/*        have not been loaded into the kernel pool, the error */
/*        SPICE(KERNELVARNOTFOUND) is signalled. */

/* $ Files */

/*      None. */

/* $ Particulars */

/*      The constants necessary for computing the offset are taken */
/*      from the kernel pool, where they are assumed to have been */
/*      loaded from a kernel file. */

/*      The tables are consulted to determine the number of leap seconds 
*/
/*      preceding the input epoch. Also, an approximation to the periodic 
*/
/*      yearly variation (which has an amplitude of just under two */
/*      milliseconds) in the difference between ET and TAI (Atomic Time) 
*/
/*      is computed. The final value of Delta ET is given by */

/*            Delta ET = ( ET - TAI ) + leap seconds */

/* $ Examples */

/*      The following example shows how DELTET may be used to convert */
/*      from UTC seconds past J2000 to ephemeris seconds past J2000. */

/*            CALL DELTET ( UTCSEC, 'UTC', DELTA ) */
/*            ET = UTCSEC + DELTA */

/*      The following example shows how DELTET may be used to convert */
/*      from ephemeris seconds past J2000 to UTC seconds past J2000. */

/*            CALL DELTET ( ET, 'ET', DELTA ) */
/*            UTCSEC = ET - DELTA */

/*      See the TIME required reading for further examples. */

/* $ Restrictions */

/*      The routines UTC2ET and ET2UTC are preferred for conversions */
/*      between UTC and ET. This routine is provided mainly as a utility 
*/
/*      for UTC2ET and ET2UTC. */

/*      The kernel pool containing leapseconds and relativistic terms */
/*      MUST be loaded prior to calling this subroutine. Examples */
/*      demonstrating how to load a kernel pool are included in the */
/*      Required Reading file TIME.REQ and in the "C$ Examples" */
/*      section of this header. For more general information about */
/*      kernel pools, please consult the Required Reading file */
/*      KERNEL.REQ. */

/* $ Author_and_Institution */

/*      W.M. Owen       (JPL) */
/*      I.M. Underwood  (JPL) */

/* $ Literature_References */

/*      Astronomical Almanac. */

/* $ Version */

/* -     SPICELIB Version 1.0.1, 10-MAR-1992 (WLT) */

/*         Comment section for permuted index source lines was added */
/*         following the header. */

/* -     SPICELIB Version 1.0.0, 31-JAN-1990 (WMO) (IMU) */

/* -& */
/* $ Index_Entries */

/*     difference between ephemeris time and utc */

/* -& */
/* $ Revisions */

/* -     Beta Version 1.1.0, 06-OCT-1988 (IMU) */

/*         Tim Colvin of Rand noticed that times returned by UTC2ET */
/*         and TPARSE differed by one second. Upon closer inspection, */
/*         crack NAIF staff members deduced that in fact Mr. Colvin */
/*         had not loaded the kernel pool, and were surprised to learn */
/*         that no error had occurred. */

/*         Multiple FOUND flags and a bevy of new error messages were */
/*         implemented to cope with this unfortunate oversight. */

/* -& */

/*     SPICELIB functions */


/*     Local variables */


/*     Saved variables */


/*     Initial values */


/*     Standard SPICE error handling. */

    if (return_()) {
	return 0;
    } else {
	chkin_("DELTET", 6L);
    }

/*     Convert the epoch type to uppercase, to simplify comparisons. */

    ucase_(eptype, type, eptype_len, 4L);

/*     Extract the necessary constants from the kernel pool. */
/*     Leap seconds and their epochs are interleaved in DELTA_AT. */

/*     DLEAP(1,i) is the number of leap seconds at DLEAP(2,i) UTC */
/*     seconds past J2000. */

    rtpool_("DELTET/DELTA_T_A", &n, &dta, found, 16L);
    rtpool_("DELTET/K", &n, &k, &found[1], 8L);
    rtpool_("DELTET/EB", &n, &eb, &found[2], 9L);
    rtpool_("DELTET/M", &n, m, &found[3], 8L);
    rtpool_("DELTET/DELTA_AT", &nleap, dleap, &found[4], 15L);
    nleap /= 2;
    if (! (found[0] && found[1] && found[2] && found[3] && found[4])) {
	setmsg_("The following, needed to compute Delta ET (ET - UTC), could\
 not be found in the kernel pool: #", 94L);
	for (i = 1; i <= 5; ++i) {
	    if (! found[i - 1]) {
		errch_("#", missed + (i - 1) * 20, 1L, 20L);
	    }
/* L50001: */
	}
	errch_(", #", ".", 3L, 1L);
	sigerr_("SPICE(KERNELVARNOTFOUND)", 24L);
	chkout_("DELTET", 6L);
	return 0;
    }

/*     There are two separate quantities to be determined. First, */
/*     the appropriate number of leap seconds. Second, the size of */
/*     the periodic term ET-TAI. */


/*     For epochs before the first leap second, return Delta ET at */
/*     the epoch of the leap second minus one second. */

    leaps = dleap[0] - 1;

/*     When counting leap seconds for UTC epochs, we can compare */
/*     directly against the values in DLEAP. */

    if (s_cmp(type, "UTC", 4L, 3L) == 0) {
	i__1 = nleap;
	for (i = 1; i <= i__1; ++i) {
	    if (*epoch >= dleap[(i << 1) - 1]) {
		leaps = dleap[(i << 1) - 2];
	    }
/* L50002: */
	}

/*     For ET epochs, things are a little tougher. In order to compare
 */
/*     the input epoch against the epochs of the leap seconds, we need
 */
/*     to compute ET-TAI at each of the leap epochs. To make sure that
 */
/*     the computation is reversible, it is always done at the nearest
 */
/*     ET second (the "approximate ET", or AET). */

/*     There must be a hundred ways to do this more efficiently. */
/*     For now, we'll settle for one that works. */

    } else if (s_cmp(type, "ET", 4L, 2L) == 0) {
	i__1 = nleap;
	for (i = 1; i <= i__1; ++i) {
	    if (*epoch > dleap[(i << 1) - 1]) {
		d__1 = dleap[(i << 1) - 1] + dta + dleap[(i << 1) - 2];
		aet = d_nint(&d__1);
		ma = m[0] + m[1] * aet;
		ea = ma + eb * sin(ma);
		ettai = k * sin(ea);
		et = dleap[(i << 1) - 1] + dta + dleap[(i << 1) - 2] + ettai;
		if (*epoch >= et) {
		    leaps = dleap[(i << 1) - 2];
		}
	    }
/* L50003: */
	}

/*     Uh, those are the only choices. */

    } else {
	setmsg_("Epoch type was #", 16L);
	errch_("#", type, 1L, 4L);
	sigerr_("SPICE(INVALIDEPOCH)", 19L);
	chkout_("DELTET", 6L);
	return 0;
    }

/*     Add the constant offset, leap seconds, and the relativistic term */
/*     (as before, computed at the nearest ET second). */

    if (s_cmp(type, "ET", 4L, 2L) == 0) {
	aet = d_nint(epoch);
    } else if (s_cmp(type, "UTC", 4L, 3L) == 0) {
	d__1 = *epoch + dta + leaps;
	aet = d_nint(&d__1);
    }
    ma = m[0] + m[1] * aet;
    ea = ma + eb * sin(ma);
    ettai = k * sin(ea);
    *delta = dta + leaps + ettai;
    chkout_("DELTET", 6L);
    return 0;
} /* deltet_ */


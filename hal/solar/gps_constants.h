#ifndef _GPS_CONSTANTS_H
#define _GPS_CONSTANTS_H

/*************************************************************************
 *
 * File Name: gps_constants.h
 *
 * Description: Define addresses for the Truetime 560-5618 GPS receiver.
 *
 *************************************************************************/

/* base address for the GPS receiver and time code readers */
#define GPS_BASE_ADDRESS	0x8000

/* configuration register 1 */
#define GPS_CONFIG_REG1		GPS_BASE_ADDRESS + 0x00A0

/* configuration register 2 */
#define GPS_CONFIG_REG2		GPS_BASE_ADDRESS + 0x00A2

/* position mode register */
#define GPS_POSITION_REG	GPS_BASE_ADDRESS + 0x00A4

/* local offset register */
#define GPS_LOCOFF_REG		GPS_BASE_ADDRESS + 0x00A6

/* rate output selection register */
#define GPS_RATE_REG		GPS_BASE_ADDRESS + 0x00A8

/* position update flag */
#define GPS_UPDATE_FLAG		GPS_BASE_ADDRESS + 0x00BE

/* self test word */
#define GPS_SELFTEST		GPS_BASE_ADDRESS + 0x00DE

/* coincidence time compare register 1 - 0x00E0 to 0x00EE */
#define GPS_COMPARE_REG1	GPS_BASE_ADDRESS + 0x00E0

/* coincidence time compare register 2 - 0x00F0 to 0x00FE */
#define GPS_COMPARE_REG2	GPS_BASE_ADDRESS + 0x00F0

/* a read to this address freezes the time into capture register 1 */
#define GPS_CAPTURE_REG1	GPS_BASE_ADDRESS + 0x0040
#define GPS_RELEASE_REG1	GPS_BASE_ADDRESS + 0x0044

/* interrupt control/vector registers */
#define GPS_INT1_CTL_REG	GPS_BASE_ADDRESS + 0x0000
#define GPS_INT2_CTL_REG	GPS_BASE_ADDRESS + 0x0002
#define GPS_INT3_CTL_REG	GPS_BASE_ADDRESS + 0x0004
#define GPS_INT4_CTL_REG	GPS_BASE_ADDRESS + 0x0006
#define GPS_INT1_VECT_REG	GPS_BASE_ADDRESS + 0x0008
#define GPS_INT2_VECT_REG	GPS_BASE_ADDRESS + 0x000A
#define GPS_INT3_VECT_REG	GPS_BASE_ADDRESS + 0x000C
#define GPS_INT4_VECT_REG	GPS_BASE_ADDRESS + 0x000E

#define GPS_REG1_WORD1		GPS_BASE_ADDRESS + 0x0042 /* msec,usec */
#define GPS_REG1_WORD2		GPS_BASE_ADDRESS + 0x0080 /* sec,msec */
#define GPS_REG1_WORD3		GPS_BASE_ADDRESS + 0x0082 /* hrs,mins */
#define GPS_REG1_WORD4		GPS_BASE_ADDRESS + 0x0084 /* stat,days */
#define GPS_REG1_WORD5		GPS_BASE_ADDRESS + 0x0086 /* years */

/* capture register 2 is frozen by an external event input pulse */
#define GPS_REG2_WORD1		GPS_BASE_ADDRESS + 0x0046 /* msec,usec */
#define GPS_REG2_WORD2		GPS_BASE_ADDRESS + 0x0088 /* sec,msec */
#define GPS_REG2_WORD3		GPS_BASE_ADDRESS + 0x008A /* hrs,mins */
#define GPS_REG2_WORD4		GPS_BASE_ADDRESS + 0x008C /* stat,days */
#define GPS_REG2_WORD5		GPS_BASE_ADDRESS + 0x008E /* years */

/* longitude, latitude, and elevation: available on receiver only */
#define GPS_LAT_WORD1		GPS_BASE_ADDRESS + 0x0090 /* deg */
#define GPS_LAT_WORD2		GPS_BASE_ADDRESS + 0x0092 /* min,sec */
#define GPS_LAT_WORD3		GPS_BASE_ADDRESS + 0x0094 /* N/S, csec */

#define GPS_LON_WORD1		GPS_BASE_ADDRESS + 0x0096 /* deg */
#define GPS_LON_WORD2		GPS_BASE_ADDRESS + 0x0098 /* min,sec */
#define GPS_LON_WORD3		GPS_BASE_ADDRESS + 0x009A /* E/W, csec */

#define GPS_ELE_WORD1		GPS_BASE_ADDRESS + 0x009C /* meters */
#define GPS_ELE_WORD2		GPS_BASE_ADDRESS + 0x009E /* meters */

/***************************************************************************/
/* define a structure for the GPS time */
 
struct gps_struct {
    /* raw data from the gps */
    unsigned short microseconds;
    unsigned short milliseconds;
    unsigned short seconds;
    unsigned short minutes;
    unsigned short hours;
    unsigned short days;
    unsigned short years;
    unsigned short status;
    /* calculated data */
    unsigned short months;      /* month of the year (1 to 12) */
    long today_msec;            /* milliseconds since midnight */
    };
typedef struct gps_struct gps_struct;
 
/*****************************************************************************/ 

#endif /* _GPS_CONSTANTS_H */

/*************************************************************************/

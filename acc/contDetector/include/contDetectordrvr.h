/*
 * $Log: contDetectordrvr.h,v $
 * Revision 1.5  2008/05/14 12:36:02  rwilson
 * Define acc 9&10 carrier bases
 *
 * Revision 1.4  2006/10/07 17:26:33  thunter
 * Change the IP address for acc9 and 10 in the .h file
 *
 * Revision 1.3  2004/11/19 16:01:52  rwilson
 * if statements for CAMBRIDGE_ACC2
 *
 * Revision 1.2  2003/03/20 14:34:41  rwilson
 * driver works
 *
 * Revision 1.1.1.1  2002/06/13 15:12:26  rwilson
 * Early non-working version
 *
 * Include file for continuum detector integrator driver using an Acromag
 * IP480-6 card.  User programs should just include iPEncoder.h.
 *
 * Use the virt address in a driver (or in user space in LynxOS 2.5).  Use
 * the PHYS address to map the board to a shared memory segment in user
 * space in LynxOS 3.1.0+
 */

#define VME_A16_PHYS_ADDRESS 0xefff0000 /* Physical address of short I/O page */
#define VME_A16_VIRT_ADDRESS 0xcfff0000 /* Virtual address of short I/O page */
#define ACROMAG_CARRIER_BASE 0xc000	/* Address of IP carrier in A16 space
					 * as set by its jumpers */
#define ACROMAG_MODULE 0
#define ACC9_ACROMAG_CARRIER_BASE 0xb200
#define ACC9_ACROMAG_MODULE 0
#define ACC10_ACROMAG_CARRIER_BASE 0x7000
#define ACC10_ACROMAG_MODULE 0
#define IP_MODULE_OFFSET 0x100
#define INT_VECTOR 110

/* Define Bits in the Counter Control Register */
#define DISABLED_MODE 0
#define WATCH_DOG_MODE 3
#define PULSE_WIDTH_MODE 5
#define OUTPUT_ACTIVE_HIGH 8
#define INPUT_ACTIVE_HIGH 0x10
#define EXT_TRIGGER_LOW_HIGH 0x20
#define EXT_TRIGGER_SOURCE 0x40
#define INTERRUPT_ENABLE 0x80
#define USE_32_BITS 0x100
#define EXT_CLOCK 0x600
#define EXT_TRIGGERED_WD_LOAD 0x800
#define DEBOUNCE_INPUT 0x1000

/* Structure to define the IP module's registers.  The first part is for data
 * and the second is the ID PROM.
 */

typedef struct {
	volatile unsigned short cntl[6];	/* Counter Control R/W */
	volatile unsigned short rb[6];		/* Counter Readback R */
	volatile unsigned short initValue[6];	/* Counter Constant W */
	volatile unsigned short triggerCntl;	/* Soft Trigger Bits 5..0 */
	volatile unsigned short softReset;	/* Software Reset Bit 0, W */
	volatile unsigned short rbLatch;	/* Readback Latch Bit 0, R/W */
	volatile unsigned short intPend;	/* Interrupt Pending/Clear
						 * Bits 13..8, R/W */
	volatile unsigned short intVect;	/* Interrupt Vector
						 * Bits 7..0, R/W */
	/* Space to the ID PROM which is at an offset of 128 */
	volatile unsigned short dummy1[64 - 23];
	volatile unsigned short ascii[4];    /* {'I', 'P', 'A', 'C'} */
	volatile unsigned short manufacturer;	/* Acromag = 0xa3 */
	volatile unsigned short modelNo;	/* 0x16 */
	volatile unsigned short unused[4];
	volatile unsigned short idBytes;	/* 0C = # of ID Space Bytes */
	volatile unsigned short crc;		/* 1E = ID Space CRC */
	volatile unsigned short dummy2[64 - 12];/* fill out 256 byte block */
} ACROMAG;

/* Unidig defs for reading PMAC output */

#if CAMBRIDGE_ACC2
#define UNIDIG_READ_ADDRESS (UNIDIG_CARRIER_BASE + IP_MODULE_OFFSET * \
	UNIDIG_MODULE + UD_READ_REGISTER)
#define FREE_RUN 1
#define CHOPPER_IN_POSITION 2
#define POSITION_1 4
#define POSITION_2 8
#endif

struct contDetectorInfo {
	ACROMAG *cntr;				/* Address of Acromag module */
	unsigned int intVect;			/* Acromag's interrupt vector */
};

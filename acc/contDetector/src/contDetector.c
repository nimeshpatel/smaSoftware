/* Version of the driver for temporarily measuring the timing of the
 * chopper for autocorrelation.
 * RWW 3/23/04
 */
/* contDetector.c
 * $Log: contDetector.c,v $
 * Revision 1.11  2008/06/02 20:48:57  rwilson
 * enable->restore
 *
 * Revision 1.10  2005/07/08 19:46:29  rwilson
 * add to status instead of or
 *
 * Revision 1.9  2005/04/15 15:16:02  rwilson
 * Put in release IOCTL, using swait instaed of tswait
 *
 * Revision 1.8  2005/03/10 18:47:32  rwilson
 * temporary code to report on tswait state
 *
 * Revision 1.7  2004/11/24 21:26:48  rwilson
 * why
 *
 * Revision 1.6  2004/11/19 15:54:51  rwilson
 * Set up counters every time
 *
 * Revision 1.5  2004/06/30 17:42:25  rwilson
 * changes for testing in ant 3
 *
 * Revision 1.4  2004/03/23 15:13:52  rwilson
 * removed executable file from archivs
 *
 * Revision 1.3  2003/04/01 14:50:12  rwilson
 * Increase timeout
 *
 * Revision 1.2  2003/03/20 14:15:19  rwilson
 * ContDetector works as driver with test setup
 *
 * Revision 1.1.1.1  2002/06/13 15:12:26  rwilson
 * Early non-working version
 *
 *
 * Driver for the continuum detector using an Acromag counter
 * ip module to count the output of the v/f converters.
 *
 * Bob Wilson, May 2002
 */

#define __LYNXOS 1

#include <sys/types.h>
#include <kernel.h>
extern int iointset _AP((int vector, void (*function)(void *), char *argument));
/* #include <port_ops_ppc.h> The defs here are not correct */
#include <dldd.h>
/* #include <kern_proto.h> This causes lots of errors, so the needed defs
 * are here */
extern void cprintf(char *, ...);
extern char *sysbrk(long);
extern int sysfree(char *, long);
extern int tswait(int *, int, int);
extern int ssignal(int *);
extern int sreset(int *);
extern int swait(int *, int);
#include <semaphore.h>

#include <sys/ioctl.h>
#include <sys/file.h>
#include <errno.h>
#include <limits.h>

#include "contDetector.h"
#include "contDetectordrvr.h"

/****************************/
/* driver statics structure */
/****************************/
struct contDetectorStatics {
	ACROMAG *cntr;
	int intVect;
	int waitSem;
	int debug;
	int open;
	int intTime;
	enum mode mode;
	int status;
	int waitReleased;
};
#define dprintf  if(s->debug) cprintf

/* contDetector.c */
void cSetCntl(ACROMAG *cntr);
void iSetCntl(ACROMAG *cntr);
void intHndlr(struct contDetectorStatics *s);

/**********************/
/* open() entry point */
/**********************/
int contDetectorOpen(struct contDetectorStatics *s, int devno, struct file *f) {

	if((f->access_mode ^ FREAD) != 0) {
	    pseterr(EINVAL);
	    return(SYSERR);
	}
	/* Only allow one process to open at any time */
	if(s->open) {
	    pseterr(EBUSY);
	    return(SYSERR);
	}
	s->open = TRUE;
	dprintf ("contDetectorOpen: opened\r\n");
	return (OK);
}

/***********************/
/* close() entry point */
/***********************/
int contDetectorClose(struct contDetectorStatics *s, int devno, struct file *f) {

	/* we only get called when the last process closes the device */
	s->open = FALSE;

	dprintf("contDetectorClosed\r\n");
	return (OK);
}

/**********************/
/* read() entry point */
/**********************/

int contDetectorRead(struct contDetectorStatics *s, struct file *f, char *buff,
		int count) {
	contDetector_result_t *re = (contDetector_result_t *)buff;
	int i, stat, ps;

	if(s->mode == IND) {
/* test to see if this helps prevent lock ups */
iSetCntl(s->cntr);
	    s->cntr->initValue[4] = s->intTime >> 16;
	    s->cntr->initValue[5] = s->intTime & 0xffff;
	    disable(ps);
	    s->cntr->triggerCntl = 0x2a;
	    s->status = 1;
	    stat = swait(&s->waitSem, SEM_SIGABORT);
/*	    stat = tswait(&s->waitSem, SEM_SIGABORT, 2 + s->intTime/10000); */
	    s->status = 0;
	    restore(ps);
	} else if(s->mode == CHOP) {
	    s->cntr->triggerCntl = 0x2a;
	    for(i = 0; i < 200 && s->cntr->rb[5] == 0; i++) {
		s->status = 2;
		stat = tswait(&s->waitSem, SEM_SIGABORT, 1);
		s->status = 0;
	    }
	    s->status = 3;
	    stat = tswait(&s->waitSem, SEM_SIGABORT, 200);
	    s->status = 0;
	} else {
	    return(0);
	}
	if(stat==TSWAIT_NOTOUTS) {
	    dprintf("contDetector: "
              "tswait() reports no timeouts available\r\n");
	    pseterr(EAGAIN);
	    return(SYSERR);
	} else if(stat==TSWAIT_ABORTED) {
	    dprintf("contDetector: "
              "tswait() released on signal\r\n");
	    pseterr(EINTR);
	    return(SYSERR);
	} else if(stat==TSWAIT_TIMEDOUT || s->waitReleased != 0) {
	    dprintf("contDetector: "
              "tswait() timed out before semaphore was posted\r\n");
	    s->waitReleased = 0;
	    pseterr(ETIMEDOUT);
	    return(SYSERR);
	}
	re->lowRx = (s->cntr->rb[0] << 16) | s->cntr->rb[1];
	re->highRx = (s->cntr->rb[2] << 16) | s->cntr->rb[3];
	re->timer = (s->cntr->rb[4] << 16) | s->cntr->rb[5];
	return(sizeof(contDetector_result_t));
}

/***************************************************/
/* write() and select entry points not implemented */
/***************************************************/

int contDetectorInvalid(void) {
	pseterr(EPERM);
	return(SYSERR);
}

/***********************/
/* ioctl() entry point */
/***********************/
int contDetectorioctl(struct contDetectorStatics *s, struct file *f,
	int command, contDetectorioc_t *argp) {
	int ps;

	switch(command) {
	case CONTDET_CLRDEBUG:
	    s->debug = FALSE;
	    break;
	case CONTDET_SETDEBUG:
	    s->debug = TRUE;
	    break;
	case CONTDET_RESETCARD:
	    break;
	case CONTDET_SETMODE:
	    s->cntr->softReset = 1;
	    s->mode = argp->mode;
	    s->intTime = argp->intTime;
	    s->waitSem = 0;
	    if(s->mode == IND) {
		iSetCntl(s->cntr);
	    } else if(s->mode == CHOP) {
		cSetCntl(s->cntr);
	    }
	    break;
	case CONTDET_GETMODE:
#if 1
	    /* temporary use for finding out what happens when syncdet2
	     * is stuck */
	    argp->mode = s->status;
	    argp->intTime = scount(&s->waitSem);
#else
	    argp->mode = s->mode;
	    argp->intTime = s->intTime;
#endif
	    break;
	case CONTDET_RELEASEWAIT:
	    if(scount(&s->waitSem) < 0) {
		disable(ps);
		s->waitReleased = 1;
		sreset(&s->waitSem);
		restore(ps);
	    } else {
		pseterr(ESRCH);
		return(SYSERR);
	    }
	    break;
	default:
	    pseterr(EINVAL);
	    return (SYSERR);
	}
	return(OK);
}


/***********************/
/* install entry point */
/***********************/

char *contDetectorInstall(struct contDetectorInfo *info) {
	static unsigned char idTest[] = {'I', 'P', 'A', 'C', 0xa3, 0x16, 0,
		0, 0, 0, 0xc, 0x1e};
	struct contDetectorStatics *s;
	int i;

	cprintf("Entering install of contDetector device\r\n");
	for(i = 0; i < sizeof(idTest); i++) {
	    if((info->cntr->ascii[i] & 0xff) != idTest[i]) {
		pseterr(ENXIO);
		return( (char *)SYSERR );
	    }
	}

	/* initialize statics structure */
	s = (struct contDetectorStatics *)sysbrk((long)sizeof(*s));
	if( s == NULL) {
	    pseterr(ENOMEM);
	    return( (char *)SYSERR );
	}

	s->cntr = info->cntr;
	s->debug = FALSE;
	s->open = FALSE;
	s->mode = UNDEF_MODE;
	s->cntr->softReset = 1;
	s->waitSem = 0;
	s->waitReleased = 0;
	s->intVect = info->intVect;
	s->cntr->intVect = info->intVect;
	iointset(s->intVect, (void (*)(void *))intHndlr, (void *)s);

	dprintf("contDetector driver installed\r\n");
	return ((char *)s);
}

/*************************/
/* uninstall entry point */
/*************************/
int contDetectorUninstall(struct contDetectorStatics *s) {

	dprintf("contDetectorUninstall: freeing memory for statics struct\r\n");
	iointclr(s->intVect);
	sysfree((char *)s, (long)sizeof(*s));
	return (OK);
}

/************************/
/* specify entry points */
/************************/
struct dldd entry_points = {
	contDetectorOpen,
	contDetectorClose,
	contDetectorRead,
	contDetectorInvalid,		/* write() entry point */
	contDetectorInvalid,		/* select() entry point */
	contDetectorioctl,
	contDetectorInstall,
	contDetectorUninstall,
	(char *)0
};

/* Subroutine to set the counter control registers for independent (32-bit)
 * operation */
void iSetCntl(ACROMAG *cntr) {
	int i;

	cntr->cntl[5] = WATCH_DOG_MODE | OUTPUT_ACTIVE_HIGH |
		INTERRUPT_ENABLE | USE_32_BITS;
	cntr->cntl[1] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	cntr->cntl[3] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	for(i = 0; i < 4; i++) {
	    cntr->initValue[i] = 0;
	}
}

/* Subroutine to set the counter control registers for chopper (16-bit)
 * operation */
void cSetCntl(ACROMAG *cntr) {
	int i;

	cntr->cntl[5] = PULSE_WIDTH_MODE | INTERRUPT_ENABLE  | USE_32_BITS;
	cntr->cntl[1] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK | DEBOUNCE_INPUT;
	cntr->cntl[3] = PULSE_WIDTH_MODE | INPUT_ACTIVE_HIGH | USE_32_BITS |
		EXT_CLOCK;
	for(i = 0; i < 6; i++) {
	    cntr->initValue[i] = 0;
	}
}

void intHndlr(struct contDetectorStatics *s) {
	s->cntr->intPend = 0;
	s->status += 8 + s->waitSem * 16;
	s->waitReleased = 0;
	ssignal(&s->waitSem);
}

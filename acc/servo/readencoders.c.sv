#include <sys/types.h>
#include <errno.h>
/* If this is put ahead of math.h and sys/types.h, it hides some definitions */
#define _POSIX_SOURCE 1
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include "smadaemon.h"
#include "tsshm.h"
#include "encoders.h"
#include "servo.h"

int heid_fd, acc_fd;		/* fd's for ACC and Heidenhain encoders */
enum ENCTYPE elEncType, azEncType;
int azEncoderOffset, elEncoderOffset;
int azEncoderReversed, elEncoderReversed;

/* readencoders.c */
static void GetEncoderTypes(void);

void OpenEncoders(void) {
	endatioc_t arg;

	GetEncoderTypes();
	if(azEncType == HEIDENHAIN || elEncType == HEIDENHAIN) {
	    if((heid_fd = open("/dev/endat0", O_RDONLY, 0)) < 0) {
		fprintf(stderr,
			"servo quitting: Heidenhain encoder(s) not found\n");
		exit(QUIT_RTN);
	    }
	    /* Now reset the interface cdard and the encoders */
	    arg.enc1 = (azEncType == HEIDENHAIN);
	    arg.enc2 = (elEncType == HEIDENHAIN);
	    ioctl(heid_fd, EIOCRESETCARD, &arg);
	    ioctl(heid_fd, EIOCRESETENC, &arg);
	    ioctl(heid_fd, EIOCGETOFFS, &arg);
	    if(arg.enc1) {
		azEncoderOffset = arg.offs.azOffset;
		azEncoderReversed = arg.offs.revAz;
	    }
	    if(arg.enc2) {
		elEncoderOffset = arg.offs.elOffset;
		elEncoderReversed = arg.offs.revEl;
	    }
	} else {
	    heid_fd = -1;
	}
	if(azEncType == ACC || elEncType == ACC) {
	    if((acc_fd = open("/dev/encoder0", O_RDONLY, 0)) < 0) {
		fprintf(stderr, "servo quitting: Acc encoder(s) not found\n");
		exit(QUIT_RTN);
	    }
	    ioctl(acc_fd, EIOCGETOFFS, &arg);
	    if(azEncType == ACC) {
		azEncoderOffset = arg.offs.azOffset;
		azEncoderReversed = arg.offs.revAz;
	    }
	    if(elEncType == ACC) {
		elEncoderOffset = arg.offs.elOffset;
		elEncoderReversed = arg.offs.revEl;
	    }
	} else {
	    acc_fd = -1;
	}
}

void ReadEncoders(int *encAzp, int *encElp) {
    struct enc_result heid_enc;
    struct enc_result acc_enc;
	/* Time is done, so read the encoders, etc.. */
	if(heid_fd >= 0) {
	    static short int oldAzStatus = 1;
	    static short int oldElStatus = 1;

	    read(heid_fd, &heid_enc, sizeof(heid_enc));
	    if(azEncType == HEIDENHAIN) {
		*encAzp = ENC_TO_MAS * heid_enc.az;
		if(heid_enc.statusAz != oldAzStatus) {
		    fprintf(stderr, "Az encoder status 0x%02x\n",
			    heid_enc.statusAz);
		    oldAzStatus = heid_enc.statusAz;
		}
	    }
	    if(elEncType == HEIDENHAIN) {
		*encElp = ENC_TO_MAS * heid_enc.el;
		if(heid_enc.statusEl != oldElStatus) {
		    fprintf(stderr, "El encoder status 0x%02x\n", heid_enc.statusEl);
		    oldElStatus = heid_enc.statusEl;
		}
	    }
	}
}

static void mkerror(void) {
    fprintf(stderr, "Bad or missing \"TYPES\" line in encoders.conf\n");
    exit(QUIT_RTN);
}

static void GetEncoderTypes(void) {
    FILE *configfp;
    char *cp, line[80];

    if((configfp = fopen("/instance/configFiles/encoders.conf", "r")) < 0) {
	perror("Opening encoders.conf");
	exit(QUIT_RTN);
    }
    do {
	if(fgets(line, sizeof(line), configfp) == NULL) {
	    mkerror();
	}
printf("%s", line);
    } while(strncmp(line, "TYPES", sizeof("TYPES")) != 0);
    if(line == NULL) {
    }
printf("%s", line);
    (void)strtok(line, " \t");
    if((cp = strtok(NULL, " \t")) != 0) {
    if(strncmp(cp, "HEID", 4 == 0)) {
	azEncType = HEIDENHAIN;
    } else if(strncmp(cp, "ACC", 3) == 0) {
	azEncType = ACC;
    } else {
	mkerror();
    }
    } else {
	exit(QUIT_RTN);
    }
    cp = strtok(NULL, " \t");
    if(strncmp(cp, "HEID", 4 == 0)) {
	elEncType = HEIDENHAIN;
    } else if(strncmp(cp, "ACC", 3) == 0) {
	elEncType = ACC;
    } else {
	mkerror();
    }
}
#ifdef USE_MAIN
int main(int argc, char *argv[]) {
    OpenEncoders();
    printf("AZ: type %d offset %d reversed %d   EL: type %d offset %d reversed %d\n", azEncType, azEncoderOffset, azEncoderReversed, elEncType, elEncoderOffset, elEncoderReversed);
    exit(0);
}
#endif /* USE_MAIN */

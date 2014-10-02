#include <math.h>
#include <sys/types.h>
#include <resource.h>
#include <unistd.h>
/* #define _POSIX_SOURCE */
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "patchPanelBits.h"
/* #include "iPUniDig_D.h" */

#define FULL_TEST 0

int writeMask = WRITE_BITS;
int readMask = READ_BITS;
int r = 20, t = 20000;		/* number of repeats, time to wait (usec) */

int main(int argc, char *argv[]) {
  int fd;
  int status;
  int data;
#if FULL_TEST
  int rbdata;
#endif

  fd = open("/dev/iPUniDig_D", O_RDWR);
  if (fd < 0) {
    printf("Error returned from open, errno = %d\n", errno);
    exit(-1);
  }
  argv++;
  for(;argc > 1; argc--, argv++) {
    if(argv[0][0] == '-') {
      if(argv[0][1] == 'h' || (argv[0][1] && argv[0][2] == 'h')) {
	printf("testUD will read and report the bits from /dev/iPUniDig_D\n"
		" giving -rn or -r n will cause it to do that n times\n"
		" giving -tn or -t n will change the delay between reads from\n"
		"   the default of 50000 usec to n usec.\n"
		);
	exit(0);
      } else if(argv[0][1] == 'r') {
        if(argv[0][2]) {
	  r = atoi(&argv[0][2]);
        } else if(--argc) {
  	  argv++;
  	  r = atoi(argv[0]);
        }
        if(r < 1 || r > 10000) {
  	printf("# repeats should be between 1 and 10000, not %d\n", r);
  	exit(1);
        }
      } else if(argv[0][1] == 't') {
        if(argv[0][2]) {
	  t = atoi(&argv[0][2]);
        } else if(--argc) {
  	  argv++;
  	  t = atoi(argv[0]);
        }
        if(t < 20000 || t > 100000000) {
  	printf("wait time should be between 20000 and 100000000, not %d\n", t);
  	exit(1);
        }
      }
    }
  }

#if FULL_TEST
  if ((status = ioctl(fd, IOCTL_SETIN, &readMask)) < 0) {
    printf("Error %d returned from 2nd ioctl call, errno = %d\n", status,
           errno);
    exit(-1);
  }

  if ((status = ioctl(fd, IOCTL_ENABLE_OUT, &writeMask)) < 0) {
    printf("Error %d returned from 3rd ioctl call, errno = %d\n", status,
           errno);
    exit(-1);
  }
#endif

  printf("Data bits from from /dev/iPUniDig_D: ");
  while(r) {
#if FULL_TEST
    if ((status = read(fd, (char *)(&rbdata), 3)) < 0) {
      printf("Error %d returned from 1st read, errno = %d\n", status,
  	   errno);
      exit(-1);
    }
#endif
  
    if ((status = read(fd, (char *)(&data), 4)) < 0) {
      printf("Error %d returned from 1st read, errno = %d\n", status,
  	   errno);
      exit(-1);
    }
    printf("0x%x\n", data);
    if(--r) usleep(t);
  }

#if FULL_TEST > 1
  rbdata &= writeMask;
  rbdata ^= 0x200;
  printf("Writing 0x%x\n", rbdata);
  if ((status = write(fd, (char *)(&rbdata), 4)) < 0) {
    printf("Error %d returned from write, errno = %d\n", status,
	   errno);
    exit(-1);
  }
  rbdata = 0;

  if ((status = read(fd, (char *)(&rbdata), 3)) < 0) {
    printf("Error %d returned from 1st read, errno = %d\n", status,
	   errno);
    exit(-1);
  }
  printf("Read data 0x%02x  Raw data 0x%06x  Write Mask = 0x%x\n",
	  data, rbdata & 0xffffff, writeMask);
#endif

  close(fd);
  return(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <smem.h>
#include "iP488.h" 
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

/* #define VMEBaseAddress 0xefff0000
#define SELECT 0x01
#define DATAREG 0x08
#define CONVERT 0x06
#define STROBE 0x3dff
#define FRSEL 0x0002
#define CSEL 0x0100
#define IP_THERM 0x0300
#define IF_BASE 0xa000 */

/* 
gcc -I../../includeFiles -I/usr/PowerPC/common/include lockin_test.c
*/

void initialize( void );
double convert_it( short hex_temp );

int quit, i, j;
int device = 9;
int gPIB, flag;
int count;
int dresponse;
char response[132];
char command[132];
char response1;
char ch;

main(int argc, char **argv)
{
if(argc<2)
        {
        printf("Usage: lockin <GPIB_ID> command\n");
        exit(0);
        }
/*	printf("%s %s %s\n", argv[0],argv[1],argv[2]); */
    device=atoi(argv[1]);
	sprintf(command,"%s",argv[2]);
/*	printf("%s\n", command); */
    quit = 0;


    initialize();
   
/* command response loop */
   write (gPIB, command, strlen(command));
/*   printf ("Written\n"); 
   printf("response ? (1/0): ");
   scanf ("%d", &flag); */
   if (strchr(argv[2],'?') != NULL) {
   count=read(gPIB,response,132);
   printf("read GPIB\n");
   printf("response: %d %s\n", count, response);
}
}


void initialize( void )
{

   flag = 1;

   gPIB = open("/dev/GPIB", O_RDWR);
/*   printf("The GPIB base address is 0x%x\n", gPIB); */

/*   ioctl(gPIB, RESET_CONTROLLER, &device); */


    if (gPIB < 0) {
    perror("open");
    exit(-1);
  }
  if (flag == 0) {
    printf("Resetting GPIB bus\n");
    if (ioctl(gPIB, RESET_CONTROLLER, &device) < 0) {
      perror("Zeroth ioctl");
      exit(-1);
    }
    exit(0);
  }
/*   printf("Setting device to %d\n", device); */
if (ioctl(gPIB, SELECT_DEVICE, &device) < 0) {
    perror("First ioctl");
    exit(-1);
  }
   flag = 0;
  if (flag == 1) {
    printf("Send Device Clear\n");
    if (ioctl(gPIB, DEVICE_CLEAR, &device) < 0) {
      perror("Second ioctl");
      exit(-1);
    }
  }
}

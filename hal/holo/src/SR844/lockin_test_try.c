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
#define IF_BASE 0xa000
*/

void initialize( void );

int quit, count;
int device = 8;
int gPIB, flag;
int dresponse;
char response[80];

main()
{
    quit = 0;
    initialize();

/* first command and response */
   printf("Enter your command, q for quit: ");
   scanf ("%s", response);
   printf ("%s\n", response);
        if (!(strcmp(response,"q"))) quit=1;

/* command response loop */
   while(quit != 1) {
   write (gPIB, response, strlen(response));
   printf ("Written\n");
   printf("response ? (1/0): ");
   scanf ("%d", &flag);
   if (flag == 1) {
   count=read(gPIB,response,80);
   printf("%d %s\n", count, response);
   }
   printf("Enter your command, q for quit: ");
   scanf ("%s", response);
   printf ("%s\n", response);
        if (!(strcmp(response,"q"))) quit=1;
}
}


void initialize( void )
{

   flag = 1;

   gPIB = open("/dev/GPIB", O_RDWR);
   printf("The GPIB base address is 0x%x\n", gPIB);

   ioctl(gPIB, RESET_CONTROLLER, &device);


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
   printf("Setting device to %d\n", device);
if (ioctl(gPIB, SELECT_DEVICE, &device) < 0) {
    perror("First ioctl");
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


    write (gPIB, "*IDN?", 5);
    read(gPIB,response,80);
    printf("%s\n", response);

    write (gPIB, "LIAS?0", 6);
    read(gPIB, response, 80);
    sscanf(response,"%d",&dresponse);
    printf("LOCK: %d\n",dresponse);

}

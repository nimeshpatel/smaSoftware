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
        printf("Usage: lockin <GPIB_ID>\n");
        exit(0);
        }
    device=atoi(argv[1]);
    quit = 0;


    initialize();
   
/* command response loop */
   while(quit != 1) {
   printf("Enter your command, q for quit: ");
   fflush(stdout);
/* while ((ch = getchar()) != '\n' && ch != EOF); */
   gets(command);
   printf ("read : %s\n", command);
if ((strlen(command)>0) && (command[strlen(command) - 1] == '\n'))
        command[strlen(command) - 1] = '\0';
   printf ("%s\n", command);
        if ((strcmp(command,"q")) == 0 ) quit=1;
	else  {
   write (gPIB, command, strlen(command));
   printf ("Written\n");
/*   printf("response ? (1/0): ");
   scanf ("%d", &flag);
   if (flag == 1) { */
   if (strchr(command,'?') != NULL) {
   count=read(gPIB,response,132);
   printf("%d %s\n", count, response);
 /*printf("Last character: 0x%x\n", (int)response[strlen(response)-1]);*/
 printf("Last character: %c\n", response[strlen(response)-2]);
   }  
/*while ((ch = getchar()) != '\n');*/
}
/*   command[0]='\0'; */
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
/*  count=read(gPIB,&response[0],80); */
  count=read(gPIB,response,132); 
    printf("%d %s\n",count,  response); 
/*    while (read(gPIB,&response1,1) != 0) {
    printf("%s", response1);
    } */
    printf("\n");



/*    write (gPIB, "LIAS?0", 6);
    read(gPIB, response, 132);
    sscanf(response,"%d",&dresponse);
    printf("%d\n",dresponse);
*/
   
}

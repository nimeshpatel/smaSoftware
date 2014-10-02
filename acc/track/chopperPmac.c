#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include<string.h>
#include "../../iPOctal232/includeFiles/iPOctal232.h"


int pmacOut;

char *serialpmac(char *command)
{
  int i;
  int nbytes=0,newbytes=0;
	char outChar,inChar;
  char buffer[50],rdBuf[50];
	int inbytes=0;
	int status;

  pmacOut = open("/dev/iPOctal232-3", O_RDWR );
  if (pmacOut == -1) {
    perror("First open");
    exit(-1);
   }

	status=ioctl(pmacOut,IOCTL_FLUSH,&buffer);
	if(status) printf("flush returned %d\n",status);
	strcpy(buffer,command);
	strcat(buffer,"\r");
	inbytes=write(pmacOut, buffer,strlen(buffer));
/*
	printf("wrote %d bytes to the pmac: %s\n",inbytes,buffer);
*/
	sleep(1);
/*
	printf("pmac response:\n");
*/
	inChar=' ';
	i=0;
	while(1)
	{
	nbytes=read(pmacOut,&inChar,1);
/*
	printf("%d 0x%x %c\n",i,inChar,inChar);
*/
	rdBuf[i]=inChar;
	i++;
	if(inChar==0x6) break;
	}
	rdBuf[i-2]=0x0;
	close(pmacOut);
	return(rdBuf);
}


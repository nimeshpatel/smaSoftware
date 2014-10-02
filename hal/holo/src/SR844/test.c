#include <stdio.h>
#include <stdlib.h>
#include <smem.h>
#include "iP488.h"
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

/*
gcc -I../../includeFiles -I/usr/PowerPC/common/include test.c
*/

char ch[1],command[132];
int i,end;

main(int argc, char **argv)
{
command[0] = '\0';
 while ((strcmp(command,"q"))){
/* first command and response */
   printf("Enter your command, q for quit: ");
   fgets(command,132,stdin);
if ((strlen(command)>0) && (command[strlen(command) - 1] == '\n'))
        command[strlen(command) - 1] = '\0';
   printf ("%s\n", command);
}
}


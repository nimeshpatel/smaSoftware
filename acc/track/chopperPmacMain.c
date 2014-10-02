#include<stdio.h>
#include<string.h>
char *serialpmac(char *command);
main(int argc, char *argv[])
{
char response[50];
strcpy(response,serialpmac(argv[1]));
printf("%s\n",response);
}

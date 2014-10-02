#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	pid_t getpid(void);

	while(1){ /* infinity loop */
		while(1){
			printf("sigtest\n");
			sleep(1);
		}
	printf("get signal\n");
	}
}

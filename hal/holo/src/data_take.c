/* gcc -o data_take data_take.c */
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
/*#include <fcntl.h>
#include "../xVME564/includeFiles/xVME564.h"*/

/* ADC inputs */
union ad_union {
	short word;
	char byte[2];
	};
union ad_union amp_vvm1, pha_vvm1, amp_vvm2, pha_vvm2;

int main(int argc, char *argv[]) {
	long j;
	int adc0_fd, adc1_fd, adc2_fd, adc3_fd, count=2;
	int adc0_rd, adc1_rd, adc2_rd, adc3_rd;
	int max_count;
	if(argc<2)
        {
        printf("Usage: data_take <sleep> <count>\n");
        exit(0);
        }


	max_count = atoi(argv[2]);
	printf("sleep: %d max_count: %d", atoi(argv[1]),max_count);
	adc0_fd = open("/dev/xVME564-0", O_RDWR, 0);
	if(adc0_fd<0){printf("cannot open device 0\n");exit(1);}
	adc1_fd = open("/dev/xVME564-1", O_RDWR, 0);
	if(adc1_fd<0){printf("cannot open device 1\n");exit(1);}
	for(j=0;j<max_count;j++){
	adc0_rd=read(adc0_fd, amp_vvm1.byte, count);
        if(adc0_rd<0){printf("cannot read device 0\n");
	printf("errno=%d\n",errno);exit(1);}
        printf("no.=%d amp=%d value=%6d ",j,adc0_rd,amp_vvm1.word);
        
        adc1_rd=read(adc1_fd, pha_vvm1.byte, count);
        if(adc1_rd<0){printf("cannot read device 1\n");exit(1);}
        printf("phase=%d value=%6d\n",adc1_rd,pha_vvm1.word);
	sleep(1);
	}
        
	close(adc0_fd);
	close(adc1_fd);
}

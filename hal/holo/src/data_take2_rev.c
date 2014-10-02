/* gcc -o data_take data_take.c */
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
/*#include <fcntl.h>
#include "../xVME564/includeFiles/xVME564.h"*/
#include "rm.h"

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
        int rm_status,  antlist[RM_ARRAY_SIZE],ant_scn;
        float az,el,chop_x,chop_y,chop_z,chop_angle;
        if(argc < 4)
        {
        printf("Usage: data_take <sleep> <count> <ant>\n");
        exit(0);
        }
        ant_scn=atoi(argv[3]);
/* initializing ref. mem. */
        rm_status=rm_open(antlist);  
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_open()");
            exit(1);
        }
/* configure for interrupt through RM */
        rm_status=rm_monitor(ant_scn,"RM_HOLO_ENC_HNDSHK_S");
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_monitor()");
                exit(1);
        }
	max_count = atoi(argv[2]);
	printf("!sleep: %d max_count: %d \n", atoi(argv[1]),max_count);

	adc0_fd = open("/dev/xVME564-0", O_RDWR, 0);
	if(adc0_fd<0){printf("cannot open device 0\n");exit(1);}

	adc1_fd = open("/dev/xVME564-1", O_RDWR, 0);
	if(adc1_fd<0){printf("cannot open device 1\n");exit(1);}

	adc2_fd = open("/dev/xVME564-2", O_RDWR, 0);
	if(adc2_fd<0){printf("cannot open device 0\n");exit(1);}

	adc3_fd = open("/dev/xVME564-3", O_RDWR, 0);
	if(adc3_fd<0){printf("cannot open device 1\n");exit(1);}


	for(j=0;j<max_count;j++){

	adc0_rd=read(adc0_fd, amp_vvm1.byte, count);
        if(adc0_rd<0) {
		printf("cannot read device 0\n");
		printf("errno=%d\n",errno);exit(1);
		}
/*        printf("no %d amp%d value%6d ",j,adc0_rd,amp_vvm1.word); */
        printf("no %d amp1%6d ",j,amp_vvm1.word);
        
        adc1_rd=read(adc1_fd, pha_vvm1.byte, count);
        if(adc1_rd<0) {
		printf("cannot read device 1\n");exit(1);
		}
/*        printf("phase=%d value=%6d ",adc1_rd,pha_vvm1.word); */
        printf("pha1%6d ",pha_vvm1.word);

	adc2_rd=read(adc2_fd, amp_vvm2.byte, count);
        if(adc2_rd<0) {
		printf("cannot read device 0\n");
		printf("errno=%d\n",errno);exit(1);
		}
/*        printf("no.%d amp%d value=%6d ",j,adc2_rd,amp_vvm2.word);*/
        printf("amp2%6d ",amp_vvm2.word);
        
        adc3_rd=read(adc3_fd, pha_vvm2.byte, count);
        if(adc3_rd<0) {
		printf("cannot read device 1\n");exit(1);
		}
/*        printf("phase=%d value=%6d",adc3_rd,pha_vvm2.word);*/
        printf("pha2%6d",pha_vvm2.word);

/* read az and elevation values */
        rm_status=rm_read(ant_scn,"RM_ACTUAL_AZ_DEG_F",&az);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read()");
            exit(1);
        }   
        rm_status=rm_read(ant_scn,"RM_ACTUAL_EL_DEG_F",&el);
        if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read()");
            exit(1);
         }  
         rm_status=rm_read(ant_scn,"RM_CHOPPER_ANGLE_F",&chop_angle);
         if(rm_status != RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read()");
            exit(1);
         }  
         rm_status=rm_read(ant_scn,"RM_CHOPPER_X_MM_F",&chop_x);
         if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
         }      
         rm_status=rm_read(ant_scn,"RM_CHOPPER_Y_MM_F",&chop_y);
         if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
         }      
         rm_status=rm_read(ant_scn,"RM_CHOPPER_Z_MM_F",&chop_z);
         if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
         }      
        printf(" az %f el %f c_x %f c_y %f c_z %f c_T %f \n",az,el,chop_x,chop_y,chop_z,chop_angle);
	sleep(1);
	}
        
	close(adc0_fd);
	close(adc1_fd);
	close(adc2_fd);
	close(adc3_fd);
}

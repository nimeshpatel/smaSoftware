/* dt.c
Used to be called data_take*.c residing in $HALAPP/holo/src
- code for reading vvm, for phase stability tests.
Makefile: Makefile.dt. To rebuild:
make -f Makefile.dt
Added RM variables. 2 May 2002, NaP */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <errno.h>
#include "rm.h"

#define ANT1 4
#define ANT2 5

/* ADC inputs */
union ad_union {
	short word;
	char byte[2];
	};

union ad_union amp_vvm1, pha_vvm1, amp_vvm2, pha_vvm2;
union ad_union amp2_vvm1, pha2_vvm1, amp2_vvm2, pha2_vvm2;

int main(int argc, char *argv[]) {
	long j;
	int adc0_fd, adc1_fd, adc2_fd, adc3_fd, count=2;
	int adc0_rd, adc1_rd, adc2_rd, adc3_rd;
	int max_count;

	/* place holder variables to read RM */
	short Short;
	float Float;
	double Double;
	int	Int;
	
	int antlist[RM_ARRAY_SIZE],rm_status;

	short phaseLock1, phaseLock2, yig1Locked1, yig1Locked2, yig2Locked1;
	short yig2Locked2, gunn1Locked1, gunn1Locked2, gunn2Locked1;
	short gunn2Locked2;

	float cabinTemperature1 ,cabinTemperature2 ,gunnBias1 ,gunnBias2;
	float gunnPLLIFPower1,gunnPLLIFPower2, gunnPLLPhaseNoise1;
	float gunnPLLPhaseNoise2,  sisMixer0Voltage1;
	float sisMixer0Voltage2, sisMixer0Current1, sisMixer0Current2;
	float sisMixer0BField1, sisMixer0BField2, sisMixer0Power1;
	float  sisMixer1Voltage1, sisMixer1Voltage2;
	float sisMixer1Current1, sisMixer1Current2, sisMixer1BField1;
	float sisMixer1BField2, sisMixer1Power1, sisMixer1Power2;
	float yig1Quadrature1, yig1Quadrature2, yig1Stress1;
	float yig1Stress2, yig2Quadrature1, yig2Quadrature2;
	float yig2Stress1, weatherTemperature;
	float weatherHumidity, weatherPressure, weatherWindSpeed;
	float weatherWindDirection, csoTau, csoTauRms, saoPhase;
	float saoPhaseRMS, saoSeeing,utc;

	int yigTimestamp1, yigTimestamp2,csoTauTimestamp,saoPhaseTimeStamp;


	double chopperXCounts1, chopperXCounts2, chopperYCounts1;
	double chopperYCounts2, chopperZCounts1, chopperZCounts2;
	double chopperTiltCounts1, chopperTiltCounts2,tjd,tsys1,tsys2;
	double totalPowerVoltsA1,totalPowerVoltsA2;
	double totalPowerVoltsB1,totalPowerVoltsB2;

	int	antenna1,antenna2;	

	
	antenna1=ANT1;	
	antenna2=ANT2;
	

	rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
        }



	if(argc<2)
        {
        printf("Usage: data_take <sleep> <count>\n");
	printf("The following quantities are printed out:

        1 j
	2 adc0_rd
	3 amp_vvm1.word 
	4 adc1_rd
	5 pha_vvm1.word 
	6 adc2_rd
	7 amp2_vvm1.word
        8 adc3_rd
	9 pha2_vvm1.word 
	10 tjd
	11 utc
	12 tsys1
	13 tsys2
	14 cabinTemperature1
	15 cabinTemperature2
	16 weatherTemperature
	17 weatherHumidity
	18 weatherPressure
	19 weatherWindSpeed
	20 weatherWindDirection
	21 csoTau
	22 csoTauRms
	23 csoTauTimestamp
	24 saoPhase
	25 saoPhaseRMS
	26 saoSeeing
	27 saoPhaseTimeStamp
	28 short phaseLock1
	29 phaseLock2
	30 gunn1Locked1
	31 gunn1Locked2
	32 gunn2Locked1
	33 gunn2Locked2
	34 yig1Locked1
	35 yig1Locked2
	36 yig2Locked1
	37 yig2Locked2
	38 gunnBias1
	39 gunnBias2
	40 gunnPLLIFPower1
	41 gunnPLLIFPower2
	42 gunnPLLPhaseNoise1
	43 gunnPLLPhaseNoise2
	44 gunnPLLPhaseNoise1
	45 sisMixer0Voltage1
	46 sisMixer0Voltage2
	47 sisMixer0Current1
	48 sisMixer0Current2
	49 sisMixer0BField1
	50 sisMixer0BField2
	51 sisMixer0Power1
	52 sisMixer0Power1
	53 sisMixer1Voltage1
	54 sisMixer1Voltage2
	55 sisMixer1Current1
	56 sisMixer1Current2
	57 sisMixer1BField1
	58 sisMixer1BField2
	59 sisMixer1Power1
	60 sisMixer1Power2
	61 yig1Quadrature1
	62 yig1Quadrature2
	63 yig1Stress1
	64 yig1Stress2
	65 yig2Quadrature1
	66 yig2Quadrature2
	67 yig2Stress1
	68 yig2Stress1
	69 yigTimestamp1
	70 yigTimestamp2
	71 chopperXCounts1
	72 chopperXCounts2
	73 chopperYCounts1
	74 chopperYCounts2
	75 chopperZCounts1
	76 chopperZCounts2
	77 chopperTiltCounts1
	78 chopperTiltCounts2
	79 totalPowerVoltsA1
	80 totalPowerVoltsA2
	81 totalPowerVoltsB1
	82 totalPowerVoltsB2
 	83 antenna1
	84 antenna2\n");

        exit(0);
        }


	max_count = atoi(argv[2]);
/*
	printf("sleep: %d max_count: %d", atoi(argv[1]),max_count);
*/


	adc0_fd = open("/dev/xVME564-0", O_RDWR, 0);
	if(adc0_fd<0){printf("cannot open device 0\n");exit(1);}

	adc1_fd = open("/dev/xVME564-1", O_RDWR, 0);
	if(adc1_fd<0){printf("cannot open device 1\n");exit(1);}

	adc2_fd = open("/dev/xVME564-2", O_RDWR, 0);
	if(adc2_fd<0){printf("cannot open device 0\n");exit(1);}

	adc3_fd = open("/dev/xVME564-3", O_RDWR, 0);
	if(adc3_fd<0){printf("cannot open device 1\n");exit(1);}


	for(j=1;j<=max_count;j++){

	adc0_rd=read(adc0_fd, amp_vvm1.byte, count);
        if(adc0_rd<0) {
		printf("cannot read device 0\n");
		printf("errno=%d\n",errno);exit(1);
		}
        
        adc1_rd=read(adc1_fd, pha_vvm1.byte, count);
        if(adc1_rd<0) {
		printf("cannot read device 1\n");exit(1);
		}

	adc2_rd=read(adc2_fd, amp2_vvm1.byte, count);
        if(adc2_rd<0) {
		printf("cannot read device 0\n");
		printf("errno=%d\n",errno);exit(1);
		}
        
        adc3_rd=read(adc3_fd, pha2_vvm1.byte, count);
        if(adc3_rd<0) {
		printf("cannot read device 1\n");exit(1);
		}

	/* read several variables from RM */
	
	rm_read(antenna1,"RM_TJD_D",&Double);
	tjd=Double;
	rm_read(antenna1,"RM_UTC_HOURS_F",&Float);
	utc=Float;
	rm_read(antenna1,"RM_TSYS_D",&Double);
	tsys1=Double;
	rm_read(antenna2,"RM_TSYS_D",&Double);
	tsys2=Double;
	rm_read(antenna1,"RM_CABIN_TEMPERATURE_F",&Float);
	cabinTemperature1=Float;
	rm_read(antenna2,"RM_CABIN_TEMPERATURE_F",&Float);
	cabinTemperature2=Float;
	rm_read(antenna1,"RM_PHASE_LOCK_S",&Short);
	phaseLock1=Short;
	rm_read(antenna2,"RM_PHASE_LOCK_S",&Short);
	phaseLock2=Short;
	rm_read(antenna1,"RM_GUNN_BIAS_F",&Float);
	gunnBias1=Float;
	rm_read(antenna2,"RM_GUNN_BIAS_F",&Float);
	gunnBias2=Float;
	rm_read(antenna1,"RM_GUNN_PLL_IFPOWER_F",&Float);
	gunnPLLIFPower1=Float;
	rm_read(antenna2,"RM_GUNN_PLL_IFPOWER_F",&Float);
	gunnPLLIFPower2=Float;
	rm_read(antenna1,"RM_GUNN_PLL_PHASENOISE_F",&Float);
	gunnPLLPhaseNoise1=Float;
	rm_read(antenna2,"RM_GUNN_PLL_PHASENOISE_F",&Float);
	gunnPLLPhaseNoise2=Float;
	rm_read(antenna1,"RM_",&Float);
	gunnPLLPhaseNoise1=Float;

	rm_read(antenna1,"RM_SIS_MIXER0_VOLTAGE_F",&Float);
	sisMixer0Voltage1=Float;
	rm_read(antenna2,"RM_SIS_MIXER0_VOLTAGE_F",&Float);
	sisMixer0Voltage2=Float;
	rm_read(antenna1,"RM_SIS_MIXER0_CURRENT_F",&Float);
	sisMixer0Current1=Float;
	rm_read(antenna2,"RM_SIS_MIXER0_CURRENT_F",&Float);
	sisMixer0Current2=Float;
	rm_read(antenna1,"RM_SIS_MIXER0_BFIELD_F",&Float);
	sisMixer0BField1=Float;
	rm_read(antenna2,"RM_SIS_MIXER0_BFIELD_F",&Float);
	sisMixer0BField2=Float;
	rm_read(antenna1,"RM_SIS_MIXER0_POWER_F",&Float);
	sisMixer0Power1=Float;
	rm_read(antenna2,"RM_SIS_MIXER0_POWER_F",&Float);
	sisMixer0Power1=Float;
	rm_read(antenna1,"RM_SIS_MIXER1_VOLTAGE_F",&Float);
	sisMixer1Voltage1=Float;
	rm_read(antenna2,"RM_SIS_MIXER1_VOLTAGE_F",&Float);
	sisMixer1Voltage2=Float;
	rm_read(antenna1,"RM_SIS_MIXER1_CURRENT_F",&Float);
	sisMixer1Current1 =Float;
	rm_read(antenna2,"RM_SIS_MIXER1_CURRENT_F",&Float);
	sisMixer1Current2 =Float;
	rm_read(antenna1,"RM_SIS_MIXER1_BFIELD_F",&Float);
	sisMixer1BField1=Float;
	rm_read(antenna2,"RM_SIS_MIXER1_BFIELD_F",&Float);
	sisMixer1BField2=Float;
	rm_read(antenna1,"RM_SIS_MIXER1_POWER_F",&Float);
	sisMixer1Power1=Float;
	rm_read(antenna2,"RM_SIS_MIXER1_POWER_F",&Float);
	sisMixer1Power2=Float;


	rm_read(antenna1,"RM_YIG1_LOCKED_S",&Short); 
	yig1Locked1=Short;
	rm_read(antenna2,"RM_YIG1_LOCKED_S",&Short); 
	yig1Locked2=Short;
	rm_read(antenna1,"RM_YIG1_QUADRATURE_F",&Float);    
	yig1Quadrature1=Float;
	rm_read(antenna2,"RM_YIG1_QUADRATURE_F",&Float);    
	yig1Quadrature2=Float;
	rm_read(antenna1,"RM_YIG1_STRESS_F",&Float);       
	yig1Stress1=Float;
	rm_read(antenna2,"RM_YIG1_STRESS_F",&Float);       
	yig1Stress2=Float;
	rm_read(antenna1,"RM_YIG2_LOCKED_S",&Short);       
	yig2Locked1=Short;
	rm_read(antenna2,"RM_YIG2_LOCKED_S",&Short);       
	yig2Locked2=Short;
	rm_read(antenna1,"RM_YIG2_QUADRATURE_F",&Float);    
	yig2Quadrature1=Float;
	rm_read(antenna2,"RM_YIG2_QUADRATURE_F",&Float);    
	yig2Quadrature2=Float;
	rm_read(antenna1,"RM_YIG2_STRESS_F",&Float);        
	yig2Stress1=Float;
	rm_read(antenna2,"RM_YIG2_STRESS_F",&Float);        
	yig2Stress1=Float;
	rm_read(antenna1,"RM_YIG_SVC_TIMESTAMP_L",&Int);
	yigTimestamp1=Int;
	rm_read(antenna2,"RM_YIG_SVC_TIMESTAMP_L",&Int);
	yigTimestamp2=Int;

	rm_read(antenna1,"RM_WEATHER_TEMP_F",&Float);
	weatherTemperature=Float;
	rm_read(antenna1,"RM_WEATHER_HUMIDITY_F",&Float);   
	weatherHumidity=Float;
	rm_read(antenna1,"RM_WEATHER_MBAR_F",&Float);       
	weatherPressure=Float;
	rm_read(antenna1,"RM_WEATHER_WINDSPEED_F",&Float);  
	weatherWindSpeed=Float;
	rm_read(antenna1,"RM_WEATHER_WINDDIR_F",&Float);    
	weatherWindDirection=Float;

	rm_read(antenna1,"RM_CHOPPER_X_COUNTS_D",&Double);       
	chopperXCounts1=Double;
	rm_read(antenna2,"RM_CHOPPER_X_COUNTS_D",&Double);       
	chopperXCounts2=Double;
	rm_read(antenna1,"RM_CHOPPER_Y_COUNTS_D",&Double);       
	chopperYCounts1=Double;
	rm_read(antenna2,"RM_CHOPPER_Y_COUNTS_D",&Double);       
	chopperYCounts2=Double;
	rm_read(antenna1,"RM_CHOPPER_Z_COUNTS_D",&Double);       
	chopperZCounts1=Double;
	rm_read(antenna2,"RM_CHOPPER_Z_COUNTS_D",&Double);       
	chopperZCounts2=Double;
	rm_read(antenna1,"RM_CHOPPER_TILT_COUNTS_D",&Double);      
	chopperTiltCounts1=Double;
	rm_read(antenna2,"RM_CHOPPER_TILT_COUNTS_D",&Double);      
	chopperTiltCounts2=Double;

	rm_read(antenna1,"RM_CSO_225GHZ_TAU_F",&Float);
	csoTau=Float;
	rm_read(antenna1,"RM_CSO_225GHZ_TAU_RMS_F",&Float);
	csoTauRms=Float;
	rm_read(antenna1,"RM_CSO_225GHZ_TAU_TSTAMP_L",&Int);
	csoTauTimestamp=Int;

	rm_read(antenna1,"RM_SAO_PHASE_F",&Float);
	saoPhase=Float;
	rm_read(antenna1,"RM_SAO_PHASE_RMS_F",&Float);
	saoPhaseRMS=Float;
	rm_read(antenna1,"RM_SAO_SEEING_F",&Float);
	saoSeeing=Float;
	rm_read(antenna1,"RM_SAO_PHASE_TSTAMP_L",&Int);
	saoPhaseTimeStamp=Int;

	rm_read(antenna1,"RM_GUNN1_LOCKED_S",&Short);  
	gunn1Locked1=Short;
	rm_read(antenna2,"RM_GUNN1_LOCKED_S",&Short);  
	gunn1Locked2=Short;
	rm_read(antenna1,"RM_GUNN2_LOCKED_S",&Short);  
	gunn2Locked1=Short;
	rm_read(antenna2,"RM_GUNN2_LOCKED_S",&Short);  
	gunn2Locked2=Short;

	rm_read(antenna1,"RM_TOTAL_POWER_VOLTS_D",&Double);      
	totalPowerVoltsA1=Double;
	rm_read(antenna1,"RM_TOTAL_POWER_VOLTS_D",&Double);      
	totalPowerVoltsA2=Double;

	rm_read(antenna1,"RM_TOTAL_POWER_VOLTS2_D",&Double);      
	totalPowerVoltsB1=Double;
	rm_read(antenna1,"RM_TOTAL_POWER_VOLTS2_D",&Double);      
	totalPowerVoltsB2=Double;


printf("%d	%d	%6d	%d	%6d	%d	%6d	%d	%6d	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%d	%e	%e	%e	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%d	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%e 	%d	%d	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%e	%d	%d\n",

      j,adc0_rd,amp_vvm1.word, adc1_rd,pha_vvm1.word, adc2_rd,amp2_vvm1.word,
        adc3_rd,pha2_vvm1.word, tjd ,utc ,tsys1,tsys2, cabinTemperature1,
	cabinTemperature2, weatherTemperature, weatherHumidity,
	weatherPressure, weatherWindSpeed, weatherWindDirection,
	csoTau, csoTauRms, csoTauTimestamp, saoPhase, saoPhaseRMS,
	saoSeeing, saoPhaseTimeStamp, phaseLock1, phaseLock2, gunn1Locked1,
	gunn1Locked2, gunn2Locked1, gunn2Locked2, yig1Locked1, yig1Locked2,
	yig2Locked1, yig2Locked2,gunnBias1,gunnBias2,gunnPLLIFPower1,
	gunnPLLIFPower2, gunnPLLPhaseNoise1, gunnPLLPhaseNoise2,
	gunnPLLPhaseNoise1, sisMixer0Voltage1, sisMixer0Voltage2,
	sisMixer0Current1, sisMixer0Current2, sisMixer0BField1,
	sisMixer0BField2, sisMixer0Power1, sisMixer0Power1, sisMixer1Voltage1,
	sisMixer1Voltage2, sisMixer1Current1, sisMixer1Current2,
	sisMixer1BField1, sisMixer1BField2, sisMixer1Power1,
	sisMixer1Power2, yig1Quadrature1, yig1Quadrature2, yig1Stress1,
	yig1Stress2, yig2Quadrature1, yig2Quadrature2, yig2Stress1,
	yig2Stress1, yigTimestamp1, yigTimestamp2,
	chopperXCounts1, chopperXCounts2, chopperYCounts1, chopperYCounts2,
	chopperZCounts1, chopperZCounts2, chopperTiltCounts1,
	chopperTiltCounts2,totalPowerVoltsA1,totalPowerVoltsA2,
	totalPowerVoltsB1,totalPowerVoltsB2,antenna1,antenna2);


	sleep(1);
	}
        
	close(adc0_fd);
	close(adc1_fd);
	close(adc2_fd);
	close(adc3_fd);
}

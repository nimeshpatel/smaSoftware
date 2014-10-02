/***************************************************************
	chartCommand.c
	client program for radio pointing using continuum detector
	10 Jul 2000 by M. Saito
*****************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<rpc/rpc.h>
#include "/usr/PowerPC/applications/hal/continuum/includeFiles/chart.h"
#include "/usr/PowerPC/applications/hal/continuum/includeFiles/chartu.h"
#include "smapopt.h"

void usage(int exitcode, char *error, char *addl);
void usage(int exitcode, char *error, char *addl) {

	if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
	fprintf(stderr, "Usage: chartCommand --antenna [options] \n"
		"[options] include:\n"
		"  -h or --help	 this help\n"
		"  -a<n> or --antenna<n> antenna (n is the antenna number)\n"
		"  -z or --azscan       azscan\n"
		"  -e or --elscan       elscan\n"
		"  -c or --chop	 chopping mode\n"
		"  -t or --totalp       total power mode\n"
		"  -o or --saveon       save data\n"
		"  -f or --saveoff      stop saving data\n"
		"  -q or --quit      	quit\n"
		"  -i<n> or --integ<n>  (n is the integration time)
			(default: 1 second)\n");
	exit(exitcode);
}

main(int argc, char *argv[])
{
	FILE *fps,*fp;
	char c,set_ant=0,file_n[40];
	int integ_time;
	smapoptContext optCon;
	struct  smapoptOption optionsTable[] = {
		{"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"azscan",'z',SMAPOPT_ARG_NONE,0,'z'},
		{"elscan",'e',SMAPOPT_ARG_NONE,0,'e'},
		{"chop",'c',SMAPOPT_ARG_NONE,0,'c'},
		{"totalp",'t',SMAPOPT_ARG_NONE,0,'t'},
		{"saveon",'o',SMAPOPT_ARG_NONE,0,'o'},
		{"saveoff",'f',SMAPOPT_ARG_NONE,0,'f'},
		{"quit",'q',SMAPOPT_ARG_NONE,0,'q'},
		{"antennas",'a',SMAPOPT_ARG_INT,&ant_num,'a'},
		{"integ",'i',SMAPOPT_ARG_INT,&integ_time,'i'},
		{NULL,0,0,NULL,0}
	};


	if(argc<3) usage(-1,"Insufficient number of arguments","");
	optCon = smapoptGetContext("chartuCommand", argc, argv, optionsTable,0);

	while ((c = smapoptGetNextOpt(optCon)) >= 0) {
	switch(c) {
		case 'h':
		usage(0,NULL,NULL);
		break;

		case 'z':
		chart_stat.scanflg=1;
		chart_stat.quitflg=0;
		break;

		case 'e':
		chart_stat.scanflg=0;
		chart_stat.quitflg=0;
		break;

		case 'c':
		chart_stat.chopflg=1;
		chart_stat.quitflg=0;
		break;

		case 't':
		chart_stat.chopflg=0;
		chart_stat.quitflg=0;
		break;

		case 'o':
		chart_stat.saveflg=1;
		chart_stat.quitflg=0;
		break;

		case 'f':
		chart_stat.saveflg=0;
		chart_stat.quitflg=0;
		break;

		case 'i':
		chart_stat.integtime=integ_time;
		chart_stat.quitflg=0;
		break;

		case 'q':
		chart_stat.quitflg=1;
		break;

		case 'a':
		set_ant=1;
		sprintf(file_n,"/common/data/rpoint/ant%1d/stat.ant%1d",ant_num,ant_num);
		if ((fp=fopen(file_n,"r"))==NULL)
		{
			printf("cannot open file\n");
			exit(1);
		}
		fscanf(fp,"%d %d %d %d %d\n",&chart_stat.saveflg, &chart_stat.scanflg, &chart_stat.chopflg, &chart_stat.integtime, &chart_stat.quitflg);
/*		printf("read status = %d %d %d %d %d\n",chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime, chart_stat.quitflg);*/
		fclose(fp);
		break;
		}
	}

	if(set_ant==0) usage(-2,"No antenna specified\n","Antenna number is required\n");

	if(c<-1) {
	fprintf(stderr, "%s: %s\n",
		smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
		smapoptStrerror(c));
	}

	sleep(1);
	sprintf(file_n,"/common/data/rpoint/ant%1d/stat.ant%1d",ant_num,ant_num);
	if ((fps=fopen(file_n,"w"))==NULL)
	{
		printf("cannot open status file\n");
		exit(1);
	}

/*	printf("%d %d %d %d %d\n",chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime,chart_stat.quitflg);*/
	fprintf(fps,"%d %d %d %d %d\n",chart_stat.saveflg, chart_stat.scanflg, chart_stat.chopflg, chart_stat.integtime,chart_stat.quitflg);
	fflush(fps);
	fclose(fps);
}

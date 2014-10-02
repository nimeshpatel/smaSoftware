#include<stdio.h>
#include "smapopt.h"

void usage(int exitcode, char *error, char *addl) {

	if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
	fprintf(stderr, "Usage: smapoptest [options] <sourcename>\n"
		"[options] include:\n"
		"  -h or --help    this help\n"
		"  -w or --wait    for waiting to acquire source\n"
	        "		   (default: nowait)\n"
		"  -n or --norpc   to avoid communicating to DDS and\n"
	        "                  smadata about source change \n"
		"		   (default: rpc)\n"
		"  -a<n> or --antenna <n> (n is the antenna number)\n"
	        "         	   (default: all antennas)\n");
	exit(exitcode);
}

int main(int argc, char *argv[]) {
	char c,*source;
	int antenna,raw = 0;
	smapoptContext optCon; 
	int wait=0, rpc=1,gotsource=0;

	struct  smapoptOption optionsTable[] = {
		{"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"norpc",'n',SMAPOPT_ARG_NONE,0,'n'},
		{"wait",'w',SMAPOPT_ARG_NONE,0,'w'},
		{"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
		{"source",'s',SMAPOPT_ARG_STRING,&source,'s'},
		{NULL,0,0,NULL,0}
	};

	if(argc<2) usage(-1,"Insufficient number of arguments","At least source-name required.");

	optCon = smapoptGetContext("smapoptest", argc, argv, optionsTable,0);

	while ((c = smapoptGetNextOpt(optCon)) >= 0) {
	
	switch(c) {
		case 'h':
		usage(0,NULL,NULL);
		break;
		
		case 'n':
		rpc=0;
		printf("No RPC\n");
		break;

		case 'a':
		printf("Antenna=%d\n",antenna);
		break;

		case 'w':
		wait=1;
		printf("Waiting to acquire the source...\n");
		break;

		case 's':
		gotsource=1;
		printf("Source name=%s\n",source);
		break;

	
		}


	}

	if(gotsource!=1) usage(-2,"No source specified","Source name is required.\n");
	
	if(c<-1) {
	fprintf(stderr, "%s: %s\n",
		smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
		smapoptStrerror(c));
	return 1;
	}

	smapoptFreeContext(optCon);
}
		

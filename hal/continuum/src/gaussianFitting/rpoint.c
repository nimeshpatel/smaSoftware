#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nr.h"

/* how many points to throw out of each scan on both the high end */
/* and the low end, i.e. for a total of 2*THROW_NUM */
#define THROW_NUM 0 

/* NAP  4 October 2003, modified for the additional channel */
/* NAP 31 March   2003, modified for the new format of input data file */
/* TK  27 Apr     2006, added option -o to invoke offline parameters in */
/*                      aperOffLine.Inp for aperEff */

int spikeFilter3(char *infile, char *outfile, int throw_num, int ant_num); 

void nrpoint(float x[],float y[],float azy[],float ely[],float azmod[],float elmod[],
		float sig[],int ndata,int num_gauss,int flag,int ant_num,int plotflag,
char *header, int aperoffline);
void nrpoint2(float x[],float y[],float azy[],float ely[],float azmod[],float elmod[],
		float sig[],int ndata,int num_gauss,int flag,int ant_num,int plotflag,
char *header);

int copyFile(char *in, int ant_num);
void usage(){
	printf("Usage: rpoint -a<n> -p (optional) -L/-H\n"
		"  -h  this help\n"
		"  -p  generate a PGplot window (this is now the default)\n"
		"  -n  do not generate a PGplot window\n"
		"  -v  give verbose output but no datapoints\n"
		"  -V  give verbose output, including datapoints\n"
	        "  -L use the low-frequency continuum channel\n"
	        "  -H use the high-frequency continuum channel\n"
		"  -a<n> (n is the antenna number)\n"
                "  -o  use offline parameter file for aperture efficiency\n"
 	        "  -s <spikesToThrowOut: default=%d on both min&max>\n",THROW_NUM);
	exit(1);
}

#include "rpoint.h"
int verbose = 0;
int lowfreqflag=-1;
char rxString[10];  /* either 'lowfreq' or 'highfreq' */
char summary_file_name[SUMMARY_FILE_NAME_LEN];

int main(int argc, char *argv[])
{
	FILE *fp;
	extern char *optarg; 	/* for command arguments */
	char rmCommand[130];
	float *x,*y,*sig,*azy,*ely,*azpmd,*elpmd,**covar,**alpha,*a;
	float utc,az,el,azoff,eloff,output,output2,az_err,el_err,az_mod,el_mod;
	float chop_angle,chop_x,chop_y,chop_z;
	int throw_num=THROW_NUM;
	int i,j,k,l,numplot,num_of_data,num_gauss, MA, NPT,*ia,flag,ant_num=0;
	int plotflag=1;
	int aperoffline=0;
	char c,f_line[2000],header[2000],source[40],*t  ;
	char hashsymbol[2];
	char fname[30],despike_name[120],data_name[120],a1[30],a2[30],a3[30];
	long pretime=2.0e9,curtime=-1;
	time_t new_time;
	struct stat sbuf;
	struct tm ts;

	while ((c = getopt(argc,argv,"hpnla:s:oLHvV")) != -1) {
	  /*	  fprintf(stderr,"about to parse %c\n",c);*/
	  switch(c) {
	  case 'h':
	    usage();
	    break;
	  case 'a':
	    ant_num=atoi(optarg);
	    printf("ant=%d\n",ant_num);
	    break;
	  case 's':
	    throw_num=atoi(optarg);
	    break;
	  case 'p':
	    plotflag=1;
	    break;
	  case 'n':
	    plotflag=0;
	    break;
	  case 'H':
	    lowfreqflag=0;
	    break;
	  case 'L':
	  case 'l':
	    lowfreqflag=1;
	    break;
	  case 'v':
	    verbose = 1;
	    break;
	  case 'V':
	    verbose = 2;
	    break;
	  case 'o':
	    aperoffline = 1;
            printf("OFFLINE PARAMETERS FOR APERTURE EFFICIENCY!\n");
	    break;
	  default:
	    usage();
	    break;
	  }
	}

	if (lowfreqflag < 0) {
	  printf("You must specify either -L (low-freq rx) or -H (high-freq rx)\n");
	  exit(1);
	}
	if((ant_num < 1) || (ant_num > 8)){
	  printf("ant %d? ant 1-8\n",ant_num);
	  usage();
	  exit(1);
	}

	if (lowfreqflag == 0) {
          strcpy(rxString,"highfreq");
	} else {
          strcpy(rxString,"lowfreq");
	}
	printf("using rx = %s\n");
	new_time=time(NULL);
	sprintf(fname,"ant%1d",ant_num);
#if __linux__
	sprintf(data_name,"/sma/rtData/engineering/rpoint/ant%1d/tmp.dat.%s",ant_num,rxString);
	sprintf(despike_name,"/sma/rtData/engineering/rpoint/ant%1d/despike.dat.%s",ant_num,rxString);
#else
	sprintf(data_name,"/data/engineering/rpoint/ant%1d/tmp.dat.%s",ant_num,rxString);
	sprintf(despike_name,"/data/engineering/rpoint/ant%1d/despike.dat.%s",ant_num,rxString);
#endif
	ts = *localtime(&new_time);
#if __linux__
	sprintf(summary_file_name, "/sma/rtData/engineering/rpoint/ant%1d/%04d%02d%02d.%s.%s",ant_num,ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,fname,rxString);
#else
	sprintf(summary_file_name, "/data/engineering/rpoint/ant%1d/%04d%02d%02d.%s.%s",ant_num,ts.tm_year+1900,ts.tm_mon+1,ts.tm_mday,fname,rxString);
#endif
	printf("output filename will be %s\n",summary_file_name);
	printf("spikes to throw out = %d*2 = %d\n",throw_num,throw_num*2);

	while(1){ /* infinity loop */
	  while(curtime <= pretime){
	    if(curtime>0) pretime=curtime;
	    stat(data_name,&sbuf);
	    curtime=sbuf.st_mtime;
	    /*
	      if(plotflag) printf("ant%1d pre=%ld cur=%ld:plot mode\n",ant_num,pretime,curtime);
	      else printf("ant%1d pre=%ld cur=%ld:no plot mode\n",ant_num,pretime,curtime);
	    */
	    
	    sleep(1);
	  }
	  pretime=curtime;
	  
	  /* spikefilter test */
	  printf("throw_num given to spikeFilter3= %d\n",throw_num);	
	  spikeFilter3(data_name,despike_name,throw_num,ant_num);
	  
	  
	  if ((fp=fopen(despike_name,"r"))==NULL)
	    {
	      printf("cannot open data file\n");
	      exit(1);
	    }
	  
	  num_of_data=0;
	  
	  while(feof(fp)==0){
	    fgets(f_line,2000,fp);
	    if(num_of_data==0) sscanf(f_line,"%s %d %d\n",hashsymbol,&flag,&l);
	    if(num_of_data==1) {
	      strcpy(header,f_line);
	      if (verbose==2) {
		printf("header=%s fline=%s\n",header,f_line);
	      }
	    }
	    num_of_data++;
	  }
	  
	  if(flag&&l) num_gauss=2;
	  else num_gauss=1;
	  
	  printf("numgauss=%d\n",num_gauss);
	  
	  MA=5*num_gauss;
	  NPT=num_of_data-4;
	  printf("number of data = %d num of fitting components = %d\n", NPT,num_gauss);
	  
	  /* vector business */
	  x=vector(1,NPT);
	  y=vector(1,NPT);
	  azy=vector(1,NPT);
	  ely=vector(1,NPT);
	  azpmd=vector(1,NPT);
	  elpmd=vector(1,NPT);
	  sig=vector(1,NPT);
	  rewind(fp);
	  
	  /* read data */
	  /*
	    printf("fitting data...\n");
	  */
	  for (i=1;i<=(num_of_data-2);i++)
	    {
	      fgets(f_line,2000,fp);
	      if(i>2) {
		j=i-2;	
		sscanf(f_line,"%f %f %f %f %f %f %f %f %f %f %f\n",
		       &utc,&az,&el,&azoff,&eloff,&output,&output2,&az_err,&el_err,&az_mod,&el_mod);
		if(flag==1) x[j]=azoff-az_err; else x[j]=eloff-el_err;
		if(lowfreqflag==1) y[j]=output;
		if(lowfreqflag==0) y[j]=output2;
		if (verbose==2) {
		  printf("lowfreqflag=%d output=%f output2 = %f y[j]=%f\n",
			 lowfreqflag,output,output2,y[j]);
		}
		azy[j]=az;
		ely[j]=el;
		azpmd[j]=az_mod;
		elpmd[j]=el_mod;
		sig[j]=1.0;
		/*
		  printf("%d %f %f\n",j,x[j],y[j]);
		*/
	      }
	    }
	  /*
	    printf("end of fitting data.\n");
	  */
	  
	  new_time=time(NULL);
	  ts = *localtime(&new_time);
	  t=ctime(&new_time);
	  if(num_gauss==1) nrpoint(x,y,azy,ely,azpmd,elpmd,sig,NPT,MA,flag,ant_num,plotflag,header,aperoffline);
	  else nrpoint2(x,y,azy,ely,azpmd,elpmd,sig,NPT,MA,flag,ant_num,plotflag,header);
	  free_vector(sig,1,NPT);
	  free_vector(y,1,NPT);
	  free_vector(x,1,NPT);
	  free_vector(azy,1,NPT);
	  free_vector(ely,1,NPT);
	  free_vector(azpmd,1,NPT);
	  free_vector(elpmd,1,NPT);
	  
	  /* The following command was added by Todd Hunter, as it is in the standard
	   * Unix library and does not require a system() call. */
	  remove(despike_name);
	  /*
	    sprintf(rmCommand,"\rm %s",despike_name);
	    system(rmCommand);
	  */
	  printf("done removing the file = %s\n",despike_name);
	  printf("Waiting for next scan to arrive...\n");
	}
}


int spikeFilter3(char *infile, char *outfile, int throw_num, int ant_num) {
  FILE *fpi,*fpo;
  int iline=1, nlines=0;
  int keptlines;
  char headCommand[130];
  char grepCommand[130];
  char sortCommand[256];
  char nawkCommand[256];
  char nawk1Command[256];
  char wcCommand[130];
  char head2Command[130];
  char tailCommand[130];
  char inputline[500];
  char temp_file[130];
  char temp_file2[130];
  char temp_file3[130];
  char diffs_srt[130];
  
  sprintf(headCommand,"head -2 %s > %s",infile,outfile);
  printf("headCommand=%s\n",headCommand);
  system(headCommand);
  chmod(outfile,0666);
  copyFile(outfile,ant_num);
  if (verbose) {
    printf("Changed permission to 0666 on file = %s\n",outfile);
  }
#if __linux__
  sprintf(temp_file,"/sma/rtData/engineering/rpoint/ant%1d/temp_file.%s",ant_num,rxString);
  sprintf(temp_file2,"/sma/rtData/engineering/rpoint/ant%1d/temp_file2.%s",ant_num,rxString);
  sprintf(temp_file3,"/sma/rtData/engineering/rpoint/ant%1d/temp_file3.%s",ant_num,rxString);
  sprintf(diffs_srt,"/sma/rtData/engineering/rpoint/ant%1d/diffs_srt.%s",ant_num,rxString);
#else
  sprintf(temp_file,"/data/engineering/rpoint/ant%1d/temp_file.%s",ant_num,rxString);
  sprintf(temp_file2,"/data/engineering/rpoint/ant%1d/temp_file2.%s",ant_num,rxString);
  sprintf(temp_file3,"/data/engineering/rpoint/ant%1d/temp_file3.%s",ant_num,rxString);
  sprintf(diffs_srt,"/data/engineering/rpoint/ant%1d/diffs_srt.%s",ant_num,rxString);
#endif
  sprintf(grepCommand,"grep -v \"#\" %s > %s",infile,temp_file);
  printf("grepCommand=%s\n",grepCommand);
  system(grepCommand);
  chmod(temp_file,0666);
  if (verbose) {
    printf("Changed permission to 0666 on file = %s\n",temp_file);
  }  
  printf("lowfreqflag from within spikeFilter3 = %d\n",lowfreqflag);
  if(lowfreqflag==1) {
    sprintf(nawkCommand,
	    "awk \'(NR == 1) {old=$6} (NR!=1) {printf \"%%s %%f %%f \\n\", $0,NR, ($6-old)\; old=$6}\' %s | sort -n -k 13 > %s",temp_file,diffs_srt);
  } else {
    sprintf(nawkCommand,
	    "awk \'(NR == 1) {old=$7} (NR!=1) {printf \"%%s %%f %%f \\n\", $0,NR, ($7-old)\; old=$7}\' %s | sort -n -k 13 > %s",temp_file,diffs_srt);
  }
  printf("nawkCommand=%s\n",nawkCommand);
  system(nawkCommand);
  chmod(diffs_srt,0666);
  if (verbose) {
    printf("Changed permission to 0666 on file = %s\n",diffs_srt);
  }  
  fpi=fopen(diffs_srt,"r");
  if(fpi==NULL) {
    printf("Could not read %s.\n",diffs_srt); exit(-1);
  }

  /*  while (fscanf(fpi,"%s",inputline)!=EOF) { */

  while (fgets(inputline,sizeof(inputline),fpi) != NULL) {
    iline++;
  }
  close(fpi);
  
  nlines=iline-1;
  
  fpi=fopen(diffs_srt,"r");
  if (fpi==NULL) {
    printf("Could not read %s.\n",diffs_srt); exit(-1);
  }
  iline=1;
  fpo=fopen(temp_file2,"w");
  if(fpo==NULL) {
    printf("Could not create %s\n",temp_file2); exit(-1);
  }
  chmod(temp_file2,0666);
  if (verbose) {
    printf("Changed permission to 0666 on file = %s\n",temp_file2);
  }  
  keptlines = 0;
  while (fgets(inputline,sizeof(inputline),fpi)!=0) {
    if((iline>=throw_num)&&(iline<(nlines-throw_num))) {
      fprintf(fpo,"%s",inputline);
      keptlines++;
    }
    if (verbose==2) {
      printf("---%s",inputline);
    }
    iline++;
  }
  fclose(fpo);
  fclose(fpi);
  printf("Kept %d out of %d lines\n",keptlines,nlines);
  sprintf(sortCommand,"sort -n -k 12 %s > %s",temp_file2,temp_file3);
  printf("sortCommand=%s\n",sortCommand);
  system(sortCommand);
  chmod(temp_file3,0666);
  if (verbose) {
    printf("Changed permission to 0666 on file = %s\n",temp_file3);
  }
  sprintf(nawk1Command,
          "awk '{print $1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11}' %s >> %s",temp_file3,outfile);
  printf("nawk1Command=%s\n",nawk1Command);
  system(nawk1Command);
  sprintf(tailCommand,"tail -1 %s >> %s",infile,outfile);
  printf("tailCommand=%s\n",tailCommand);
  system(tailCommand);
}

int copyFile(char *in, int ant_num) {
  FILE *fin, *fout;
  char buffer[256];
  char fullfilename[120];
  int openlimit = 10;
  int finCtr = 0;
  int foutCtr = 0;

#if __linux__
  sprintf(fullfilename,"/sma/rtData/engineering/rpoint/ant%d/header.dat.%s",ant_num,rxString);
#else
  sprintf(fullfilename,"/data/engineering/rpoint/ant%d/header.dat.%s",ant_num,rxString);
#endif
  fprintf(stderr,"copying the file to %s\n",fullfilename);
  do {
    fin = fopen(in,"r");
    if (fin == NULL) {
      fprintf(stderr,"Failed to open input file = %s\n",in);
    }
  } while (fin == NULL && ++finCtr < openlimit);
  if (finCtr >= openlimit) {
    fprintf(stderr,"Giving up on %s\n",in);
    return(-1);
  }
  do {
    fout = fopen(fullfilename,"w");
    if (fout == NULL) {
      fprintf(stderr,"1. Failed to open output file = %s. Sleeping(1).\n",fullfilename);
      sleep(1);
    }
  } while (fout == NULL && foutCtr < openlimit);
  if (foutCtr >= openlimit) {
    fprintf(stderr,"Giving up on %s\n",fullfilename);
    return(-2);
  }

  while (fgets(buffer,sizeof(buffer),fin) != NULL) {
    fputs(buffer,fout);
  }
  fclose(fin);
  fclose(fout);
  chmod(fullfilename,0666);
  return(0);
}

int tokenize(char *input, char *tokenArray[MAX_TOKENS]) {
  int i;
  int non_blanks = 0;
  int tokens = 0;

  if (strlen(input) > 0) {
    for (i=0; i<strlen(input); i++) {
      if (input[i] != ' ') {
        non_blanks = 1; break;
      }
    }
    if (non_blanks == 0) return(0);
    tokenArray[tokens++] = strtok(input,",");
    while ((tokenArray[tokens] = strtok(NULL,",")) != NULL) {
      tokens++;
    }
  }
  return(tokens);
}


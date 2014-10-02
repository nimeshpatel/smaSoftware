#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <smapopt.h>
#include "rm.h"

char *lower(char *upper);

void usage(int exitcode, char *error, char *addl) {
        if (error) fprintf(stderr, "
%s: %s

", error, addl);
        fprintf(stderr, "Usage: value [options] -v <variablename>
"
                "[options] include:
"
                "  -h or --help    this help
"
                "  -l or --list    to print out a list of available
variables
"
                "  -a<n> or --antenna <n> (n is the antenna number)
"
                "                  (default: all antennas)");
        exit(exitcode);
}

void main(int argc, char *argv[])  
{
	struct utsname unamebuf;
	size_t sublength;
	int ifound,length,C,V;
	char underscore,letterV;
	char *p,*n,*q;
	int iarray, jarray;
	char *arraysizestr;
	int arraysize, arraysize2;
	char datatype[256];
	char variable_name[256], vname[256];
        char c,*command,String[256];
	char **namelist;
	int num_alloc;
        short Short,Shortvector[256];
	char Bit, Bitvector[256];
	int antlist[RM_ARRAY_SIZE],rm_status;
	long Long, Longvector[256], *Long2vector;
	int status,i,l,v1,inum;
	int vectorflag;
	float Float, Floatvector[256];
	double Double, Doublevector[256];
	smapoptContext optCon;
	int listflag=0,gotvariable=0,antenna;
	struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"list",'l',SMAPOPT_ARG_NONE,0,'l'},
                {"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"variable",'v',SMAPOPT_ARG_STRING,&command,'v'},
                {NULL,0,0,NULL,0}
        };
if(argc<2) usage(-1,"Insufficient number of arguments","At least
variablename is required. Try value -h");

        optCon = smapoptGetContext("value", argc, argv, optionsTable,0);
           while ((c = smapoptGetNextOpt(optCon)) >= 0) {

        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;

                case 'l':
                listflag=1;
                break;

                case 'a':
                break;

                case 'v':
                gotvariable=1;
                break;
                }
	}



        if((gotvariable!=1)&&(listflag==0)) usage(-2,"No variable
specified","Variable name required; try -l to see the list of available variables.
");

if(c<-1) {
        fprintf(stderr, "%s: %s
",
                smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
                smapoptStrerror(c));
        }
         smapoptFreeContext(optCon);


	/* check if not running on hal9000*/
        uname(&unamebuf);
        if(strcmp(unamebuf.nodename,"hal9000")) antenna=0;

	underscore='_';
	C=(int)underscore;

	letterV = 'v';
	V=(int)letterV;


	for(i=0;i<256;i++) variable_name[i]='\0';
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
        }

/* report the list of allocation names */
  status = rm_get_num_alloc(&num_alloc);
  if(status != RM_SUCCESS) {
    rm_error_message(status, "rm_get_num_alloc()");
    exit(1);
  }

  namelist = (char **)malloc(num_alloc*sizeof(char *));
  for(inum=0; inum<num_alloc; inum++) 
    namelist[inum] = (char *)malloc(RM_NAME_LENGTH);

  status = rm_get_alloc_names(namelist);
  if(status != RM_SUCCESS) {
    rm_error_message(status, "rm_get_alloc_names()");
    exit(1);
  }
	ifound=-1;
  for(inum=0; inum<num_alloc; inum++) {
	p=strchr(namelist[inum],C);
	p++;
	length=strlen(p);
	/* get the last token after underscore, for datatype */
	q=strrchr(namelist[inum],C);
	/* this includes the underscore, so increment by 1 */
	q++;
	strcpy(datatype,q);
	length=length-strlen(datatype)-1;
	sublength=(size_t)length;
	for(l=0;l<256;l++) variable_name[l]='\0';
	strncpy(variable_name,p,sublength);
	strcpy(variable_name,lower(variable_name));

	if(listflag==1) printf("%s\n",variable_name);

	vectorflag=0;
	/* check if this variable is a vector */
	for(l=0;l<strlen(variable_name);l++) {
	  v1=(int)(variable_name[l+1]);
	  if((variable_name[l]=='v')&&(isdigit(v1)!=0)) vectorflag++;
	}
	if(vectorflag>0) {
	  n=strrchr(variable_name,C);
	  n++;
	  arraysizestr=strchr(n,V);
	  arraysizestr++;
	  arraysize=atoi(arraysizestr);	
	  arraysize2=1;
	  if (vectorflag > 1) {
	    arraysize2 = arraysize;
            sublength = (size_t)(n-variable_name-1);
            strncpy(vname,variable_name,sublength);
	    n=strrchr(vname,C);
	    if (n != NULL) {
	      n++;
	      arraysizestr=strchr(n,V);
	      if (arraysizestr != NULL) {
		arraysizestr++;
		arraysize=atoi(arraysizestr);	
	      }
	    }
	    fprintf(stderr,"Found a 2-dim array of size (%dx%d)\n",
                    arraysize,arraysize2);
	  }
	}
	
	
	if(listflag==0) {
	if(!strcmp(command,variable_name)) {
		ifound=inum;
		break;
		}
	if(!strcmp(lower(command),variable_name)) {
		ifound=inum;
		break;
		}
	  }
	}

	if(listflag==0) {

	if(ifound!=-1) {
	  if(!strcmp(datatype,"L")) {
	    switch(vectorflag) {
	    case 1:
	      rm_status=rm_read(antenna,namelist[ifound],Longvector);
	      if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
	      } 
	      for(iarray=0;iarray<arraysize;iarray++) {
		printf("%ld ",Longvector[iarray]);
	      }
	      printf("\n");
	      break;
	    case 2:
	      Long2vector = (long *)calloc(arraysize*arraysize2,sizeof(long));
	      rm_status=rm_read(antenna,namelist[ifound],Long2vector);
	      if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
	      }
	      for(iarray=0;iarray<arraysize;iarray++) {
		for(jarray=0;jarray<arraysize2;jarray++) {
		  printf("%ld ",Long2vector[iarray*arraysize+jarray]);
		}
		printf("\n");
	      }
	      free(Long2vector);
	      break;
	    default:
	      rm_status=rm_read(antenna,namelist[ifound],&Long);
	      if(rm_status != RM_SUCCESS) {
		rm_error_message(rm_status,"rm_read()");
		exit(1);
	      }
	      printf("%ld\n",Long);
	    }
	  }
	  
		if(!strcmp(datatype,"S")) {
		   if(vectorflag==1) {
           	      rm_status=rm_read(antenna,namelist[ifound],&Shortvector);
        	 	if(rm_status != RM_SUCCESS) {
                	rm_error_message(rm_status,"rm_read()");
                	exit(1);
			}
			for(iarray=0;iarray<arraysize;iarray++) 
			printf("%d ",Shortvector[iarray]);
			printf("\n");
		      } else {
		      rm_status=rm_read(antenna,namelist[ifound],&Short);
        	      if(rm_status != RM_SUCCESS) {
                      rm_error_message(rm_status,"rm_read()");
                      exit(1);
		      }
		   printf("%d\n",Short);
		   }
		}

		if(!strcmp(datatype,"B")) {
		   if(vectorflag==1) {
           	      rm_status=rm_read(antenna,namelist[ifound],&Bitvector);
        	 	if(rm_status != RM_SUCCESS) {
                	rm_error_message(rm_status,"rm_read()");
                	exit(1);
			}
			for(iarray=0;iarray<arraysize;iarray++) 
			printf("%d ",Bitvector[iarray]);
			printf("\n");
		      } else {
		      rm_status=rm_read(antenna,namelist[ifound],&Bit);
        	      if(rm_status != RM_SUCCESS) {
                      rm_error_message(rm_status,"rm_read()");
                      exit(1);
		      }
		   printf("%d\n",Bit);
		   }
		}

		if(!strcmp(datatype,"F")) {
		   if(vectorflag==1) {
           	      rm_status=rm_read(antenna,namelist[ifound],&Floatvector);
        	 	if(rm_status != RM_SUCCESS) {
                	rm_error_message(rm_status,"rm_read()");
                	exit(1);
			}
			for(iarray=0;iarray<arraysize;iarray++) 
				{
			if(Floatvector[iarray] < 1.e-5) 
				printf("%e ",Floatvector[iarray]);
			else printf("%f ",Floatvector[iarray]);
				}
			printf("\n");
		      } else {
		   rm_status=rm_read(antenna,namelist[ifound],&Float);
        	   if(rm_status != RM_SUCCESS) {
                   rm_error_message(rm_status,"rm_read()");
                   exit(1);
		   }
			if(Float < 1.e-5) printf("%e\n",Float);
			else printf("%f\n",Float);
		   }
		}

		if(!strcmp(datatype,"D"))
		{
		   if(vectorflag==1) {
           	      rm_status=rm_read(antenna,namelist[ifound],&Doublevector);
        	 	if(rm_status != RM_SUCCESS) {
                	rm_error_message(rm_status,"rm_read()");
                	exit(1);
			}
			for(iarray=0;iarray<arraysize;iarray++) 
				{
			if(Doublevector[iarray] < 1.e-5) 
				printf("%e ",Doublevector[iarray]);
			else printf("%f ",Doublevector[iarray]);
				}
			printf("\n");
		      } else {
		   rm_status=rm_read(antenna,namelist[ifound],&Double);
        	   if(rm_status != RM_SUCCESS) {
                   rm_error_message(rm_status,"rm_read()");
                   exit(1);
		   }
			if(Double < 1.e-5) printf("%e\n",Double);
			else printf("%lf\n",Double);
		   }
		}

		if(datatype[0]=='C')
		{
		rm_status=rm_read(antenna,namelist[ifound],String);
        	if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
		}
		printf("%s\n",String);
		}
	}
	else
	{
	printf("This variable does not exist.\n");
	}
	} /* listflag==0 */
}

char *lower(char *upper) {
int i,c;
for(i=0;i<strlen(upper);i++) {
c=(int)(upper[i]);
upper[i]=(char)tolower(c);
}
return upper;
}

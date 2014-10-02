CC = gcc
CFLAGS =  -Wall -g -c
BFLAGS =  -g 
LFLAGS = -L$(COMMONLIB)
INC = -I$(COMMONINC) 
UNIDIGINC = -I/usr/PowerPC/applications/acc/iPUniDig_D/includeFiles
LIBS = -llynx -mthreads -lsmapopt
RM = $(COMMONLIB)/rm.o
SHM = $(COMMONLIB)/openshm.o
SYNCDET = syncdet 

all :   $(SYNCDET)

syncdet.o : syncdet.c 
	${CC} ${CFLAGS} -o syncdet.o syncdet.c  ${INC} ${UNIDIGINC}

syncdet : syncdet.o 
	${CC} ${BFLAGS} -o syncdet syncdet.o ${RM} ${LFLAGS} ${LIBS} ${SHM}


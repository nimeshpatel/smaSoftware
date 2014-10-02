CC=gcc
BFLAGS=-O -g
CFLAGS= -O -g -Wall $(INC)
LFLAGS=-L$(COMMONLIB)
INC=-I$(COMMONINC) -I$(GLOBALINC)
RM=$(COMMONLIB)/rm.o
PH=$(COMMONLIB)/ph.o
DSM=$(COMMONLIB)/libdsm.a 
LIBS=-lrpc -lnetinet -llynx -lsmapopt -mthreads
SOURCES=recordTilts.c recordContinuum.c
BINS=$(HALBIN)/recordTilts $(HALBIN)/recordContinuum

all: $(BINS)

$(HALBIN)/recordTilts: recordTilts.o ${RM} ${PH} ${DSM}
	${CC} ${BFLAGS} -o $(HALBIN)/recordTilts recordTilts.o ${RM} ${PH} \
	${DSM} ${LFLAGS} ${LIBS} 

$(HALBIN)/recordContinuum: recordContinuum.o ${RM} ${PH} ${DSM}
	${CC} ${BFLAGS} -o $(HALBIN)/recordContinuum recordContinuum.o \
	${RM} ${PH} ${DSM} ${LFLAGS} ${LIBS} 

depend: 
	$(CC) -MM $(INC) $(SOURCES) > dependencies

include dependencies

clean:
	\rm recordTilts.o recordContinuum.o

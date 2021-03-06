BINDIR=../../bin
CC = gcc -Wall
CFLAGS = -O -Wall -g -c
BFLAGS = -O -g 
LFLAGS = -L$(COMMONLIB)
INC = -I$(GLOBALINC) -I$(COMMONINC) 
LIBS = -llynx -lsmapopt
RM = $(COMMONLIB)/rm.o
DSM = $(COMMONLIB)/libdsm.a -lrpc -lnetinet
POPT = $(COMMONLIB)/libsmapopt.a
CLIB = $(COMMONLIB)/commonLib
SOURCES= getAntList.c

all : $(BINDIR)/getAntList $(BINDIR)/antennaWait \
	$(BINDIR)/writeDSM $(BINDIR)/readDSM $(BINDIR)/sampleRM \
	$(ACCBIN)/sampleRM $(BINDIR)/readLastCpoint \
	$(BINDIR)/readAndWriteLastCpoint

acc: $(ACCBIN)/readDSM $(ACCBIN)/writeDSM

colossus: $(BINDIR)/getAntList $(BINDIR)/writeDSM $(BINDIR)/readDSM

getAntList.o : getAntList.c
	${CC} ${CFLAGS} -o getAntList.o getAntList.c ${INC}

$(BINDIR)/getAntList : getAntList.o
	${CC} -o $(BINDIR)/getAntList getAntList.o ${LFLAGS} ${RM} \
		${LIBS} ${CLIB}

writeDSM.o : writeDSM.c
	${CC} ${CFLAGS} -o writeDSM.o writeDSM.c ${INC}

$(BINDIR)/writeDSM: writeDSM.o $(COMMONLIB)/libdsm.a
	${CC} -o $(BINDIR)/writeDSM writeDSM.o ${LFLAGS} ${RM} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB}

$(ACCBIN)/writeDSM: writeDSM.o $(COMMONLIB)/libdsm.a
	${CC} -o $(ACCBIN)/writeDSM writeDSM.o ${LFLAGS} ${RM} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB}

readDSM.o : readDSM.c
	${CC} ${CFLAGS} -o readDSM.o readDSM.c ${INC}

$(BINDIR)/readDSM: readDSM.o $(COMMONLIB)/libdsm.a
	${CC} -o $(BINDIR)/readDSM readDSM.o ${LFLAGS} ${RM} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB}

$(ACCBIN)/readDSM: readDSM.o $(COMMONLIB)/libdsm.a
	${CC} -o $(ACCBIN)/readDSM readDSM.o ${LFLAGS} ${RM} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB}

readLastCpoint.o : readLastCpoint.c
	${CC} ${CFLAGS} -o readLastCpoint.o readLastCpoint.c ${INC}

$(BINDIR)/readLastCpoint: readLastCpoint.o $(COMMONLIB)/libdsm.a
	${CC} -o $(BINDIR)/readLastCpoint readLastCpoint.o ${LFLAGS} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB}

readAndWriteLastCpoint.o : readAndWriteLastCpoint.c
	${CC} ${CFLAGS} -o readAndWriteLastCpoint.o readAndWriteLastCpoint.c ${INC}

$(BINDIR)/readAndWriteLastCpoint: readAndWriteLastCpoint.o $(COMMONLIB)/libdsm.a
	${CC} -o $(BINDIR)/readAndWriteLastCpoint readAndWriteLastCpoint.o ${LFLAGS} \
		${LIBS} -ldsm -lrpc -lnetinet ${CLIB} ${RM}

$(BINDIR)/antennaWait: antennaWait.o ../../lib/functionLib antennaWaitLib.o $(COMMONLIB)/libdsm.a
	$(CC) ../../lib/functionLib  antennaWaitLib.o antennaWait.o -o $@ $(RM) $(DSM) $(CLIB) $(POPT) -lm

sampleRM_hal.o: sampleRM.c
	$(CC) $(CFLAGS) -DFOR_HAL=1 -o sampleRM_hal.o $(INC)  $<

$(BINDIR)/sampleRM: sampleRM_hal.o
	$(CC) sampleRM_hal.o -o $@ $(RM) $(LFLAGS) $(POPT) 

sampleRM_acc.o: sampleRM.c
	$(CC) $(CFLAGS) -DFOR_HAL=0 -o sampleRM_acc.o $(INC)  $<

$(ACCBIN)/sampleRM: sampleRM_acc.o $(COMMONLIB)/rm_dsm_names.o
	$(CC) sampleRM_acc.o -o $@ $(RM) $(LFLAGS) $(POPT) \
		$(COMMONLIB)/rm_dsm_names.o

antennaWait.o: antennaWait.c antennaWait.h $(COMMONINC)/smapopt.h
	$(CC) $(INC) -c $< -o $@

antennaWaitLib.o: antennaWaitLib.c antennaWait.h $(COMMONINC)/astrophys.h
	$(CC) $(INC) -c $< -o $@
	ar -r ../../lib/functionLib $@

depend: $(SOURCES) Makefile
	$(CC) $(CFLAGS) -${INC} -I$(COMMONINC) -MM $(SOURCES) > depend

include depend

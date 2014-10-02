CC = gcc -D LINUX -I$(COMMONINC) -I$(GLOBALINC) -I$(ACCAPP)/rxCode/src/include \
     -I$(ACCAPP)/cryoCode/src/include
CFLAGS = -g -O -Wall
LIB = -lpthread -lrt $(COMMONLIB)/rm.o $(COMMONLIB)/libdsm.a /application/smapopt/libsmapopt.a\
    $(COMMONLIB)/commonLib -lcurses -lm

COMMONINC = /common/include/
COMMONLIB = /common/lib/
GLOBALINC = /global/include/

IS_DOUBLE_BANDWIDTH = /global/isDoubleBandwidth/isDoubleBandwidth.c
IS_FULL_POLARIZATION = /global/isFullPolarization/isFullPolarization.c

SOURCES = gasdev.c nrutil.c svbksb.c svdcmp.c svdfit.c svdvar.c ran1.c \
    monitor.c arrayMonitor.c antMonitor.c  dDSCursesMonitor.c iFLOMonitor.c \
    iFLOMonitorPage2.c iFLOMonitorPage3.c iFLOMonitorPage4.c deicemon.c \
    commandMonitor.c correlatorMonitor.c allanVariance.c genset.c \
    aCMonitor.c ipcensus.c driversLoaded.c networkPage.c opointPage.c \
    receiverMonitor.c help.c users.c intruderPage.c messagePage.c airHandler.c stateCounts.c \
    smainitMonitor.c listDead.c antPage2.c timeStamp.c dewarpage.c coherence.c opMsg.c \
    correlatorSummary.c opticsPage.c lomotorpage.c pmodels.c projectpage.c c1DC.c \
    polar.c upspage.c upspage2.c weather.c receiverMonitorHighFreq.c chopperPage.c\
    parseFITS.c rscanpage.c arraymap.c tiltpage.c uptimes.c mRG.c hangarPage.c seeing.c\
    weatherPage2.c gpsPage.c flagging.c opTel.c solveGains.c BDC_8_12.c swarmPage.c \
    yIGFrequencies.c

OBJECTS = gasdev.o nrutil.o svbksb.o svdcmp.o svdfit.o svdvar.o ran1.o \
    monitor.o arrayMonitor.o antMonitor.o  dDSCursesMonitor.o iFLOMonitor.o \
    iFLOMonitorPage2.o iFLOMonitorPage3.o iFLOMonitorPage4.o deicemon.o \
    commandMonitor.o correlatorMonitor.o allanVariance.o genset.o solveGains.o \
    aCMonitor.o ipcensus.o driversLoaded.o networkPage.o opointPage.o \
    receiverMonitor.o help.o users.o intruderPage.o messagePage.o airHandler.o stateCounts.o \
    smainitMonitor.o listDead.o antPage2.o timeStamp.o dewarpage.o coherence.o opMsg.o \
    correlatorSummary.o opticsPage.o lomotorpage.o pmodels.o projectpage.o c1DC.o \
    polar.o upspage.o upspage2.o weather.o receiverMonitorHighFreq.o chopperPage.o\
    parseFITS.o rscanpage.o arraymap.o tiltpage.o uptimes.o mRG.o hangarPage.o seeing.o\
    weatherPage2.o gpsPage.o flagging.o opTel.o BDC_8_12.o swarmPage.o yIGFrequencies.o

all: monitor

clean:
	rm *.o ./monitor

install: all
	cp monitor $(OBSBIN)/

monitor: $(OBJECTS) $(IS_DOUBLE_BANDWIDTH) $(IS_FULL_POLARIZATION) ./Makefile
	$(CC) -o monitor $(OBJECTS) $(IS_DOUBLE_BANDWIDTH) $(IS_FULL_POLARIZATION) $(LIB)

depend: ./Makefile
	$(CC) -MM $(SOURCES) > dependencies

include dependencies

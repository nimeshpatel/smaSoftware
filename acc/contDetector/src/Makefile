#$Log: Makefile,v $
#Revision 1.13  2014/01/08 21:24:27  rwilson
#make depend automatic
#
#Revision 1.12  2006/05/17 15:49:27  rwilson
#New DSm location
#
#Revision 1.11  2006/03/09 14:42:18  rwilson
#DSM in global, dependencies
#
#Revision 1.10  2006/01/25 16:36:51  rwilson
#add depend
#
#Revision 1.9  2005/09/26 14:11:26  rwilson
#Updated ld dependencies
#
#Revision 1.8  2005/05/28 00:25:08  rwilson
#Writing DSM_CHOPPER_TIMING_V6_D to crates
#
#Revision 1.7  2004/11/24 21:25:50  rwilson
#rmeove -llynx
#
#Revision 1.6  2004/11/11 17:59:50  rwilson
#Add Gunn Bias to testCD
#
#Revision 1.5  2004/09/01 15:25:22  rwilson
#Add smapopt to setcdmode -g for syncdet2
#
#Revision 1.4  2004/08/23 14:18:52  rwilson
#Add syncdet2, new cmds for testCD
#
#Revision 1.3  2004/06/30 17:42:25  rwilson
#changes for testing in ant 3
#
#Revision 1.2  2003/03/20 14:15:19  rwilson
#ContDetector works as driver with test setup
#
#Revision 1.1.1.1  2002/06/13 15:12:26  rwilson
#Early non-working version
#
# Makefile for the driver for the continuum detector integrator
# using the Acromag IP480-6 counter card.
#
# Charlie's scheme to adapt ot different hosts
# pick the definitions based on the OS
ifeq ($(HOSTTYPE),i386-linux)
  CFLAGS=-c -Wall
  DSMLD= $(GLOBALLIB)/libdsm.a -lrt -lpthread
  DSMLIB=$(GLOBALLIB)/libdsm.a
  INC= -I$(GLOBALINC) -I$(COMMONINC)
  OSFOUND = 1
endif

ifeq ($(HOSTTYPE),lynxos-powerpc)
  CFLAGS=-c -Wall
  DSMLD= $(COMMONLIB)/libdsm.a -lrpc -lnetinet 
  DSMLIB=$(COMMONLIB)/libdsm.a
  INC= -I$(COMMONINC) -I$(GLOBALINC) -I../include
  OSFOUND = 1
endif

ifeq ($(HOSTTYPE),sun4)
  CFLAGS=-c -Wall
  DSMLD= $(COMMONLIB)/libdsm.a -lrpc -lnetinet 
  DSMLIB=$(COMMONLIB)/libdsm.a
  INC= -I$(COMMONINC) -I$(GLOBALINC) -I../include
  OSFOUND = 1
endif

KINC = $(ENV_PREFIX)/sys/include/kernel
PINC = $(ENV_PREFIX)/sys/include/family/ppc/
UDINC = $(ACCAPP)/iPUniDig_D/includeFiles
SHM = $(ACCAPP)/servo/openshm.o
# SHM = $(COMMONLIB)/openshm.o
#THLIBS = -llynx -mthreads
THLIBS = -mthreads
CLIB = $(COMMONLIB)/commonLib
CC = gcc
#CFLAGS = -ansi -pedantic -Wall -Wcast-align -O -finline-functions\
#	-I$(INC) -I$(KINC) -I$(PINC)
CFLAGS = -ansi -Wall -Wcast-align -O -finline-functions $(INC)
LD = ld
CC=gcc
OBJ =contDetector.obj mkcontDetectorInfo contDetector.import \
	testCD testIP $(COMMONBIN)/setcdmode $(ACCBIN)/testUD \
	../../bin/syncdet2
#	testcntr
SOURCES = contDetector.c mkcontDetectorInfo.c syncdet2.c setcdmode.c \
	testCD.c testIP.c testUD.c

all: depend $(OBJ)
#all: testcntr testUD mkcontDetectorInfo contDetector.obj

contDetector.obj: contDetector.o contDetector.import
	$(LD) -bM:SRE -bimport:contDetector.import \
           -o contDetector.obj contDetector.o

contDetector.o: contDetector.c ../include/contDetectordrvr.h\
	../include/contDetector.h
	$(CC) $(CFLAGS) -c -I$(KINC) -I$(PINC) -o contDetector.o contDetector.c

contDetector.s: contDetector.c ../include/contDetectordrvr.h\
	../include/contDetector.h
	$(CC) $(CFLAGS) -S -I$(KINC) -I$(PINC) contDetector.c

contDetector.import: contDetector.o
	nm -u contDetector.o > contDetector.import

mkcontDetectorInfo: mkcontDetectorInfo.c ../include/contDetectordrvr.h
	$(CC) $(INC) mkcontDetectorInfo.c -o mkcontDetectorInfo

testcntr: testcntr.o
	$(CC) -o testcntr testcntr.o $(ENV_PREFIX)/usr/lib/libreadline.a

testcntr.o: testcntr.c ../include/contDetectordrvr.h ../include/contDetector.h
	$(CC) $(CFLAGS) -I$(KINC) -I$(PINC) -c testcntr.c

../../bin/syncdet2: syncdet2.o $(DSMLIB) $(SHM) $(COMMONLIB)/rm.o
	$(CC) -g -o ../../bin/syncdet2 $(COMMONLIB)/rm.o syncdet2.o \
		$(COMMONLIB)/commonLib $(DSMLD) $(SHM) $(THLIBS)

syncdet2.o: syncdet2.c $(COMMONINC)/patchPanelBits.h \
	../include/contDetectordrvr.h ../include/contDetector.h
	$(CC) -g $(CFLAGS) -I$(COMMONINC) -I$(UDINC) -c syncdet2.c

syncdet2.s: syncdet2.c $(COMMONINC)/patchPanelBits.h \
	../include/contDetectordrvr.h ../include/contDetector.h
	gcc -c $(CFLAGS) -g -O2 -Wa,-ahl,-L -I$(COMMONINC) -I$(UDINC) syncdet2.c > syncdet2.s

testCD: testCD.o
	$(CC) -o testCD testCD.o $(COMMONLIB)/rm.o -mthreads \
		$(ENV_PREFIX)/usr/lib/libreadline.a

testCD.o: testCD.c $(COMMONINC)/patchPanelBits.h ../include/contDetectordrvr.h \
	../include/contDetector.h
	$(CC) $(CFLAGS) -I$(COMMONINC) -c testCD.c

testIP: testIP.o
	$(CC) -o testIP testIP.o

testIP.o: testIP.c ../include/contDetectordrvr.h
	$(CC) $(CFLAGS) -c testIP.c

$(ACCBIN)/testUD: testUD.o
	$(CC) -o $(ACCBIN)/testUD testUD.o

testUD.o: testUD.c $(COMMONINC)/patchPanelBits.h
	$(CC) $(CFLAGS) -c -I$(COMMONINC) testUD.c

$(COMMONBIN)/setcdmode: setcdmode.o $(DSMLIB)
	$(CC) -o $(COMMONBIN)/setcdmode setcdmode.o $(DSMLD) $(CLIB) \
		$(COMMONLIB)/libsmapopt.a

setcdmode.o: setcdmode.c
	$(CC) $(CFLAGS) -c setcdmode.c

depend: $(SOURCES) Makefile
	$(CC) $(CFLAGS) $(INC) -I$(KINC) -I$(PINC) -I$(GLOBALINC) -MM $(SOURCES) > depend

include depend

clean:
	rm -f $(OBJ) *.o

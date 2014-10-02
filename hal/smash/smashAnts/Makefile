TMP=/usr/lynx/3.1.0/ppc/tmp
BINDIR=../../bin
CC = gcc
CFLAGS = -O -Wall -g -c
LFLAGS = -L$(COMMONLIB)
INC = -I$(COMMONINC) -I$(GLOBALINC) -I$(HALINC) 
LIBS = -llynx -lsmapopt
RM = $(COMMONLIB)/rm.o
DSM = $(COMMONLIB)/libdsm.a -lrpc -lnetinet
CLIB = $(COMMONLIB)/commonLib
VPATH = $(COMMONINC):$(GLOBALINC)
TUNECLNT = $(COMMONLIB)/tune_xdr.o $(COMMONLIB)/tune_clnt.o

all : $(BINDIR)/setSunSafeMinutes $(BINDIR)/azoff  $(BINDIR)/eloff $(BINDIR)/value $(BINDIR)/tsys

$(BINDIR)/setSunSafeMinutes: setSunSafeMinutes.o
	$(CC) -o $(BINDIR)/setSunSafeMinutes setSunSafeMinutes.o $(LFLAGS) \
	$(LIBS) $(RM) $(CLIB)

setSunSafeMinutes.o: setSunSafeMinutes.c
	$(CC) $(CFLAGS) setSunSafeMinutes.c $(INC)

$(BINDIR)/azoff: azoff.c
	$(CC) $^ -o $(BINDIR)/azoff $(INC) $(RM) $(LFLAGS) $(LIBS)

$(BINDIR)/eloff: eloff.c
	$(CC) $^ -o $(BINDIR)/eloff $(INC) $(RM) $(LFLAGS) $(LIBS)

value: value.c
	$(CC) $^ -o $(TMP)/value $(INC) $(RM) $(LFLAGS) $(LIBS)

$(BINDIR)/value: value.c
	$(CC) $^ -o $(BINDIR)/value $(INC) $(RM) $(LFLAGS) $(LIBS)

#tsysnew.o: tsysnew.c commonLib.h tune.h dsm.h rm.h smapopt.h tune6status.h
#	$(CC) $(CFLAGS) $< $(INC) -o $@

#$(BINDIR)/tsys: tsysnew.o $(COMMONLIB)/tune_xdr.o $(COMMONLIB)/tune_clnt.o
#	$(CC) -g $^ -o $(BINDIR)/tsys -lm -mthreads $(LFLAGS) $(LIBS) \
#		$(DSM) $(RM) $(COMMONLIB)/commonLib

tsys.o: tsys.c  rm.h smapopt.h
	$(CC) $(CFLAGS) $< -I$(HALAPP)/include $(INC) -o $@

$(BINDIR)/tsys: tsys.o tsyssub.o $(TUNECLNT)
	$(CC) -g $^ -o $(BINDIR)/tsys -lm -mthreads $(LFLAGS) $(LIBS) \
		$(DSM) $(RM) $(COMMONLIB)/commonLib

tsyssub.o: tsyssub.c commonLib.h tune.h dsm.h rm.h smapopt.h tune6status.h
	$(CC) $(CFLAGS) $< $(INC) -o $@
	ar r $(HALLIB)/smashLib.a tsyssub.o
#

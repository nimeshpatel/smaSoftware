CC = gcc -g -Wall
BINDIR=../bin
BIN = $(BINDIR)/chopperControl
LFLAGS = -L$(COMMONLIB)
#LIB = -llynx  -mthreads 
LIB = -mthreads 
INC = -I$(COMMONINC)
INC232= -I$(ACCAPP)/iPOctal232/includeFiles
SOURCES=  chopperControl.c 232talk.c

all: depend $(BINDIR)/chopperControl $(BINDIR)/232talk

$(BINDIR)/chopperControl: chopperControl.o $(COMMONLIB)/rm.o \
	$(COMMONLIB)/commonLib
	$(CC) -o $(BINDIR)/chopperControl chopperControl.o $(COMMONLIB)/rm.o \
	$(COMMONLIB)/commonLib $(LIB) ${LFLAGS} -lrpc -lnetinet

chopperControl.o: chopperControl.c
	$(CC) -c chopperControl.c $(INC) $(INC232)

$(BINDIR)/232talk: 232talk.o
	$(CC) -o $(BINDIR)/232talk 232talk.o $(LIB)

232talk.o: 232talk.c
	$(CC) -c 232talk.c $(INC232)

clean:
	rm -f $(BINDIR)/chopperControl $(BINDIR)/232talk *.o

depend: $(SOURCES) Makefile
	$(CC) $(INC) $(INC232) -MM $(SOURCES) > depend

include depend

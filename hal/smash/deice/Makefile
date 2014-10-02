CFLAGS = -Wall -g -c
LFLAGS = -L$(COMMONLIB)
INC = -I$(COMMONINC)
LIBS = -llynx -lsmapopt
RM = $(COMMONLIB)/rm.o
BINDIR = ../../bin
OBJS=deice.o deicemon.o
BINS = $(BINDIR)/deicemon $(BINDIR)/deice

all: $(BINS)

$(BINDIR)/deice : deice.o $(RM) $(COMMONLIB)
	$(CC) -o $(BINDIR)/deice deice.o $(RM) $(LFLAGS) -lrpc -lnetinet $(LIBS)
	cp $(BINDIR)/deice $(ACCBIN)

deice.o : deice.c
	$(CC) $(CFLAGS) -o deice.o deice.c  $(INC)

$(BINDIR)/deicemon: deicemon.o
	$(CC) -o $(BINDIR)/deicemon deicemon.o $(LFLAGS) $(RM) -lcurses

deicemon.o: deicemon.c
	$(CC) $(CFLAGS) $(INC) deicemon.c

clean:
	rm -f $(BINS) $(OBJS)

depend: ${OBJS:.o=.c}
	$(CC) $(CFLAGS) $(INC) -MM ${OBJS:.o=.c} > depend

include depend

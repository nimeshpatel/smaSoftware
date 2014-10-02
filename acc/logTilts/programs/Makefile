RM = $(COMMON)/reflmem/api
BIN = ../test/

all: $(BIN)logTiltsMain

install: all
	cp $(BIN)logTiltsMain $(ACCBIN)/

$(BIN)logTiltsMain: logTiltsMain.c $(ACCLIB)/functionLib(logTilts.o) Makefile
	gcc -g -o $(BIN)logTiltsMain -I$(RM) -I$(COMMONINC) logTiltsMain.c \
	$(ACCLIB)/functionLib $(COMMONLIB)/commonLib $(RM)/rm.o -lrpc -lnetinet

$(ACCLIB)/functionLib(logTilts.o): logTilts.c Makefile
	gcc -g -c -I$(RM) -I$(COMMONINC) logTilts.c
	ar -r $(ACCLIB)/functionLib logTilts.o

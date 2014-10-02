BINDIR=$(ACCBIN)
CC=gcc
CFLAGS=-c -Wall -I../include
INITINC=$(COMMON)/smainit
UNIDIGINC=$(ACCAPP)/iPUniDig_D/includeFiles

all: chopperBits

chopperBits: chopperBits.c
	$(CC) -o chopperBits -I $(UNIDIGINC) chopperBits.c

deiced.o: deiced.c  ../include/deiced.h $(INITINC)/smadaemon.h \
	$(INC485)/iPOctal485.h
	$(CC) $(CFLAGS) -I$(INITINC) -I$(INC485) deiced.c

BINDIR=../../bin
CC = gcc
CFLAGS = -O -Wall -g -c
BFLAGS = -O -g 
LFLAGS = -L$(COMMONLIB)
INC = -I$(COMMONINC) 
LIBS = -llynx -lsmapopt
RM = $(COMMONLIB)/rm.o
#DSM = $(COMMONLIB)/libdsm.a
CLIB = $(COMMONLIB)/commonLib
COMMANDS = $(BINDIR)/checkChopperHome $(BINDIR)/farChopper\
	$(BINDIR)/homeChopper $(BINDIR)/initChopper $(BINDIR)/nearChopper\
	$(BINDIR)/startChopping $(BINDIR)/stopChopping $(BINDIR)/stowChopper \
	$(BINDIR)/resetChopper $(BINDIR)/restartChopper \
	$(BINDIR)/driftChopper
FOCUSCOMMANDS = $(BINDIR)/freezeFocus $(BINDIR)/focusCurveOff
SOURCES = chopperCommands.c iChopper.c positionChopper.c focusCurveOn.c 


all : depend $(BINDIR)/iChopper $(BINDIR)/chopperCommands \
	$(BINDIR)/positionChopper ${COMMANDS} $(FOCUSCOMMANDS)

${COMMANDS}: $(BINDIR)/chopperCommands
	rm -f $@; ln $(BINDIR)/chopperCommands $@

iChopper.o : iChopper.c
	${CC} ${CFLAGS} -o iChopper.o iChopper.c ${INC}

$(BINDIR)/iChopper : iChopper.o
	${CC} ${BFLAGS} -o $(BINDIR)/iChopper iChopper.o ${LFLAGS} ${RM} \
		${LIBS} $(ENV_PREFIX)/usr/lib/libreadline.a

chopperCommands.o: chopperCommands.c
	${CC} ${CFLAGS} chopperCommands.c ${INC}

$(BINDIR)/chopperCommands: chopperCommands.o
	${CC} -o $(BINDIR)/chopperCommands chopperCommands.o \
		${LFLAGS} ${RM} ${LIBS} ${CLIB}

positionChopper.o: positionChopper.c
	${CC} ${CFLAGS} positionChopper.c ${INC}

$(BINDIR)/positionChopper: positionChopper.o
	${CC} -o $(BINDIR)/positionChopper positionChopper.o \
		${LFLAGS} ${RM} ${LIBS} ${CLIB}

$(BINDIR)/focusCurveOn: focusCurveOn.o
	${CC} -o $(BINDIR)/focusCurveOn focusCurveOn.o \
		${LFLAGS} ${RM} ${LIBS} ${CLIB}

${FOCUSCOMMANDS}: $(BINDIR)/focusCurveOn
	rm -f $@; ln $(BINDIR)/focusCurveOn $@

focusCurveOn.o: focusCurveOn.c
	${CC} ${CFLAGS} focusCurveOn.c ${INC}

clean:
	rm -f $(BINDIR)/iChopper $(BINDIR)/chopperCommands \
		$(BINDIR)/positionChopper ${COMMANDS} $(FOCUSCOMMANDS) *.o

depend: $(SOURCES) Makefile
	$(CC) $(CFLAGS) ${INC}  -MM $(SOURCES) > depend

include depend

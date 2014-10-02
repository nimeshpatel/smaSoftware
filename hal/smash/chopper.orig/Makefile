CC = gcc
CFLAGS = -O -Wall -g -c
BFLAGS = -O -g 
LFLAGS = -L$(COMMONLIB)
INC = -I$(COMMONINC) 
LIBS = -llynx -lsmapopt
RM = $(COMMONLIB)/rm.o
DSM = $(COMMONLIB)/libdsm.a
CLIB = $(COMMONLIB)/commonLib
SMASH = startChopping stopChopping  homeChopper stowChopper chopperX \
chopperY chopperZ  farChopper nearChopper chopperCommand \
chopperTilt resetChopLimits stowChopper

all :   $(SMASH) iChopper


iChopper.o : iChopper.c
	${CC} ${CFLAGS} -o iChopper.o iChopper.c ${INC}

iChopper : iChopper.o
	${CC} ${BFLAGS} -o iChopper iChopper.o ${LFLAGS} ${RM} ${LIBS} \
		$(ENV_PREFIX)/usr/lib/libreadline.a

nearChopper.o : nearChopper.c
	${CC} ${CFLAGS} -o nearChopper.o nearChopper.c ${INC}

nearChopper : nearChopper.o
	${CC} ${BFLAGS} -o nearChopper nearChopper.o ${LFLAGS} ${RM} ${LIBS}

farChopper.o : farChopper.c
	${CC} ${CFLAGS} -o farChopper.o farChopper.c ${INC}

farChopper : farChopper.o
	${CC} ${BFLAGS} -o farChopper farChopper.o ${LFLAGS} ${RM} ${LIBS}

resetChopLimits.o : resetChopLimits.c
	${CC} ${CFLAGS} -o resetChopLimits.o resetChopLimits.c ${INC}

resetChopLimits : resetChopLimits.o
	${CC} ${BFLAGS} -o resetChopLimits resetChopLimits.o ${LFLAGS} ${RM} ${LIBS}

chopperTilt.o : chopperTilt.c
	${CC} ${CFLAGS} -o chopperTilt.o chopperTilt.c ${INC}

chopperTilt : chopperTilt.o
	${CC} ${BFLAGS} -o chopperTilt chopperTilt.o ${LFLAGS} ${RM} ${LIBS}

chopperZ.o : chopperZ.c
	${CC} ${CFLAGS} -o chopperZ.o chopperZ.c ${INC}

chopperZ : chopperZ.o
	${CC} ${BFLAGS} -o chopperZ chopperZ.o ${LFLAGS} ${RM} ${LIBS}

chopperY.o : chopperY.c
	${CC} ${CFLAGS} -o chopperY.o chopperY.c ${INC}

chopperY : chopperY.o
	${CC} ${BFLAGS} -o chopperY chopperY.o ${LFLAGS} ${RM} ${LIBS}

chopperX.o : chopperX.c
	${CC} ${CFLAGS} -o chopperX.o chopperX.c ${INC}

chopperX : chopperX.o
	${CC} ${BFLAGS} -o chopperX chopperX.o ${LFLAGS} ${RM} ${LIBS}

stowChopper.o : stowChopper.c
	${CC} ${CFLAGS} -o stowChopper.o stowChopper.c ${INC}

stowChopper : stowChopper.o
	${CC} ${BFLAGS} -o stowChopper stowChopper.o ${LFLAGS} ${RM} ${LIBS}

homeChopper.o : homeChopper.c
	${CC} ${CFLAGS} -o homeChopper.o homeChopper.c ${INC}

homeChopper : homeChopper.o
	${CC} ${BFLAGS} -o homeChopper homeChopper.o ${LFLAGS} ${RM} ${LIBS}

chopperCommand.o : chopperCommand.c
	${CC} ${CFLAGS} -o chopperCommand.o chopperCommand.c ${INC}

chopperCommand : chopperCommand.o
	${CC} ${BFLAGS} -o chopperCommand chopperCommand.o ${LFLAGS} ${RM} ${LIBS}

startChopping.o : startChopping.c
	${CC} ${CFLAGS} -o startChopping.o startChopping.c ${INC}

startChopping : startChopping.o
	${CC} ${BFLAGS} -o startChopping startChopping.o ${LFLAGS} ${RM} ${LIBS}

stopChopping.o : stopChopping.c
	${CC} ${CFLAGS} -o stopChopping.o stopChopping.c ${INC}

stopChopping : stopChopping.o
	${CC} ${BFLAGS} -o stopChopping stopChopping.o ${LFLAGS} ${RM} ${LIBS}

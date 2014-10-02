all: perlscripts tiltfit

OBJS = tiltfn.o tiltfit.o gasdev.o nrutil.o svbksb.o svdcmp.o svdfit.o svdvar.o ran1.o

CC = gcc 
CFLAGS = 
#-DANSI
INC = -I./

perlscripts: fitdata fitresults getAllTilts getTilts putTilts reduceTilts tiltplot
	cp $^ /application/bin

tiltfit : ${OBJS}
	${CC} ${CFLAGS} -o tiltfit ${OBJS} ${INC} -lm
ran1.o : ran1.c
	${CC} -c ran1.c ${INC}
tiltfn.o : tiltfn.c
	${CC} -c tiltfn.c ${INC}
gasdev.o : gasdev.c
	${CC} -c gasdev.c 
nrutil.o : nrutil.c
	${CC} -c nrutil.c 
svbksb.o : svbksb.c
	${CC} -c svbksb.c 
svdcmp.o : svdcmp.c
	${CC} -c svdcmp.c 
svdfit.o : svdfit.c
	${CC} -c svdfit.c 
svdvar.o : svdvar.c
	${CC} -c svdvar.c 
tiltfit.o : tiltfit.c
	${CC} -c tiltfit.c 

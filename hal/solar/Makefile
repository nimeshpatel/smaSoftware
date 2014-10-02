INC = -I$(COMMONINC) -I$(COMMON)/vme_sg_simple/
CC = gcc -g
CFLAGS = -c $(INC)
LDLIBS = 
OTHERLIBS = -lm 

BIN_DIR = ./
OBJ_DIR = ./
OBJ_FILES = $(COMMONLIB)/rm.o



PROG = solar
PROGRAM = $(BIN_DIR)$(PROG)
PROG_OBJS = $(OBJ_DIR)solar.o	\
	$(OBJ_DIR)nod.o 	\
	$(OBJ_DIR)etilt.o 	\
	$(OBJ_DIR)refract.o 	\
	$(OBJ_DIR)sidtim.o	\
	$(OBJ_DIR)novas.o	\
	$(OBJ_DIR)solsys3.o

all:	$(PROGRAM)

$(PROGRAM):	$(PROG_OBJS)
	$(CC) -o $(PROGRAM) $(PROG_OBJS) $(OBJ_FILES) $(OTHERLIBS) 

$(OBJ_DIR)solar.o: solar.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)solar.o solar.c

$(OBJ_DIR)nod.o: nod.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)nod.o nod.c

$(OBJ_DIR)etilt.o: etilt.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)etilt.o etilt.c

$(OBJ_DIR)refract.o: refract.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)refract.o refract.c

$(OBJ_DIR)sidtim.o: sidtim.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)sidtim.o sidtim.c

$(OBJ_DIR)novas.o: novas.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)novas.o novas.c

$(OBJ_DIR)solsys3.o: solsys3.c
	$(CC) $(CFLAGS) -o $(OBJ_DIR)solsys3.o solsys3.c

clean:
	rm -f $(PROG_OBJS)
	rm -f $(PROGRAM)

CC     = g++ -g
CFLAGS = -O2 $(INCLUDE) $(LIBRARY) -Wall
INCLUDE    = -I . 
LIBRARY    = -L /usr/local/lib 
LDFLAGS = -lsbigudrv -lcfitsio -lpthread -lm -lz 
INCLUDE_FITSIO = 1
OUT_DIR = ./

all: snapshot

snapshot: snapshot.o csbigcam.o csbigimg.o 
	$(CC) $(CFLAGS) snapshot.o csbigcam.o csbigimg.o $(LIBRARY) $(LDFLAGS) -o snapshot

snapshot.o : snapshot.cpp csbigcam.h csbigimg.h
	  $(CC) $(CFLAGS) -o snapshot.o -c snapshot.cpp

csbigcam.o : csbigcam.cpp csbigcam.h csbigimg.h
	  $(CC) $(CFLAGS) -o csbigcam.o -c csbigcam.cpp

csbigimg.o : csbigimg.cpp csbigimg.h
	  $(CC) $(CFLAGS) -o csbigimg.o -c csbigimg.cpp

clean:
		rm -f *.o

install:
		cp snapshot /usr/local/bin/snapshot

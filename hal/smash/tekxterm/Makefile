CC=gcc -Wall

all: testtek $(HALBIN)/stripchart $(HALBIN)/stripchart8 $(HALBIN)/ivplot8 \
     $(HALBIN)/stripchart8sync $(HALBIN)/ivplot

VPATH=$(COMMONLIB):$(COMMONINC)

testtek: testtek.o tek_driv.o
	$(CC) $^ -o $@

testtek.o: testtek.c tek_driv.h
	$(CC) -c $< -o $@

$(HALBIN)/stripchart: stripchart.o tek_driv.o rm.o
	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

$(HALBIN)/stripchart8: stripchart8.o tek_driv.o rm.o
	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

$(HALBIN)/stripchart8sync: stripchart8sync.o tek_driv.o rm.o
	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

$(HALBIN)/ivplot8: ivplot8.o tek_driv.o rm.o
	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

$(HALBIN)/ivplot: ivplot.o tek_driv.o rm.o
	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

#$(ACCBIN)/stripchart: stripchart.o tek_driv.o rm.o
#	$(CC) -L$(COMMONLIB) -lm $^ -lsmapopt -o $@

stripchart8sync.o: stripchart8sync.c tek_driv.h
	$(CC) -I$(COMMONINC) -c $< -o $@

stripchart8.o: stripchart8.c tek_driv.h
	$(CC) -I$(COMMONINC) -c $< -o $@

ivplot8.o: ivplot8.c tek_driv.h
	$(CC) -I$(COMMONINC) -c $< -o $@

ivplot.o: ivplot.c tek_driv.h
	$(CC) -I$(COMMONINC) -c $< -o $@

stripchart.o: stripchart.c tek_driv.h
	$(CC) -I$(COMMONINC) -c $< -o $@

tek_driv.o: tek_driv.c tek_driv.h
	$(CC) -c $< -o $@
#

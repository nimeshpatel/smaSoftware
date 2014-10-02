# ************************************************************** 
# If you get the following error when trying to run make on Solaris:
#
#   make: Fatal error in reader: Makefile, line 8: Unexpected end of line seen
#
# Please remember to run "gmake" instead of make.  We need to run
# gmake now because it is compatible with Linux (gnu) make, and provides for
# an easy way to detect when we are compiling under Linux. - Thanks, Todd
#
#
CC = gcc -g
BIN = rpoint
FCC = g77 -g
PH=
#PH=/application/lib/ph.o

ifeq ($(HOSTTYPE),sun4)
  LIBPGPLOT=/usr/local/pgplot
  all: 
else
  LIBPGPLOT=/home/miriad/lib/linux
  all: rpoint /application/bin/aperEff
endif


/application/bin/aperEff: apereff.c
	$(CC) apereff.c -o $@ -lm

rpoint:rpoint.o nrutil.o nrpoint.o nrpoint2.o mrqmin.o mrqcof.o covsrt.o gaussj.o fgauss2.o rms.o $(PH)
	$(FCC) -o rpoint rpoint.o nrpoint.o nrpoint2.o nrutil.o mrqmin.o mrqcof.o \
	covsrt.o gaussj.o fgauss2.o rms.o $(PH) $(LIBPGPLOT)/libcpgplot.a \
        $(LIBPGPLOT)/libpgplot.a -lm \
        -L/usr/X11R6/lib -lX11

nrpoint.o:nrpoint.c mrqmin.o mrqcof.o covsrt.o gaussj.o fgauss2.o rms.o rpoint.h
	$(CC) -c nrpoint.c -o nrpoint.o

nrpoint2.o:nrpoint2.c mrqmin.o mrqcof.o covsrt.o gaussj.o fgauss2.o rms.o rpoint.h
	$(CC) -c nrpoint2.c -o nrpoint2.o

rpoint.o:rpoint.c rpoint.h
	$(CC) -c rpoint.c -o rpoint.o 

nrutil.o:nrutil.c
	$(CC) -c nrutil.c -o nrutil.o

mrqmin.o:mrqmin.c
	$(CC) -c mrqmin.c -o mrqmin.o

mrqcof.o:mrqcof.c
	$(CC) -c mrqcof.c -o mrqcof.o

covsrt.o:covsrt.c
	$(CC) -c covsrt.c -o covsrt.o

gaussj.o:gaussj.c
	$(CC) -c gaussj.c -o gaussj.o

fgauss2.o:fgauss2.c
	$(CC) -c fgauss2.c -o fgauss2.o

rms.o:rms.c
	$(CC) -c rms.c -o rms.o

clean cleanup:
	rm -f *.o $(BIN)

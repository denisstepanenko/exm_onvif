CFLAGS=-g -Wall
CC=arm-hisiv100nptl-linux-gcc
AR=ar
RANLIB=ranlib
LIBS=-L./ -lcgic

all: libcgic.a cgictest.cgi capture

install: libcgic.a
	cp libcgic.a /usr/local/lib
	cp cgic.h /usr/local/include
	@echo libcgic.a is in /usr/local/lib. cgic.h is in /usr/local/include.

libcgic.a: cgic.o cgic.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o
	$(RANLIB) libcgic.a
	cp -rfv libcgic.a ../../lib/

#mingw32 and cygwin users: replace .cgi with .exe

cgictest.cgi: cgictest.o libcgic.a
	$(CC) cgictest.o -o cgictest.cgi ${LIBS}

capture: capture.o libcgic.a
	$(CC) capture.o -o capture ${LIBS}

cp:    
	cp -rfv cgictest.cgi /nfs/zsk/www/cgi-bin
clean:
	rm -f *.o *.a cgictest.cgi capture


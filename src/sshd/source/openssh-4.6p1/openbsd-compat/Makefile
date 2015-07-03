# $Id: Makefile.in,v 1.40 2006/08/30 17:24:41 djm Exp $

sysconfdir=${prefix}/etc
piddir=/var/run
srcdir=.
top_srcdir=..


CC=/opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/bin/arm-hisiv200-linux-gcc
LD=/opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/bin/arm-hisiv200-linux-gcc
CFLAGS=-g -O2 -Wall -Wpointer-arith -Wuninitialized -Wsign-compare -Wno-pointer-sign 
CPPFLAGS=-I. -I.. -I$(srcdir) -I$(srcdir)/.. -I/home/lc/vserver/GTIP-E_studio/libsrc/openssl-0.9.7e/include -I/home/lc/vserver/GTIP-E_studio/libsrc/zlib/bin//include  -DHAVE_CONFIG_H
LIBS=-lcrypto -lutil -lz -lnsl  -lcrypt -lresolv -lresolv
AR=/opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/bin/arm-hisiv200-linux-ar
RANLIB=arm-hisiv200-linux-ranlib
INSTALL=/usr/bin/install -c
LDFLAGS=-L. -L/home/lc/vserver/GTIP-E_studio/libsrc/openssl-0.9.7e -L/home/lc/vserver/GTIP-E_studio/libsrc/zlib/bin//lib 

OPENBSD=base64.o basename.o bindresvport.o daemon.o dirname.o getcwd.o getgrouplist.o getopt.o getrrsetbyname.o glob.o inet_aton.o inet_ntoa.o inet_ntop.o mktemp.o readpassphrase.o realpath.o rresvport.o setenv.o setproctitle.o sha2.o sigact.o strlcat.o strlcpy.o strmode.o strsep.o strtonum.o strtoll.o strtoul.o vis.o

COMPAT=bsd-arc4random.o bsd-asprintf.o bsd-closefrom.o bsd-cray.o bsd-cygwin_util.o bsd-getpeereid.o bsd-misc.o bsd-nextstep.o bsd-openpty.o bsd-snprintf.o bsd-waitpid.o fake-rfc2553.o openssl-compat.o xmmap.o xcrypt.o

PORTS=port-aix.o port-irix.o port-linux.o port-solaris.o port-tun.o port-uw.o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

all: libopenbsd-compat.a

$(COMPAT): ../config.h
$(OPENBSD): ../config.h
$(PORTS): ../config.h

libopenbsd-compat.a:  $(COMPAT) $(OPENBSD) $(PORTS)
	$(AR) rv $@ $(COMPAT) $(OPENBSD) $(PORTS)
	$(RANLIB) $@

clean:
	rm -f *.o *.a core 

distclean: clean
	rm -f Makefile *~

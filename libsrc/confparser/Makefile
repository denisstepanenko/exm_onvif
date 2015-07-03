#
# iniparser Makefile
#

ARFLAGS = rcv
RM      = rm -f

ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS += -DEMBEDED=1
	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
	LIBDIR = /usr/local/arm/2.95.3/arm-linux/lib
	AR          = arm-linux-ar
	OUTDIR = $(ROOTDIR)/../emblib
else
	CC = gcc
	CFLAGS += -DEMBEDED=0
	AR          = ar
	OUTDIR = $(ROOTDIR)/../pclib
endif
CFLAGS  += -O3 -I$(ROOTDIR)/../include
# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c

	#@(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = confparser.c
OBJS = $(SRCS:.c=.o)


default:	libconfparser.a

libconfparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libconfparser.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libconfparser.a

veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libconfparser.a
	rm -rf ./html ; mkdir html
symlink:
docs:
	@(cd doc ; $(MAKE))
	
check:
	@(cd test ; $(MAKE))

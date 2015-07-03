#
# audiofmt  Makefile
#

ARFLAGS = rcv
RM      = rm -f

ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS += -DEMBEDED=1
	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
	LIBDIR = /usr/local/arm/2.95.3/arm-linux/lib
	AR          = arm-linux-ar
	OUTDIR = ../../emblib#$(ROOTDIR)/../emblib
else
	CC = gcc
	CFLAGS += -DEMBEDED=0
	AR          = ar
	OUTDIR = ../../pclib#$(ROOTDIR)/../pclib
endif
#CFLAGS  += -O3 -I$(ROOTDIR)/../include
CFLAGS += -O3 -I../../include
# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c

	#@(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = audiofmt.c
OBJS = $(SRCS:.c=.o)


default:	libaudiofmt.a

libaudiofmt.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libaudiofmt.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libaudiofmt.a

veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libaudiofmt.a
	rm -rf ./html ; mkdir html
symlink:
docs:
	@(cd doc ; $(MAKE))
	
check:
	@(cd test ; $(MAKE))

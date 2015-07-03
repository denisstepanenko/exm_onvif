#
# iniparser Makefile
#

#CC      = gcc
#EMBED = 1
ARFLAGS = rcv
RM      = rm -f



ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS += -DEMBEDED=1
	AR          = arm-linux-ar

	OUTDIR = $(ROOTDIR)/../emblib
else
	CC = gcc
	CFLAGS += -DEMBEDED=0
	AR          = ar
	OUTDIR = $(ROOTDIR)/../pclib
endif

CFLAGS  += -I../../include -O3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = fixdisk.c

OBJS = $(SRCS:.c=.o)


default:	libfixdisk.a

libfixdisk.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libfixdisk.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libfixdisk.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libfixdisk.a
	rm -rf ./html ; mkdir html


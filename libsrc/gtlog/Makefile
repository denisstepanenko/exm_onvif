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

CFLAGS  += -I../../include -O3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = gtlog.c

OBJS = $(SRCS:.c=.o)


default:	libgtlog.a

libgtlog.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libgtlog.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libgtlog.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libgtlog.a
	rm -rf ./html ; mkdir html


#
# hdctl Makefile
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


SRCS = hdctl.c  smart.c

OBJS = $(SRCS:.c=.o)


default:	libhdctl.a

libhdctl.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libhdctl.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libhdctl.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libhdctl.a
	rm -rf ./html ; mkdir html


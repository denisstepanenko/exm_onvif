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

CFLAGS  += -I../../include -I. -O1
#3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $< )


SRCS = src/gt_dev_api.c  

OBJS = $(SRCS:.c=.o)


default:	libgtdevapi.a

libgtdevapi.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libgtdevapi.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libgtdevapi.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libgtdevapi.a
	rm -rf ./html ; mkdir html


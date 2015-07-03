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


SRCS = media_api.c venc_read.c
SRCS += aenc_read.c
OBJS = $(SRCS:.c=.o)


default:	libmedia_api.a

libmedia_api.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libmedia_api.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libmedia_api.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libmedia_api.a
	rm -rf ./html ; mkdir html


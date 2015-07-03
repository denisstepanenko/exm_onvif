#
# iniparser Makefile
#

#CC      = gcc
EMBED = 1
ARFLAGS = rcv
RM      = rm -f

ifdef EMBED
	CC=arm-hisiv200-linux-gcc
	CFLAGS += -DEMBEDED=1
	AR          = arm-hisiv200-linux-ar
	OUTDIR = ../../lib/
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


SRCS = ../gtcomapi_ssl/gt_com_api.c ../gtcomapi_ssl/tab_crc32.c 

OBJS = $(SRCS:.c=.o)


default:	libgtcomapi.a

libgtcomapi.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libgtcomapi.a $(OBJS))
	$(RM) $(OBJS)
clean:
	$(RM) $(OBJS) $(OUTDIR)/libgtcomapi.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libgtcomapi.a
	rm -rf ./html ; mkdir html


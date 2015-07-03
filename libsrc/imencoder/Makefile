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
ifndef ROOTDIR
	ROOTDIR=..
endif
	OUTDIR = $(ROOTDIR)/../pclib
endif

CFLAGS  += -I../../include -O2




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = AVIEncoder.c ime6410api.c iic.c 
SRCS +=imemotion.c 

OBJS = $(SRCS:.c=.o)


default:	libimencoder.a

libimencoder.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libimencoder.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libimencoder.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libimencoder.a
	rm -rf ./html ; mkdir html


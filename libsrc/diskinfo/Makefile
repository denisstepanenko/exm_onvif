
#CC      = gcc
#EMBED = 1
ARFLAGS = rcv
RM      = rm -f

ifdef EMBED
	CC=arm-hisiv200-linux-gcc
	CFLAGS += -DEMBEDED=1
	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
	LIBDIR = /usr/local/arm/2.95.3/arm-linux/lib
	AR          = arm-hisiv200-linux-ar
	OUTDIR = $(ROOTDIR)/../emblib
else
	CC = gcc
	CFLAGS += -DEMBEDED=0
	AR          = ar
	OUTDIR = $(ROOTDIR)/../pclib
endif
CFLAGS  += -O1 -I$(ROOTDIR)/../include
#CFLAGS  += -I../../include -I. -O1
#3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $< )


SRCS = diskinfo.c 

OBJS = $(SRCS:.c=.o)


default:	libdiskinfo.a

libdiskinfo.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libdiskinfo.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libdiskinfo.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libdiskinfo.a
	rm -rf ./html ; mkdir html


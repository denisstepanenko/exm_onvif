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

CFLAGS  += -I../../include 
CFLAGS  += -O3 -fpic
LDLIBS = -shared 
# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = devinfo.c devtype.c devres.c devinfo_virdev.c
OBJS = $(SRCS:.c=.o)
OUTPUT = libdevinfo.so

default:	$(OUTPUT)
libdevinfo.so: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUTDIR)/$(OUTPUT) $(LDLIBS)

#libdevinfo.a:	$(OBJS)
#	@($(AR) $(ARFLAGS) $(OUTDIR)/libdevinfo.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/$(OUTPUT) $(OUTDIR)/libdevinfo.a -f
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/$(OUTPUT)
	rm -rf ./html ; mkdir html


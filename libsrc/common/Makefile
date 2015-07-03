#
# iniparser Makefile
#

#CC      = gcc
EMBED = 1
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

CFLAGS  += -I../../include -I../../include/hi3520D/ 
#-O3
#CFLAGS += -I../../include/exdrv_3520Ademo/hi_rtc/



# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = commonlib.c 

OBJS = $(SRCS:.c=.o)


default:	libcommonlib.a

libcommonlib.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libcommonlib.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libcommonlib.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libcommonlib.a


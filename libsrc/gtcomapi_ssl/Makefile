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
	ifndef ROOTDIR
		OUTDIR=./
	else
		OUTDIR = $(ROOTDIR)/../emblib
	endif
else
	CC = gcc
	CFLAGS += -DEMBEDED=0
	AR          = ar
	ifndef ROOTDIR
		OUTDIR=./
	else
		OUTDIR = $(ROOTDIR)/../pclib
	endif
endif

CFLAGS  += -DUSE_SSL -I../../include -I/opt/hisi-linux/x86-arm/arm-hisiv200-linux/include/ -I. -O1
#3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $< )


SRCS = gt_com_api.c tab_crc32.c envelop/loadkeys.c envelop/gt_env.c

OBJS = $(SRCS:.c=.o)


default:	libgtcomapi_ssl.a

libgtcomapi_ssl.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libgtcomapi_ssl.a $(OBJS))
	$(RM) $(OBJS)
clean:
	$(RM) $(OBJS) $(OUTDIR)/libgtcomapi_ssl.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libgtcomapi_ssl.a
	rm -rf ./html ; mkdir html


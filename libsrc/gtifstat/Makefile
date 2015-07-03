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

CFLAGS  += -I../../include -O3




# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c
	#(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = src/gtifstat.c

OBJS = $(SRCS:.c=.o)

LIBA = libgtifstat.a


default: $(LIBA)	

$(LIBA):	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/$(LIBA) $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/$(LIBA)
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/$(LIBA)
	rm -rf ./html ; mkdir html


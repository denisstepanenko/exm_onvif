#
# iniparser Makefile
#

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
CFLAGS  += -O3 -I$(ROOTDIR)/../include

# Implicit rules

SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c

	#@(echo "compiling $< ...")
.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = src/iniparser.c \
	   src/dictionary.c \
	   src/strlib.c

OBJS = $(SRCS:.c=.o)


default:	libiniparser.a

libiniparser.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libiniparser.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libiniparser.a

veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libiniparser.a
	rm -rf ./html ; mkdir html
	cd test ; $(MAKE) veryclean
symlink:
docs:
	@(cd doc ; $(MAKE))
	
check:
	@(cd test ; $(MAKE))

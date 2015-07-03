
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


SRCS = tw9903api.c 

OBJS = $(SRCS:.c=.o)


default:	libtw9903api.a

libtw9903api.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libtw9903api.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libtw9903api.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libtw9903api.a
	rm -rf ./html ; mkdir html


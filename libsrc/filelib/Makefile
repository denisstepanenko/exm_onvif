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


SRCS = ftw_sort.c rm_file_select.c ftw_sort_user.c

OBJS = $(SRCS:.c=.o)


default:	libfilelib.a

libfilelib.a:	$(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libfilelib.a $(OBJS))

clean:
	$(RM) $(OBJS) $(OUTDIR)/libfilelib.a
symlink:
veryclean:
	$(RM) $(OBJS) $(OUTDIR)/libfilelib.a
	rm -rf ./html ; mkdir html


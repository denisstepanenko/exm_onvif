#
# iniparser Makefile
#

#CC      = gcc
#EMBED = 1
ARFLAGS = rcv
RM      = rm -f

TARGETLIB = libixml.a  libthreadutil.a  libupnp.a 

ifdef EMBED
	OUTDIR = $(ROOTDIR)/../emblib
	OBJDIR = arm
else
	OUTDIR = $(ROOTDIR)/../pclib
	OBJDIR = pc
endif


# Implicit rules

SUFFIXES = .o .c .h .a .so .sl


all:	
	cp $(OBJDIR)/*.a  $(OUTDIR)/	

clean:

	$(RM) $(OBJS) $(OUTDIR)/libixml.a $(OUTDIR)/libthreadutil.a $(OUTDIR)/libupnp.a
symlink:


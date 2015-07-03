###########################################################################
##
## Copyright (c) 2000-2003 Intel Corporation 
## All rights reserved. 
##
## Redistribution and use in source and binary forms, with or without 
## modification, are permitted provided that the following conditions are met: 
##
## * Redistributions of source code must retain the above copyright notice, 
## this list of conditions and the following disclaimer. 
## * Redistributions in binary form must reproduce the above copyright notice, 
## this list of conditions and the following disclaimer in the documentation 
## and/or other materials provided with the distribution. 
## * Neither name of Intel Corporation nor the names of its contributors 
## may be used to endorse or promote products derived from this software 
## without specific prior written permission.
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
## ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
## A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
## CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
## EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
## PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
## PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
## OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
## NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##
###########################################################################

ifdef EMBED
       CC=arm-linux-gcc
       CFLAGS += -DEMBEDED=1
#       INCDIR = -I/usr/local/arm/3.4.1/arm-linux/include
#       LIBDIR = /usr/local/arm/3.4.1/arm-linux/lib
       AR     = arm-linux-ar
       OUTDIR = $(ROOTDIR)/../emblib
else
       CC = gcc
       CFLAGS += -DEMBEDED=0
       AR          = ar
       OUTDIR = $(ROOTDIR)/../pclib
endif

CFLAGS  += -O3 -I$(ROOTDIR)/../include
#INCDIR  += $(ROOTDIR)/
#INCDIR  += $(ROOTDIR)/inc
CFLAGS += $(INCDIR)

ifndef DEBUG
export DEBUG = 0
endif


SUFFIXES = .o .c .h .a .so .sl

COMPILE.c=$(CC) $(CFLAGS) -c

       #@(echo "compiling $< ...")

.c.o:
	($(COMPILE.c) -o $@ $<)


SRCS = src/attr.c \
       src/document.c \
        src/element.c \
	src/ixml.c \
	src/ixmlmembuf.c \
	src/ixmlparser.c \
	src/namedNodeMap.c \
	src/node.c \
	src/nodeList.c





OBJS = $(SRCS:.c=.o)

default: libxmlparser.a

libxmlparser.a: $(OBJS)
	@($(AR) $(ARFLAGS) $(OUTDIR)/libxmlparser.a $(OBJS))






##all:
##	@if [ ! -d bin ] ; then mkdir -p bin ; fi
##	@$(MAKE) DEBUG=$(DEBUG) -C ./src 

clean:
	$(RM) $(OBJS) $(OUTDIR)/libxmlparser.a

#	@if [ -d bin ] ; then  rm -rf bin ; fi
#	@if [ -d doc/html ]; then rm -rf doc/html; fi
#	@if [ -f "doc/ixml.tex"     ]; then rm -f doc/ixml.tex; fi
#	@if [ -f "doc/pdflatex.log" ]; then rm -f doc/pdflatex.log; fi
#	@if [ -f "doc/ixml.log"     ]; then rm -f doc/ixml.log; fi
#	@if [ -f "doc/ixml.aux"     ]; then rm -f doc/ixml.aux; fi
#	@$(MAKE) clean -C ./src

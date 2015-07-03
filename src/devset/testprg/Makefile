TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = prdtest


SRC  = test.c #tab_crc32.c nv_pair.c gt_dev_api.c 
OBJS = $(SRC:.c=.o)

ifdef EMBED
        CC=arm-linux-gcc
        CFLAGS +=-DEMBEDED=1
#       INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#       LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
        LIBDIR +=-L../../emblib -s
else
        CC = gcc
        CFLAGS +=-DEMBEDED=0
        LIBDIR +=-L../../pclib
endif
INCDIR +=-I../../include
CFLAGS +=-ggdb3 $(INCDIR)
#CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lssl -lcrypto -lgtthread -lpthread  -lgtdevapi  -lnv_pair -lcommonlib -lgtlog

all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBDIR) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.o

cp :
	chmod 777 prdtest
	cp prdtest /home/temp/nfs/lsk
symlink:

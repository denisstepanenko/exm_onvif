TOP_DIR  = $(shell pwd)
 

EXEC = trigtest
SRC  += testmod.c io_test.c com_mod.c
SRC += ../boardtest/multicast_ctl.c
SRC += ../ipmain/gtvs_io_api.c
SRC += put_errr.c
OBJS = $(SRC:.c=.o)
EMBED = 1
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS += -DEMBEDED=1  
#	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR = -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR += -L../../lib -s
else
	CC = gcc
	CFLAGS += -g -DEMBEDED=0
	LIBDIR += -L../../pclib
endif
INCDIR +=-I ../../include
INCDIR +=-I ../boardtest
INCDIR +=-I ../ipmain
CFLAGS += $(INCDIR)
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
LDLIBS += -lpthread  -lgtthread -ldevinfo -lgtlog -liniparser -lcommonlib -lnv_pair -lgtdevapi



all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	chmod 777 trigtest
#	cp -frv trigtest /home/temp/nfs/zw/rw/vs3iotest
	cp -frv $(EXEC)	/home/temp/nfs/$(shell whoami)
dist :
#	chmod 777 trigtest
	cp trigtest /home/temp/nfs/dist/conf/gt1000
test :
	cp trigtest /home/temp/nfs/vs3test/conf/gt1000  
symlin:

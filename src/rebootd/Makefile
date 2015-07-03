TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = rebootd swrbt hwrbt 
#resetcmd 
SRC=rebootd.c
SRC_RBT=sysreboot.c
OBJS = $(SRC:.c=.o)
OBJS_RBT=$(SRC_RBT:.c=.o)
CC=arm-hisiv100nptl-linux-gcc
CFLAGS +=-DEMBEDED=1  
LIBDIR +=-L../../lib -s
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
#ifdef USE_VM
#	CFLAGS+=-DUSE_VM=1
#endif
LDLIBS +=$(LIBDIR)  -lcommonlib -lgtlog -ldevinfo -liniparser
all: $(EXEC)

rebootd:
	$(CC) $(CFLAGS) $(SRC) -o rebootd $(LDLIBS)
swrbt:
	$(CC) $(CFLAGS) -DSOFT_REBOOT $(SRC_RBT) -o swrbt 
#$(LDLIBS)
hwrbt:
	$(CC) $(CFLAGS) -DHARD_REBOOT $(SRC_RBT) -o hwrbt 
#$(LDLIBS)
#resetcmd:
#	$(CC) $(CFLAGS) resetcmd.c crc16.c  -o resetcmd
clean:
	-rm -f $(EXEC) $(OBJS) $(OBJS_RBT) *.elf *.gdb *.o 
cp :
	cp rebootd /nfs/lc
	cp swrbt /nfs/lc
	cp hwrbt /nfs/lc
dist:
	chmod 777 rebootd
	chmod 777 swrbt
	chmod 777 hwrbt
	chmod 777 resetcmd
	cp rebootd /home/temp/nfs/dist/conf/gt1000
	cp swrbt /home/temp/nfs/dist/conf/gt1000
	cp hwrbt /home/temp/nfs/dist/conf/gt1000
	cp resetcmd /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 rebootd
	chmod 777 swrbt
	chmod 777 hwrbt
	chmod 777 resetcmd
	cp rebootd /home/temp/nfs/test/conf/gt1000
	cp swrbt /home/temp/nfs/test/conf/gt1000
	cp hwrbt /home/temp/nfs/test/conf/gt1000
	cp resetcmd /home/temp/nfs/test/conf/gt1000
vmtest:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vm_test/conf/gt1000
symlink:

TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = initdisk
SRC  = main.c multicast_ctl.c fmat_disk.c

OBJS = $(SRC:.c=.o)
EMBED=1
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../../../lib 	
endif
INCDIR +=-I../../include
CFLAGS +=-ggdb3 $(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread -lnv_pair -ldevinfo  -lcommonlib -ldiskinfo -lgtdevapi -liniparser -lgtlog
#LDLIBS += -lxmlparser 

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	#cp testbd /home/temp/nfs/shixin
	#cp initdisk /home/temp/nfs/lsk
	cp -frv initdisk /home/temp/nfs/$(shell whoami)
dist:
	chmod 777 initdisk
	cp initdisk  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 initdisk
	cp initdisk  /home/temp/nfs/test/conf/gt1000
symlink:

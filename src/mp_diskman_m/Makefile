TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = diskman_m
SRC  = main.c diskmanager.c process_modcmd.c  
OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
#	INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../pclib 	
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS += -lgtlog
LDLIBS += -lhdutil -lmpdisk -ldiskinfo -lfileindex 
LDLIBS +=$(LIBDIR) -lfilelib -lpthread -lmodcom -liniparser -lcommonlib 
LDLIBS += -ldevinfo  
LDLIBS += -lfixdisk -lgtlog	
LDLIBS += -lavilib
LDLIBS += -lsqlite3
LDLIBS += -lmodsocket -lgtsocket
LDLIBS += -lgtthread
#-limencoder
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	cp $(EXEC) /home/temp/nfs/$(shell whoami)
	
dist :
#	chmod 777 $(EXEC)
#	cp $(EXEC) /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vs3test/conf/gt1000/real

symlink:

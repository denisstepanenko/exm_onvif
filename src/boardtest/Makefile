TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = testbd
SRC  = main.c multicast_ctl.c testmod.c rtc_test.c usb_test.c testIDE.c

OBJS = $(SRC:.c=.o)
EMBED=1
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
#	INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../lib 	
endif
INCDIR +=-I../../include
CFLAGS +=-ggdb3 $(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -limencoder -lpthread  -ldevinfo -liniparser -lnv_pair  -lcommonlib -lgtdevapi 
LDLIBS += -lxmlparser -ltw9903api -lgtlog -ldiskinfo

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	#cp testbd /home/temp/nfs/shixin
	cp testbd /home/temp/nfs/lsk
dist:
	chmod 777 testbd
	cp testbd  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 testbd
	cp testbd  /home/temp/nfs/test/conf/gt1000
symlink:

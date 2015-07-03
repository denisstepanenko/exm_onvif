TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = onvifbox 
SRC  = main.c videoencoder.c watch_process.c onvif_system.c
SRC  += process_modcmd.c
SRC  += ip_device_system.c


OBJS = $(SRC:.c=.o)

ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS += -g -DEMBEDED=1  
	LIBDIR +=-L../../lib 
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../lib 	
	CFLAGS += -ggdb3
endif
INCDIR +=-I../../include
INCDIR +=-I ./
CFLAGS += $(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread  -lmodcom  -lgtlog -lgtcomapi  
LDLIBS += -lmedia_api -lgtipcs
LDLIBS += -ldevinfo
LDLIBS += -liniparser 
LDLIBS += -lmodsocket -lgtsocket -lgtthread -lcommonlib
LDLIBS += -lonviflib
LDLIBS += -lstdc++ -lm
all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
print: printdevlist.c
	$(CC) $(CFLAGS) printdevlist.c -o printdev -L../emblib -ldevinfo -lgtlog -liniparser -lcommonlib
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp -frv $(EXEC) /mnt/zsk/
test:
	chmod 777 $(EXEC) printdev
	cp $(EXEC) printdev /home/temp/nfs/vs3test/conf/gt1000

symlink:

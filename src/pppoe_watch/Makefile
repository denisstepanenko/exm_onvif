TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = pppoe_watch
SRC  = main.c 
SRC += send_state.c
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
	LIBDIR +=-L../lib 
	CFLAGS +=-ggdb3 	
endif

INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lmodcom  -lmodsocket -lgtsocket
LDLIBS += -lgtthread -lpthread -lcommonlib  -ldevinfo -liniparser 
LDLIBS += -lgtlog
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp $(EXEC) /nfs/$(shell whoami)
dist:
	chmod 777 $(EXEC)
	cp $(EXEC)  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(EXEC)
	cp $(EXEC)  /home/temp/nfs/test/conf/gt1000

vmtest:
	chmod 777 $(EXEC)
	cp $(EXEC)  /home/temp/nfs/vm_test/conf/gt1000


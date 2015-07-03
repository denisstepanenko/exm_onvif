TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = watch_tcprt
SRC  = watch_tcprt.c
OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../emblib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../../pclib 	
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR) 
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread -lmodcom -ldevinfo -liniparser -lgtcomapi -lcommonlib -lgtthread -lnetinfo
all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp $(EXEC) /home/temp/nfs/$(shell whoami)
dist :
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/test/conf/gt1000

ulaw:
	$(CC) $(CFLAGS) soundapi.c -o ulaw $(LDLIBS)
symlink:

TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = csvtest
SRC  += ../csvparser.c
SRC  += main.c 
OBJS = $(SRC:.c=.o)
#EMBED=1
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../../emblib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../../../pclib 	
endif
INCDIR +=-I../../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread -ldevinfo -liniparser -lgtcomapi -lcommonlib -lgtthread -lgtlog -lcommonlib #-lcsvparser
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	chmod 777 csvtest
	cp csvtest /home/temp/nfs/lsk
	#cp csvtest /home/temp/nfs/shixin
dist:
	chmod 775 csvtest
	cp csvtest  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 775 csvtest
	cp csvtest  /home/temp/nfs/test/conf/gt1000
symlink:

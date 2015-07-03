TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = csvtest
SRC  = main.c #csvparser.c
OBJS = $(SRC:.c=.o)
EMBED=1
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1  
#	INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../../emblib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../../../pclib 	
endif
INCDIR +=-I../../../include
CFLAGS +=-ggdb3 $(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lgtthread -limencoder -lpthread  -lcommonlib -liniparser -ldevinfo -lconfparser -lgtdevapi -lnv_pair -lupdate -lcsvparser -lcommonlib -liniparser -lgtlog
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

TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = format
SRC  = format.c diskinfo.c #tw9903api.c 
OBJS = $(SRC:.c=.o)
#EMBED=1
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
LDLIBS +=$(LIBDIR) -lpthread 
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	chmod 777 format
	#	cp iictest /home/temp/nfs/lsk
	cp format /home/temp/nfs/lsk
dist:
	chmod 775 format
	cp format  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 775 format
	cp format  /home/temp/nfs/test/conf/gt1000
symlink:

TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = test
SRC = gtshm.c
OBJS = $(SRC:.c=.o)

ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS += -DEMBEDED=1  
	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
	LIBDIR = -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR += -L../emblib -s
else
	CC = gcc
	CFLAGS += -g -DEMBEDED=0 -ggdb3
	LIBDIR += -L../pclib
endif
INCDIR +=-I../include
CFLAGS += $(INCDIR) 
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
LDLIBS += 

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	#cp $(EXEC)  /home/temp/nfs/gjt
	cp $(EXEC)  /home/temp/nfs/shixin

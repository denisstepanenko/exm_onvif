EXEC = test 
SRC  = test.c
#SRC += memwatch.c
SRC += ../ftw_sort_user.c 
OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../../emblib -s
#	LIBDIR +=-L../
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../../../lib
#	LIBDIR +=-L../lib 
	CFLAGS +=-ggdb3 	
endif
INCDIR +=-I../../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
#MEM = -DMEMWATCH -DMEMWATCH_STDIO
LDLIBS += $(LIBDIR)
#LDLIBS += -lftw_sort
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp $(EXEC) /home/temp/nfs/zhangwei
mem:
	$(CC) -o $(EXEC) $(MEM) $(SRC) $(CFLAGS) $(LDIBS)

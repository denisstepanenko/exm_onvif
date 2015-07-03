EXEC = test 
SRC  = test.c
SRC += ../rm_file_select.c 
OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../emblib -s
#	LIBDIR +=-L../
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../
#	LIBDIR +=-L../lib 
	CFLAGS +=-ggdb3 	
endif
INCDIR +=-I../../../include -I../
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
#LDLIBS += -lftw_sort
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp $(EXEC) /home/temp/nfs/zhangwei

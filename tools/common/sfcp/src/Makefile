TOP_DIR  = $(shell pwd)

EXEC = sfcp
SRC  += sfcp.c
SRC  += tab_crc32.c
OBJS = $(SRC:.c=.o)

ifdef EMBED
        CC=arm-linux-gcc
        CFLAGS += -DEMBEDED=1
        LIBDIR += -L../../../../emblib -s
else
        CC = gcc
        CFLAGS += -g -DEMBEDED=0
        LIBDIR += -L../../../../pclib
endif
INCDIR +=-I ../../../../include
CFLAGS += $(INCDIR)
CFLAGS += -Wall
LDLIBS += $(LIBDIR)

#LDLIBS += -lfilelib -ldevinfo -liniparser -lcommonlib -lgtlog -lfixdisk

LDLIBS += -lfilelib -lgtlog 





all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	chmod 777 $(EXEC) 
	cp -frv $(EXEC) /home/temp/nfs/zw/rw/sfcptest

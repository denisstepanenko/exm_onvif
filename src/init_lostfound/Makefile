TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = initlostfd
SRC  +=init_lostfound.c
OBJS = $(SRC:.c=.o)
#EMBED = 1
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS += -DEMBEDED=1  
#	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR = -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR += -L../../lib -s
INCDIR +=-I ../../include
CFLAGS += $(INCDIR)
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
LDLIBS += -lhdutil -lmpdisk -lfileindex -lcommonlib -ldevinfo -liniparser\
		-lgtlog  -lsqlite3 -ldiskinfo -lfilelib  -lfixdisk\
		 -lavilib -lcommonlib -lpthread
#LDLIBS +=$(LIBDIR) -lgtlog -ldiskinfo -lcommonlib

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o initlostfd $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o

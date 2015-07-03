TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = init_all_ide_drvs
SRC  += main.c
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
LDLIBS +=-ldiskinfo  -ldevinfo -lgtlog -liniparser -lcommonlib  -lgtdevapi

#LDLIBS +=$(LIBDIR) -lgtlog -ldiskinfo -lcommonlib

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o

TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = watch_proc
SRC  = watch_process.c 
#converts.c
#SRC += confparser.c
OBJS = $(SRC:.c=.o)
#OBJS = main.o rtimg_para.o rtnetproc.o rtcmdproc.o tw2824api.o rtstream.o 
#ime6410api.o AVIEncoder.o iic.o
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	#INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
	#LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../pclib 	
	CFLAGS+= -ggdb3
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR)  -liniparser -lcommonlib -ldevinfo -lgtlog
#-limencoder
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	cp -frv $(EXEC) /nfs/$(shell whoami)
dist :
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/dist/conf/gt1000
test :
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/test/conf/gt1000
vs3test:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vs3test/conf/gt1000
test26:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/test26/conf/gt1000

symlink:

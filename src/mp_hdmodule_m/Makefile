
TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = hdmodule_m
SRC  = hdmain.c hdmod.c rtpool.c process_modcmd.c 
#
#SRC += ../../libsrc/gtipcs/mshm.c ../../libsrc/gtipcs/mshmpool.c
#SRC += ../../libsrc/media_api/media_api.c
#SRC += ../../libsrc/media_api/venc_read.c
#SRC += ../../libsrc/media_api/aenc_read.c
#SRC += ../../libsrc/avilib/avilib.c
#SRC += ../../libsrc/imencoder/iic.c
#SRC += ../../libsrc/imencoder/ime6410api.c

OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
#	INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../lib -s
#	LIB_PATH=-L../../lib/lib_hi3520A -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 -ggdb3
	LIBDIR +=-L../pclib 	
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lfilelib -lavilib -lgtlog -lmedia_api_m -lpthread -lgtthread -lmodcom -liniparser -lgtcomapi -lcommonlib
LDLIBS += -ldevinfo
LDLIBS += -lmpdisk -ldiskinfo -lfileindex
LDLIBS += -lfixdisk
LDLIBS += -lhdutil
LDLIBS += -lsqlite3 -lmodsocket -lgtsocket
#LDLIBS += -lavformat -lavcodec -lavutil -lm -ldl -laudiofmt
#LDLIBS += -les2ps
#LDLIBS += -laudiofmt
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) -lfilelib -lgtthread -lgtipcs
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	cp -frv $(EXEC) /home/temp/nfs/$(shell whoami)

dist :
#	chmod 777 $(EXEC)
#	cp $(EXEC) /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vs3test/conf/gt1000

symlink:

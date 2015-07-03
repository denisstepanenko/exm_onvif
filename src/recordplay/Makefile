
TOP_DIR  = $(shell pwd)
EMBED = 1 

EXEC = playback
SRC  = playback_main.c playback_file.c playback_pool.c playback_modcmd.c 
OBJS = $(SRC:.c=.o)
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 -ggdb3
	LIBDIR +=-L../pclib 	
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lfilelib -lavilib -lmedia_api -lpthread -lgtthread -lmodcom -liniparser -lgtcomapi -lcommonlib
LDLIBS += -ldevinfo 
LDLIBS += -lcommonlib -lmpdisk -ldiskinfo -lfileindex
LDLIBS += -liniparser -lfixdisk
LDLIBS += -lgtlog
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
	cp -frv $(EXEC) /nfs/liubai/$(shell whoami)

dist :
#	chmod 777 $(EXEC)
#	cp $(EXEC) /nfs/liubai
test:
	chmod 777 $(EXEC)
	cp $(EXEC) /nfs/liubai

symlink:

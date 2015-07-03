top_DIR  = $(shell pwd)

EXEC = upnpd
SRC  = src/upnpd.c  
SRC += src/send_state.c
OBJS = $(SRC:.c=.o)

#ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS += -DEMBEDED=1  
#	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR = -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR = -L../../lib -s
#else
#	CC = gcc
#	CFLAGS += -g -DEMBEDED=0
#	LIBDIR += -L../pclib
#endif
INCDIR +=-I../../include
CFLAGS += $(INCDIR) 
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
#LDLIBS += -limencoder
LDLIBS += -lpthread  -liniparser  -lgtsocket 
LDLIBS += -lgtcomapi_ssl
#LDLIBS += -lgtcomapi
LDLIBS +=  -lcommonlib 
LDLIBS +=  -lssl -lcrypto
LDLIBS += -lconfparser -ldevinfo 
LDLIBS += -lgtthread
LDLIBS += -lgtlog
LDLIBS += -lmodcom -lmodsocket
LDLIBS += -lgtsocket
LDLIBS += -lupnp
LDLIBS += -lixml
LDLIBS += -lthreadutil
LDLIBS += -lcsvparser
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	cp $(EXEC) /nfs/$(shell whoami)
vmdist :
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vm_test/conf/gt1000
vmtest:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vm_test/conf/gt1000
symlink:

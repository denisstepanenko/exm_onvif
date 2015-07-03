TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = devset
#SRC  = main.c system_para_set.c system_test.c system_update.c cmd_process.c
#SRC += nv_pair.c
SRC  = main.c system_para_set.c system_test.c cmd_process.c rm_file_select.c set_com_null.c
OBJS = $(SRC:.c=.o)
EMBED=1
ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=-DEMBEDED=1  
#	INCDIR =-I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR =-s -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR +=-L../../lib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 
	LIBDIR +=-L../lib 	
endif
INCDIR +=-I../../include
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lgtthread -limencoder -lpthread  -liniparser -ldevinfo -lconfparser -lgtdevapi -lnv_pair -lupdate -lcommonlib
LDLIBS += -lgtlog -lxmlparser #-lfilelib
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
cp :
	chmod 777 devset
	cp devset /nfs/$(shell whoami)
dist:
	chmod 775 devset
	cp devset  /home/temp/nfs/dist/conf/gt1000
test:
	chmod 775 devset
	cp devset  /home/temp/nfs/test/conf/gt1000
vmtest: 
	chmod 775 devset
	cp devset /home/temp/nfs/vm_test/conf/gt1000
symlink:

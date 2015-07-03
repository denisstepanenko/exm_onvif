CFLAGS	=-g -Wall
EXEC	=cgi_main
CC	=arm-hisiv100nptl-linux-gcc

SRC 	= cgi_main.c  
OBJS 	= $(SRC:.c=.o)
INCDIR	= -I../../include
CFLAGS+=$(INCDIR)
LIBDIR 	= -L../../lib -s 
LIBSRC 	= -liniparser -lcommonlib -lgtlog -ldevinfo -lcgic -liconv -ldiskinfo -lmodsocket -lgtsocket -lgtthread -lpthread
LDLIBS	= $(LIBDIR) $(LIBSRC)
all: $(EXEC) 

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC)  $(LDLIBS) 
	cp -rfv $(EXEC) /mnt/zsk/www/cgi-bin/



clean:
	rm -f $(EXEC) $(OBJS)
cp :
	cp -rfv $(EXEC) /mnt/zsk/www/cgi-bin/



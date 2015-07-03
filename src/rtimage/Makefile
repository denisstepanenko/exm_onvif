TOP_DIR  = $(shell pwd)

EXEC = onvif_rtimage
SRC  = main.c rtimg_para2.c  avserver.c maincmdproc.c net_avstream.c net_aplay.c play_back.c #audio_pool.c 
OBJS = $(SRC:.c=.o)

WATCH = watch_tcprt
WATCH_SRC  = watch/watch_tcprt.c
WATCH_SRC += ../../libsrc/gtipcs/mshm.c
WATCH_OBJS = $(WATCH_SRC:.c=.o)

DEAF = deafmute
DEAF_SRC = deaf/deafmute.c
DEAF_OBJS = $(DEAF_SRC:.c=.o)

#CC=arm-hisiv200-linux-gcc
CC=arm-hisiv100nptl-linux-gcc
CFLAGS +=-DEMBEDED=1 -s 
CFLAGS += -DIPCALL=1
LIBDIR +=-L../../lib  
LIB_PATH=-L../../lib/lib_hi3520D 

INCDIR +=-I../../include
INCDIR +=-I../../include/include_hi3520D

CFLAGS +=$(INCDIR) 
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread -lmodcom -ldevinfo -liniparser -lcommonlib -lgtthread -lnetinfo -lgtcomapi
LDLIBS +=   -lmodsocket -lgtsocket -lgtlog -laudiofmt -lgtifstat  -lmedia_api -lgtipcs -lavilib
LDLIBS +=  -limencoder 
LDLIBS +=	-ltw9903api

#LDLIBS +=$(LIB_PATH) -lmpi  -ltde   -ljpeg -laec -lanr -lVoiceEngine -lhdmi -lresampler 
#-lmpi -lVoiceEngine  -laec  -lresampler  -ltde -lhdmi -lpciv -lanr  -ljpeg 
all: $(EXEC) $(WATCH) 
#$(DEAF) zsk del

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
$(WATCH):$(WATCH_OBJS)
	$(CC) $(CFLAGS) $(WATCH_SRC) -o $(WATCH) $(LIBDIR) -ldevinfo -liniparser -lcommonlib -lpthread -lgtlog -lgtipcs
$(DEAF): $(DEAF_OBJS)
	$(CC) $(CFLAGS) $(DEAF_SRC) -o $(DEAF) $(LDLIBS)$(LIBDIR)  -lcommonlib -lpthread -laudiofmt

clean:
	-rm -f $(EXEC) $(WATCH) $(DEAF) $(OBJS) *.elf *.gdb *.o datastream.avi
cp :
	cp -fv $(EXEC) $(WATCH) /mnt/zsk/
dist :
	chmod 777 $(EXEC) $(WATCH) $(DEAF)
	cp $(EXEC) $(WATCH) $(DEAF) /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(EXEC) $(WATCH) $(DEAF)
	cp $(EXEC) $(WATCH) $(DEAF) /home/temp/nfs/test/conf/gt1000
vs3test:
	chmod 777 $(EXEC) $(WATCH) $(DEAF)
	cp $(EXEC) $(WATCH) $(DEAF) /home/temp/nfs/vs3test/conf/gt1000
ulaw:
	$(CC) $(CFLAGS) soundapi.c -o ulaw $(LDLIBS)
symlink:

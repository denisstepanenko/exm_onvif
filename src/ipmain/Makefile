top_DIR  = $(shell pwd)
EMBED = 1 

EXEC = ipmain
SRC  = ipmain.c mainnetproc.c maincmdproc.c ipmain_para.c watch_board.c watch_process.c gate_connect.c process_rtimg.c netcmdproc.c trans_com.c alarm_process.c
#SRC += process_kbd.c
#SRC += process_diskman.c  
SRC += hdmodapi.c
SRC += ../../libsrc/update/update.c
SRC += ../../libsrc/gtsocket/src/gtsocket.c
SRC += leds_api.c
SRC += audioout_api.c
SRC += infodump.c
SRC += devstat.c
SRC += process_hdmod.c
SRC += process_pppoe_watch_cmd.c
SRC += process_videoenc.c
SRC += process_hw_diag.c
SRC += process_diskman.c
SRC += remote_file.c
SRC += process_upnpd_cmd.c
SRC += process_hdplayback.c
#SRC += display_osd.c
SRC += gtvs_io_api.c
SRC += video_para_api.c
#SRC += gpsupport.c
#SRC += connect_other_modules.c
#SRC += hisvi_vda_process.c
######
OSD_SRC_DIR = ../lib/osd_api
######OSD_SRC_DIR = ../lib/fromosd
OSD_SRC += $(OSD_SRC_DIR)/osd_api.c
OSD_SRC += $(OSD_SRC_DIR)/raw_video/rawvideo_osd.c
OSD_SRC += $(OSD_SRC_DIR)/osd_dev/osd_dev.c
OSD_SRC += $(OSD_SRC_DIR)/osd_tw2835/osd_tw2835.c
OSD_SRC += $(OSD_SRC_DIR)/osd_tw2835/tw2835_priv_osd.c
######
#SRC += $(OSD_SRC)
#SRC += osd_api.c 
OBJS = $(SRC:.c=.o)

ifdef EMBED
	CC=arm-hisiv100nptl-linux-gcc
	CFLAGS +=  -DEMBEDED=1  
#	INCDIR = -I/usr/local/arm/2.95.3/arm-linux/include
#	LIBDIR = -L/usr/local/arm/2.95.3/arm-linux/lib
	LIBDIR += -L ../../lib
        LIBDIR += -L ../../lib/lib_hi3520D -s
else
	CC = gcc
	CFLAGS += -g -DEMBEDED=0
	LIBDIR += -L../pclib
endif
INCDIR +=-I../../include
INCDIR +=-I../../include/include_hi3520A
INCDIR +=-I../../include/include_hi3520A/common
OSD_INCDIR += -I$(OSD_SRC_DIR)/raw_video -I$(OSD_SRC_DIR)/osd_dev -I$(OSD_SRC_DIR)/osd_tw2835 -I$(OSD_SRC_DIR)
INCDIR += $(OSD_INCDIR)
CFLAGS += $(INCDIR) 
CFLAGS += -Wall
LDLIBS += $(LIBDIR)
#LDLIBS += -limencoder
LDLIBS += -lpthread -lmodcom -liniparser  
LDLIBS += -lgtcomapi_ssl
#LDLIBS += -lgtcomapi 
LDLIBS += -lcommonlib -ldevinfo 
LDLIBS += -lssl -lcrypto 
LDLIBS += -lconfparser 
#LDLIBS += -ltw2835api
LDLIBS += -lcsvparser 
#LDLIBS += -losd_api 
#LDLIBS += -lupdate 
LDLIBS += -lnetinfo
LDLIBS += -lxmlparser
LDLIBS += -lgtlog
LDLIBS += -lhdctl
LDLIBS += -lxmlparser
LDLIBS += -lsqlite3
#LDLIBS += -ltw2835api
#LDLIBS += -ltw9910api
#LDLIBS += -losd_api
#LDLIBS += -ltwlock
#LDLIBS += -liiclib
LDLIBS += -lmodsocket -lgtthread
LDLIBS += -ldiskinfo
#LDLIBS += -lmpi -lhdmi
#LDLIBS += -lVoiceEngine -laec -lresampler -lanr
#LDLIBS += -ljpeg
all: $(EXEC)


$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
clean:
	-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o
cp :
	cp -frv $(EXEC) /nfs/$(shell whoami)
test:
	chmod 777 $(EXEC)
	cp $(EXEC) /home/temp/nfs/vs3test/conf/gt1000
symlink:

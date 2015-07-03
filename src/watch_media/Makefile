HICHIP=HI3520A_V100 
CHIP_TYPE=CHIP_TYPE_hi3520Av100
DDDD = 1

CC=arm-hisiv100nptl-linux-gcc
CFLAGS = -s 

# 目标文件名字
EXEC = watch_media
# c源文件 
 

#库
#SRC += ./extern_src/gtipcs/mshm.c  ./extern_src/gtipcs/mshmpool.c

#源文件
SRC +=  main.c tcprtimg_show.c  video_enc_show.c audio_enc_show.c
# 头文件路径，可以任意指定头文件
#海思
#源文件
INCLUDE += -I ./src
#库文件
INCLUDE +=  -I ../../include

  
CFLAGS += $(INCLUDE)
# 链接库
#库名字
LIBS= -lpthread -lgtipcs -ldevinfo -liniparser -lgtlog -lcommonlib
#库路径
LIBDIR = -L ../../lib 
LDLIBS= $(LIBDIR) $(LIBS)


# object文件
OBJS = $(SRC:.c=.o)

all:$(EXEC)
# 定义目标文件生成规则
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDLIBS) 
	cp $(EXEC) /mnt/zsk/
	
clean:
		-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
	

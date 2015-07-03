HICHIP=HI3520A_V100 
CHIP_TYPE=CHIP_TYPE_hi3520Av100


#CC = /opt/hisi-linux-nptl/arm-hisiv100-linux/bin/arm-hisiv100-linux-uclibcgnueabi-gcc
CC=/opt/hisi-linux/x86-arm/arm-hisiv200-linux/bin/arm-hisiv200-linux-gnueabi-gcc
CFLAGS = -g 

# 目标文件名字 
EXEC1 = write_sda
EXEC2 = readforlive555
# c源文件 
#海思
#SRC := 
 	
#库
#源文件
SRC +=    src/year_blocks.c src/day_blocks.c  src/blocks.c  src/sec_blocks.c  src/direct_io_sda.c

SRC += ./extern_src/avilib/avilib.c
SRC += ./extern_src/gtipcs/mshm.c  ./extern_src/gtipcs/mshmpool.c
SRC += ./extern_src/media_api/media_api.c
#SRC += tools/test_vi_frame_rate.c
#SRC += tools/vi_dump.c
# 头文件路径，可以任意指定头文件
#海思

#INCLUDE += -I ./common
#gclib
INCLUDE += -I ./gclib_include
#源文件
INCLUDE += -I ./src
#库文件
INCLUDE +=  -I./include -I./extern_src
INCLUDE +=  -I./driver_include
CFLAGS += $(INCLUDE)

#库名字
LIBS= -lpthread -lsgutils2
#库路径
LIBDIR = -L  ./lib
LDLIBS= $(LIBDIR) $(LIBS)
# object文件 
OBJS = $(SRC:.c=.o) 

all:$(EXEC1) $(EXEC2)
# 定义目标文件生成规则
$(EXEC1) : $(OBJS) $(EXEC1).o
	$(CC) $(CFLAGS) $(SRC) src/$(EXEC1).c  -o $(EXEC1) $(LDLIBS)
	arm-hisiv200-linux-strip $(EXEC1)
	cp $(EXEC1) /mnt/yk
	
$(EXEC2) : $(OBJS) $(EXEC2).o
	$(CC) $(CFLAGS) $(SRC) src/$(EXEC2).c -o $(EXEC2) $(LDLIBS)
	cp $(EXEC2) /mnt/yk
	
	
	
$(EXEC1).o:src/$(EXEC1).c 
	$(CC) $(CFLAGS) src/$(EXEC1).c -c -o $(EXEC1).o 
$(EXEC2).o:src/$(EXEC2).c 
	$(CC) $(CFLAGS) src/$(EXEC2).c -c -o $(EXEC2).o
	echo "ada" 
clean:
		-rm -f $(EXEC) $(OBJS) *.elf *.gdb *.o 
	

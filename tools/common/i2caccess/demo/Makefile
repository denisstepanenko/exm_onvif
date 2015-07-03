CC = arm-linux-gcc
STRIP =arm-linux-strip
INCLUDE = -I../include
CC = arm-linux-gcc
STRIP =arm-linux-strip

TARGET1=iaccess
all:
	$(CC) $(INCLUDE) iaccess.c -o $(TARGET1) -L../emblib -ltw9910api -lgtlog -lcommonlib -ldevinfo -liniparser
	$(STRIP) $(TARGET1)	
cp:
	cp $(TARGET1) /home/temp/nfs/$(shell whoami)
clean:
	rm -f iaccess  

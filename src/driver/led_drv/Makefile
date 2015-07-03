
KDIR = /usr/local/gm_target/linux-2.6.14-fa
#KDIR = ../../gm_linux2.6/linux-2.6.14-fa
CFLAGS += -DGM8180
CFLAGS += -DKERNEL26 
obj-m = ledsdrv.o
KERNEL26=1
ifdef KERNEL26
default: compile_info.h
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	mv ledsdrv.ko vs3_ledsdrv.ko
else
all: $(target_mod)
endif

$(target_mod): compile_info.h $(OBJS)
	$(LD) -r -o $@ $(OBJS)

compile_info.h: ledsdrv.c
	@echo "creating $@ ..."; \
	version=$(shell whoami):`date +"%Y%m%d-%H%M"`; \
	echo "#ifndef COMPILE_INFO" > $@; \
	echo "#define COMPILE_INFO \"$$version\"" >> $@; \
	echo "#endif /* COMPILE_INFO */" >> $@

clean:
	-rm -f $(target_mod) $(OBJS) *.elf *.gdb *.o compile_info.h *.ko *.mod.c  *.ko.cmd
cp :
	chmod 777 . -R
	-cp *.ko /home/temp/nfs/$(shell whoami)
	
test:
	chmod 777 . -R

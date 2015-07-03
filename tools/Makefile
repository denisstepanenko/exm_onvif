ROOTDIR  = $(shell pwd)
#add the directory to here that you want to make
dir_y += common
dir_y += ip_tools
#dir_y += vm_tools
dir_y += vs_tools/smarttools
#dir_y += pc_tools

all:
	for i in $(sort $(dir_y)) ; do \
		[ ! -d $$i ] || make -C $$i || exit $$? ; \
	done
clean:
	-for i in $(dir_v) $(sort $(dir_y) )  ; do \
		[ ! -d $$i ] || make -C $$i clean; \
	done
test:
	-for i in $(dir_v) $(sort $(dir_y)) ; do \
		[ ! -d $$i ] || make -C $$i test; \
	done
vmtest:
	-for i in $(dir_v) $(sort $(dir_y) )  ; do \
		[ ! -d $$i ] || make -C $$i vmtest; \
	done
vs3test:
	-for i in $(dir_v) $(sort $(dir_y) )  ; do \
		[ ! -d $$i ] || make -C $$i vs3test; \
	done
	#cp vs_tools/smarttools/smartd /home/temp/nfs/test
	#cp vs_tools/smarttools/smartctl /home/temp/nfs/test

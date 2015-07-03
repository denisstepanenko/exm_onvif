#
#	Makefile -- Build instructions for user level apps
#

.EXPORT_ALL_VARIABLES:
.PHONY: all  clean 

#
# Include architecture specific build rules.
#
ROOTDIR=$(shell pwd)

EMB_OPT=-e CC=arm-hisiv100nptl-linux-gcc -e EMBED=1  -e AR=arm-hisiv100nptl-linux-ar -e OUTDIR=$(ROOTDIR)/../lib -I../include/ 
#add the directory to here that you want to make
dir_y += ipmain 
dir_y += rtimage 
#dir_y += encbox 
dir_y += mp_hdmodule_m 
dir_y += mp_diskman_m 
dir_y += recordplay
dir_y += watch_process

all: 
	for i in $(dir_y); do \
		[ ! -d $$i ] || make -j 4 -C $$i $(EMB_OPT) || exit $$? ; \
	done

cp:     
	for i in $(dir_y); do \
		[ ! -d $$i ] || make -j 4 -C $$i $(EMB_OPT) cp ; \
	done


clean:
	for i in $(dir_y)  ; do \
		[ ! -d $$i ] || make -j 4 -C $$i $(EMB_OPT)  clean ; \
	done



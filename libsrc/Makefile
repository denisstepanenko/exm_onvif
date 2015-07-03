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
dir_y += filelib
dir_y += fixdisk
dir_y += hdutil
dir_y += hdctl
dir_y += diskinfo
dir_y += mpdisk
dir_y += fileindex
dir_y += modsocket
dir_y += soundapi
dir_y += audiofmt
dir_y += iniparser
dir_y += confparser
dir_y += csvparser
dir_y += xmlparser
dir_y += twlock
dir_y += iiclib
dir_y += avilib
dir_y += gtdevapi
dir_y += devinfo
dir_y += gtthread
dir_y += gtipcs
dir_y += nv_pair
dir_y += media_api
dir_y += media_api_m
dir_y += gtsocket
dir_y += common
#dir_y += hddbuf
dir_y += gtipcs
dir_y += gtcomapi
dir_y += gtcomapi_ssl
dir_y += modcom
dir_y += netinfo
dir_y += gtlog
dir_y += gtifstat
dir_y += imencoder
dir_y += tw9903api
#dir_y += openssl
dir_y += update
#dir_y += libupnp
#dir_y += tw2835api
#dir_y += osd_api
dir_y += diskinfo
#dir_y += sqlite3
all: 
	for i in $(sort $(dir_y)) $(dir_v) $(dir_p); do \
		[ ! -d $$i ] || make -C $$i $(EMB_OPT) || exit $$? ; \
	done

cp:     
	cp -rfv ../lib/libdevinfo.so /nfs/$(shell whoami)	
clean:
	for i in $(dir_v) $(sort $(dir_y) $(dir_n) $(dir_)) $(dir_p) ; do \
		[ ! -d $$i ] || make -C $$i $(EMB_OPT)  clean ; \
	done



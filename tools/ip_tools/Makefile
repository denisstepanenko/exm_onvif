TOP_DIR  = $(shell pwd)
EXEC = getver para_conv shutdownd convavi ifcheck extract_prog ini_conv trigsmarttest diskcheck show_adsl
##TOOLS = extract_prog shutdownd
#EXEC += GEsetscreen
#EXEC += GEptzctl
COMPILE_F = $(EXEC) 
#COMPILE_F += $(TOOLS)
#COMPILE_F += extract_prog shutdownd
SCRIPTS = shscript/hardreboot  shscript/killimg  shscript/killmain  shscript/killwatch  shscript/softreboot
CPFILES= $(EXEC) $(SCRIPTS)
#SRC_DIFF = testdiff.c
#SRC_PARACONV = para_conv.c
#SRC_INISET = iniset.c
CC=arm-hisiv100nptl-linux-gcc
CFLAGS +=-DEMBEDED=1  
LIBDIR +=-L../../lib -s
LDLIBS +=$(LIBDIR) 
LDLIBS += -lcommonlib -lpthread -ldevinfo -liniparser -lcommonlib -lgtlog
INCDIR +=-I../../include

CFLAGS +=$(INCDIR)
CFLAGS += -Wall

all: $(COMPILE_F)
getver:
	$(CC) $(CFLAGS) getver.c -o getver $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
#iniset:
#	$(CC) $(CFLAGS) iniset.c -o iniset $(LDLIBS) -liniparser -lcommonlib -ldevinfo -lgtlog
#killmod:
#	$(CC) $(CFLAGS) killmod.c -o killmod $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
para_conv:
	$(CC) $(CFLAGS) para_conv.c -o para_conv $(LDLIBS) -ldevinfo -liniparser -lconfparser -lcommonlib -ldevinfo -lgtlog
shutdownd:
	$(CC) $(CFLAGS) shutdownd.c -o shutdownd $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
show_adsl:
	$(CC) $(CFLAGS) show_adsl.c -o show_adsl $(LDLIBS) -liniparser -lgtlog -ldevinfo -lcommonlib -lgtsocket -lgtthread -lpthread	
convavi:
	$(CC) $(CFLAGS) convavi.c -o convavi $(LDLIBS)
ifcheck:
	$(CC) $(CFLAGS) ifcheck.c -o ifcheck $(LDLIBS) -lnetinfo -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog
extract_prog:
	$(CC) $(CFLAGS) extract_prog.c -o extract_prog $(LIBDIR) -lgtlog
ini_conv:
	$(CC) $(CFLAGS) ini_conv.c -o ini_conv $(LDLIBS)
				
#GEptzctl:
#	$(CC) $(CFLAGS) GEptzctl.c -o GEptzctl $(LIBDIR) -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog
#GEsetscreen:
#	$(CC) $(CFLAGS) GEsetscreen.c -o GEsetscreen $(LIBDIR) -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog


inittrigin:
	$(CC) $(CFLAGS) -I../../src/ipmain inittrigin.c ../../src/ipmain/gtvs_io_api.c -o inittrigin $(LDLIBS)
trigsmarttest:
	$(CC) $(CFLAGS) trig_smarttest.c -o trigsmarttest $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog -lhdctl -lcommonlib

#mountnfs:
#	$(CC) $(CFLAGS) mountnfs.c -o mountnfs $(LDLIBS) -lnetinfo -lgtlog
#patchconf:
#	$(CC) $(CFLAGS) patchconf.c converts.c -o patchconf $(LDLIBS) -liniparser -lconfparser -ldevinfo -lcommonlib -lgtlog 
#patchvsftpd:
#	$(CC) $(CFLAGS) patchvsftpd.c -o patchvsftpd $(LDLIBS) -lconfparser -lcommonlib -ldevinfo -liniparser -lgtlog
diskcheck:
	$(CC) $(CFLAGS) diskcheck.c -o diskcheck $(LIBDIR) -lnv_pair -ldiskinfo -ldevinfo -lcommonlib -liniparser -lgtlog -lmpdisk

#sfcp/src/sfcp:
#	$(CC) $(CFLAGS) sfcp/src/sfcp.c sfcp/src/tab_crc32.c -o sfcp/src/sfcp $(LDLIBS)	-lfilelib -lgtlog -lhdutil
#record_sort:
#	$(CC) $(CFLAGS) record_sort.c -o record_sort $(LDLIBS)

clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
cp :
	cp -rf $(CPFILES) /nfs/$(shell whoami)
dist:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/dist/conf/ip1004
iptest:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/iptest/conf/ip1004
symlink:

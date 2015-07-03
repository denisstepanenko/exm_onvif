TOP_DIR  = $(shell pwd)
EXEC = testdiff iniset killmod  mountnfs show_adsl patchconf
TOOLS = shutdownd
#EXEC += GEsetscreen
#EXEC += GEptzctl
COMPILE_F = $(EXEC) 
COMPILE_F += $(TOOLS)
#COMPILE_F += extract_prog shutdownd
SCRIPTS = checkps.sh 
CPFILES= $(EXEC) $(SCRIPTS)
SRC_DIFF = testdiff.c
SRC_PARACONV = para_conv.c
SRC_INISET = iniset.c
CC=arm-hisiv100nptl-linux-gcc
CFLAGS +=-DEMBEDED=1  
LIBDIR +=-L../../lib -s
INCDIR +=-I../../include

CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR)  
all: $(COMPILE_F)
#GEptzctl:
#	$(CC) $(CFLAGS) GEptzctl.c -o GEptzctl $(LIBDIR) -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog
#GEsetscreen:
#	$(CC) $(CFLAGS) GEsetscreen.c -o GEsetscreen $(LIBDIR) -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog
extract_prog:
	$(CC) $(CFLAGS) extract/extract_prog.c -o extract_prog $(LIBDIR) -lgtlog
#ifcheck:
#	$(CC) $(CFLAGS) ifcheck.c ../../libsrc/netinfo/netinfo.c -o ifcheck $(LDLIBS) -lcommonlib -lconfparser -ldevinfo -liniparser -lgtlog
testdiff: 
	$(CC) $(CFLAGS) testdiff.c -o testdiff $(LDLIBS) -liniparser -lcommonlib -ldevinfo -lgtlog
iniset:
	$(CC) $(CFLAGS) iniset.c -o iniset $(LDLIBS) -liniparser -lcommonlib -ldevinfo -lgtlog
#para_conv:
#	$(CC) $(CFLAGS) para_conv.c -o para_conv $(LDLIBS) -ldevinfo -liniparser -lconfparser -lcommonlib -ldevinfo -lgtlog
killmod:
	$(CC) $(CFLAGS) killmod.c -o killmod $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
mountnfs:
	$(CC) $(CFLAGS) mountnfs.c -o mountnfs $(LDLIBS) -lnetinfo -lgtlog
shutdownd:
	$(CC) $(CFLAGS) shutdownd.c -o shutdownd $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
#getver:
#	$(CC) $(CFLAGS) getver.c -o getver $(LDLIBS) -lcommonlib -ldevinfo -liniparser -lgtlog
patchconf:
	$(CC) $(CFLAGS) patchconf.c converts.c -o patchconf $(LDLIBS) -liniparser -lconfparser -ldevinfo -lcommonlib -lgtlog 
patchvsftpd:
	$(CC) $(CFLAGS) patchvsftpd.c -o patchvsftpd $(LDLIBS) -lconfparser -lcommonlib -ldevinfo -liniparser -lgtlog
show_adsl:
	$(CC) $(CFLAGS) show_adsl.c -o show_adsl $(LDLIBS) -liniparser -lgtlog -ldevinfo -lcommonlib -lgtsocket -lgtthread -lpthread
#sfcp/src/sfcp:
#	$(CC) $(CFLAGS) sfcp/src/sfcp.c sfcp/src/tab_crc32.c -o sfcp/src/sfcp $(LDLIBS)	-lfilelib -lgtlog
record_sort:
	$(CC) $(CFLAGS) record_sort.c -o record_sort $(LDLIBS)
clean:
	-rm -f $(EXEC)  *.elf *.gdb *.o 
cp :
	cp $(CPFILES) /nfs/$(shell whoami)
dist:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/dist/conf/gt1000
test:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/test/conf/gt1000
vmtest:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/vm_test/conf/gt1000
vs3test:
	chmod 777 $(CPFILES)
	cp $(CPFILES) /home/temp/nfs/vs3test/conf/gt1000
	cp $(TOOLS) /home/temp/nfs/vs3test/conf/tools
symlink:

################OPTION###################
ifdef EMBED
	OUTPUT = libonviflib.a onviflib_test
	CROSS=arm-hisiv100nptl-linux-
else
	OUTPUT = libonviflib.a onviflib_test #onvif_utest
	CROSS=
endif

CCOMPILE = $(CROSS)gcc
CPPCOMPILE = $(CROSS)g++
COMPILEOPTION = -c -g
INCLUDEDIR = -I./bm -I./onvif -I../include -I./rtsp -I./rtp
LINK = $(CROSS)g++
LINKOPTION = -g 
LINKOPTION += -std=gnu++0x
LIBRARY_LINK=$(CROSS)ar cr

OBJS = ./bm/base64.o ./bm/hqueue.o ./bm/linked_list.o ./bm/ppstack.o ./bm/rfc_md5.o ./bm/sha1.o ./bm/sys_buf.o ./bm/sys_log.o ./bm/sys_os.o ./bm/util.o ./bm/word_analyse.o ./onvif/http_cln.o ./onvif/http_parse.o ./onvif/http_srv.o ./onvif/hxml.o ./onvif/onvif.o ./onvif/onvif_api.o ./onvif/onvif_cln.o ./onvif/onvif_event.o ./onvif/onvif_pkt.o ./onvif/onvif_probe.o ./onvif/onvif_utils.o ./onvif/soap.o ./onvif/soap_parser.o ./onvif/xml_node.o  ./onviflib.o ./rtsp/rtsp.o ./rtsp/rtsp_parse.o ./rtsp/rua.o ./rtp/h264_rtp_tx.o ./rtp/rtp_queue.o


SHAREDLIB = -lpthread
APPENDLIB = 

PROC_OPTION = DEFINE=_PROC_ MODE=ORACLE LINES=true CODE=CPP
ESQL_OPTION = -g
################OPTION END################
ESQL = esql
PROC = proc

all: $(OUTPUT)

libonviflib.a:$(OBJS) $(APPENDLIB)
	$(LIBRARY_LINK) ../lib/libonviflib.a $(LIBDIRS)   $(OBJS)
	cp ../lib/libonviflib.a ../../../lib/ -rf


onvif_utest:onvif_utest.o gtest_main.a
	$(LINK) -o onvif_utest $(LINKOPTION) onvif_utest.o gtest_main.a -L../lib/ -lonviflib $(SHAREDLIB) $(APPENDLIB)  
onvif_utest.o:utest/onviflib_utest.cpp ../include/onviflib.h
	$(CPPCOMPILE) $(LINKOPTION) $(INCLUDEDIR) -c utest/onviflib_utest.cpp -o onvif_utest.o
 
onviflib_test:onviflib_test.o $(OBJS)
	$(CPPCOMPILE)  -o onviflib_test $(LINKOPTION) $(INCLUDEDIR) onviflib_test.cpp $(LIBDIRS) -L../lib/ -lonviflib $(SHAREDLIB) 
	chmod a+x ./onviflib_test

clean: 
	rm -f $(OBJS)
	rm -f ../utest/*.o
	rm -f ./*.o
	rm -f ../lib/*.a
	rm -f $(OUTPUT)

test:
	./onvif_utest --gtest_output=xml:result.xml



.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

#onviflib_utest.o: ./utest/onviflib_utest.cpp ../include/onviflib.h
#	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  utest/onviflib_utest.cpp


.cpp.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp
	
.cc.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.cxx.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR)  $*.cpp

.c.o:
	$(CCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.c

.C.o:
	$(CPPCOMPILE) -c -o $*.o $(COMPILEOPTION) $(INCLUDEDIR) $*.C	

.ecpp.C:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ecpp 
	
.ec.c:
	$(ESQL) -e $(ESQL_OPTION) $(INCLUDEDIR) $*.ec
	
.pc.cpp:
	$(PROC)  CPP_SUFFIX=cpp $(PROC_OPTION)  $*.pc

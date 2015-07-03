################OPTION###################
CCOMPILE = gcc
#CCOMPILE = arm-hisiv100nptl-linux-gcc
CPPCOMPILE = g++
#CPPCOMPILE = arm-hisiv100nptl-linux-g++
COMPILEOPTION = -c -g
INCLUDEDIR = -I../bm -I../onvif 
LINK = g++
#LINK = arm-hisiv100nptl-linux-g++
LINKOPTION = -g -o onviftest
LIBDIRS = 
OBJS = ../bm/base64.o ../bm/hqueue.o ../bm/linked_list.o ../bm/ppstack.o ../bm/rfc_md5.o ../bm/sha1.o ../bm/sys_buf.o ../bm/sys_log.o ../bm/sys_os.o ../bm/util.o ../bm/word_analyse.o ../onvif/http_cln.o ../onvif/http_parse.o ../onvif/http_srv.o ../onvif/hxml.o ../onvif/onvif.o ../onvif/onvif_api.o ../onvif/onvif_cln.o ../onvif/onvif_event.o ../onvif/onvif_pkt.o ../onvif/onvif_probe.o ../onvif/onvif_utils.o ../onvif/soap.o ../onvif/soap_parser.o ../onvif/xml_node.o OnvifTest.o
OUTPUT = onviftest
SHAREDLIB = -lpthread
APPENDLIB = 
PROC_OPTION = DEFINE=_PROC_ MODE=ORACLE LINES=true CODE=CPP
ESQL_OPTION = -g
################OPTION END################
ESQL = esql
PROC = proc
$(OUTPUT):$(OBJS) $(APPENDLIB)
	$(LINK) $(LINKOPTION) $(LIBDIRS)   $(OBJS) $(SHAREDLIB) $(APPENDLIB) 

clean: 
	rm -f $(OBJS)
	rm -f $(OUTPUT)
all: clean $(OUTPUT)
.PRECIOUS:%.cpp %.c %.C
.SUFFIXES:
.SUFFIXES:  .c .o .cpp .ecpp .pc .ec .C .cc .cxx

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

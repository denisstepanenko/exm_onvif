CROSS_COMPILE?=		arm-hisiv200-linux-
CXX = $(CROSS_COMPILE)g++
CC =  $(CROSS_COMPILE)gcc
LINK =			$(CROSS_COMPILE)g++ -o

INCLUDES = -I ./
COMPILE_OPTS= $(INCLUDES) 

C_FLAGS =		$(COMPILE_OPTS) -Wall 
CPLUSPLUS_FLAGS = -I../src -I../extern_src/include -Wall 
LINK_OPTS = -L../lib
LINK_OPTS +=-lpthread
LINK_OPTS += -lgtsda -lsda -lsgutils2 -lmedia_api -lgtlog








all:testlib 
.c.o:
	$(CC) -c $(C_FLAGS) $<
.cpp.o:
	$(CXX) -c $(CPLUSPLUS_FLAGS) $<	
OBJS=	 testlib.o
testlib.c:cliboftestsda.h



testlib: $(OBJS) libofc.a
	$(LINK) testlib $(OBJS) -L. -lofc  $(LINK_OPTS)
	cp testlib /mnt/yk
	
lib:libofc.a
	cp libofc.a /mnt/yk

libofc.a:cliboftestsda.o
	arm-hisiv200-linux-ar cr libofc.a cliboftestsda.o 
cliboftestsda.o:cliboftestsda.cpp cliboftestsda.h
		$(CXX) -c $(CPLUSPLUS_FLAGS) $<	
clean:
	rm *.o
	
#打包库，并把库复制到指定文件夹	
tar:
	tar zcvf ../testlib.tar.gz ../extern_src/ ../lib ../libofc/
	mv ../testlib.tar.gz /mnt/yk

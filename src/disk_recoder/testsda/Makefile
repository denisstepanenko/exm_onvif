CPP=arm-hisiv200-linux-g++
CXX=arm-hisiv200-linux-g++
LINK=arm-hisiv200-linux-g++ -o
LIBRARY_LINK =		arm-hisiv200-linux-ar cr 

live555top =../live555

EXEC = testsda
OUT=./OUT
SRCDIR = ./src


INCLUDE=
# 头文件路径，可以任意指定头文件
INCLUDE += -I./extern_src/include -I $(SRCDIR)  -I$(SRCDIR)/test  -I./mediaserver


#库文件
CXXFLAGS =  -std=c++0x
#库名字
LIBSS= -lpthread  -lsda -lsgutils2 -lmedia_api -lgtlog 
#库路径
LIBDIR = -L  ./lib  
LDLIBS= $(LIBDIR) $(LIBSS)

CXXFLAGS += $(INCLUDE) $(LDLIBS)

all: testread testsda rw libgtsda.a
	cp testread /mnt/yk/testread.0.5
	cp testsda /mnt/yk/testsda.0.5
	cp rw /mnt/yk/rw
	mv libgtsda.a lib
$(OUT)/%.o:$(SRCDIR)/%.cpp
	$(CXX) -c  $< -o $@ $(CXXFLAGS) 




CPPOBJS = $(OUT)/Blocks.o $(OUT)/DataWR.o $(OUT)/DayBlocks.o\
		 $(OUT)/err.o $(OUT)/FrameQueue.o $(OUT)/hdwr.o $(OUT)/MultBlocks.o $(OUT)/MultMediaBlocks.o\
		 $(OUT)/RWlogic.o\
		 $(OUT)/SecBlocks.o $(OUT)/wrap.o $(OUT)/YearBlocks.o \
		 
		 
Blocks.cpp:Blocks.h
DataWR.cpp:DataWR.h Blocks.h
DayBlocks.cpp:DayBlocks.h Blocks.h
err.cpp:err.h
FrameQueue.cpp:FrameQueue.h Blocks.h
hdwr.cpp:hdwr.h Blocks.h
MultBlocks.cpp:MultBlocks.h Blocks.h
MultMediaBlocks.cpp: MultMediaBlocks.h
RWlogic.cpp:RWlogic.h Blocks.h
SecBlocks.cpp:SecBlocks.h Blocks.h
wrap.cpp:wrap.h
YearBlocks.cpp:YearBlocks.h Blocks.h

testread.cpp:DataWR.h DayBlocks.h RWlogic.h


libgtsda.a:$(CPPOBJS) 
	$(LIBRARY_LINK) $@ $(CPPOBJS) 

testsda: $(OUT)/testsda.o $(CPPOBJS)
	$(CXX) $< -o $@  $(CPPOBJS) $(CXXFLAGS)  $(LIBS)
$(OUT)/testsda.o: $(SRCDIR)/testsda.cpp 	
	$(CXX) -c  $< -o $@ $(CXXFLAGS)
	

testread: $(OUT)/testread.o  $(CPPOBJS) 
	$(CXX) $< -o $@  $(CPPOBJS) $(CXXFLAGS)  $(LIBS)
$(OUT)/testread.o: $(SRCDIR)/testread.cpp 	
	$(CXX) -c  $< -o $@ $(CXXFLAGS)

rw:$(OUT)/rw.o  $(CPPOBJS) 
	$(CXX) $< -o $@  $(CPPOBJS) $(CXXFLAGS)  $(LIBS)
$(OUT)/rw.o:$(SRCDIR)/test/rw.cpp
	$(CXX) -c  $< -o $@ $(CXXFLAGS)


print:
	echo $(CPPOBJS)

clean:
	rm $(OUT)/* 
.PHONY: clean






























USAGE_ENVIRONMENT_DIR = $(live555top)/UsageEnvironment
USAGE_ENVIRONMENT_LIB = $(USAGE_ENVIRONMENT_DIR)/libUsageEnvironment.a
BASIC_USAGE_ENVIRONMENT_DIR = $(live555top)/BasicUsageEnvironment
BASIC_USAGE_ENVIRONMENT_LIB = $(BASIC_USAGE_ENVIRONMENT_DIR)/libBasicUsageEnvironment.a
LIVEMEDIA_DIR = $(live555top)/liveMedia
LIVEMEDIA_LIB = $(LIVEMEDIA_DIR)/libliveMedia.a
GROUPSOCK_DIR = $(live555top)/groupsock
GROUPSOCK_LIB = $(GROUPSOCK_DIR)/libgroupsock.a
LOCAL_LIBS =	$(LIVEMEDIA_LIB) $(GROUPSOCK_LIB) \
		$(BASIC_USAGE_ENVIRONMENT_LIB) $(USAGE_ENVIRONMENT_LIB)
LIBS =			$(LOCAL_LIBS) $(LIBS_FOR_CONSOLE_APPLICATION)

live555MediaServer$(EXE):	$(MEDIA_SERVER_OBJS) $(LOCAL_LIBS)
	$(LINK)$@ $(CONSOLE_LINK_OPTS) $(MEDIA_SERVER_OBJS) $(LIBS)
	
		



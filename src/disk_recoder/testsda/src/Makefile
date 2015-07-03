CPP=arm-hisiv200-linux-g++
CXX=arm-hisiv200-linux-g++
LINK=arm-hisiv200-linux-g++ -o
LIBRARY_LINK =		arm-hisiv200-linux-ar cr 

EXEC = testsda
OUT=../OUT
SRCDIR = ../src


INCLUDE=
# 头文件路径，可以任意指定头文件
INCLUDE += -I../extern_src/include -I $(SRCDIR)  -I$(SRCDIR)/test 


#库文件
CXXFLAGS =  -std=c++0x
#库名字
LIBS= -lpthread  -lsda -lsgutils2 -lmedia_api -lgtlog 
#库路径
LIBDIR = -L  ../lib  
LDLIBS= $(LIBDIR) $(LIBS)

CXXFLAGS += $(INCLUDE)

all: libgtsda.a
	cp libgtsda.a ../
	cp libgtsda.a ../lib


$(OUT)/%.o:$(SRCDIR)/%.cpp
	$(CXX) -c  $< -o $@ $(CXXFLAGS)




CPPOBJS = $(OUT)/Blocks.o $(OUT)/DataWR.o $(OUT)/DayBlocks.o\
		 $(OUT)/err.o $(OUT)/FrameQueue.o $(OUT)/hdwr.o $(OUT)/MultBlocks.o $(OUT)/MultMediaBlocks.o\
		 $(OUT)/RWlogic.o\
		 $(OUT)/SecBlocks.o $(OUT)/wrap.o $(OUT)/YearBlocks.o $(OUT)/DayBlocks.o\
		 
		 
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

libgtsda.a:$(CPPOBJS) 
	$(LIBRARY_LINK) $@ $(CPPOBJS) 




clean:
	rm $(OUT)/* 
.PHONY: clean




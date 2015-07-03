CPP=arm-hisiv200-linux-g++
CXX=arm-hisiv200-linux-g++
 


EXEC = testsda
OUT=OUT
SRCDIR = src
CPPSRC = $(SRCDIR)/Blocks.cpp $(SRCDIR)/DataWR.cpp $(SRCDIR)/DayBlocks.cpp\
		 $(SRCDIR)/err.cpp $(SRCDIR)/hdwr.cpp $(SRCDIR)/MultBlocks.cpp \
		 $(SRCDIR)/SecBlocks.cpp $(SRCDIR)/YearBlocks.cpp
		 
CPPSRC += $(SRCDIR)/test/test_lib_write_and_read.cpp
# 头文件路径，可以任意指定头文件
INCLUDE += -I./extern_src/include -I $(SRCDIR)  -I$(SRCDIR)/test 


#库文件
CPPFLAGS = $(INCLUDE)
#库名字
LIBS= -lpthread  -lsda -lsgutils2 -lmedia_api -lgtlog
#库路径
LIBDIR = -L  ./lib 
LDLIBS= $(LIBDIR) $(LIBS)
# object文件 
OBJS = $(CPPSRC:.cpp=.o) 
CXXFLAGS += $(LIBDIR) $(LIBS) $(INCLUDE)




CPPOBJS = $(SRCDIR)/Blocks.o $(SRCDIR)/DataWR.o $(SRCDIR)/DayBlocks.o\
		 $(SRCDIR)/err.o $(SRCDIR)/hdwr.o $(SRCDIR)/MultBlocks.o\
		 $(SRCDIR)/SecBlocks.o $(SRCDIR)/YearBlocks.o $(SRCDIR)/DayBlocks.o
CPPOBJS += $(SRCDIR)/test/test_lib_write_and_read.o		
CPPOBJS1 = Blocks.o DataWR.o DayBlocks.o err.o hdwr.o MultBlocks.o\
			SecBlocks.o YearBlocks.o
			
#CPPOBJS1 += test_lib_write_and_read.o	
CPPOBJS1 +=   RWlogic.o 

all:$(EXEC) testread rw libgtsda.a
	cp $(EXEC) /mnt/yk/$(EXEC).0.1
	cp testread /mnt/yk/testread.0.1
	cp rw /mnt/yk/rw

$(EXEC):	Blocks.o DataWR.o DayBlocks.o err.o hdwr.o \
MultBlocks.o	SecBlocks.o YearBlocks.o RWlogic.o testsda.o FrameQueue.o wrap.o
	$(CPP) -g -o $(EXEC) $(CPPOBJS1) testsda.o FrameQueue.o wrap.o  $(CPPFLAGS) $(LDLIBS)  -std=c++0x
	
testread:	Blocks.o DataWR.o DayBlocks.o err.o hdwr.o \
MultBlocks.o	SecBlocks.o YearBlocks.o  RWlogic.o testread.o FrameQueue.o wrap.o
	$(CPP) -g -o testread $(CPPOBJS1) testread.o FrameQueue.o wrap.o $(CPPFLAGS) $(LDLIBS)
rw:	Blocks.o DataWR.o DayBlocks.o err.o hdwr.o \
MultBlocks.o	SecBlocks.o YearBlocks.o  RWlogic.o  FrameQueue.o wrap.o rw.o
	$(CPP) -g -o rw $(CPPOBJS1)  FrameQueue.o wrap.o rw.o $(CPPFLAGS) $(LDLIBS)

Blocks.o:	src/Blocks.cpp src/Blocks.h
	$(CPP) -g -c src/Blocks.cpp $(CPPFLAGS)
DayBlocks.o:	src/DayBlocks.cpp src/DayBlocks.h
	$(CPP) -g  -c src/DayBlocks.cpp $(CPPFLAGS)
YearBlocks.o:	src/YearBlocks.cpp src/YearBlocks.h
	$(CPP) -g  -c src/YearBlocks.cpp $(CPPFLAGS)
DataWR.o:	src/DataWR.cpp src/DataWR.h
	$(CPP) -g -c src/DataWR.cpp $(CPPFLAGS)
err.o:		src/err.cpp src/err.h
	$(CPP) -g -c src/err.cpp $(CPPFLAGS)
hdwr.o:		src/hdwr.cpp src/hdwr.h
	$(CPP) -g -c src/hdwr.cpp $(CPPFLAGS)
MultBlocks.o:	src/MultBlocks.h src/MultBlocks.cpp
	$(CPP) -g -c src/MultBlocks.cpp $(CPPFLAGS)
SecBlocks.o:	 src/SecBlocks.cpp  src/SecBlocks.h
	$(CPP) -g -c src/SecBlocks.cpp $(CPPFLAGS)
test_lib_write_and_read.o:		src/test/test_lib_write_and_read.cpp
	$(CPP) -g -c src/test/test_lib_write_and_read.cpp $(CPPFLAGS)
testsda.o: src/testsda.cpp
	$(CPP) -g -c src/testsda.cpp $(CPPFLAGS)
RWlogic.o: src/RWlogic.cpp src/RWlogic.h
	$(CPP) -g -c src/RWlogic.cpp $(CPPFLAGS) -std=c++0x
testread.o: src/testread.cpp 
	$(CPP) -g -c src/testread.cpp $(CPPFLAGS)
rw.o:	src/test/rw.cpp
	$(CPP) -g -c src/test/rw.cpp $(CPPFLAGS)
wrap.o: src/wrap.cpp src/wrap.h
	$(CPP) -g -c src/wrap.cpp $(CPPFLAGS)
FrameQueue.o:	src/FrameQueue.cpp src/FrameQueue.h src/Blocks.h
	$(CPP) -g -c src/FrameQueue.cpp $(CPPFLAGS)
	
libgtsda.a:Blocks.o DataWR.o DayBlocks.o err.o hdwr.o \
MultBlocks.o	SecBlocks.o YearBlocks.o RWlogic.o testsda.o FrameQueue.o wrap.o
	arm-hisiv200-linux-ar rs libgtsda.a $(CPPOBJS1)  FrameQueue.o wrap.o   








clean:
	rm ./*.o 
	rm $(EXEC)

.PHONY: clean




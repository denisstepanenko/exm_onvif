CROSS_COMPILE?=		arm-hisiv200-linux-


CXX = $(CROSS_COMPILE)g++
CC =  $(CROSS_COMPILE)gcc

 
 
TOP = $(PWD)
OUT = $(TOP)/../OUT
OUTLIB = $(TOP)/../lib

vpath %.cpp $(TOP)/
vpath %.o $(OUT) $(TOP)

live555top =../../live555
testsda =../../testsda


CXXFLAGS=-g -DGTSDA

INCLUDES = -I$(live555top)/UsageEnvironment/include -I$(live555top)/groupsock/include -I$(live555top)/liveMedia/include -I$(live555top)/BasicUsageEnvironment/include
INCLUDES += -I../src/ -I../extern_src/include

CXXFLAGS += $(INCLUDES) $(LIBS) -L../lib  -lpthread -lgtsda -lsda -lsgutils2 -lgtlog -lmedia_api
OBJDIR = $(OUT)
SRCS = $(wildcard $(TOP)/*.cpp)
dir=$(notdir $(SRCS))
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(dir))
all:  $(OBJS) libliveapp.a mediaserver
	cp mediaserver /mnt/yk
	
libliveapp.a:$(OBJS) 
	arm-hisiv200-linux-ar cr  libliveapp.a  $^  
	cp libliveapp.a $(OUTLIB)


#$@
mediaserver: $(OBJS)
	$(CXX) -o mediaserver $^ $(CXXFLAGS)  


$(OBJDIR)/%.o:$(TOP)/%.cpp
	$(CXX) -c -o $@ $<  $(CXXFLAGS)
	
	
	
	
	
	
	
	
	
	
	
	
	




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
	
		
print:
	echo $(SRCS)
	echo $(OBJS)
	echo $(OBJDIR)/%.o
	echo ""
	
.PHONY: clean	
clean:
	rm $(OBJS) 

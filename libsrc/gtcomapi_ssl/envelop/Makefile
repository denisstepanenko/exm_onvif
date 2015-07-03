SSL_HOMEDIR=../../../lib
#libssl
#../../reference/openssl-0.9.7e
SSL_INCLUDEDIR=../../../include
#$(SSL_HOMEDIR)/include
SSL_LIBDIR=$(SSL_HOMEDIR)

RM=rm -f
CC=arm-linux-gcc
DEBUG_FLAGS=-g -ggdb3 -Wall -Wshadow
INCLUDE_FLAGS=-I$(SSL_INCLUDEDIR)
CFLAGS=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) -DNO_CONFIG_H
COMPILE=$(CC) $(CFLAGS) -c

# Edit, particularly the "-ldl" if not building with "dlfcn" support
LINK_FLAGS=-L$(SSL_LIBDIR) -lssl -lcrypto 

SRCS=loadkeys.c gt_env.c env_test.c 
OBJS=loadkeys.o gt_env.o env_test.o

TARGETS=env_test

default: $(TARGETS)

clean:
	$(RM) $(OBJS) $(TARGETS) *.bak core

.c.o:
	$(COMPILE) $<

env_test: $(OBJS)
	$(CC) -o env_test $(OBJS) $(LINK_FLAGS)
cp:
	cp env_test /home/shixin/nfs/shixin/envelop
# Extra dependencies, should really use makedepend
loadkeys.o: loadkeys.c gt_env.h
gt_env.o: gt_env.c gt_env.h
env_test.o: env_test.c gt_env.h

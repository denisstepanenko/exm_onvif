TOP_DIR  = $(shell pwd)
#EMBED = 1 

EXEC = savem printpool printele
#testm savem testp printpool printele
#recvp
CPFILES= $(EXEC) 
#fixdiskproblem
SRC_TESTM = ../mshm.c ../mshmpool.c main.c
SRC_SAVEM = ../mshm.c ../mshmpool.c savem.c
SRC_TESTP = ../mshm.c ../mshmpool.c testp.c
SRC_RECVP = ../mshm.c ../mshmpool.c recvp.c
SRC_PRINTPOOL = ../mshm.c ../mshmpool.c printpool.c
SRC_PRINTELE = ../mshm.c ../mshmpool.c printeles.c
ifdef EMBED
	CC=arm-linux-gcc
	CFLAGS +=-DEMBEDED=1
	LIBDIR +=-L../emblib -s
else
	CC = gcc
	CFLAGS +=-DEMBEDED=0 -ggdb3
	LIBDIR +=-L../pclib
endif
INCDIR +=-I../
CFLAGS +=$(INCDIR)
CFLAGS += -Wall
LDLIBS +=$(LIBDIR) -lpthread 
all: $(EXEC)
testshm:
	$(CC) $(CFLAGS) testshm.c -o testshm 
testm:
	$(CC) $(CFLAGS) $(SRC_TESTM) -o testm $(LDLIBS)
savem:
	$(CC) $(CFLAGS) $(SRC_SAVEM) -o savem $(LDLIBS)
testp:
	$(CC) $(CFLAGS) $(SRC_TESTP) -o testp $(LDLIBS)
recvp:
	$(CC) $(CFLAGS) $(SRC_RECVP) -o recvp $(LDLIBS)
printpool:
	$(CC) $(CFLAGS) $(SRC_PRINTPOOL) -o printpool $(LDLIBS)
printele:
	$(CC) $(CFLAGS) $(SRC_PRINTELE) -o printele $(LDLIBS)
cp:
	cp $(EXEC) /home/temp/nfs/shixin
clean:
	-rm -f $(EXEC) *.elf *.gdb *.o 
symlink:

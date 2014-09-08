CC = g++
CFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -c
LFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -Wall

UTIL_DIR = ../revised_version/
INC_UTIL = -I$(UTIL_DIR)

all : test

test : warp_protocol.o util.o test.o
	$(CC) warp_protocol.o test.o util.o $(LFLAGS) -o test.out

test.o : test.cpp
	$(CC) test.cpp $(CFLAGS) -o test.o

warp_protocol.o : warp_protocol.cpp
	$(CC) warp_protocol.cpp $(CFLAGS) -o warp_protocol.o

util.o : $(UTIL_DIR)util.cpp
	$(CC) $(UTIL_DIR)util.cpp $(CFLAGS) $(INC_UTIL) -o util.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output

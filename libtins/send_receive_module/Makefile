CC = g++
CFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -c
LFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
UTIL_DIR = ../revised_version/

INC_WARP = -I$(WARP_PROTOCOL_DIR)
INC_UTIL = -I$(UTIL_DIR)

all : test

test : warp_protocol.o util.o fragment_sender.o config.o
	$(CC) fragment_sender.o warp_protocol.o util.o config.o $(LFLAGS) -o test.out

fragment_receiver.o : fragment_receiver.cpp
	$(CC) fragment_receiver.cpp $(CFLAGS) -o fragment_receiver.o

fragment_sender.o : fragment_sender.cpp
	$(CC) fragment_sender.cpp $(CFLAGS) -o fragment_sender.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC_WARP) -o warp_protocol.o

util.o : $(UTIL_DIR)util.cpp
	$(CC) $(UTIL_DIR)util.cpp $(CFLAGS) $(INC_UTIL) -o util.o

config.o : $(UTIL_DIR)config.cpp
	$(CC) $(UTIL_DIR)config.cpp $(CFLAGS) $(INC_UTIL) -o config.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output

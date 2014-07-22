CC = g++
CFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -c
LFLAGS = -O3 -std=c++0x -lpthread -ltins -lpcap -Wall

all : test

test : warp_transmit.o warp_protocol.o test.o
	$(CC) warp_transmit.o warp_protocol.o test.o $(LFLAGS) -o test.out

test.o : test.cpp
	$(CC) test.cpp $(CFLAGS) -o test.o

warp_protocol.o : warp_protocol.cpp
	$(CC) warp_protocol.cpp $(CFLAGS) -o warp_protocol.o

warp_transmit.o : warp_transmit.cpp
	$(CC) warp_transmit.cpp $(CFLAGS) -o warp_transmit.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output

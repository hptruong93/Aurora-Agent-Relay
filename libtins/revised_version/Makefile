CC = g++
CFLAGS = -O3 -lpthread -ltins -lpcap -c 
LFLAGS = -O3 -lpthread -ltins -lpcap -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
INC = -I$(WARP_PROTOCOL_DIR)

all : pc_to_warp warp_to_pc

pc_to_warp : pc_to_warp.cpp
	$(CC) pc_to_warp.cpp $(CFLAGS) -o pc_to_warp.out

warp_to_pc : warp_protocol.o warp_to_pc.o 
	$(CC) warp_to_pc.o warp_protocol.o $(LFLAGS) -o warp_to_pc.out

warp_to_pc.o : warp_to_pc.cpp
	$(CC) warp_to_pc.cpp $(CFLAGS) -o warp_to_pc.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC) -o warp_protocol.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output

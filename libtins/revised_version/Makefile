CC = g++
CFLAGS = -O3 -lpthread -ltins -lpcap 
LFLAGS = -O3 -lpthread -ltins -lpcap -Wall

all : pc_to_warp warp_to_pc

pc_to_warp : pc_to_warp.cpp
	$(CC) pc_to_warp.cpp $(CFLAGS) -o pc_to_warp.out

warp_to_pc : warp_to_pc.cpp
	$(CC) warp_to_pc.cpp $(CFLAGS) -o warp_to_pc.out

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output

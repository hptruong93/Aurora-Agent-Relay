CC = g++
# -pthread can be removed when c++11 is used
COMMON_FLAGS = -g -O3 -pthread -ltins -lpcap -lzmq -ljansson -std=c++0x
CFLAGS = $(COMMON_FLAGS) -c
LFLAGS = $(COMMON_FLAGS) -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
INC_WARP_PROTOCOL = -I$(WARP_PROTOCOL_DIR)

SEND_RECEIVE_MODULE_DIR = ../send_receive_module/
INC_SEND_MODULE = -I$(SEND_RECEIVE_MODULE_DIR)

UTILITY_DIR = ../revised_version/
INC_UTILITY = -I$(UTILITY_DIR)

DPM_DIR = ../datapath_manager/
INC_DPM = -I$(DPM_DIR)

RELAY_AGENT_MODULE_DIR = ../relay_agent_module/
INC_RELAY_AGENT = -I$(RELAY_AGENT_MODULE_DIR)

FILTER_DIR = ../filters/
INC_FILTER = -I$(FILTER_DIR)

###########################################################################################################

all: agents

########################################  Linking  ########################################################

agents : util.o config.o warp_protocol.o fragment_receiver.o warp_protocol_sender.o relay_agent.o wlan_to_warp_agent.o mon_to_warp_agent.o warp_to_wlan_agent.o comms_agent.o dpm_agent.o agents.o packet_filter.o management_filter.o dot11_filter.o
	$(CC) util.o config.o warp_protocol.o fragment_receiver.o warp_protocol_sender.o relay_agent.o wlan_to_warp_agent.o mon_to_warp_agent.o warp_to_wlan_agent.o comms_agent.o dpm_agent.o agents.o $(LFLAGS) -o agents.out

###################################Compiling main files####################################################
dpm_agent.o: $(DPM_DIR)dpm_agent.cpp
	$(CC) $(DPM_DIR)dpm_agent.cpp $(CFLAGS) $(INC_RELAY_AGENT) $(INC_UTILITY) -o dpm_agent.o

comms_agent.o: comms_agent.cpp
	$(CC) comms_agent.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o comms_agent.o

wlan_to_warp_agent.o : wlan_to_warp_agent.cpp
	$(CC) wlan_to_warp_agent.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o wlan_to_warp_agent.o

mon_to_warp_agent.o : mon_to_warp_agent.cpp
	$(CC) mon_to_warp_agent.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o mon_to_warp_agent.o

warp_to_wlan_agent.o : warp_to_wlan_agent.cpp
	$(CC) warp_to_wlan_agent.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o warp_to_wlan_agent.o

relay_agent.o : relay_agent.cpp
	$(CC) relay_agent.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o relay_agent.o

agents.o : agents.cpp
	$(CC) agents.cpp $(CFLAGS) $(INC_DPM) $(INC_WARP_PROTOCOL) $(INC_SEND_MODULE) $(INC_UTILITY) -o agents.o

##################################Files to link later######################################################

packet_filter.o : $(FILTER_DIR)packet_filter.cpp
	$(CC) $(FILTER_DIR)packet_filter.cpp $(CFLAGS) $(INC_FILTER) -o packet_filter.o

management_filter.o : $(FILTER_DIR)management_filter.cpp
	$(CC) $(FILTER_DIR)management_filter.cpp $(CFLAGS) $(INC_FILTER) -o management_filter.o

dot11_filter.o : $(FILTER_DIR)dot11_filter.cpp
	$(CC) $(FILTER_DIR)dot11_filter.cpp $(CFLAGS) $(INC_FILTER) -o dot11_filter.o


fragment_receiver.o : $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp $(CFLAGS) $(INC_SEND_MODULE) -o fragment_receiver.o

warp_protocol_sender.o : $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp $(CFLAGS) $(INC_SEND_MODULE) -o warp_protocol_sender.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) -o warp_protocol.o

util.o : $(UTILITY_DIR)util.cpp
	$(CC) $(UTILITY_DIR)util.cpp $(CFLAGS) $(INC_UTILITY) -o util.o

config.o : $(UTILITY_DIR)config.cpp
	$(CC) $(UTILITY_DIR)config.cpp $(CFLAGS) $(INC_UTILITY) -o config.o

###########################################################################################################

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output
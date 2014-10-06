#include "../revised_version/config.h"
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

#include <string>

#include <zmq.hpp>
#include <jansson.h>
#include <tins/tins.h>

#ifndef AGENTS_H_
#define AGENTS_H_

#define DEFAULT_MSG_SIZE                1024

// Agent parameters
#define INTERFACE_IN                    "in_interface"
#define INTERFACE_OUT                   "out_interface"
#define MAC_ADDRESS                     "macaddr"
#define CHANNEL                         "channel"
#define HW_MODE                         "hwmode"
#define TX_POWER                        "txpower"
#define DISABLED                        "disabled"

#define AGENT_TYPE                      "agent_type"

// Agent types
#define WLAN_TO_WARP                    "wlan_to_warp"
#define MON_TO_WARP                     "mon_to_warp"
#define WARP_TO_WLAN                    "warp_to_wlan"

using namespace std;

namespace RelayAgents {

    enum class ErrorCode {
        OK = 0,
        ERROR = 1
    };

    // Utility class
    class AgentUtil {
        string send_port;
        string recv_port;
        unique_ptr<string> out_interface;
        unique_ptr<WARP_ProtocolSender> protocol_sender;
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> pub_socket;
        unique_ptr<zmq::socket_t> sub_socket;
        public:
            AgentUtil(const char *init_out_interface = "eth1", string init_send_port = "5555", string init_recv_port = "5556");
            ErrorCode parse_json(const char *json_string);
            void spin();
    };
    
    // Receives command and initialize different agents
    class AgentFactory {
        public:
            AgentFactory();
            void spawn_agent_thread(const char* agent_type, int argc, char *argv[]);
    };
}

RelayAgents::ErrorCode parse_mac(const char *origin, uint8_t dest[]);

#endif
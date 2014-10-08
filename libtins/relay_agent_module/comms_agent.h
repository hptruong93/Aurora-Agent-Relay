#ifndef COMMSAGENT_H_
#define COMMSAGENT_H_

#include "../revised_version/config.h"
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

#include <string>

#include <zmq.hpp>
#include <jansson.h>
#include <tins/tins.h>

#define DEFAULT_MSG_SIZE                1024

// Agent parameters
#define MAC_ADDRESS                     "macaddr"
#define CHANNEL                         "channel"
#define HW_MODE                         "hwmode"
#define TX_POWER                        "txpower"
#define DISABLED                        "disabled"

#define AGENT_TYPE                      "agent_type"

namespace RelayAgents {

    enum class ErrorCode {
        OK = 0,
        ERROR = 1
    };

    // Utility class
    class CommsAgent {
        std::string send_port;
        std::string recv_port;
        std::unique_ptr<std::string> out_interface;
        std::unique_ptr<WARP_ProtocolSender> protocol_sender;
        std::unique_ptr<zmq::context_t> context;
        std::unique_ptr<zmq::socket_t> pub_socket;
        std::unique_ptr<zmq::socket_t> sub_socket;
        public:
            CommsAgent(const char *init_out_interface = "eth1", std::string init_send_port = "5555", std::string init_recv_port = "5556");
            ErrorCode parse_json(const char *json_string);
            void spin();
    };
}

RelayAgents::ErrorCode parse_mac(const char *origin, uint8_t dest[]);

#endif
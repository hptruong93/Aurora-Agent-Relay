#ifndef COMMSAGENT_H_
#define COMMSAGENT_H_

#include "../revised_version/config.h"
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <semaphore.h>

#include <zmq.hpp>
#include <jansson.h>
#include <tins/tins.h>

#include "relay_agent.h"

#define DEFAULT_MSG_SIZE                1024

// Agent parameters
#define MAC_ADDRESS                     "macaddr"
#define CHANNEL                         "channel"
#define HW_MODE                         "hwmode"
#define TX_POWER                        "txpower"
#define DISABLED                        "disabled"

// Command for factory
#define WLAN_TO_WARP                    "wlan_to_warp"
#define MON_TO_WARP                     "mon_to_warp"
#define WARP_TO_WLAN                    "warp_to_wlan"

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
            CommsAgent(const char *init_out_interface = NULL, std::string init_send_port = "5555", std::string init_recv_port = "5556");
            ErrorCode parse_json(const char *json_string);
            void set_out_interface(const char *new_out_interface);
            void spin();

            // TODO
            static std::mutex message_lock;
            static std::unique_ptr<string> send_message;
            static sem_t signal;
    };
    
    // Receives command and initialize different agents
    class AgentFactory {
        public:
            // The map stores both thread information and pointers to agents that are being used
            static std::map<int, std::shared_ptr<RelayAgent>> agent_threads;
            static int current_thread_id;
            static std::mutex lock;
            static void spawn_agent_thread(std::vector<std::string>& args);
            static void kill_agent_thread(int id);
    };
}

// Function declarations

RelayAgents::ErrorCode parse_mac(const char *origin, uint8_t dest[]);
#endif
#ifndef COMMSAGENT_H_
#define COMMSAGENT_H_

#include "../revised_version/config.h"
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <semaphore.h>

#include <zmq.hpp>
#include <jansson.h>
#include <tins/tins.h>

#include "relay_agent.h"
#include "bssid_node.h"

#define DEFAULT_MSG_SIZE                1024

// Command types from received json object
#define RADIO_SET_CMD                   "_radio_set_command"
#define RADIO_BULK_SET_CMD              "_bulk_radio_set_command"
#define UCI_DELETE_SECTION              "_uci_delete_section_name"
#define UCI_DELETE_BSS                  "_uci_delete_bss_index"

// Json parameters
#define JSON_COMMAND                    "command"
#define JSON_CHANGES                    "changes"
#define JSON_MAC_ADDRESS                "macaddr"
#define JSON_CHANNEL                    "channel"
#define JSON_HW_MODE                    "hwmode"
#define JSON_TX_POWER                   "txpower"
#define JSON_DISABLED                   "disabled"

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
        std::unique_ptr<std::string> send_port;
        std::unique_ptr<std::string> recv_port;
        std::unique_ptr<std::string> peer_ip_addr;
        std::unique_ptr<zmq::socket_t> pub_socket;
        std::unique_ptr<zmq::socket_t> sub_socket;
        std::mutex mac_add_success_lock;
        bool mac_add_success;
        public:
            CommsAgent(const char *init_send_port = "5555", const char *init_recv_port = "5556", const char *init_peer_ip_addr = "localhost");
            ErrorCode parse_json(const char *json_string);
            // ZMQ communications
            void send_loop();
            void recv_loop();
            // Json parsing loop
            void parse_loop();
            void set_error_msg(const std::string& message = std::string(""));
            void set_msg(const std::string& message = std::string(""));
            // Bssid update
            void update_bssids(int operation_code, void* bssid);
            void add_to_bssid_group(BssidNode* node);
            void set_warp_to_wlan_agent(BssidNode* agent);
        private:
            std::mutex bssid_update_group_mux;
            std::mutex warp_to_wlan_agent_mux;
            std::vector<BssidNode*> bssid_update_group;
            std::shared_ptr<BssidNode> warp_to_wlan_agent;
            // Used by zmq receiver to signal the send to send the first packet recieved
            // Back to the source
            std::mutex message_lock;
            std::unique_ptr<string> send_message;
            sem_t signal;
            // Used for command queue
            std::mutex command_queue_lock;
            std::queue<std::string> command_queue;
            sem_t new_command;
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
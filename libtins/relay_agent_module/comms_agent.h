#ifndef COMMSAGENT_H_
#define COMMSAGENT_H_

#include "config.h"
#include "warp_protocol_sender.h"
#include "fragment_receiver.h"
#include "warp_protocol.h"

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
#define UCI_DELETE_SECTION              "_delete_section_name"
#define UCI_DELETE_BSS                  "_delete_bss_index"
#define MAC_ASSOCIATE_CMD               "_mac_associate"
#define MAC_DISASSOCIATE_CMD            "_mac_disassociate"
#define SHUTDOWN_CMD                    "shutdown"

// Json parameters
#define JSON_COMMAND                    "command"
#define JSON_CHANGES                    "changes"
#define JSON_MAC_ADDRESS                "macaddr"
#define JSON_CHANNEL                    "channel"
#define JSON_HW_MODE                    "hwmode"
#define JSON_TX_POWER                   "txpower"
#define JSON_DISABLED                   "disabled"
#define JSON_BSSID                      "bssid"
#define JSON_RADIO                      "radio"
#define RESPONSE_HEADER                 "111 "

// Command for factory
#define WLAN_TO_WARP                    "wlan_to_warp"
#define MON_TO_WARP                     "mon_to_warp"
#define WARP_TO_WLAN                    "warp_to_wlan"

#define AGENT_TYPE                      "agent_type"

// Return codes
#define RETURN_CODE_OK                  0x00
#define RETURN_CODE_ERROR               0x01
#define RETURN_CODE_SEND_RESPONSE       0x02
#define RETURN_CODE_NO_RESPONSE         0x04

namespace RelayAgents {

    // Utility class
    class CommsAgent : public BssidNode {
        std::unique_ptr<std::string> send_port;
        std::unique_ptr<std::string> recv_port;
        std::unique_ptr<std::string> peer_ip_addr;
        std::unique_ptr<zmq::socket_t> pub_socket;
        std::unique_ptr<zmq::socket_t> sub_socket;
        std::mutex mac_add_success_lock;
        bool mac_add_success;
        public:
            CommsAgent(const char *init_send_port = "6001", const char *init_recv_port = "6000", const char *init_peer_ip_addr = "localhost");
            uint8_t parse_json(const char *json_string);
            // ZMQ communications
            void send_loop();
            void recv_loop();
            // Json parsing loop
            void parse_loop();
            void set_error_msg(const std::string& command, const std::string& message = std::string(""));
            void set_success_msg(const std::string& command, const std::string& radio, const std::string& message = std::string(""));
            void set_msg(const std::string& message = std::string(""));
            void send_msg(const std::string& message = std::string(""));
            // Bssid update
            void update_bssids(int operation_code, void* bssid);
            void add_to_bssid_group(BssidNode* node);
            void set_warp_to_wlan_agent(BssidNode* agent);
            // Override
            int sync(int operation_code, void* command);
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
uint8_t parse_mac(const char *origin, uint8_t dest[]);
#endif

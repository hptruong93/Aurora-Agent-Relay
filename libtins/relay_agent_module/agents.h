#include <string>
#include <zmq.hpp>
#include <jansson.h>

#ifndef AGENTS_H_
#define AGENTS_H_

#define DEFAULT_MSG_SIZE        1024
#define INTERFACE_IN            "in_interface"
#define INTERFACE_OUT           "out_interface"
#define MAC_ADDRESS             "mac_addr"

using namespace std;

namespace RelayAgents {
    // Utility class
    class AgentUtil {
        unique_ptr<char> in_interface;
        unique_ptr<char> out_interface;
        unique_ptr<char> mac_addr;
        int parameters;
        public:
            AgentUtil();
            int parse_json(const char *json_string);
            int get_parameter_count() const;
            char* transfer_parameter(const char *parameter_name);
            char* get_parameter(const char *parameter_name) const;
            void reset();
    };
    
    // Receives command and initialize different agents
    class AgentFactory {
        string port;
        AgentUtil util;
        unique_ptr<zmq::context_t> context;
        unique_ptr<zmq::socket_t> socket;
        public:
            AgentFactory(string init_port = "5556");
            void spin();
            // void spawn_agent_thread();
    };
}


#endif
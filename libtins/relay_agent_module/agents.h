#include <string>
#include <zmq.hpp>

#ifndef AGENTS_H_
#define AGENTS_H_

#define DEFAULT_MSG_SIZE        1024

using namespace std;

class Agents {
    string port;
    unique_ptr<zmq::context_t> context;
    unique_ptr<zmq::socket_t> socket;
    public:
        Agents(string init_port = "5556");
        void spin();
        // void spawn_agent_thread();
};


#endif
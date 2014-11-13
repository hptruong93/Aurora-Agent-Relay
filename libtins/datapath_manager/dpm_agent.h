#ifndef DPM_AGENT_H_
#define DPM_AGENT_H_

#include "../relay_agent_module/bssid_node.h"

#include <string>
#include <stdlib.h>
#include <thread>

#define BASE_COMMAND_STR            "python -u ../temp_test/datapath_manager.py"
#define DEFAULT_OVS_NAME			"tb"

class DPMAgent : public BssidNode
{
    public:
    	DPMAgent();
    	int init(const std::string& ovs_name = std::string(DEFAULT_OVS_NAME));
    	int add(const std::string& bssid, const std::string& ethernet_interface = std::string("eth1"));
    	int remove(const std::string& bssid, const std::string& ethernet_interface = std::string("eth1"));
        // override BssidNode
        int sync(int operation_code, void* bssid);
    private:
    	std::string ovs_name;
    	std::string socket_path;
    	std::unique_ptr<std::thread> ovs_thread;
    	void initialize(std::string ovs);
        int execute_command(std::string command);
};

#endif
#ifndef DPM_AGENT_H_
#define DPM_AGENT_H_

#include "../relay_agent_module/bssid_node.h"

#include <string>
#include <stdlib.h>
#include <thread>
#include <vector>

#define BASE_COMMAND_STR            "python -u ../temp_test/datapath_manager.py"
#define DEFAULT_OVS_NAME			"tb"
#define HOSTAPD_COMMNAD             "hostapd_cli all_sta"

class DPMAgent : public BssidNode
{
    public:
    	DPMAgent();
    	int init(const std::string& ovs_name = std::string(DEFAULT_OVS_NAME));
    	int add(const std::string& bssid, const std::string& ethernet_interface = std::string("eth1"));
    	int remove(const std::string& bssid, const std::string& ethernet_interface = std::string("eth1"));
        int associate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        int disassociate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        // Periodically check hostapd and associate/disassociate
        void timed_check(int period);
        // override BssidNode
        int sync(int operation_code, void* bssid);
    private:
    	std::string ovs_name;
    	std::string socket_path;
    	std::unique_ptr<std::thread> ovs_thread;
        std::vector<std::string> associated_mac_addr;
    	void initialize(std::string ovs);
        int execute_command(std::string command);
};

#endif
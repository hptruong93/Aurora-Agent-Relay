#ifndef DPM_AGENT_H_
#define DPM_AGENT_H_

#include "../relay_agent_module/bssid_node.h"

#include <string>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <map>
#include <set>

#define BASE_COMMAND_STR            "python -u ../datapath_manager/datapath_manager.py"
#define DEFAULT_OVS_NAME			"tb"
#define ALL_STATION_COMMAND             "hostapd_cli all_sta -i "

class DPMAgent : public BssidNode
{
    public:
    	DPMAgent();
    	int init(const std::string& ovs_name = std::string(DEFAULT_OVS_NAME));
    	int add(const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
    	int remove(const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        int associate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        int disassociate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        // Periodically check hostapd and associate/disassociate
        void timed_check(float seconds);
        // override BssidNode
        int sync(int operation_code, void* bssid);
    private:
    	std::string ovs_name;
    	std::string socket_path;
    	std::unique_ptr<std::thread> ovs_thread;
        unsigned int virtual_ethernet_count;
        // Store virtual wlan interfaces and pid
        std::map<std::string, int> vwlan_pids;

        // Store added interface names and associated station mac addresses
        std::map<std::string, std::set<std::string>> interface_mac;
    	void initialize(std::string ovs);
        int execute_command(std::string command);
};

#endif
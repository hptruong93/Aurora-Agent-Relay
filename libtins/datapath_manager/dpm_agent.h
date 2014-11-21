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
#define ALL_STATION_COMMAND         "hostapd_cli all_sta -i "
// Json builders
#define LEFT_BRACKET                std::string("{")
#define RIGHT_BRACKET               std::string("}")
#define QUOTE                       std::string("\"")
#define COLON                       std::string(":")
#define COMMA                       std::string(",")

class DPMAgent : public BssidNode
{
    public:
    	DPMAgent();
    	int init(const std::string& ovs_name = std::string(DEFAULT_OVS_NAME));
    	int add(const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
    	int remove(const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        int associate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        int disassociate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface = std::string("eth1"));
        void set_agent(BssidNode *agent) { comms_agent = agent; };
        // Periodically check hostapd and associate/disassociate
        void timed_check(float seconds);
        // override BssidNode
        int sync(int operation_code, void* bssid);
    private:
    	std::string ovs_name;
    	std::string socket_path;
    	std::unique_ptr<std::thread> ovs_thread;
        unsigned int virtual_ethernet_count;
        BssidNode *comms_agent;
        // Store virtual wlan interfaces and pid
        std::map<std::string, int> vwlan_pids;

        // Store added interface names and associated station mac addresses. Format: (interface_name, vector(mac_addr))
        std::map<std::string, std::set<std::string>> interface_mac;
        // Store added interface names and its bssid. Format: (interface_name, bssid)
        std::map<std::string, std::string> interface_bssid;
        // Reverse map :(bssid, interface_name)
        std::map<std::string, std::string> bssid_interface;
    	void initialize(std::string ovs);
        int execute_command(std::string command);
};

#endif
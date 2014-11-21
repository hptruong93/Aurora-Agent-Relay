#include "dpm_agent.h"
#include "util.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>

using namespace std;

DPMAgent::DPMAgent()
{
    this->virtual_ethernet_count = 0;
}

int DPMAgent::init(const std::string& init_ovs_name)
{
    ovs_name = init_ovs_name;
    ovs_thread = std::unique_ptr<std::thread>(new std::thread(&DPMAgent::initialize, this, init_ovs_name));
    ovs_thread.get()->detach();
}

void DPMAgent::initialize(std::string ovs_name)
{
    FILE *fp;
    char *line = (char*)malloc(1024 * sizeof(char));
    int index = 0;
    char c = 's';

    fp = popen((std::string(BASE_COMMAND_STR) + " " + "init" + " " + ovs_name).c_str(), "r");
    while (true)
    {
        c = fgetc(fp);
        if (c == '\n') {
            break;
        }
        line[index++] = c;
    }
    line[index] = '\0';

    std::string line_str = std::string(line);

    free(line);

    line_str.erase(0, line_str.find('/'));

    socket_path = line_str;
    //std::cout << "Found socket path " << socket_path << std::endl;
}

int DPMAgent::add(const std::string& wlan_interface, const std::string& ethernet_interface)
{
    std::string virtual_interface = std::string("v" + std::string(wlan_interface) + "-dpm");

    std::string virtual_ethernet;
    virtual_ethernet = std::string("v" + std::string(ethernet_interface) + "-dpm");
    if (this->virtual_ethernet_count == 0) {
        this->virtual_ethernet_count++;
    }
    
    std::cout << "Creating virtual interfaces... " << virtual_interface << " and " << virtual_ethernet << std::endl;
    system(("vethd -v " + virtual_interface + " -e " + std::string(wlan_interface)).c_str());
    system(("ifconfig " + virtual_interface + " up").c_str());

    //Getting vethd pid for the created interface
    std::string a("vethd -v " + virtual_interface + " -e " + std::string(wlan_interface));
    std::vector<int> v;
    get_pid(a, &v, 1);
    printf("Found pid %d\n", *(v.begin()));
    vwlan_pids.insert(std::pair<std::string, int>(std::string(wlan_interface), *(v.begin())));

    if (this->virtual_ethernet_count == 1) {
        system(("vethd -v " + virtual_ethernet + " -e " + ethernet_interface).c_str());
        system(("ifconfig " + virtual_ethernet + " up").c_str());
    }

    std::cout<<"Command: " << "add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet <<std::endl;
    return execute_command("add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet);
}

int DPMAgent::remove(const std::string& wlan_interface, const std::string& ethernet_interface)
{
    //Get pid 
    auto search = vwlan_pids.find(wlan_interface);
    if(search != vwlan_pids.end()) {
        std::cout << "Found ----------------------------------- " << search->first << " " << search->second << '\n';
    }

    cout << "--------------------" << ("kill -SIGTERM " + std::to_string(search->second)) << endl;
    system(("kill -SIGTERM " + std::to_string(search->second)).c_str());
    vwlan_pids.erase(wlan_interface);

    std::string virtual_interface = std::string("v" + std::string(wlan_interface) + "-dpm");
    return execute_command("remove " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface);   
}

int DPMAgent::associate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface)
{
    return execute_command("associate" + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface + " " + bssid);
}

int DPMAgent::disassociate(const std::string& bssid, const std::string& virtual_interface, const std::string& ethernet_interface)
{
    return execute_command("disassociate" + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface + " " + bssid);
}

void DPMAgent::timed_check(float seconds)
{
    while (true)
    {
        // Main check function
        for (std::map<std::string, std::set<std::string>>::iterator it = interface_mac.begin(); it != interface_mac.end(); it++)
        {
            std::string virtual_interface = it->first;

            // Call hostapd command
            char *output_str = get_command_output(std::string(ALL_STATION_COMMAND + virtual_interface));
            std::string output(output_str);
            free(output_str);

            // Get all mac addr from command output
            std::set<std::string> output_mac_addr;
            split_string(output, "\n", &output_mac_addr);

            // Common mac addr
            std::vector<std::string> common_vec;
            std::set_intersection(it->second.begin(), it->second.end(), output_mac_addr.begin(), output_mac_addr.end(), std::back_inserter(common_vec));
            std::set<std::string> common(common_vec.begin(), common_vec.end());

            // To be associated
            std::vector<std::string> to_associate;
            std::set_difference(output_mac_addr.begin(), output_mac_addr.end(), common.begin(), common.end(), std::back_inserter(to_associate));

            // To be disassociated
            std::vector<std::string> to_disassociate;
            std::set_difference(it->second.begin(), it->second.end(), common.begin(), common.end(), std::back_inserter(to_disassociate));

            // Associate new mac addr
            for (int i = 0; i < to_associate.size(); i++)
            {
                associate(to_associate[i], virtual_interface);
            }

            // Disassociate old mac addr
            for (int i = 0; i < to_disassociate.size(); i++)
            {
                disassociate(to_disassociate[i], virtual_interface);
            }

            // Update associated mac addr
            it->second.clear();
            it->second = output_mac_addr;
        }

        sleep(seconds);
    }
}

int DPMAgent::sync(int operation_code, void* bssid)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;

    char* interface_name = get_interface_name(std::string((char*)bssid));
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

    switch(op)
    {
        case BSSID_NODE_OPS::BSSID_ADD:
            // Add new virutal interface to map
            if (interface_mac.find(virtual_interface) == interface_mac.end())
            {
                interface_mac.insert(std::pair<std::string, std::set<std::string>>(virtual_interface, std::set<std::string>()));
            }

            // Execute add command
            return add(virtual_interface);
        case BSSID_NODE_OPS::BSSID_REMOVE:
            // Disassociate all devices connected to virutal interface
            for (std::set<std::string>::iterator it = interface_mac.at(virtual_interface).begin(); it != interface_mac.at(virtual_interface).end(); it++)
            {
                disassociate(*it, virtual_interface);
            }
            interface_mac.erase(virtual_interface);

            // Execute remove command
            return remove(virtual_interface);
    }
}

int DPMAgent::execute_command(std::string command)
{
    return system((std::string(BASE_COMMAND_STR) + " " + command).c_str());
}
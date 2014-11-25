#include "dpm_agent.h"
#include "comms_agent.h"
#include "util.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>

using namespace std;

/***********************************Helper****************************************************
*********************************************************************************************/
std::string create_virtual_interface(std::string interface) {
    return std::string("v" + std::string(interface) + "-dpm");
}

std::string create_virtual_ethernet(std::string wlan_interface, std::string ethernet_interface) {
    //Do not create virtual ethernet since not needed.
    //Creating virtual ethernet interface is architecturely more consistent, but not functionally necessary.
    // return std::string("v" + std::string(ethernet_interface) + "-" + std::string(wlan_interface));
    return std::string(ethernet_interface + "");
}


/***********************************Constructor***********************************************
*********************************************************************************************/
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
    std::string virtual_interface = create_virtual_interface(wlan_interface);
    std::string virtual_ethernet = create_virtual_ethernet(wlan_interface, ethernet_interface);
    
    std::cout << "Creating virtual interfaces... " << virtual_interface << " and " << virtual_ethernet << std::endl;
    std::string vwlan_command("vethd -v " + virtual_interface + " -e " + std::string(wlan_interface));
    system(vwlan_command.c_str());
    system(("ifconfig " + virtual_interface + " up").c_str());

    //Finished creating virtual interfaces. Now call the ovs on those created interfaces.
    std::cout<<"Command: " << "add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet <<std::endl;
    return execute_command("add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet);
}

int DPMAgent::remove(const std::string& wlan_interface, const std::string& ethernet_interface)
{
    std::string virtual_interface = create_virtual_interface(wlan_interface);
    std::string virtual_ethernet = create_virtual_ethernet(wlan_interface, ethernet_interface);

    std::string vwlan_command("vethd -v " + virtual_interface + " -e " + std::string(wlan_interface));

    int error = execute_command("remove " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet);
    if (error != 0) {
        return error;
    }

    //Get pid wlan
    std::vector<int> vwlan;
    get_pid(vwlan_command, &vwlan, 1);
    int vwlan_pid = *(vwlan.begin());
    printf("Found pid %d\n", vwlan_pid);

    cout << "--------------------" << ("kill -SIGTERM " + std::to_string(vwlan_pid)) << endl;
    return system(("kill -SIGTERM " + std::to_string(vwlan_pid)).c_str());
}

int DPMAgent::associate(const std::string& bssid, const std::string& wlan_interface, const std::string& ethernet_interface)
{
    std::string virtual_interface = create_virtual_interface(wlan_interface);
    std::string virtual_ethernet = create_virtual_ethernet(wlan_interface, ethernet_interface);
    return execute_command("associate " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet + " " + bssid);
}

int DPMAgent::disassociate(const std::string& bssid, const std::string& wlan_interface, const std::string& ethernet_interface)
{
    std::string virtual_interface = create_virtual_interface(wlan_interface);
    std::string virtual_ethernet = create_virtual_ethernet(wlan_interface, ethernet_interface);
    return execute_command("disassociate " + socket_path + " " + ovs_name + " " + virtual_interface + " " + virtual_ethernet + " " + bssid);
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
                // associate(to_associate[i], virtual_interface);
                // Build json
                // Format: {"command": _cmd, "changes": {"bssid": _bssid, "macaddr": _mac_addr}}
                // std::cout << "to associate: " << to_associate[i] << std::endl;

                //Why don't we use sprintf instead of this?
                std::string json_associate(LEFT_BRACKET + QUOTE + JSON_COMMAND + QUOTE + COLON + QUOTE + MAC_ASSOCIATE_CMD + QUOTE + COMMA
                                                + QUOTE + JSON_CHANGES + QUOTE + COLON + LEFT_BRACKET
                                                + QUOTE + JSON_BSSID + QUOTE + COLON + QUOTE + interface_bssid.at(virtual_interface) + QUOTE + COMMA
                                                + QUOTE + JSON_MAC_ADDRESS + QUOTE + COLON + QUOTE + to_associate[i] + QUOTE
                                                + RIGHT_BRACKET
                                                + RIGHT_BRACKET);

                std::cout << "DPM agent associating: " << json_associate << std::endl;

                this->comms_agent->sync(BSSID_NODE_OPS::COMMAND_ADD, (char*)json_associate.c_str());
            }

            // Disassociate old mac addr
            for (int i = 0; i < to_disassociate.size(); i++)
            {
                // disassociate(to_disassociate[i], virtual_interface)
                // Build json
                // Format: {"command": _cmd, "changes": {"bssid": _bssid, "macaddr": _mac_addr}}
                // std::cout << "to disassociate: " << to_disassociate[i] << std::endl;
                std::string json_associate(LEFT_BRACKET + QUOTE + JSON_COMMAND + QUOTE + COLON + QUOTE + MAC_DISASSOCIATE_CMD + QUOTE + COMMA
                                                + QUOTE + JSON_CHANGES + QUOTE + COLON + LEFT_BRACKET
                                                + QUOTE + JSON_BSSID + QUOTE + COLON + QUOTE + interface_bssid.at(virtual_interface) + QUOTE + COMMA
                                                + QUOTE + JSON_MAC_ADDRESS + QUOTE + COLON + QUOTE + to_disassociate[i] + QUOTE
                                                + RIGHT_BRACKET
                                                + RIGHT_BRACKET);

                std::cout << "DPM agent disassociating: " << json_associate << std::endl;

                this->comms_agent->sync(BSSID_NODE_OPS::COMMAND_ADD, (char*)json_associate.c_str());
            }

            // Update associated mac addr
            it->second.clear();
            it->second = output_mac_addr;
        }

        sleep(seconds);
    }
}

int DPMAgent::sync(int operation_code, void* message)
{
    cout << "DPMAgent sync with op: " << operation_code << endl;
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;

    std::string msg((char*)message);
    std::string virtual_interface;

    // Parse the message depending on different op codes
    if (op == BSSID_NODE_OPS::BSSID_ADD || op == BSSID_NODE_OPS::BSSID_REMOVE)
    {
        char* interface_name = get_interface_name(msg);
        virtual_interface = std::string(interface_name);
        free(interface_name);
    }
    else if (op == BSSID_NODE_OPS::BSSID_MAC_ASSOCIATE || op == BSSID_NODE_OPS::BSSID_MAC_DISASSOCIATE)
    {
        char* interface_name = get_interface_name(msg.substr(0, msg.find("|")));
        virtual_interface = std::string(interface_name);
        free(interface_name);
    }

    switch(op)
    {
        case BSSID_NODE_OPS::BSSID_ADD:
            // Add interface and its bssid
            if (interface_bssid.find(virtual_interface) == interface_bssid.end())
            {
                interface_bssid.insert(std::pair<std::string, std::string>(virtual_interface, std::string((char*)message)));
                bssid_interface.insert(std::pair<std::string, std::string>(std::string((char*)message), virtual_interface));
            }

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
            bssid_interface.erase(interface_bssid.at(virtual_interface));
            interface_bssid.erase(virtual_interface);

            // Execute remove command
            return remove(virtual_interface);
        case BSSID_NODE_OPS::BSSID_MAC_ASSOCIATE:
            cout << "Associating " << msg.substr(msg.find("|") + 1) << " to " << virtual_interface << endl;
            return associate(msg.substr(msg.find("|") + 1), virtual_interface);
        case BSSID_NODE_OPS::BSSID_MAC_DISASSOCIATE:
            return disassociate(msg.substr(msg.find("|") + 1), virtual_interface);
    }
}

int DPMAgent::execute_command(std::string command)
{
    std::cout << " $" << std::string(BASE_COMMAND_STR) << " " << command << endl;
    return system((std::string(BASE_COMMAND_STR) + " " + command).c_str());
}
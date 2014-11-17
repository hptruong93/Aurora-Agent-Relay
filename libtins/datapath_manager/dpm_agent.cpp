#include "dpm_agent.h"
#include "util.h"

#include <iostream>
#include <unistd.h>
#include <algorithm>

char* get_interface_name(const std::string& addr)
{
    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    std::string command = std::string("ifconfig | grep -E ") + std::string("'") +  std::string("HWaddr ") + addr + std::string("'");
    fp = popen(command.c_str(), "r");

    while ((c = fgetc(fp)) != EOF)
    {
        if ((char) c == ' ')
        {
            break;
        }
        interface_name[interface_name_len++] = (char)c;
    }

    interface_name[interface_name_len] = '\0';

    return interface_name;
}

DPMAgent::DPMAgent()
{

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
    std::cout << "Found socket path " << socket_path << std::endl;
}

int DPMAgent::add(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = get_interface_name(bssid);
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

    std::cout<<"Command: "<<"add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface<<std::endl;
    return execute_command("add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface);
}

int DPMAgent::remove(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = get_interface_name(bssid);
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

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
        // Call hostapd
        FILE *fp;
        char *output_str = (char*)malloc(1024 * sizeof(char));
        int index = 0;
        char c = 's';

        fp = popen((std::string(BASE_COMMAND_STR) + " " + "init" + " " + ovs_name).c_str(), "r");
        while ((c = fgetc(fp)) != EOF)
        {

            output_str[index++] = c;
        }
        output_str[index] = '\0';

        std::string output(output_str);
        free(output_str);

        std::vector<std::string> output_mac_addr;
        split_string(output, "\n", &output_mac_addr);

        std::vector<std::string> new_associated;

        // Associate new mac addresses
        for (int i = 0; i < output_mac_addr.size(); i++)
        {
            // Hacky way of detecting a mac address
            if (output_mac_addr[i].length() == 17)
            {
                if (std::find(associated_mac_addr.begin(), associated_mac_addr.end(), output_mac_addr[i]) == associated_mac_addr.end())
                {
                    // Temporary
                    associate(output_mac_addr[i], "wlan0", "eth1");
                }

                new_associated.push_back(associated_mac_addr[i]);
            }
        }

        // Disassociate old mac addresses
        for (int i = 0; i < associated_mac_addr.size(); i++)
        {
            if (std::find(new_associated.begin(), new_associated.end(), associated_mac_addr[i]) == new_associated.end())
            {
                // Temporary
                disassociate(output_mac_addr[i], "wlan0", "eth1");
            }
        }

        // Update associated mac addr
        associated_mac_addr = new_associated;

        sleep(seconds);
    }
}

int DPMAgent::sync(int operation_code, void* bssid)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::BSSID_ADD:
            return add(std::string((char*)bssid));
        case BSSID_NODE_OPS::BSSID_REMOVE:
            return remove(std::string((char*)bssid));
    }
}

int DPMAgent::execute_command(std::string command)
{
    return system((std::string(BASE_COMMAND_STR) + " " + command).c_str());
}